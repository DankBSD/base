/*-
 * Copyright (c) 2020 Greg V <greg@unrelenting.technology>
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

#include <dev/backlight/backlight.h>
#include <contrib/dev/acpica/include/acpi.h>
#include <contrib/dev/acpica/include/accommon.h>

#include "acpi_if.h"
#include "backlight_if.h"
#include <sys/module.h>
#include <dev/acpica/acpivar.h>

ACPI_MODULE_NAME("CROS_KBLT")

struct acpi_cros_kblt_softc {
	device_t	dev;
	ACPI_HANDLE	handle;
	int		val;

	struct cdev	*cdev;
};

static int
acpi_cros_kblt_resume(device_t dev)
{
	struct acpi_cros_kblt_softc *sc = device_get_softc(dev);

	ACPI_FUNCTION_TRACE((char *)(uintptr_t)__func__);

	if (ACPI_FAILURE(acpi_SetInteger(sc->handle, "KBCM", sc->val))) {
		device_printf(sc->dev, "ACPI failure\n");
		return (ENXIO);
	}

	return (0);
}

static char *cros_kblt_ids[] = {"GOOG0002", NULL};

static int
acpi_cros_kblt_probe(device_t dev)
{
	int err;

	if (acpi_disabled("cros_kblt") ||
	    device_get_unit(dev) != 0)
		return (ENXIO);

	err = ACPI_ID_PROBE(device_get_parent(dev), dev, cros_kblt_ids, NULL);

	if (err <= 0)
		device_set_desc(dev, "Chromium EC Keyboard Backlight Control");

	return (err);
}

static int
acpi_cros_kblt_attach(device_t dev)
{
	struct acpi_cros_kblt_softc	*sc;

	sc = device_get_softc(dev);
	sc->dev = dev;
	sc->handle = acpi_get_handle(dev);

	if (ACPI_FAILURE(acpi_GetInteger(sc->handle, "KBQC", &sc->val))) {
		device_printf(sc->dev, "'KBQC' ACPI method failed\n");
		return (ENXIO);
	}

	sc->cdev = backlight_register("acpi_cros_kblt", dev);
	if (sc->cdev == NULL)
		device_printf(dev, "Cannot register as a backlight\n");

	return (0);
}

static int
acpi_cros_kblt_update_status(device_t dev, struct backlight_props *props)
{
	struct acpi_cros_kblt_softc	*sc;

	sc = device_get_softc(dev);
	sc->val = props->brightness;

	if (ACPI_FAILURE(acpi_SetInteger(sc->handle, "KBCM", sc->val))) {
		device_printf(sc->dev, "'KBCM' ACPI method failed\n");
		return (ENXIO);
	}

	return (0);
}

static int
acpi_cros_kblt_get_status(device_t dev, struct backlight_props *props)
{
	struct acpi_cros_kblt_softc	*sc;

	sc = device_get_softc(dev);

	if (ACPI_FAILURE(acpi_GetInteger(sc->handle, "KBQC", &sc->val))) {
		device_printf(sc->dev, "'KBQC' ACPI method failed\n");
		return (ENXIO);
	}

	props->brightness = sc->val;
	props->nlevels = 0;

	return (0);
}

static int
acpi_cros_kblt_get_info(device_t dev, struct backlight_info *info)
{

	info->type = BACKLIGHT_TYPE_KEYBOARD;
	strlcpy(info->name, "Chromium EC Keyboard", BACKLIGHTMAXNAMELENGTH);

	return (0);
}

static device_method_t acpi_cros_kblt_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe, acpi_cros_kblt_probe),
	DEVMETHOD(device_attach, acpi_cros_kblt_attach),
	DEVMETHOD(device_resume, acpi_cros_kblt_resume),

	/* Backlight interface */
	DEVMETHOD(backlight_update_status, acpi_cros_kblt_update_status),
	DEVMETHOD(backlight_get_status, acpi_cros_kblt_get_status),
	DEVMETHOD(backlight_get_info, acpi_cros_kblt_get_info),

	DEVMETHOD_END
};

static driver_t	acpi_cros_kblt_driver = {
	"acpi_cros_kblt",
	acpi_cros_kblt_methods,
	sizeof(struct acpi_cros_kblt_softc),
};

static devclass_t acpi_cros_kblt_devclass;

DRIVER_MODULE(acpi_cros_kblt, acpi, acpi_cros_kblt_driver,
		acpi_cros_kblt_devclass, 0, 0);
MODULE_DEPEND(acpi_cros_kblt, acpi, 1, 1, 1);
