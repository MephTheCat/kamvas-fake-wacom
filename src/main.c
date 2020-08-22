#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <time.h>

#include <tablets.h>
#include <emulated/emulated.h>
#include <config/config.h>

int         running        = 1;

emulated_tablet_config_t emu_config = {
	.abs_xy = {
		(struct input_absinfo) {},
		(struct input_absinfo) {},
	},
	.abs_pressure = {},
	.abs_tilt = {},
	
	.name   = "Huion Kamvas 13",
	.tablet = NULL
};

void catch_sigint(int sig) {
	(void)(sig);
	
	running = 0;
}

int main_setup(tablet_device_t **huion, emulated_tablet_t *emu, libusb_context *ctx, const char *tablet_ref) {
	int err = 0;
	
	*huion = find_tablet_by_reference(tablet_ref);
	
	if (!(*huion)) {
		printf("No known tablet with reference named '%s'\n", tablet_ref);
		return -1;
	}
	
	emu->evdev = NULL;
	
	err = libusb_init(&ctx);
	
	if (err < 0) {
		printf("LibUSB init error %d\n", err);
		return -1;
	}
	
	if ((*huion)->initialize(*huion, ctx)) {
		printf("Could not initialize tablet\n");
		return -1;
	}
	
	printf("Tablet initialized at bus %d, address %d\n", (*huion)->usb_bus, (*huion)->usb_address);
	
	emu_config.tablet = *huion;
	if (emulated_tablet_initialize(emu, &emu_config)) {
		printf("Could not initialize emulated tablet\n");
		return -1;
	}
	
	return 0;
}

int main_loop(tablet_device_t *huion, emulated_tablet_t *emu) {
	int            err   = 0;
	tablet_event_t event;
	
	running = 1;
	while (running) {
		// Not having this usleep doesn't abuse the CPU it seems,
		// but I'd rather sleep for something, just in case it
		// starts swamping the CPU
		usleep(100);
		
		err = huion->poll(huion, &event);
		
		if (event.evt_class == TAB_EVT_ERROR) {
			if (err != LIBUSB_ERROR_NO_DEVICE)
				printf("LibUSB error %d\n", err);
			running = 0;
		} else {
			emulated_tablet_send(emu, &event);
		}
	}
	
	return err;
}

int main_cleanup(tablet_device_t *huion, emulated_tablet_t *emu, libusb_context *ctx) {
	if (huion)
		huion->cleanup(huion);
	
	if (emu->evdev)
		emulated_tablet_cleanup(emu);
	
	if (ctx)
		libusb_exit(ctx);
	
	return 0;
}

int main(void) {
	tablet_device_t   *tablet  = NULL;
	libusb_context    *ctx     = NULL;
	emulated_tablet_t  emu     = {};
	int                err     = 0;
	int                ret_val = EXIT_SUCCESS;
	const char        *search  = "kamvas13";
	
	if (getuid() != 0) {
		printf("The driver must be run as root.\n");
		return EXIT_FAILURE;
	}
	
	signal(SIGINT, catch_sigint);
	
	while (running) {
		err = main_setup(&tablet, &emu, ctx, search);
		//printf("Setup error: %d\n", err);
		
		if (err == 0) {
			printf("Ready\n");
			
			err = main_loop(tablet, &emu);
			
			// If it's gotten this far and gets this error,
			// then the device has been unplugged.
			if (err == LIBUSB_ERROR_NO_DEVICE) {
				printf("Tablet unplugged\n");
				running = 1;
			}
			
			err = main_cleanup(tablet, &emu, ctx);
		} else {
			if (err == LIBUSB_ERROR_IO) {
				printf("Tablet not found\n");
				sleep(5);
			} else {
				printf("Setup error\n");
				running = 0;
			}
		}
	}
	
	return ret_val;
}
