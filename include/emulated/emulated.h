#ifndef EMULATED_EMULATED_H
#define EMULATED_EMULATED_H

#include <libevdev/libevdev.h>
#include <libevdev/libevdev-uinput.h>
#include <linux/input-event-codes.h>

#include <tablet_common.h>

#define EMU_TABLET_MAX_KEYS 8

typedef struct {
	int codes[4];
} emulated_tablet_keystroke_t;

typedef enum {
	ROTATION_NORMAL,
	ROTATION_LEFT,
	ROTATION_RIGHT,
	ROTATION_INVERTED
} tablet_rotation_t;

typedef struct {
	const char           *name;
	struct input_absinfo  abs_xy[2];
	struct input_absinfo  abs_pressure;
	struct input_absinfo  abs_tilt[2];
	tablet_device_t      *tablet;
} emulated_tablet_config_t;

typedef struct {
	struct libevdev        *evdev;
	struct libevdev_uinput *uinput;
	
	emulated_tablet_config_t *config;
} emulated_tablet_t;


int emulated_tablet_initialize(emulated_tablet_t *emu, emulated_tablet_config_t *config);
int emulated_tablet_cleanup(emulated_tablet_t *emu);
int emulated_tablet_send(emulated_tablet_t *emu, tablet_event_t *event);

#endif
