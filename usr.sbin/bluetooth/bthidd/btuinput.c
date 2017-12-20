/*-
 * SPDX-License-Identifier: BSD-2-Clause-FreeBSD
 *
 * Copyright (c) 2015-2017 Vladimir Kondratyev <wulf@FreeBSD.org>
 * All rights reserved.
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
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD$
 */

#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/kbio.h>
#include <sys/sysctl.h>

#include <dev/evdev/input.h>
#include <dev/evdev/uinput.h>
#include <dev/usb/usb.h>
#include <dev/usb/usbhid.h>

#include <assert.h>
#define L2CAP_SOCKET_CHECKED
#include <bluetooth.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>
#include <usbhid.h>

#include "bthid_config.h"
#include "bthidd.h"
#include "btuinput.h"

#define AMT_DIMENSION_X 13000.0f
#define AMT_MIN_X -2909
#define AMT_MAX_X 3167
#define AMT_RES_X ((AMT_MAX_X - AMT_MIN_X) / (AMT_DIMENSION_X / 100))
#define AMT_DIMENSION_Y 11000.0f
#define AMT_MIN_Y -2456
#define AMT_MAX_Y 2565
#define AMT_RES_Y ((AMT_MAX_Y - AMT_MIN_Y) / (AMT_DIMENSION_Y / 100))

static int16_t const mbuttons[8] = {
	BTN_LEFT,
	BTN_MIDDLE,
	BTN_RIGHT,
	BTN_SIDE,
	BTN_EXTRA,
	BTN_FORWARD,
	BTN_BACK,
	BTN_TASK
};

static uint16_t const led_codes[3] = {
	LED_CAPSL,	/* CLKED */
	LED_NUML,	/* NLKED */
	LED_SCROLLL,	/* SLKED */
};

#define	NONE	KEY_RESERVED

static uint16_t const keymap[0x100] = {
	/* 0x00 - 0x27 */
	NONE,	NONE,	NONE,	NONE,	KEY_A,	KEY_B,	KEY_C,	KEY_D,
	KEY_E,	KEY_F,	KEY_G,	KEY_H,	KEY_I,	KEY_J,	KEY_K,	KEY_L,
	KEY_M,	KEY_N,	KEY_O,	KEY_P,	KEY_Q,	KEY_R,	KEY_S,	KEY_T,
	KEY_U,	KEY_V,	KEY_W,	KEY_X,	KEY_Y,	KEY_Z,	KEY_1,	KEY_2,
	KEY_3,	KEY_4,	KEY_5,	KEY_6,	KEY_7,	KEY_8,	KEY_9,	KEY_0,
	/* 0x28 - 0x3f */
	KEY_ENTER,	KEY_ESC,	KEY_BACKSPACE,	KEY_TAB,
	KEY_SPACE,	KEY_MINUS,	KEY_EQUAL,	KEY_LEFTBRACE,
	KEY_RIGHTBRACE,	KEY_BACKSLASH,	KEY_BACKSLASH,	KEY_SEMICOLON,
	KEY_APOSTROPHE,	KEY_GRAVE,	KEY_COMMA,	KEY_DOT,
	KEY_SLASH,	KEY_CAPSLOCK,	KEY_F1,		KEY_F2,
	KEY_F3,		KEY_F4,		KEY_F5,		KEY_F6,
	/* 0x40 - 0x5f */
	KEY_F7,		KEY_F8,		KEY_F9,		KEY_F10,
	KEY_F11,	KEY_F12,	KEY_SYSRQ,	KEY_SCROLLLOCK,
	KEY_PAUSE,	KEY_INSERT,	KEY_HOME,	KEY_PAGEUP,
	KEY_DELETE,	KEY_END,	KEY_PAGEDOWN,	KEY_RIGHT,
	KEY_LEFT,	KEY_DOWN,	KEY_UP,		KEY_NUMLOCK,
	KEY_KPSLASH,	KEY_KPASTERISK,	KEY_KPMINUS,	KEY_KPPLUS,
	KEY_KPENTER,	KEY_KP1,	KEY_KP2,	KEY_KP3,
	KEY_KP4,	KEY_KP5,	KEY_KP6,	KEY_KP7,
	/* 0x60 - 0x7f */
	KEY_KP8,	KEY_KP9,	KEY_KP0,	KEY_KPDOT,
	KEY_102ND,	KEY_COMPOSE,	KEY_POWER,	KEY_KPEQUAL,
	KEY_F13,	KEY_F14,	KEY_F15,	KEY_F16,
	KEY_F17,	KEY_F18,	KEY_F19,	KEY_F20,
	KEY_F21,	KEY_F22,	KEY_F23,	KEY_F24,
	KEY_OPEN,	KEY_HELP,	KEY_PROPS,	KEY_FRONT,
	KEY_STOP,	KEY_AGAIN,	KEY_UNDO,	KEY_CUT,
	KEY_COPY,	KEY_PASTE,	KEY_FIND,	KEY_MUTE,
	/* 0x80 - 0x9f */
	KEY_VOLUMEUP,	KEY_VOLUMEDOWN,	NONE,		NONE,
	NONE,		KEY_KPCOMMA,	NONE,		KEY_RO,
	KEY_KATAKANAHIRAGANA,	KEY_YEN,KEY_HENKAN,	KEY_MUHENKAN,
	KEY_KPJPCOMMA,	NONE,		NONE,		NONE,
	KEY_HANGEUL,	KEY_HANJA,	KEY_KATAKANA,	KEY_HIRAGANA,
	KEY_ZENKAKUHANKAKU,	NONE,	NONE,		NONE,
	NONE,		NONE,		NONE,		NONE,
	NONE,		NONE,		NONE,		NONE,
	/* 0xa0 - 0xbf */
	NONE,		NONE,		NONE,		NONE,
	NONE,		NONE,		NONE,		NONE,
	NONE,		NONE,		NONE,		NONE,
	NONE,		NONE,		NONE,		NONE,
	NONE,		NONE,		NONE,		NONE,
	NONE,		NONE,		NONE,		NONE,
	NONE,		NONE,		NONE,		NONE,
	NONE,		NONE,		NONE,		NONE,
	/* 0xc0 - 0xdf */
	NONE,		NONE,           NONE,		NONE,
	NONE,		NONE,           NONE,		NONE,
	NONE,		NONE,           NONE,		NONE,
	NONE,		NONE,           NONE,		NONE,
	NONE,		NONE,           NONE,		NONE,
	NONE,		NONE,           NONE,		NONE,
	NONE,		NONE,           NONE,		NONE,
	NONE,		NONE,           NONE,		NONE,
	/* 0xe0 - 0xff */
	KEY_LEFTCTRL,	KEY_LEFTSHIFT,	KEY_LEFTALT,	KEY_LEFTMETA,
	KEY_RIGHTCTRL,	KEY_RIGHTSHIFT,	KEY_RIGHTALT,	KEY_RIGHTMETA,
	KEY_PLAYPAUSE,	KEY_STOPCD,	KEY_PREVIOUSSONG,KEY_NEXTSONG,
	KEY_EJECTCD,	KEY_VOLUMEUP,	KEY_VOLUMEDOWN, KEY_MUTE,
	KEY_WWW,	KEY_BACK,	KEY_FORWARD,	KEY_STOP,
	KEY_FIND,	KEY_SCROLLUP,	KEY_SCROLLDOWN,	KEY_EDIT,
	KEY_SLEEP,	KEY_COFFEE,	KEY_REFRESH,	KEY_CALC,
	NONE,		NONE,		NONE,		NONE,
};

/* Consumer page usage mapping */
static uint16_t const consmap[0x300] = {
	[0x030] = KEY_POWER,
	[0x031] = KEY_RESTART,
	[0x032] = KEY_SLEEP,
	[0x034] = KEY_SLEEP,
	[0x035] = KEY_KBDILLUMTOGGLE,
	[0x036] = BTN_MISC,
	[0x040] = KEY_MENU,
	[0x041] = KEY_SELECT,
	[0x042] = KEY_UP,
	[0x043] = KEY_DOWN,
	[0x044] = KEY_LEFT,
	[0x045] = KEY_RIGHT,
	[0x046] = KEY_ESC,
	[0x047] = KEY_KPPLUS,
	[0x048] = KEY_KPMINUS,
	[0x060] = KEY_INFO,
	[0x061] = KEY_SUBTITLE,
	[0x063] = KEY_VCR,
	[0x065] = KEY_CAMERA,
	[0x069] = KEY_RED,
	[0x06a] = KEY_GREEN,
	[0x06b] = KEY_BLUE,
	[0x06c] = KEY_YELLOW,
	[0x06d] = KEY_ZOOM,
	[0x06f] = KEY_BRIGHTNESSUP,
	[0x070] = KEY_BRIGHTNESSDOWN,
	[0x072] = KEY_BRIGHTNESS_TOGGLE,
	[0x073] = KEY_BRIGHTNESS_MIN,
	[0x074] = KEY_BRIGHTNESS_MAX,
	[0x075] = KEY_BRIGHTNESS_AUTO,
	[0x082] = KEY_VIDEO_NEXT,
	[0x083] = KEY_LAST,
	[0x084] = KEY_ENTER,
	[0x088] = KEY_PC,
	[0x089] = KEY_TV,
	[0x08a] = KEY_WWW,
	[0x08b] = KEY_DVD,
	[0x08c] = KEY_PHONE,
	[0x08d] = KEY_PROGRAM,
	[0x08e] = KEY_VIDEOPHONE,
	[0x08f] = KEY_GAMES,
	[0x090] = KEY_MEMO,
	[0x091] = KEY_CD,
	[0x092] = KEY_VCR,
	[0x093] = KEY_TUNER,
	[0x094] = KEY_EXIT,
	[0x095] = KEY_HELP,
	[0x096] = KEY_TAPE,
	[0x097] = KEY_TV2,
	[0x098] = KEY_SAT,
	[0x09a] = KEY_PVR,
	[0x09c] = KEY_CHANNELUP,
	[0x09d] = KEY_CHANNELDOWN,
	[0x0a0] = KEY_VCR2,
	[0x0b0] = KEY_PLAY,
	[0x0b1] = KEY_PAUSE,
	[0x0b2] = KEY_RECORD,
	[0x0b3] = KEY_FASTFORWARD,
	[0x0b4] = KEY_REWIND,
	[0x0b5] = KEY_NEXTSONG,
	[0x0b6] = KEY_PREVIOUSSONG,
	[0x0b7] = KEY_STOPCD,
	[0x0b8] = KEY_EJECTCD,
	[0x0bc] = KEY_MEDIA_REPEAT,
	[0x0b9] = KEY_SHUFFLE,
	[0x0bf] = KEY_SLOW,
	[0x0cd] = KEY_PLAYPAUSE,
	[0x0cf] = KEY_VOICECOMMAND,
	[0x0e2] = KEY_MUTE,
	[0x0e5] = KEY_BASSBOOST,
	[0x0e9] = KEY_VOLUMEUP,
	[0x0ea] = KEY_VOLUMEDOWN,
	[0x0f5] = KEY_SLOW,
	[0x181] = KEY_BUTTONCONFIG,
	[0x182] = KEY_BOOKMARKS,
	[0x183] = KEY_CONFIG,
	[0x184] = KEY_WORDPROCESSOR,
	[0x185] = KEY_EDITOR,
	[0x186] = KEY_SPREADSHEET,
	[0x187] = KEY_GRAPHICSEDITOR,
	[0x188] = KEY_PRESENTATION,
	[0x189] = KEY_DATABASE,
	[0x18a] = KEY_MAIL,
	[0x18b] = KEY_NEWS,
	[0x18c] = KEY_VOICEMAIL,
	[0x18d] = KEY_ADDRESSBOOK,
	[0x18e] = KEY_CALENDAR,
	[0x18f] = KEY_TASKMANAGER,
	[0x190] = KEY_JOURNAL,
	[0x191] = KEY_FINANCE,
	[0x192] = KEY_CALC,
	[0x193] = KEY_PLAYER,
	[0x194] = KEY_FILE,
	[0x196] = KEY_WWW,
	[0x199] = KEY_CHAT,
	[0x19c] = KEY_LOGOFF,
	[0x19e] = KEY_COFFEE,
	[0x19f] = KEY_CONTROLPANEL,
	[0x1a2] = KEY_APPSELECT,
	[0x1a3] = KEY_NEXT,
	[0x1a4] = KEY_PREVIOUS,
	[0x1a6] = KEY_HELP,
	[0x1a7] = KEY_DOCUMENTS,
	[0x1ab] = KEY_SPELLCHECK,
	[0x1ae] = KEY_KEYBOARD,
	[0x1b1] = KEY_SCREENSAVER,
	[0x1b4] = KEY_FILE,
	[0x1b6] = KEY_IMAGES,
	[0x1b7] = KEY_AUDIO,
	[0x1b8] = KEY_VIDEO,
	[0x1bc] = KEY_MESSENGER,
	[0x1bd] = KEY_INFO,
	[0x201] = KEY_NEW,
	[0x202] = KEY_OPEN,
	[0x203] = KEY_CLOSE,
	[0x204] = KEY_EXIT,
	[0x207] = KEY_SAVE,
	[0x208] = KEY_PRINT,
	[0x209] = KEY_PROPS,
	[0x21a] = KEY_UNDO,
	[0x21b] = KEY_COPY,
	[0x21c] = KEY_CUT,
	[0x21d] = KEY_PASTE,
	[0x21f] = KEY_FIND,
	[0x221] = KEY_SEARCH,
	[0x222] = KEY_GOTO,
	[0x223] = KEY_HOMEPAGE,
	[0x224] = KEY_BACK,
	[0x225] = KEY_FORWARD,
	[0x226] = KEY_STOP,
	[0x227] = KEY_REFRESH,
	[0x22a] = KEY_BOOKMARKS,
	[0x22d] = KEY_ZOOMIN,
	[0x22e] = KEY_ZOOMOUT,
	[0x22f] = KEY_ZOOMRESET,
	[0x233] = KEY_SCROLLUP,
	[0x234] = KEY_SCROLLDOWN,
	[0x23d] = KEY_EDIT,
	[0x25f] = KEY_CANCEL,
	[0x269] = KEY_INSERT,
	[0x26a] = KEY_DELETE,
	[0x279] = KEY_REDO,
	[0x289] = KEY_REPLY,
	[0x28b] = KEY_FORWARDMAIL,
	[0x28c] = KEY_SEND,
	[0x2c7] = KEY_KBDINPUTASSIST_PREV,
	[0x2c8] = KEY_KBDINPUTASSIST_NEXT,
	[0x2c9] = KEY_KBDINPUTASSIST_PREVGROUP,
	[0x2ca] = KEY_KBDINPUTASSIST_NEXTGROUP,
	[0x2cb] = KEY_KBDINPUTASSIST_ACCEPT,
	[0x2cc] = KEY_KBDINPUTASSIST_CANCEL,
};

static int32_t
uinput_open_common(hid_device_p const p, bdaddr_p local, const uint8_t *name)
{
	struct uinput_setup	uisetup;
	uint8_t			phys[UINPUT_MAX_NAME_SIZE];
	uint8_t			uniq[UINPUT_MAX_NAME_SIZE];
	struct hostent		*hp;
	int32_t			fd;

	/* Take local and remote bdaddr */
	bt_ntoa(local, phys);
	bt_ntoa(&p->bdaddr, uniq);

	/* Take device name from bthidd.conf. Fallback to generic name. */
	if (p->name != NULL)
		name = p->name;
	else if ((hp = bt_gethostbyaddr((const char *)&p->bdaddr,
				sizeof(p->bdaddr), AF_BLUETOOTH)) != NULL)
		name = hp->h_name;

	/* Set device name and bus/vendor information */
	memset(&uisetup, 0, sizeof(uisetup));
	snprintf(uisetup.name, UINPUT_MAX_NAME_SIZE,
	    "%s, bdaddr %s", name, uniq);
	uisetup.id.bustype = BUS_BLUETOOTH;
	uisetup.id.vendor  = p->vendor_id;
	uisetup.id.product = p->product_id;
	uisetup.id.version = p->version;

	fd = open("/dev/uinput", O_RDWR | O_NONBLOCK);

	if (ioctl(fd, UI_SET_PHYS, phys) < 0 ||
	    ioctl(fd, UI_SET_BSDUNIQ, uniq) < 0 ||
	    ioctl(fd, UI_DEV_SETUP, &uisetup) < 0)
		return (-1);

	return (fd);
}

/*
 * Setup uinput device as 8button mouse with wheel(s)
 * TODO: bring in more feature detection code from ums
 */
int32_t
uinput_open_mouse(hid_device_p const p, bdaddr_p local, int is_apple_trackpad)
{
	size_t	i;
	int32_t	fd;

	assert(p != NULL);

	if ((fd = uinput_open_common(p, local, "Bluetooth Mouse")) < 0)
		goto bail_out;

	/* Advertise multi-touch support */
	if (is_apple_trackpad) {
		if (ioctl(fd, UI_SET_EVBIT, EV_ABS) < 0 ||
		    ioctl(fd, UI_SET_EVBIT, EV_KEY) < 0 ||
		    ioctl(fd, UI_SET_KEYBIT, BTN_LEFT) < 0 ||
		    ioctl(fd, UI_SET_KEYBIT, BTN_TOOL_FINGER) < 0 ||
		    ioctl(fd, UI_SET_KEYBIT, BTN_TOOL_DOUBLETAP) < 0 ||
		    ioctl(fd, UI_SET_KEYBIT, BTN_TOOL_TRIPLETAP) < 0 ||
		    ioctl(fd, UI_SET_KEYBIT, BTN_TOOL_QUADTAP) < 0 ||
		    ioctl(fd, UI_SET_KEYBIT, BTN_TOOL_QUINTTAP) < 0 ||
		    ioctl(fd, UI_SET_KEYBIT, BTN_TOUCH) < 0 ||
		    ioctl(fd, UI_SET_PROPBIT, INPUT_PROP_POINTER) < 0 ||
		    ioctl(fd, UI_SET_PROPBIT, INPUT_PROP_BUTTONPAD) < 0)
			goto bail_out;

		struct uinput_abs_setup abs_x;
		abs_x.code = ABS_X;
		abs_x.absinfo.value = 0;
		abs_x.absinfo.minimum = AMT_MIN_X;
		abs_x.absinfo.maximum = AMT_MAX_X;
		abs_x.absinfo.resolution = AMT_RES_X;
		abs_x.absinfo.fuzz = 4;
		abs_x.absinfo.flat = 0;
		if (ioctl(fd, UI_ABS_SETUP, &abs_x) < 0)
			goto bail_out;

		struct uinput_abs_setup abs_y;
		abs_y.code = ABS_Y;
		abs_y.absinfo.value = 0;
		abs_y.absinfo.minimum = AMT_MIN_Y;
		abs_y.absinfo.maximum = AMT_MAX_Y;
		abs_y.absinfo.resolution = AMT_RES_Y;
		abs_y.absinfo.fuzz = 4;
		abs_y.absinfo.flat = 0;
		if (ioctl(fd, UI_ABS_SETUP, &abs_y) < 0)
			goto bail_out;

		struct uinput_abs_setup abs_x_mt;
		abs_x_mt.code = ABS_MT_POSITION_X;
		abs_x_mt.absinfo.value = 0;
		abs_x_mt.absinfo.minimum = AMT_MIN_X;
		abs_x_mt.absinfo.maximum = AMT_MAX_X;
		abs_x_mt.absinfo.resolution = AMT_RES_X;
		abs_x_mt.absinfo.fuzz = 4;
		abs_x_mt.absinfo.flat = 0;
		if (ioctl(fd, UI_ABS_SETUP, &abs_x_mt) < 0)
			goto bail_out;

		struct uinput_abs_setup abs_y_mt;
		abs_y_mt.code = ABS_MT_POSITION_Y;
		abs_y_mt.absinfo.value = 0;
		abs_y_mt.absinfo.minimum = AMT_MIN_Y;
		abs_y_mt.absinfo.maximum = AMT_MAX_Y;
		abs_y_mt.absinfo.resolution = AMT_RES_Y;
		abs_y_mt.absinfo.fuzz = 4;
		abs_y_mt.absinfo.flat = 0;
		if (ioctl(fd, UI_ABS_SETUP, &abs_y_mt) < 0)
			goto bail_out;

		struct uinput_abs_setup abs_slot;
		abs_slot.code = ABS_MT_SLOT;
		abs_slot.absinfo.value = 0;
		abs_slot.absinfo.minimum = 0;
		abs_slot.absinfo.maximum = 15;
		abs_slot.absinfo.resolution = 0;
		abs_slot.absinfo.fuzz = 0;
		abs_slot.absinfo.flat = 0;
		if (ioctl(fd, UI_ABS_SETUP, &abs_slot) < 0)
			goto bail_out;

		struct uinput_abs_setup abs_track;
		abs_track.code = ABS_MT_TRACKING_ID;
		abs_track.absinfo.value = 0;
		abs_track.absinfo.minimum = 0;
		abs_track.absinfo.maximum = 0xffff;
		abs_track.absinfo.resolution = 0;
		abs_track.absinfo.fuzz = 0;
		abs_track.absinfo.flat = 0;
		if (ioctl(fd, UI_ABS_SETUP, &abs_track) < 0)
			goto bail_out;

		struct uinput_abs_setup abs_touch_major;
		abs_touch_major.code = ABS_MT_TOUCH_MAJOR;
		abs_touch_major.absinfo.value = 0;
		abs_touch_major.absinfo.minimum = 0;
		abs_touch_major.absinfo.maximum = 255 << 2;
		abs_touch_major.absinfo.resolution = 0;
		abs_touch_major.absinfo.fuzz = 4;
		abs_touch_major.absinfo.flat = 0;
		if (ioctl(fd, UI_ABS_SETUP, &abs_touch_major) < 0)
			goto bail_out;

		struct uinput_abs_setup abs_touch_minor;
		abs_touch_minor.code = ABS_MT_TOUCH_MINOR;
		abs_touch_minor.absinfo.value = 0;
		abs_touch_minor.absinfo.minimum = 0;
		abs_touch_minor.absinfo.maximum = 255 << 2;
		abs_touch_minor.absinfo.resolution = 0;
		abs_touch_minor.absinfo.fuzz = 4;
		abs_touch_minor.absinfo.flat = 0;
		if (ioctl(fd, UI_ABS_SETUP, &abs_touch_minor) < 0)
			goto bail_out;

		struct uinput_abs_setup abs_orientation;
		abs_orientation.code = ABS_MT_ORIENTATION;
		abs_orientation.absinfo.value = 0;
		abs_orientation.absinfo.minimum = -31;
		abs_orientation.absinfo.maximum = 32;
		abs_orientation.absinfo.resolution = 0;
		abs_orientation.absinfo.fuzz = 1;
		abs_orientation.absinfo.flat = 0;
		if (ioctl(fd, UI_ABS_SETUP, &abs_orientation) < 0)
			goto bail_out;
	} else {
		/* Advertise events and axes */
		if (ioctl(fd, UI_SET_EVBIT, EV_SYN) < 0 ||
		    ioctl(fd, UI_SET_EVBIT, EV_KEY) < 0 ||
		    ioctl(fd, UI_SET_EVBIT, EV_REL) < 0 ||
		    ioctl(fd, UI_SET_RELBIT, REL_X) < 0 ||
		    ioctl(fd, UI_SET_RELBIT, REL_Y) < 0 ||
		    (p->has_wheel && ioctl(fd, UI_SET_RELBIT, REL_WHEEL) < 0) ||
		    (p->has_hwheel && ioctl(fd, UI_SET_RELBIT, REL_HWHEEL) < 0) ||
		    ioctl(fd, UI_SET_PROPBIT, INPUT_PROP_POINTER) < 0)
			goto bail_out;

		/* Advertise mouse buttons */
		for (i = 0; i < nitems(mbuttons); i++)
			if (ioctl(fd, UI_SET_KEYBIT, mbuttons[i]) < 0)
				goto bail_out;
	}

	if (ioctl(fd, UI_DEV_CREATE) >= 0)
		return (fd); /* SUCCESS */

bail_out:
	if (fd >= 0)
		close(fd);
	return (-1);
}

/*
 * Setup uinput keyboard
 */
int32_t
uinput_open_keyboard(hid_device_p const p, bdaddr_p local)
{
	size_t	i;
	int32_t	fd;

	assert(p != NULL);

	if ((fd = uinput_open_common(p, local, "Bluetooth Keyboard")) < 0)
		goto bail_out;

	/* Advertise key events and LEDs */
	if (ioctl(fd, UI_SET_EVBIT, EV_KEY) < 0 ||
	    ioctl(fd, UI_SET_EVBIT, EV_LED) < 0 ||
	    ioctl(fd, UI_SET_EVBIT, EV_SYN) < 0 ||
	    ioctl(fd, UI_SET_EVBIT, EV_REP) < 0 ||
	    ioctl(fd, UI_SET_LEDBIT, LED_CAPSL) < 0 ||
	    ioctl(fd, UI_SET_LEDBIT, LED_NUML) < 0 ||
	    ioctl(fd, UI_SET_LEDBIT, LED_SCROLLL))
		goto bail_out;

	/* Advertise keycodes */
	for (i = 0; i < nitems(keymap); i++)
		if (keymap[i] != NONE &&
		    ioctl(fd, UI_SET_KEYBIT, keymap[i]) < 0)
			goto bail_out;

	/* Advertise consumer page keys if any */
	if (p->has_cons) {
		for (i = 0; i < nitems(consmap); i++) {
			if (consmap[i] != NONE &&
			    ioctl(fd, UI_SET_KEYBIT, consmap[i]) < 0)
				goto bail_out;
		}
	}

	if (ioctl(fd, UI_DEV_CREATE) >= 0)
		return (fd); /* SUCCESS */

bail_out:
	if (fd >= 0)
		close(fd);
	return (-1);
}

/* from sys/dev/evdev/evdev.h */
#define	EVDEV_RCPT_HW_MOUSE	(1<<2)
#define	EVDEV_RCPT_HW_KBD	(1<<3)

#define	MASK_POLL_INTERVAL	5 /* seconds */
#define	MASK_SYSCTL		"kern.evdev.rcpt_mask"

static int32_t
uinput_get_rcpt_mask(void)
{
	static struct timespec last = { 0, 0 };
	struct timespec now;
	static int32_t mask = 0;
	size_t len;
	time_t elapsed;

	if (clock_gettime(CLOCK_MONOTONIC_FAST, &now) == -1)
		return mask;

	elapsed = now.tv_sec - last.tv_sec;
	if (now.tv_nsec < last.tv_nsec)
		elapsed--;

	if (elapsed >= MASK_POLL_INTERVAL) {
		len = sizeof(mask);
		if (sysctlbyname(MASK_SYSCTL, &mask, &len, NULL, 0) < 0) {
			if (errno == ENOENT)
				/* kernel is compiled w/o EVDEV_SUPPORT */
				mask = EVDEV_RCPT_HW_MOUSE | EVDEV_RCPT_HW_KBD;
			else
				mask = 0;
		}
		last = now;
	}
	return mask;
}

static int32_t
uinput_write_event(int32_t fd, uint16_t type, uint16_t code, int32_t value)
{
	struct input_event ie;

	assert(fd >= 0);

	memset(&ie, 0, sizeof(ie));
	ie.type = type;
	ie.code = code;
	ie.value = value;
	return (write(fd, &ie, sizeof(ie)));
}


int32_t
uinput_rep_sync(int32_t fd) {
	assert(fd >= 0);

	if (uinput_write_event(fd, EV_SYN, SYN_REPORT, 0) < 0)
		return (-1);

	return (0);
}

int32_t
uinput_rep_mouse(int32_t fd, int32_t x, int32_t y, int32_t z, int32_t t,
    int32_t buttons, int32_t obuttons)
{
	size_t i;
	int32_t rcpt_mask, mask;

	assert(fd >= 0);

	rcpt_mask = uinput_get_rcpt_mask();
	if (!(rcpt_mask & EVDEV_RCPT_HW_MOUSE))
		return (0);

	if ((x != 0 && uinput_write_event(fd, EV_REL, REL_X, x) < 0) ||
	    (y != 0 && uinput_write_event(fd, EV_REL, REL_Y, y) < 0) ||
	    (z != 0 && uinput_write_event(fd, EV_REL, REL_WHEEL, -z) < 0) ||
	    (t != 0 && uinput_write_event(fd, EV_REL, REL_HWHEEL, t) < 0))
		return (-1);

	for (i = 0; i < nitems(mbuttons); i++) {
		mask = 1 << i;
		if ((buttons & mask) == (obuttons & mask))
			continue;
		if (uinput_write_event(fd, EV_KEY, mbuttons[i],
		    (buttons & mask) != 0) < 0)
			return (-1);
	}

	if (uinput_write_event(fd, EV_SYN, SYN_REPORT, 0) < 0)
		return (-1);

	return (0);
}

/*
 * Translate and report trackpad multi-touch events
 */
int32_t
uinput_rep_multi_touch(int32_t fd, int32_t id, int32_t x, int32_t y,
	int32_t orientation, int32_t touch_major, int32_t touch_minor, int32_t state)
{
	assert(fd >= 0);

	if (uinput_write_event(fd, EV_ABS, ABS_MT_SLOT, id) < 0)
		return (-1);

	if (state != 0) {
		if (uinput_write_event(fd, EV_ABS, ABS_MT_TRACKING_ID, id) < 0 ||
		    uinput_write_event(fd, EV_ABS, ABS_MT_TOUCH_MAJOR, touch_major << 2) < 0 ||
		    uinput_write_event(fd, EV_ABS, ABS_MT_TOUCH_MINOR, touch_minor << 2) < 0 ||
		    uinput_write_event(fd, EV_ABS, ABS_MT_ORIENTATION, -orientation) < 0 ||
		    uinput_write_event(fd, EV_ABS, ABS_MT_POSITION_X, x) < 0 ||
		    uinput_write_event(fd, EV_ABS, ABS_MT_POSITION_Y, y) < 0)
			return (-1);
	} else {
		if (uinput_write_event(fd, EV_ABS, ABS_MT_TRACKING_ID, -1) < 0)
			return (-1);
	}

	return (0);
}

/*
 * Translate and report trackpad single-touch events
 */
int32_t
uinput_rep_single_touch(int32_t fd, int32_t x, int32_t y, int32_t state)
{
	assert(fd >= 0);

	if (uinput_write_event(fd, EV_ABS, ABS_X, x) < 0 ||
	    uinput_write_event(fd, EV_ABS, ABS_Y, y) < 0 ||
	    uinput_write_event(fd, EV_KEY, BTN_TOUCH, state != 0) < 0)
		return (-1);

	return (0);
}

/*
 * Translate and report trackpad finger count events
 */
static uint16_t fngmap[] = {
	BTN_TOOL_FINGER,
	BTN_TOOL_DOUBLETAP,
	BTN_TOOL_TRIPLETAP,
	BTN_TOOL_QUADTAP,
	BTN_TOOL_QUINTTAP,
};

int32_t
uinput_rep_fingers(int32_t fd, int32_t nfingers)
{
	assert(fd >= 0);

	for (int i = 0; i < 5; i++)
		if (uinput_write_event(fd, EV_KEY, fngmap[i], nfingers == i + 1) < 0)
			return (-1);

	return (0);
}

/*
 * Translate and report trackpad (clickpad) click events
 */
int32_t
uinput_rep_click(int32_t fd, int32_t clicked)
{
	assert(fd >= 0);

	if (uinput_write_event(fd, EV_KEY, BTN_LEFT, clicked) < 0)
		return (-1);

	return (0);
}



/*
 * Translate and report keyboard page key events
 */
int32_t
uinput_rep_key(int32_t fd, int32_t key, int32_t make)
{
	int32_t rcpt_mask;

	assert(fd >= 0);

	rcpt_mask = uinput_get_rcpt_mask();
	if (!(rcpt_mask & EVDEV_RCPT_HW_KBD))
		return (0);

	if (key >= 0 && key < (int32_t)nitems(keymap) &&
	    keymap[key] != NONE) {
		if (uinput_write_event(fd, EV_KEY, keymap[key], make) > 0 &&
		    uinput_write_event(fd, EV_SYN, SYN_REPORT, 0) > 0)
			return (0);
	}
	return (-1);
}

/*
 * Translate and report consumer page key events
 */
int32_t
uinput_rep_cons(int32_t fd, int32_t key, int32_t make)
{
	int32_t rcpt_mask;

	assert(fd >= 0);

	rcpt_mask = uinput_get_rcpt_mask();
	if (!(rcpt_mask & EVDEV_RCPT_HW_KBD))
		return (0);

	if (key >= 0 && key < (int32_t)nitems(consmap) &&
	    consmap[key] != NONE) {
		if (uinput_write_event(fd, EV_KEY, consmap[key], make) > 0 &&
		    uinput_write_event(fd, EV_SYN, SYN_REPORT, 0) > 0)
			return (0);
	}
	return (-1);
}

/*
 * Translate and report LED events
 */
int32_t
uinput_rep_leds(int32_t fd, int state, int mask)
{
	size_t i;
	int32_t rcpt_mask;

	assert(fd >= 0);

	rcpt_mask = uinput_get_rcpt_mask();
	if (!(rcpt_mask & EVDEV_RCPT_HW_KBD))
		return (0);

	for (i = 0; i < nitems(led_codes); i++) {
		if (mask & (1 << i) &&
		    uinput_write_event(fd, EV_LED, led_codes[i],
					state & (1 << i) ? 1 : 0) < 0)
			return (-1);
	}

	return (0);
}

/*
 * Process status change from evdev
 */
int32_t
uinput_kbd_status_changed(bthid_session_p s, uint8_t *data, int32_t len)
{
	struct input_event ie;
	int32_t leds, oleds;
	size_t i;

	assert(s != NULL);
	assert(s->vkbd >= 0);
	assert(len == sizeof(struct input_event));

	memcpy(&ie, data, sizeof(ie));
	switch (ie.type) {
	case EV_LED:
		ioctl(s->vkbd, KDGETLED, &oleds);
		leds = oleds;
		for (i = 0; i < nitems(led_codes); i++) {
			if (led_codes[i] == ie.code) {
				if (ie.value)
					leds |= 1 << i;
				else
					leds &= ~(1 << i);
				if (leds != oleds)
					ioctl(s->vkbd, KDSETLED, leds);
				break;
			}
		}
		break;
	case EV_REP:
		/* FALLTHROUGH. Repeats are handled by evdev subsystem */
	default:
		break;
	}

	return (0);
}
