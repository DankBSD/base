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

#include "opt_spi.h"

#include <sys/cdefs.h>
#include <sys/param.h>
#include <sys/bus.h>
#include <sys/kernel.h>
#include <sys/module.h>
#include <sys/proc.h>
#include <sys/rman.h>
#include <sys/mutex.h>
#include <sys/endian.h>

#include <dev/spibus/spi.h>
#include <dev/spibus/spibusvar.h>

#include <dev/evdev/evdev.h>
#include <dev/evdev/input.h>

#include <dev/atopcase/atopcase.h>

#include "spibus_if.h"

static uint16_t
crc16(uint16_t crc, const void *buffer, unsigned int len)
{
	const unsigned char *cp = buffer;
	/* CRC table for the CRC-16. The poly is 0x8005 (x16 + x15 + x2 + 1). */
	static uint16_t const crc16_table[256] = {
		0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
		0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
		0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
		0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
		0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
		0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
		0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
		0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
		0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
		0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
		0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
		0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
		0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
		0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
		0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
		0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
		0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
		0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
		0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
		0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
		0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
		0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
		0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
		0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
		0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
		0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
		0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
		0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
		0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
		0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
		0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
		0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040
	};

	while (len--)
		crc = (((crc >> 8) & 0xffU) ^
		    crc16_table[(crc ^ *cp++) & 0xffU]) & 0x0000ffffU;
	return crc;
}

/* logical signal quality */
#define	SN_PRESSURE	45		/* pressure signal-to-noise ratio */
#define	SN_WIDTH	25		/* width signal-to-noise ratio */
#define	SN_COORD	250		/* coordinate signal-to-noise ratio */
#define	SN_ORIENT	10		/* orientation signal-to-noise ratio */
#define	MAX_FINGER_ORIENTATION	16384

static const struct atopcase_tpad_info {
	int model;
	struct atopcase_coords {
		int x_min;
		int y_min;
		int x_max;
		int y_max;
	} pos_limits;
} atopcase_tpad_models[] = {
	{
		.model = 3, /* MacBookPro12,1/11,4/11,5 */
		.pos_limits = { -4828, -203, 5345, 6803 },
	},
	{
		.model = 4, /* MacBook8,1/9,1/10,1 */
		.pos_limits = { -5087, -182, 5579, 6089 },
	},
	{
		.model = 5, /* MacBookPro13,1/13,2/14,1/14,2 */
		.pos_limits = { -6243, -170, 6749, 7685 },
	},
	{
		.model = 6, /* MacBookPro13,3/14,3 */
		.pos_limits = { -7456, -163, 7976, 9283 },
	}
};

/* Read == device-initiated, Write == host-initiated or reply to that */
#define DIR_READ 0x20
#define DIR_WRITE 0x40

#define DEV_KBRD 1
#define DEV_TPAD 2
#define DEV_INFO 0xd0

struct atopcase_cmd_payload {
	uint16_t constant;
} __packed;

static const struct atopcase_cmd_payload cmd_enable_mt = { 0x0102 };

struct atopcase_kbrd_payload {
	uint8_t _reserved0;
	uint8_t mods;
	uint8_t _reserved1;
	uint8_t keys[6];
	uint8_t fn;
} __packed;

struct atopcase_bl_payload {
	uint16_t _const0;
	uint16_t level;
	uint16_t status;
} __packed;

struct atopcase_tpad_finger {
	int16_t origin;
	int16_t abs_x;
	int16_t abs_y;
	int16_t rel_x;
	int16_t rel_y;
	int16_t tool_major;
	int16_t tool_minor;
	int16_t orientation;
	int16_t touch_major;
	int16_t touch_minor;
	int16_t _reserved0[2];
	int16_t pressure;
	int16_t multi;
	int16_t _reserved1;
} __packed;

struct atopcase_tpad_payload {
	uint8_t _reserved0;
	uint8_t clicked;
	uint8_t _reserved1[28];
	uint8_t finger_cnt;
	uint8_t _reserved2[17];
	struct atopcase_tpad_finger fingers[];
} __packed;

struct atopcase_info_payload {
	uint8_t _reserved0[105];
	uint8_t flags;
	uint8_t model;
	uint8_t _reserved1[3];
} __packed;

struct atopcase_header {
	uint16_t type;
	uint8_t zero;
	uint8_t seq_no;
	uint16_t resp_len;
	uint16_t len;
} __packed;

struct atopcase_packet {
	uint8_t direction;
	uint8_t device;
	uint16_t offset;
	uint16_t remaining;
	uint16_t length;
	uint8_t data[246];
	uint16_t checksum;
} __packed;

struct atopcase_softc {
	struct mtx sc_mtx;
	uint8_t sc_mt_requested;
	uint8_t sc_write_seq;
	int sc_model;

	struct evdev_dev *sc_evdev_kbrd;
	uint8_t sc_last_keys[6];
	uint8_t sc_last_mods;
	uint8_t sc_last_fn;

	struct evdev_dev *sc_evdev_tpad;

	/* Writes are async (i.e. responses arrive via interrupt) and cannot be
   * interleaved (no new writes until response arrives), so we have to queue */
	struct atopcase_packet sc_write_queue[8];
	uint8_t sc_write_queue_len;
	bool sc_write_in_flight;
};

static void
atopcase_try_drain_queue(device_t dev)
{
	struct atopcase_softc *sc = device_get_softc(dev);
	struct spi_command cmd = SPI_COMMAND_INITIALIZER;
	uint8_t rx_buffer[256] = { 0 };

	mtx_assert(&sc->sc_mtx, MA_OWNED);

	if (sc->sc_write_in_flight)
		return;

	if (sc->sc_write_queue_len == 0)
		return;

	cmd.tx_cmd = (uint8_t*)&sc->sc_write_queue[0];
	cmd.tx_cmd_sz = 256;
	cmd.rx_cmd = rx_buffer;
	cmd.rx_cmd_sz = 256;
	SPIBUS_TRANSFER(device_get_parent(dev), dev, &cmd);

	sc->sc_write_queue_len--;
	memmove((uint8_t*)&sc->sc_write_queue[0], (uint8_t*)&sc->sc_write_queue[1],
	        sizeof(struct atopcase_packet) * sc->sc_write_queue_len);

	sc->sc_write_in_flight = true;
}

static void
atopcase_enqueue_msg(device_t dev, uint8_t device, uint16_t type, const uint8_t *payload, uint8_t len, uint16_t resp_len)
{
	struct atopcase_softc *sc = device_get_softc(dev);
	struct atopcase_packet *pkt;
	struct atopcase_header *hdr;
	uint16_t msg_checksum;

	KASSERT(len + 8 + 2 <= sizeof(pkt->data),	"outgoing msg must be 1 packet");
	mtx_assert(&sc->sc_mtx, MA_OWNED);

	if (sc->sc_write_queue_len >= nitems(sc->sc_write_queue)) {
		device_printf(dev, "outgoing msg queue overflow!\n");
		return;
	}

	pkt = &sc->sc_write_queue[sc->sc_write_queue_len++];
	hdr = (struct atopcase_header *)pkt->data;

	hdr->type = type;
	if (type == 0x1020)
		hdr->zero = 2; // XXX?
	hdr->seq_no = sc->sc_write_seq++ % 256;
	hdr->resp_len = resp_len || len;
	hdr->len = len;

	pkt->direction = DIR_WRITE;
	pkt->device = device;
	pkt->length = sizeof(*hdr) + hdr->len + 2;

	memcpy(pkt->data + sizeof(*hdr), payload, len);
	msg_checksum = crc16(0, pkt->data, pkt->length - 2);
	memcpy(pkt->data + sizeof(*hdr) + hdr->len,
	       (uint8_t*)&msg_checksum, 2);
	pkt->checksum = crc16(0, (uint8_t*)pkt, sizeof(*pkt) - 2);

	atopcase_try_drain_queue(dev);
}

static const unsigned char atopcase_mods[] = {
	KEY_LEFTCTRL,
	KEY_LEFTSHIFT,
	KEY_LEFTALT,
	KEY_LEFTMETA,
	0,
	KEY_RIGHTSHIFT,
	KEY_RIGHTALT,
	KEY_RIGHTMETA
};

static void
atopcase_handle_keyboard(device_t dev, struct atopcase_kbrd_payload *evt)
{
	struct atopcase_softc *sc = device_get_softc(dev);
	bool overflow = true;
	int i;


	_Static_assert(sizeof(evt->keys) == sizeof(sc->sc_last_keys), "keys match");
	mtx_assert(&sc->sc_mtx, MA_OWNED);

	/* Rollover overflow -> ignore */
	for (i = 0; i < sizeof(evt->keys); i++)
		if (evt->keys[i] != 1)
			overflow = false;
	if (overflow)
		return;

	/* Released keys */
	for (i = 0; i < sizeof(evt->keys); i++)
		if (!memchr(evt->keys, sc->sc_last_keys[i], sizeof(evt->keys)))
			evdev_push_event(sc->sc_evdev_kbrd, EV_KEY, evdev_hid2key(sc->sc_last_keys[i]), 0);

	/* Released mods */
	for (i = 0; i < nitems(atopcase_mods); i++)
		if ((sc->sc_last_mods & (1 << i)) && !(evt->mods & (1 << i)))
			evdev_push_event(sc->sc_evdev_kbrd, EV_KEY, atopcase_mods[i], 0);

	/* Pressed mods */
	for (i = 0; i < nitems(atopcase_mods); i++)
		if (!(sc->sc_last_mods & (1 << i)) && (evt->mods & (1 << i)))
			evdev_push_event(sc->sc_evdev_kbrd, EV_KEY, atopcase_mods[i], 1);

	/* Pressed keys */
	for (i = 0; i < sizeof(evt->keys); i++)
		if (evt->keys[i])
			evdev_push_event(sc->sc_evdev_kbrd, EV_KEY, evdev_hid2key(evt->keys[i]), 1);

	evdev_sync(sc->sc_evdev_kbrd);

	memcpy(sc->sc_last_keys, evt->keys, sizeof(evt->keys));
	sc->sc_last_mods = evt->mods;
	sc->sc_last_fn = evt->fn;
}

static void
atopcase_handle_touch(device_t dev, struct atopcase_tpad_payload *evt)
{
	struct atopcase_softc *sc = device_get_softc(dev);
	struct atopcase_tpad_finger *f;
	const struct atopcase_tpad_info *info;
	int i;

	mtx_assert(&sc->sc_mtx, MA_OWNED);

	if (!sc->sc_evdev_tpad)
		return;

	info = &atopcase_tpad_models[sc->sc_model];

	for (i = 0; i < evt->finger_cnt; i++) {
		f = &evt->fingers[i];
		union evdev_mt_slot slot_data = {
			.id = i,
			.x = f->abs_x,
			.y = info->pos_limits.y_min + info->pos_limits.y_max - f->abs_y,
			.p = f->pressure,
			.maj = f->touch_major << 1,
			.min = f->touch_minor << 1,
			.w_maj = f->tool_major << 1,
			.w_min = f->tool_minor << 1,
			.ori = MAX_FINGER_ORIENTATION - f->orientation,
		};
		evdev_mt_push_slot(sc->sc_evdev_tpad, i, &slot_data);
	}

	evdev_push_key(sc->sc_evdev_tpad, BTN_LEFT, evt->clicked);
	evdev_sync(sc->sc_evdev_tpad);
}

static void
atopcase_handle_info(device_t dev, struct atopcase_info_payload *evt)
{
	struct atopcase_softc *sc = device_get_softc(dev);
	const struct atopcase_tpad_info *info = NULL;
	int i;

	mtx_assert(&sc->sc_mtx, MA_OWNED);

	if (sc->sc_evdev_tpad)
		return;

	device_printf(dev, "Trackpad model 0x%x\n", evt->model);
	sc->sc_model = evt->model;

	for (i = 0; i < nitems(atopcase_tpad_models); i++)
		if (atopcase_tpad_models[i].model == evt->model)
			info = &atopcase_tpad_models[i];
	if (!info) {
		device_printf(dev, "Unknown trackpad model, assuming 4\n");
		sc->sc_model = 1;
		info = &atopcase_tpad_models[1];
	}

	sc->sc_evdev_tpad = evdev_alloc();
	evdev_set_name(sc->sc_evdev_tpad, "Apple MacBook SPI Topcase Trackpad");
	evdev_set_phys(sc->sc_evdev_tpad, device_get_nameunit(dev));
	evdev_set_id(sc->sc_evdev_tpad, BUS_SPI, 0x05ac /* Apple */, 0, 0);
	evdev_support_prop(sc->sc_evdev_tpad, INPUT_PROP_POINTER);
	evdev_support_event(sc->sc_evdev_tpad, EV_SYN);
	evdev_support_event(sc->sc_evdev_tpad, EV_ABS);
	evdev_support_event(sc->sc_evdev_tpad, EV_KEY);
	evdev_support_key(sc->sc_evdev_tpad, BTN_LEFT);
	evdev_support_prop(sc->sc_evdev_tpad, INPUT_PROP_BUTTONPAD);
	evdev_support_abs(sc->sc_evdev_tpad, ABS_MT_POSITION_X,
		info->pos_limits.x_min, info->pos_limits.x_max,
		(info->pos_limits.x_max - info->pos_limits.x_min) / SN_COORD, 0, 0);
	evdev_support_abs(sc->sc_evdev_tpad, ABS_MT_POSITION_Y,
		info->pos_limits.y_min, info->pos_limits.y_max,
		(info->pos_limits.y_max - info->pos_limits.y_min) / SN_COORD, 0, 0);
	evdev_support_abs(sc->sc_evdev_tpad, ABS_MT_PRESSURE,
		0, 2048, 2048 / SN_PRESSURE, 0, 0);
	evdev_support_abs(sc->sc_evdev_tpad, ABS_MT_TOUCH_MAJOR,
		0, 2048, 2048 / SN_WIDTH, 0, 0);
	evdev_support_abs(sc->sc_evdev_tpad, ABS_MT_TOUCH_MINOR,
		0, 2048, 2048 / SN_WIDTH, 0, 0);
	evdev_support_abs(sc->sc_evdev_tpad, ABS_MT_WIDTH_MAJOR,
		0, 2048, 2048 / SN_WIDTH, 0, 0);
	evdev_support_abs(sc->sc_evdev_tpad, ABS_MT_WIDTH_MINOR,
		0, 2048, 2048 / SN_WIDTH, 0, 0);
	evdev_support_abs(sc->sc_evdev_tpad, ABS_MT_ORIENTATION,
		-MAX_FINGER_ORIENTATION, MAX_FINGER_ORIENTATION,
	  2 * MAX_FINGER_ORIENTATION / SN_ORIENT, 0, 0);
	/* Enable automatic touch assignment for type B MT protocol */
	evdev_support_abs(sc->sc_evdev_tpad, ABS_MT_SLOT,
	    0, MAX_MT_SLOTS - 1, 0, 0, 0);
	evdev_support_abs(sc->sc_evdev_tpad, ABS_MT_TRACKING_ID,
	    0, MAX_MT_SLOTS - 1, 0, 0, 0);
	evdev_set_flag(sc->sc_evdev_tpad, EVDEV_FLAG_MT_TRACK);
	evdev_set_flag(sc->sc_evdev_tpad, EVDEV_FLAG_MT_AUTOREL);
	/* Synaptics compatibility events */
	evdev_set_flag(sc->sc_evdev_tpad, EVDEV_FLAG_MT_STCOMPAT);

	if (evdev_register(sc->sc_evdev_tpad)) {
		evdev_free(sc->sc_evdev_tpad);
		sc->sc_evdev_tpad = NULL;
		device_printf(dev, "Could not register trackpad evdev");
	}
}

static int
atopcase_probe(device_t dev)
{
	/* On Intel, atopcase_acpi explicitly creates us under a spibus. */

	device_set_desc(dev, "Apple MacBook SPI Topcase");
	return (BUS_PROBE_NOWILDCARD);
}

void
atopcase_intr(device_t dev)
{
	struct atopcase_softc *sc = device_get_softc(dev);
	struct atopcase_packet pkt = { 0 };
	struct atopcase_header *hdr = (struct atopcase_header *)pkt.data;
	struct spi_command cmd = SPI_COMMAND_INITIALIZER;
	uint8_t tx_buffer[256] = { 0 };

	mtx_lock(&sc->sc_mtx);

	cmd.tx_cmd = tx_buffer;
	cmd.tx_cmd_sz = 256;
	cmd.rx_cmd = (uint8_t*)&pkt;
	cmd.rx_cmd_sz = 256;
	SPIBUS_TRANSFER(device_get_parent(dev), dev, &cmd);

	hexdump((uint8_t*)&pkt, 256, 0, 0);

	if (pkt.checksum != crc16(0, (uint8_t*)&pkt, sizeof(pkt) - 2)) {
		device_printf(dev, "failed packet checksum\n");
		goto end;
	}

	// TODO: Big TODO. Only required for many-many fingers on the trackpad.
	if (pkt.remaining != 0) {
		device_printf(dev, "TODO: multi-packet messages\n");
		goto end;
	}

	if (hdr->len + 8 + 2 >= sizeof(pkt.data)) {
		device_printf(dev, "TODO: multi-packet messages\n");
		goto end;
	}

	if (pkt.length - 2 > sizeof(pkt.data) ||
	    *(uint16_t*)(pkt.data + sizeof(*hdr) + hdr->len) != crc16(0, pkt.data, pkt.length - 2)) {
		device_printf(dev, "failed message checksum\n");
		goto end;
	}

	if (pkt.device == DEV_KBRD) {
		if (hdr->type != 0x0110) {
			device_printf(dev, "unexpected type %d for keyboard\n", hdr->type);
			goto end;
		}
		if (hdr->len != sizeof(struct atopcase_kbrd_payload)) {
			device_printf(dev, "unexpected len %d for keyboard\n", hdr->len);
			goto end;
		}
		atopcase_handle_keyboard(dev, (struct atopcase_kbrd_payload *)(pkt.data + sizeof(*hdr)));
	} else if (pkt.device == DEV_TPAD) {
		if (hdr->type == 0x252) {
			device_printf(dev, "multi-touch enabled\n");
		} else if (hdr->type == 0x210) {
			// TODO: check length, counting the fingers
			atopcase_handle_touch(dev, (struct atopcase_tpad_payload *)(pkt.data + sizeof(*hdr)));
		} else {
			device_printf(dev, "unexpected type %d for trackpad\n", hdr->type);
		}
	} else if (pkt.device == DEV_INFO) {
		if (hdr->type != 0x1020) {
			device_printf(dev, "unexpected type %d for info\n", hdr->type);
			goto end;
		}
		atopcase_handle_info(dev, (struct atopcase_info_payload *)(pkt.data + sizeof(*hdr)));
	}

end:
	if (pkt.direction == DIR_WRITE) {
		sc->sc_write_in_flight = false;
		atopcase_try_drain_queue(dev);
	}

	if (!sc->sc_mt_requested) {
		/* Request trackpad info (seems to arrive on its own anyway, but still) */
		atopcase_enqueue_msg(dev, DEV_INFO, 0x1020, NULL, 0, 0x200);
		/* Enable multi-touch */
		atopcase_enqueue_msg(dev, DEV_TPAD, 0x0252, (const uint8_t*)&cmd_enable_mt, sizeof(cmd_enable_mt), 0);
		sc->sc_mt_requested = 1;
	}
	mtx_unlock(&sc->sc_mtx);
}

static int
atopcase_detach(device_t dev);

static int
atopcase_attach(device_t dev)
{
	struct atopcase_softc *sc = device_get_softc(dev);
	int i;

	mtx_init(&sc->sc_mtx, device_get_nameunit(dev), NULL, MTX_DEF);

	sc->sc_evdev_kbrd = evdev_alloc();
	evdev_set_name(sc->sc_evdev_kbrd, "Apple MacBook SPI Topcase Keyboard");
	evdev_set_phys(sc->sc_evdev_kbrd, device_get_nameunit(dev));
	evdev_set_id(sc->sc_evdev_kbrd, BUS_SPI, 0x05ac /* Apple */, 0, 0);
	evdev_support_event(sc->sc_evdev_kbrd, EV_SYN);
	evdev_support_event(sc->sc_evdev_kbrd, EV_KEY);
	evdev_support_event(sc->sc_evdev_kbrd, EV_LED);
	evdev_support_event(sc->sc_evdev_kbrd, EV_REP);
	evdev_support_led(sc->sc_evdev_kbrd, LED_CAPSL);
	for (i = 0x00; i <= 0xFF; i++)
		evdev_support_key(sc->sc_evdev_kbrd, evdev_hid2key(i));
	evdev_set_flag(sc->sc_evdev_kbrd, EVDEV_FLAG_SOFTREPEAT);
	if (evdev_register(sc->sc_evdev_kbrd)) {
		atopcase_detach(dev);
		return (ENXIO);
	}

	return (bus_generic_attach(dev));
}

static int
atopcase_detach(device_t dev)
{
	struct atopcase_softc *sc = device_get_softc(dev);

	if (sc->sc_evdev_kbrd)
		evdev_free(sc->sc_evdev_kbrd);
	if (sc->sc_evdev_tpad)
		evdev_free(sc->sc_evdev_tpad);
	mtx_destroy(&sc->sc_mtx);

	return (bus_generic_detach(dev));
}

int
atopcase_resume(device_t dev)
{
	struct atopcase_softc *sc = device_get_softc(dev);

	/* Re-enable multi-touch on next interrupt */
	sc->sc_mt_requested = 0;

	return (0);
}

static device_method_t atopcase_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe, atopcase_probe),
	DEVMETHOD(device_attach, atopcase_attach),
	DEVMETHOD(device_detach, atopcase_detach),

	DEVMETHOD_END
};

static driver_t atopcase_driver = {
	"atopcase",
	atopcase_methods,
	sizeof(struct atopcase_softc),
};

static devclass_t atopcase_devclass;
DRIVER_MODULE(atopcase, spibus, atopcase_driver, atopcase_devclass, 0, 0);
MODULE_DEPEND(atopcase, spibus, 1, 1, 1);
MODULE_DEPEND(atopcase, evdev, 1, 1, 1);
