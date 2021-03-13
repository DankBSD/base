/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2021 Greg V <greg@unrelenting.technology>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "opt_acpi.h"

#include <sys/cdefs.h>
#include <sys/param.h>
#include <sys/bus.h>
#include <sys/kernel.h>
#include <sys/module.h>
#include <sys/proc.h>
#include <sys/rman.h>

#include <machine/bus.h>
#include <machine/resource.h>

#include <contrib/dev/acpica/include/acpi.h>
#include <contrib/dev/acpica/include/accommon.h>

#include <dev/acpica/acpivar.h>
#include <dev/acpica/acpiio.h>

#include <dev/spibus/spibusvar.h>

#include <dev/atopcase/atopcase.h>

/*
 * This driver does not inherit from atopcase, but instead manages an instance
 * of atopcase under the correct spibus from a distance. This is the best we
 * can do with how these devices are arranged.
 */

static char *atopcase_ids[] = { "APP000D", NULL };

struct atopcase_acpi_softc {
	ACPI_HANDLE sc_handle;
	device_t sc_dev;
	device_t sc_spi;
	device_t sc_atopcase;

	bool sc_printed_notify;
	bool sc_printed_intr;

	struct acpi_prw_data sc_prw;
	int			sc_irq_rid;
	struct resource		*sc_irq_res;
	void			*sc_irq_ih;
};

static int
atopcase_acpi_probe(device_t dev)
{
	int rv;

	if (acpi_disabled("atopcase"))
		return (ENXIO);

	rv = ACPI_ID_PROBE(device_get_parent(dev), dev, atopcase_ids, NULL);
	if (rv > 0)
		return (ENXIO);

	device_set_desc(dev, "Apple MacBook SPI Topcase Attachment");
	return (BUS_PROBE_DEFAULT);
}

static int
atopcase_acpi_enable_spi(struct atopcase_acpi_softc *sc)
{
	ACPI_OBJECT argobj;
	ACPI_OBJECT_LIST args;

	argobj.Type = ACPI_TYPE_INTEGER;
	argobj.Integer.Value = 1;
	args.Count = 1;
	args.Pointer = &argobj;

	if (ACPI_FAILURE(
		AcpiEvaluateObject(sc->sc_handle, "SIEN", &args, NULL)))
		return (ENXIO);

	DELAY(100);
	return (0);
}

static int
atopcase_acpi_disable_spi(struct atopcase_acpi_softc *sc)
{
	ACPI_OBJECT argobj;
	ACPI_OBJECT_LIST args;

	argobj.Type = ACPI_TYPE_INTEGER;
	argobj.Integer.Value = 0;
	args.Count = 1;
	args.Pointer = &argobj;

	if (ACPI_FAILURE(
		AcpiEvaluateObject(sc->sc_handle, "SIEN", &args, NULL)))
		return (ENXIO);

	DELAY(100);
	return (0);
}

static void
atopcase_acpi_notify(ACPI_HANDLE h, UINT32 notify, void *ctx)
{
	struct atopcase_acpi_softc *sc = ctx;
	if (!sc->sc_printed_notify) {
		device_printf(sc->sc_dev, "Using ACPI notifications\n");
		sc->sc_printed_notify = true;
	}
	atopcase_intr(sc->sc_atopcase);
}

static void
atopcase_acpi_intr(void *ctx)
{
	struct atopcase_acpi_softc *sc = ctx;
	if (!sc->sc_printed_intr) {
		device_printf(sc->sc_dev, "Using interrupts\n");
		sc->sc_printed_intr = true;
	}
	atopcase_intr(sc->sc_atopcase);
}

static int
atopcase_acpi_attach(device_t dev)
{
	struct atopcase_acpi_softc *sc = device_get_softc(dev);
	ACPI_HANDLE parent;

	sc->sc_dev = dev;

	/* We are \_SB.PCI0.SPI1.SPIT, our parent is intelspi, which has a
	 * spibus */
	sc->sc_handle = acpi_get_handle(dev);
	device_printf(dev, "%s %p\n", acpi_name(sc->sc_handle), sc->sc_handle);
	if (ACPI_FAILURE(AcpiGetParent(sc->sc_handle, &parent))) {
		device_printf(dev, "could not find ACPI parent\n");
		return (ENXIO);
	}
	device_printf(dev, "%s (%p) >>> %s (%p)\n", acpi_name(sc->sc_handle), sc->sc_handle,
	    acpi_name(parent), parent);
	sc->sc_spi = acpi_get_device(parent);
	if (!sc->sc_spi) {
		device_printf(dev, "could not find device of ACPI parent\n");
		return (ENXIO);
	}
	sc->sc_spi = device_find_child(sc->sc_spi, "spibus", -1);
	if (!sc->sc_spi) {
		device_printf(dev, "could not find spibus on parent device\n");
		return (ENXIO);
	}

	if (atopcase_acpi_enable_spi(sc) != 0) {
		device_printf(dev, "could not enable SPI communication\n");
		return (ENXIO);
	}

	sc->sc_atopcase = BUS_ADD_CHILD(sc->sc_spi, 0, "atopcase", -1);
	if (!sc->sc_atopcase) {
		device_printf(dev, "could not attach atopcase\n");
		return (ENXIO);
	}
	spibus_set_clock(sc->sc_atopcase, 1000000); // TODO: proper is 8000000 - requires DELAY(10) between chip select and data

	// XXX: ACPI_DEVICE_NOTIFY?
	if (ACPI_SUCCESS(AcpiInstallNotifyHandler(sc->sc_handle, ACPI_ALL_NOTIFY,
	    atopcase_acpi_notify, sc))) {
	  if (acpi_parse_prw(sc->sc_handle, &sc->sc_prw) == 0) {
			AcpiEnableGpe(sc->sc_prw.gpe_handle, sc->sc_prw.gpe_bit);
	  } else {
			device_printf(dev, "could not parse PRW\n");
	  }
  } else {
		device_printf(dev, "could not install ACPI notification handler\n");
  }

	sc->sc_irq_res = bus_alloc_resource_any(sc->sc_dev,
	    SYS_RES_IRQ, &sc->sc_irq_rid, RF_ACTIVE);
	if (sc->sc_irq_res) {
		if (bus_setup_intr(dev, sc->sc_irq_res, INTR_TYPE_MISC | INTR_MPSAFE,
		    NULL, atopcase_acpi_intr, sc, &sc->sc_irq_ih)) {
			device_printf(dev, "could not setup interrupt handler\n");
		}
	} else {
		device_printf(dev, "could not allocate IRQ resource\n");
	}

	return (bus_generic_attach(dev));
}

static int
atopcase_acpi_detach(device_t dev)
{
	struct atopcase_acpi_softc *sc = device_get_softc(dev);

	if (sc->sc_irq_ih)
		bus_teardown_intr(dev, sc->sc_irq_res, sc->sc_irq_ih);
	if (sc->sc_irq_res)
		bus_release_resource(dev, SYS_RES_IRQ,
		    sc->sc_irq_rid, sc->sc_irq_res);

	if (ACPI_FAILURE(
		AcpiRemoveNotifyHandler(sc->sc_handle, ACPI_ALL_NOTIFY, atopcase_acpi_notify)))
		device_printf(dev, "could not remove ACPI notification handler\n");

	device_delete_child(sc->sc_spi, sc->sc_atopcase);

	return (bus_generic_detach(dev));
}

static int
atopcase_acpi_suspend(device_t dev)
{
	struct atopcase_acpi_softc *sc = device_get_softc(dev);

	if (sc->sc_prw.gpe_handle || sc->sc_prw.gpe_bit)
		AcpiDisableGpe(sc->sc_prw.gpe_handle, sc->sc_prw.gpe_bit);

	if (atopcase_acpi_disable_spi(sc) != 0) {
		device_printf(dev, "could not disable SPI communication\n");
	}

	return (bus_generic_suspend(dev));
}

static int
atopcase_acpi_resume(device_t dev)
{
	struct atopcase_acpi_softc *sc = device_get_softc(dev);
	int err;

	if (sc->sc_prw.gpe_handle || sc->sc_prw.gpe_bit)
		AcpiEnableGpe(sc->sc_prw.gpe_handle, sc->sc_prw.gpe_bit);

	if (atopcase_acpi_enable_spi(sc) != 0) {
		device_printf(dev, "could not enable SPI communication\n");
		return (ENXIO);
	}

	spibus_set_clock(sc->sc_atopcase, 1000000); // TODO: proper is 8000000 - requires DELAY(10) between chip select and data

	err = atopcase_resume(sc->sc_atopcase);
	if (err)
		return (err);

	return (bus_generic_resume(dev));
}

static device_method_t atopcase_acpi_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe, atopcase_acpi_probe),
	DEVMETHOD(device_attach, atopcase_acpi_attach),
	DEVMETHOD(device_detach, atopcase_acpi_detach),
	DEVMETHOD(device_suspend, atopcase_acpi_suspend),
	DEVMETHOD(device_resume, atopcase_acpi_resume),

	DEVMETHOD_END
};

static driver_t atopcase_acpi_driver = {
	"atopcase_acpi",
	atopcase_acpi_methods,
	sizeof(struct atopcase_acpi_softc),
};

static devclass_t atopcase_acpi_devclass;
DRIVER_MODULE(atopcase_acpi, acpi, atopcase_acpi_driver, atopcase_acpi_devclass,
    0, 0);
MODULE_DEPEND(atopcase_acpi, acpi, 1, 1, 1);
MODULE_DEPEND(atopcase_acpi, spibus, 1, 1, 1);
