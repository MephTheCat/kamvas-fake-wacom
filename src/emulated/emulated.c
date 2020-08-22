#include <stdio.h>
#include <emulated/emulated.h>

int emulated_tablet_initialize(emulated_tablet_t *emu, emulated_tablet_config_t *config) {
	if (!emu) {
		emu->evdev = NULL;
		return -1001;
	}
	
	if (!config) {
		emu->evdev = NULL;
		return -1001;
	}
	
	if (!config->tablet) {
		emu->evdev = NULL;
		return -1001;
	}
	
	emu->config = config;
	
	emu->evdev = libevdev_new();
	
	libevdev_set_name(emu->evdev, emu->config->name);
	libevdev_enable_property(emu->evdev, INPUT_PROP_DIRECT);
	libevdev_enable_property(emu->evdev, INPUT_PROP_POINTER);
	
	libevdev_set_id_bustype(emu->evdev, BUS_USB);
	libevdev_set_id_vendor(emu->evdev, 0x056A);
	libevdev_set_id_product(emu->evdev, 0xB2);
	libevdev_set_id_version(emu->evdev, 0x100);
	
	
	
	// Setup SYN events
	libevdev_enable_event_type(emu->evdev, EV_SYN);
	libevdev_enable_event_code(emu->evdev, EV_SYN, SYN_REPORT, NULL);
	
	/*
	Event code 272 (BTN_LEFT)
    Event code 273 (BTN_RIGHT)
    Event code 274 (BTN_MIDDLE)
    Event code 275 (BTN_SIDE)
    Event code 276 (BTN_EXTRA)
    Event code 320 (BTN_TOOL_PEN)
    Event code 321 (BTN_TOOL_RUBBER)
    Event code 322 (BTN_TOOL_BRUSH)
    Event code 323 (BTN_TOOL_PENCIL)
    Event code 324 (BTN_TOOL_AIRBRUSH)
    Event code 326 (BTN_TOOL_MOUSE)
    Event code 327 (BTN_TOOL_LENS)
    Event code 330 (BTN_TOUCH)
    Event code 331 (BTN_STYLUS)
    Event code 332 (BTN_STYLUS2)*/
	
	// Setup KEY events
	libevdev_enable_event_type(emu->evdev, EV_KEY);
	libevdev_enable_event_code(emu->evdev, EV_KEY, BTN_LEFT, NULL);
	libevdev_enable_event_code(emu->evdev, EV_KEY, BTN_RIGHT, NULL);
	libevdev_enable_event_code(emu->evdev, EV_KEY, BTN_MIDDLE, NULL);
	libevdev_enable_event_code(emu->evdev, EV_KEY, BTN_SIDE, NULL);
	libevdev_enable_event_code(emu->evdev, EV_KEY, BTN_EXTRA, NULL);
	libevdev_enable_event_code(emu->evdev, EV_KEY, BTN_TOOL_PEN, NULL);
	libevdev_enable_event_code(emu->evdev, EV_KEY, BTN_TOOL_RUBBER, NULL);
	libevdev_enable_event_code(emu->evdev, EV_KEY, BTN_TOOL_BRUSH, NULL);
	libevdev_enable_event_code(emu->evdev, EV_KEY, BTN_TOOL_PENCIL, NULL);
	libevdev_enable_event_code(emu->evdev, EV_KEY, BTN_TOOL_AIRBRUSH, NULL);
	libevdev_enable_event_code(emu->evdev, EV_KEY, BTN_TOOL_MOUSE, NULL);
	libevdev_enable_event_code(emu->evdev, EV_KEY, BTN_TOOL_LENS, NULL);
	libevdev_enable_event_code(emu->evdev, EV_KEY, BTN_TOUCH, NULL);
	libevdev_enable_event_code(emu->evdev, EV_KEY, BTN_STYLUS, NULL);
	libevdev_enable_event_code(emu->evdev, EV_KEY, BTN_STYLUS2, NULL);
	
	for (int i = 0; i < 10; i++)
		libevdev_enable_event_code(emu->evdev, EV_KEY, BTN_0 + i, NULL);
	
	
	
	// Setup ABS events
	libevdev_enable_event_type(emu->evdev, EV_ABS);
	
	emu->config->abs_xy[0].minimum = 0;
	emu->config->abs_xy[0].maximum = emu->config->tablet->resolution.x;
	
	emu->config->abs_xy[1].minimum = 0;
	emu->config->abs_xy[1].maximum = emu->config->tablet->resolution.y;
	
	emu->config->abs_pressure.minimum = 0;
	emu->config->abs_pressure.maximum = config->tablet->pressure_max;
	
	emu->config->abs_tilt[0].minimum = config->tablet->tilt_min.x;
	emu->config->abs_tilt[0].maximum = config->tablet->tilt_max.x;
	
	emu->config->abs_tilt[1].minimum = config->tablet->tilt_min.y;
	emu->config->abs_tilt[1].maximum = config->tablet->tilt_max.y;
	
	libevdev_enable_event_code(emu->evdev, EV_ABS, ABS_X, &(emu->config->abs_xy[0]));
	libevdev_enable_event_code(emu->evdev, EV_ABS, ABS_Y, &(emu->config->abs_xy[1]));
	libevdev_enable_event_code(emu->evdev, EV_ABS, ABS_PRESSURE, &(emu->config->abs_pressure));
	libevdev_enable_event_code(emu->evdev, EV_ABS, ABS_TILT_X, &(emu->config->abs_tilt[0]));
	libevdev_enable_event_code(emu->evdev, EV_ABS, ABS_TILT_Y, &(emu->config->abs_tilt[1]));
	
	int err = libevdev_uinput_create_from_device(emu->evdev, LIBEVDEV_UINPUT_OPEN_MANAGED, &(emu->uinput));
	
	if (err != 0)
		return err;
	
	return 0;
}

int emulated_tablet_send(emulated_tablet_t *emu, tablet_event_t *event) {
	int err = 0;
	if (event->evt_class == TAB_EVT_PEN) {
		if (event->evt_code == TAB_EVT_PEN_HOVER || event->evt_code == TAB_EVT_PEN_DOWN) {
			libevdev_uinput_write_event(emu->uinput, EV_KEY, BTN_TOOL_PEN, 1);
			libevdev_uinput_write_event(emu->uinput, EV_KEY, BTN_TOUCH, (event->pen.buttons & 0x01) ? 1 : 0);
			libevdev_uinput_write_event(emu->uinput, EV_KEY, BTN_STYLUS, (event->pen.buttons & 0x02) ? 1 : 0);
			libevdev_uinput_write_event(emu->uinput, EV_KEY, BTN_STYLUS2, (event->pen.buttons & 0x04) ? 1 : 0);
			
			libevdev_uinput_write_event(emu->uinput, EV_ABS, ABS_X, event->pen.coord.x);
			libevdev_uinput_write_event(emu->uinput, EV_ABS, ABS_Y, event->pen.coord.y);
			libevdev_uinput_write_event(emu->uinput, EV_ABS, ABS_PRESSURE, event->pen.pressure);
			libevdev_uinput_write_event(emu->uinput, EV_ABS, ABS_TILT_X, event->pen.tilt.x);
			libevdev_uinput_write_event(emu->uinput, EV_ABS, ABS_TILT_Y, event->pen.tilt.y);
			
			libevdev_uinput_write_event(emu->uinput, EV_SYN, SYN_REPORT, 0);
		} else if (event->evt_code == TAB_EVT_PEN_LIFTED) {
			libevdev_uinput_write_event(emu->uinput, EV_KEY, BTN_TOOL_PEN, 0);
			libevdev_uinput_write_event(emu->uinput, EV_SYN, SYN_REPORT, 0);
		} else if (event->evt_code == TAB_EVT_PEN_NOT_PRESENT) {
			
		}
	} else if (event->evt_class == TAB_EVT_KEY) {
		for (int key = 0; key < EMU_TABLET_MAX_KEYS; key++) {
			if (key >= 10)
				break;
			
			if (event->evt_code == TAB_EVT_KEY_DOWN) {
				if (!(event->key.keys & (1 << key)))
					continue;
				
				libevdev_uinput_write_event(emu->uinput, EV_KEY, BTN_0 + key, 1);
				
				libevdev_uinput_write_event(emu->uinput, EV_SYN, SYN_REPORT, 0);
			} else if (event->evt_code == TAB_EVT_KEY_UP) {
				if ((event->key.keys & (1 << key)))
					continue;
				
				libevdev_uinput_write_event(emu->uinput, EV_KEY, BTN_0 + key, 0);
				
				libevdev_uinput_write_event(emu->uinput, EV_SYN, SYN_REPORT, 0);
			}
		}
	}
	
	return err;
}

int emulated_tablet_cleanup(emulated_tablet_t *emu) {
	if (emu->uinput)
		libevdev_uinput_destroy(emu->uinput);
	
	if (emu->evdev)
		libevdev_free(emu->evdev);
	
	return 0;
}
