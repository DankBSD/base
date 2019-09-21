#define HAVE_ACPI_IICBUS

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/bus.h>
#include <sys/condvar.h>
#include <sys/callout.h>
#include <sys/endian.h>
#include <sys/kernel.h>
#include <sys/lock.h>
#include <sys/malloc.h>
#include <sys/module.h>
#include <sys/mutex.h>
#include <sys/rman.h>
#include <sys/sysctl.h>
#include <sys/systm.h>
#include <sys/sx.h>
#include <sys/taskqueue.h>

#include <machine/resource.h>

#include <contrib/dev/acpica/include/acpi.h>
#include <contrib/dev/acpica/include/accommon.h>
#include <dev/acpica/acpivar.h>

#include <dev/iicbus/iicbus.h>
#include <dev/iicbus/iic.h>
#include <dev/iicbus/iiconf.h>

#include "tpm20.h"

struct tpm_gsc_softc {
	struct tpm_sc base;

	bool			probe_done;
	int			probe_result;
};

static void
tpm_gsc_intr(void *context)
{
	struct tpm_gsc_softc *sc = context;

	wakeup(sc);
	device_printf(sc->base.dev, "intr\n");
}

static int
tpm_gsc_i2c_try_transfer(device_t dev, uint16_t flags, uint8_t *buf, uint16_t len)
{
	uint16_t addr = iicbus_get_addr(dev);
	struct iic_msg msg = { addr << 1, flags, len, buf };
	int error = 0;

	for (int i = 0; i < 5; i++) {
		error = iicbus_transfer(dev, &msg, 1);

		if (error == 0)
			break;

		tsleep(device_get_softc(dev), PWAIT, "TPM", 10 * hz / 1000);
	}

	return (error);
}

static int
tpm_gsc_read_reg(device_t dev, uint8_t reg, uint8_t *buf, size_t len)
{
	struct tpm_gsc_softc *sc = device_get_softc(dev);
	int error = 0;

	error = tpm_gsc_i2c_try_transfer(dev, IIC_M_WR, &reg, 1);
	if (error != 0)
		return (error);

	tsleep(sc, PWAIT, "TPM", 20 * hz / 1000);

	error = tpm_gsc_i2c_try_transfer(dev, IIC_M_RD, buf, len);

	return (error);
}

static int
tpm_gsc_write_reg(device_t dev, uint8_t reg, uint8_t *buf, size_t len)
{
	struct tpm_gsc_softc *sc = device_get_softc(dev);
	uint8_t msg[420];
	int error = 0;

	msg[0] = reg;
	memcpy(msg + 1, buf, len);

	error = tpm_gsc_i2c_try_transfer(dev, IIC_M_WR, (uint8_t*)&msg, 1 + len);

	tsleep(sc, PWAIT, "TPM", 20 * hz / 1000);

	return (error);
}

#define	TPM_ACCESS_REQUEST_USE		0x02	/* request using locality */
#define	TPM_ACCESS_ACTIVE_LOCALITY	0x20	/* locality is active */
#define	TPM_ACCESS_VALID		0x80	/* bits are valid */

static bool
tpm_gsc_request_locality(struct tpm_gsc_softc *sc, int locality)
{
	uint8_t buf;
	int error = tpm_gsc_read_reg(sc->base.dev, locality << 4, &buf, 1);

	if (error != 0)
		return (false);

	if ((buf & (TPM_ACCESS_VALID | TPM_ACCESS_ACTIVE_LOCALITY)) ==
				(TPM_ACCESS_VALID | TPM_ACCESS_ACTIVE_LOCALITY))
		return (true);

	buf = TPM_ACCESS_REQUEST_USE;

	error = tpm_gsc_write_reg(sc->base.dev, locality << 4, &buf, 1);

	if (error != 0)
		return (false);

	for (int tries = 0; tries < 5; tries++) {
		tsleep(sc, PWAIT, "TPM", 20 * hz / 1000);
		error = tpm_gsc_read_reg(sc->base.dev, locality << 4, &buf, 1);

		if (error == 0 && (buf & (TPM_ACCESS_VALID | TPM_ACCESS_ACTIVE_LOCALITY)) ==
					(TPM_ACCESS_VALID | TPM_ACCESS_ACTIVE_LOCALITY))
			return (true);
	}

	return (false);
}

static bool
tpm_gsc_relinquish_locality(struct tpm_gsc_softc *sc, int locality)
{
	uint8_t buf = 0x20;

	int error = tpm_gsc_write_reg(sc->base.dev, locality << 4, &buf, 1);

	if (error != 0)
		return (false);

	return (true);
}

#define	TPM_STS_CMD_READY	0x00000040	/* rw chip/signal ready */
#define	TPM_STS_CMD_START	0x00000020	/* rw start */

static bool
tpm_gsc_go_ready(struct tpm_gsc_softc *sc)
{
	uint8_t buf[4] = {TPM_STS_CMD_READY};
	int error = tpm_gsc_write_reg(sc->base.dev, 0x1 /* | locality << 4*/, &buf[0], sizeof(buf));

	if (error != 0)
		return (false);

	return (true);
}

#define	TPM_STS_VALID		0x00000080	/* ro other bits are valid */
#define	TPM_STS_DATA_AVAIL	0x00000010	/* ro data available */

static uint16_t
tpm_gsc_wait_for_bursts(struct tpm_gsc_softc *sc, uint8_t mask)
{
	/* Must read 4 bytes */
	uint8_t buf[4];
	int error = 0;

	for (int tries = 0; tries < 10; tries++) {
		tsleep(sc, PWAIT, "TPM", 10 * hz / 1000);
		error = tpm_gsc_read_reg(sc->base.dev, 0x1 /* | locality << 4*/, &buf[0], sizeof(buf));

		//device_printf(sc->base.dev, "bursts [%x %x %x %x]\n", buf[0], buf[1], buf[2], buf[3]);
		if (error == 0 && (buf[0] & mask) == mask) {
			uint16_t bursts = *(uint16_t*)&buf[1];
			device_printf(sc->base.dev, "bursts %hu\n", bursts);
			if (bursts > 0 && bursts <= 63)
				return (bursts);
		}
	}

	return (0);
}

static int
tpm_gsc_transmit(struct tpm_sc *bsc, size_t length)
{
	struct tpm_gsc_softc *sc = (struct tpm_gsc_softc *)bsc;
	uint32_t cmd = TPM_STS_CMD_START;
	uint16_t burst_count = 0;
	int error = 0;
	size_t cur_len = 0;
	size_t total_len = 0;

	if (bootverbose)
		device_printf(sc->base.dev, "transmit: called with length %zu\n", length);

	if (!tpm_gsc_request_locality(sc, 0)) {
		device_printf(sc->base.dev, "error: transmit: obtain locality\n");
		return (EIO);
	}

	if (!tpm_gsc_go_ready(sc)) {
		device_printf(sc->base.dev, "error: transmit: ready\n");
		tpm_gsc_relinquish_locality(sc, 0);
		return (EIO);
	}

	burst_count = tpm_gsc_wait_for_bursts(sc, TPM_STS_VALID);
	if (burst_count < 1) {
		device_printf(sc->base.dev, "error: transmit: no bursts available\n");
		tpm_gsc_relinquish_locality(sc, 0);
		return (EIO);
	}
	if (burst_count < length) {
		device_printf(sc->base.dev, "TODO more than one burst\n");
		tpm_gsc_relinquish_locality(sc, 0);
		return (EIO);
	}

	error = tpm_gsc_write_reg(sc->base.dev, 0x5 /* | locality << 4*/, sc->base.buf, length);
	if (error != 0) {
		device_printf(sc->base.dev, "error: transmit: writing request register (%d)\n", error);
		tpm_gsc_relinquish_locality(sc, 0);
		return (EIO);
	}

	error = tpm_gsc_write_reg(sc->base.dev, 0x1 /* | locality << 4*/, (uint8_t*)&cmd, sizeof(cmd));
	if (error != 0) {
		device_printf(sc->base.dev, "error: transmit: writing command register (%d)\n", error);
		tpm_gsc_relinquish_locality(sc, 0);
		return (EIO);
	}

	burst_count = tpm_gsc_wait_for_bursts(sc, TPM_STS_VALID | TPM_STS_DATA_AVAIL);
	device_printf(sc->base.dev, "avail bursts %hu\n", burst_count);

	error = tpm_gsc_read_reg(sc->base.dev, 0x5 /* | locality << 4*/, sc->base.buf, burst_count);
	if (error != 0) {
		device_printf(sc->base.dev, "error: transmit: reading response register (%d)\n", error);
		tpm_gsc_relinquish_locality(sc, 0);
		return (EIO);
	}

	total_len = be32toh(*(uint32_t *)&sc->base.buf[2]);
	device_printf(sc->base.dev, "transmit: response size: %zu\n", total_len);
	cur_len = burst_count;

	while (cur_len < total_len) {
		burst_count = tpm_gsc_wait_for_bursts(sc, TPM_STS_VALID | TPM_STS_DATA_AVAIL);
		device_printf(sc->base.dev, "cont avail bursts %hu\n", burst_count);
		burst_count = min(burst_count, total_len - cur_len);
		device_printf(sc->base.dev, "cont fetching %hu\n", burst_count);

		error = tpm_gsc_read_reg(sc->base.dev, 0x5 /* | locality << 4*/, &sc->base.buf[cur_len], burst_count);
		if (error != 0) {
			device_printf(sc->base.dev, "error: transmit: reading response register (%d)\n", error);
			tpm_gsc_relinquish_locality(sc, 0);
			return (EIO);
		}

		cur_len += burst_count;
	}

	device_printf(sc->base.dev, "transmit: response length: %zu\n", cur_len);
	sc->base.pending_data_length = total_len;

	tpm_gsc_relinquish_locality(sc, 0);
	return (0);
}

static int
tpm_gsc_probe(device_t dev)
{
	struct tpm_gsc_softc *sc = device_get_softc(dev);
	ACPI_HANDLE handle;
	uint16_t addr = iicbus_get_addr(dev);
	UINT32	sta;

	if (sc->probe_done)
		return (sc->probe_result);

	sc->probe_done = true;
	sc->probe_result = ENXIO;

	if (acpi_disabled("tpm_gsc"))
		return (ENXIO);

	if (addr == 0)
		return (ENXIO);

	sc->base.dev = dev;

	if ((handle = acpi_get_handle(dev)) == NULL)
		return (ENXIO);

	if (!acpi_MatchHid(handle, "GOOG0005"))
		return (ENXIO);

	if (ACPI_FAILURE(acpi_GetInteger(handle, "_STA", &sta)) ||
	    !ACPI_DEVICE_PRESENT(sta))
		return (ENXIO);

	device_set_desc(dev, "Google Security Chip (Cr50) Trusted Platform Module 2.0");
	sc->probe_result = BUS_PROBE_DEFAULT;
	return (sc->probe_result);
}

static int tpm_gsc_detach(device_t dev);

static int
tpm_gsc_attach(device_t dev)
{
	struct tpm_gsc_softc* sc = device_get_softc(dev);
	uint32_t vendor = 0;
	int error = 0;

#if 0
	sc->base.intr_cookie = 0;

	sc->base.irq_res = bus_alloc_resource_any(
			sc->base.dev, SYS_RES_IRQ, &sc->base.irq_rid, RF_ACTIVE);

	if (!sc->base.irq_res) {
		device_printf(dev, "error: IRQ allocation failed\n");
		goto after_irq;
	}

	if (bus_setup_intr(dev, sc->base.irq_res, INTR_TYPE_MISC | INTR_MPSAFE,
				NULL, tpm_gsc_intr, sc, &sc->base.intr_cookie) != 0) {
		device_printf(dev, "error: interrupt setup failed\n");
	}
after_irq:
#endif

	if ((error = tpm_gsc_read_reg(dev, 0x6, (uint8_t*)&vendor, sizeof(vendor))) != 0) {
		device_printf(dev, "error: could not read vendor (%d)\n", error);
		tpm_gsc_detach(dev);
		return (ENXIO);
	}

	if (vendor != 0x281ae0) {
		device_printf(dev, "error: vendor mismatch\n");
		tpm_gsc_detach(dev);
		return (ENXIO);
	}

	sc->base.transmit = tpm_gsc_transmit;

	error = tpm20_init(&sc->base);
	if (error != 0)
		tpm_gsc_detach(dev);

	return (error);
}

static int
tpm_gsc_detach(device_t dev)
{
	struct tpm_gsc_softc* sc = device_get_softc(dev);

	if (sc->base.buf)
		tpm20_release(&sc->base);

#if 0
	if (sc->base.intr_cookie)
		bus_teardown_intr(sc->base.dev, sc->base.irq_res, sc->base.intr_cookie);

	if (sc->base.irq_res)
		bus_release_resource(dev, SYS_RES_IRQ, sc->base.irq_rid,
				sc->base.irq_res);
#endif

	if (device_is_attached(dev))
		bus_generic_detach(dev);

	return (0);
}

static int
tpm_gsc_resume(device_t dev)
{
	struct tpm_gsc_softc* sc = device_get_softc(dev);

	sx_xlock(&sc->base.dev_lock);

	/* Discard buffer to prevent weird lockups */
	memset(sc->base.buf, 0, TPM_BUFSIZE);
	sc->base.pending_data_length = 0;
	cv_signal(&sc->base.buf_cv);

	sx_xunlock(&sc->base.dev_lock);

	return (0);
}

static devclass_t tpm_gsc_devclass;

static device_method_t tpm_gsc_methods[] = {
	DEVMETHOD(device_probe,         tpm_gsc_probe),
	DEVMETHOD(device_attach,        tpm_gsc_attach),
	DEVMETHOD(device_detach,        tpm_gsc_detach),
	DEVMETHOD(device_shutdown,      tpm20_shutdown),
	DEVMETHOD(device_suspend,       tpm20_suspend),
	DEVMETHOD(device_resume,        tpm_gsc_resume),

	DEVMETHOD_END
};

static driver_t tpm_gsc_driver = {
	.name = "tpm_gsc",
	.methods = tpm_gsc_methods,
	.size = sizeof(struct tpm_gsc_softc),
};

DRIVER_MODULE(tpm_gsc, iicbus, tpm_gsc_driver, tpm_gsc_devclass, NULL, 0);
MODULE_DEPEND(tpm_gsc, iicbus, IICBUS_MINVER, IICBUS_PREFVER, IICBUS_MAXVER);
MODULE_DEPEND(tpm_gsc, acpi, 1, 1, 1);
MODULE_VERSION(tpm_gsc, 1);
