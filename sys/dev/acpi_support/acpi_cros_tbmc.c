/*-
 * Copyright (c) 2019 Greg V <greg@unrelenting.technology>
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

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "opt_acpi.h"
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/bus.h>
#include <machine/cpufunc.h>

#include <contrib/dev/acpica/include/acpi.h>
#include <contrib/dev/acpica/include/accommon.h>

#include "acpi_if.h"
#include <sys/module.h>
#include <dev/evdev/evdev.h>
#include <dev/evdev/input.h>
#include <dev/acpica/acpivar.h>

ACPI_MODULE_NAME("CROS_TBMC")

struct acpi_cros_tbmc_softc {
	device_t	dev;
	ACPI_HANDLE	handle;

	struct evdev_dev	*evdev;
};

static void
acpi_cros_tbmc_read_status(struct acpi_cros_tbmc_softc *sc)
{
	int32_t val = 0;

	ACPI_FUNCTION_TRACE((char *)(uintptr_t)__func__);

	if (ACPI_FAILURE(acpi_GetInteger(sc->handle, "TBMC", &val))) {
		device_printf(sc->dev, "ACPI failure\n");
		return;
	}

	evdev_push_sw(sc->evdev, SW_TABLET_MODE, val);
	evdev_sync(sc->evdev);
}

static void
acpi_cros_tbmc_notify(ACPI_HANDLE h, UINT32 notify, void *context)
{
	device_t	dev = context;
	struct acpi_cros_tbmc_softc *sc = device_get_softc(dev);

	ACPI_FUNCTION_TRACE_U32((char *)(uintptr_t)__func__, notify);

	if (notify != 0x80)
		device_printf(dev, "Unknown notify\n");

	acpi_cros_tbmc_read_status(sc);
}

static int
acpi_cros_tbmc_resume(device_t dev)
{
	struct acpi_cros_tbmc_softc *sc = device_get_softc(dev);

	ACPI_FUNCTION_TRACE((char *)(uintptr_t)__func__);

	acpi_cros_tbmc_read_status(sc);

	return (0);
}

static char *cros_tbmc_ids[] = {"GOOG0006", NULL};

static int
acpi_cros_tbmc_probe(device_t dev)
{
	int rv;

	if (acpi_disabled("cros_tbmc") ||
	    device_get_unit(dev) != 0)
		return (ENXIO);

	rv = ACPI_ID_PROBE(device_get_parent(dev), dev, cros_tbmc_ids, NULL);

	if (rv <= 0)
		device_set_desc(dev, "Google Tablet Motion Control");

	return (rv);
}

static int
acpi_cros_tbmc_attach(device_t dev)
{
	struct acpi_cros_tbmc_softc	*sc;

	ACPI_FUNCTION_TRACE((char *)(uintptr_t)__func__);

	sc = device_get_softc(dev);
	sc->dev = dev;
	sc->handle = acpi_get_handle(dev);

	sc->evdev = evdev_alloc();
	evdev_set_name(sc->evdev, device_get_desc(dev));
	evdev_set_phys(sc->evdev, device_get_nameunit(dev));
	evdev_set_id(sc->evdev, BUS_HOST, 0, 0, 1);
	evdev_support_event(sc->evdev, EV_SYN);
	evdev_support_event(sc->evdev, EV_SW);
	evdev_support_sw(sc->evdev, SW_TABLET_MODE);

	if (evdev_register(sc->evdev))
		return (ENXIO);

	AcpiInstallNotifyHandler(sc->handle, ACPI_ALL_NOTIFY,
	    acpi_cros_tbmc_notify, dev);

	acpi_cros_tbmc_read_status(sc);

	return (0);
}

static int
acpi_cros_tbmc_detach(device_t dev)
{
	ACPI_FUNCTION_TRACE((char *)(uintptr_t)__func__);

	struct acpi_cros_tbmc_softc *sc = device_get_softc(dev);

	evdev_free(sc->evdev);
	AcpiRemoveNotifyHandler(sc->handle, ACPI_ALL_NOTIFY, acpi_cros_tbmc_notify);

	return (0);
}

static device_method_t acpi_cros_tbmc_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe, acpi_cros_tbmc_probe),
	DEVMETHOD(device_attach, acpi_cros_tbmc_attach),
	DEVMETHOD(device_detach, acpi_cros_tbmc_detach),
	DEVMETHOD(device_resume, acpi_cros_tbmc_resume),

	DEVMETHOD_END
};

static driver_t	acpi_cros_tbmc_driver = {
	"acpi_cros_tbmc",
	acpi_cros_tbmc_methods,
	sizeof(struct acpi_cros_tbmc_softc),
};

static devclass_t acpi_cros_tbmc_devclass;

DRIVER_MODULE(acpi_cros_tbmc, acpi, acpi_cros_tbmc_driver, acpi_cros_tbmc_devclass,
	      0, 0);
MODULE_DEPEND(acpi_cros_tbmc, acpi, 1, 1, 1);
MODULE_DEPEND(acpi_cros_tbmc, evdev, 1, 1, 1);
