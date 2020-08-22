#include <stdio.h>
#include <alloca.h>
#include <tablets/kamvas13.h>

#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))

int kamvas13_initialize(tablet_device_t *tablet, libusb_context *ctx);
int kamvas13_deinitialize(tablet_device_t *tablet);
int kamvas13_poll(tablet_device_t *tablet, tablet_event_t *event);

typedef struct {
	struct libusb_endpoint_descriptor pen_endpoint;
	
	uint8_t *xfer_buffer;
} kamvas_internal_t;

typedef struct {
	uint8_t report_id;
	uint8_t pen_buttons;
	
	uint8_t x_low;
	uint8_t x_middle;
	uint8_t y_low;
	uint8_t y_middle;
	
	uint8_t pressure_low;
	uint8_t pressure_high;
	
	uint8_t x_high;
	uint8_t y_high;
	
	int8_t  x_tilt;
	int8_t  y_tilt;
} __attribute__((packed)) kamvas_report_descriptor_t;

tablet_device_t kamvas_13 = {
	.evdev_name     = "Huion Kamvas 13",
	.reference_name = "kamvas13",
	.usb_handle     = NULL,
	.usb_ctx        = NULL,
	.usb_vid        = 0x256C,
	.usb_pid        = 0x006D,
	.usb_bus        = 0,
	.usb_address    = 0,
	.config         = NULL,
	.dev_desc       = {},
	
	//.resolution = (int_pair_t) { .x = 58752, .y = 33048 },
	.resolution = (int_pair_t) { .x = 0, .y = 0 },
	
	.pressure_max = 8191,
	.tilt_min     = (int_pair_t) { .x = -60, .y = -60 },
	.tilt_max     = (int_pair_t) { .x =  60, .y =  60 },
	
	.capabilities = CAP_PEN | CAP_STYLUS_BTN1 | CAP_STYLUS_BTN2 | CAP_STYLUS_PRESSURE | CAP_STYLUS_TILT,
	.keys         = 8,
	
	.poll_interval_us = 1000000/25,
	
	.initialize = &kamvas13_initialize,
	.cleanup    = &kamvas13_deinitialize,
	.poll       = &kamvas13_poll,
	
	.internal  = NULL,
	.user_data = NULL
};

int kamvas13_initialize(tablet_device_t *tablet, libusb_context *ctx) {
	tablet->usb_ctx = ctx;
	
	tablet->internal = calloc(1, sizeof(kamvas_internal_t));
	
	tablet->usb_handle = libusb_open_device_with_vid_pid(tablet->usb_ctx, tablet->usb_vid, tablet->usb_pid);
	
	if (!(tablet->usb_handle))
		return -1001;
	
	libusb_reset_device(tablet->usb_handle);
	
	libusb_get_device_descriptor(libusb_get_device(tablet->usb_handle), &(tablet->dev_desc));
	libusb_get_config_descriptor(libusb_get_device(tablet->usb_handle), 0, &(tablet->config));
	
	if (tablet->config->bNumInterfaces < 3)
		return -1002;
	
	int err;
	for (int iface = 0; iface < 3; iface++) {
		if (libusb_kernel_driver_active(tablet->usb_handle, iface))
			libusb_detach_kernel_driver(tablet->usb_handle, iface);
		
		err = libusb_claim_interface(tablet->usb_handle, iface);
		
		if (err < 0)
			return err;
	}
	
	tablet->usb_bus     = libusb_get_bus_number(libusb_get_device(tablet->usb_handle));
	tablet->usb_address = libusb_get_device_address(libusb_get_device(tablet->usb_handle));
	
	// Pull these descriptors so that the tablet responds properly
	uint8_t desc_code[] = { 0x0E, 0x0F, 0x64, 0x65, 0x6E, 0x79, 0x7A, 0x7B, 0xC8, 0xC9, 0xCA };
	uint8_t *temp       = (uint8_t *)malloc(256);
	for (size_t i = 0; i < sizeof(desc_code)/sizeof(uint8_t); i++) {
		int len = libusb_get_string_descriptor(tablet->usb_handle, desc_code[i], 0x0409, temp, 256);
		
		/*for (int i = 0; i < len; i++) {
			//printf("%02x%c", temp[i], ((i+1)==len) ? '\n' : ' ');
		}*/
		//printf("%.128ls\n", (wchar_t *)(temp));
		
		// This one is special, as it contains information about resolution and pressure
		if (desc_code[i] == 0xC8) {
			// Bytes 2, 3, 4 encode the Max X in 24 bits
			tablet->resolution.x = ((int)temp[4] << 16) | ((int)temp[3] << 8) | (int)temp[2];
			
			// Bytes 5, 6, 7 encode the Max Y in 24 bits
			tablet->resolution.y = ((int)temp[7] << 16) | ((int)temp[6] << 8) | (int)temp[5];
			
			// Bytes 8, 9 encode the Max Pressure in 16 bits
			tablet->pressure_max = ((int)temp[9] << 8) | (int)temp[8];
			
			/*for (int i = 0; i < len; i++) {
				printf("%02x%c", temp[i], ((i+1)==len) ? '\n' : ' ');
			}*/
		}
		
		if (len < 0 && len != LIBUSB_ERROR_PIPE) {
			printf("Get descriptor error: %d\n", err);
			//return err;
		}
	}
	
	free(temp);
	
	kamvas_internal_t *internal = (kamvas_internal_t *)(tablet->internal);
	internal->xfer_buffer  = (uint8_t *)calloc(1, 1024);
	internal->pen_endpoint = tablet->config->interface[0].altsetting[0].endpoint[0];
	
	return 0;
}

int kamvas13_deinitialize(tablet_device_t *tablet) {
	if (tablet->usb_handle) {
		libusb_release_interface(tablet->usb_handle, 0);
		libusb_release_interface(tablet->usb_handle, 1);
		libusb_release_interface(tablet->usb_handle, 2);
		
		libusb_close(tablet->usb_handle);
	}
	
	if (tablet->config)
		libusb_free_config_descriptor(tablet->config);
	
	if (tablet->internal)
		free(tablet->internal);
	
	return 0;
}

int kamvas13_poll(tablet_device_t *tablet, tablet_event_t *event) {
	kamvas_internal_t *internal = (kamvas_internal_t *)(tablet->internal);
	
	event->evt_class = TAB_EVT_ERROR;
	
	int xfer = 0;
	int err = libusb_interrupt_transfer(
		tablet->usb_handle,
		internal->pen_endpoint.bEndpointAddress,
		internal->xfer_buffer,
		MIN(internal->pen_endpoint.wMaxPacketSize, 256),
		&xfer,
		100
	);
	
	if (err == 0) {
		kamvas_report_descriptor_t *report = (kamvas_report_descriptor_t *)(internal->xfer_buffer);
		
		if (report->report_id == 8) {
			int type_code = report->pen_buttons & 0b11110000;
			
				
			if (type_code == 0x00) { // Pen lifted
				event->evt_class = TAB_EVT_PEN;
				event->evt_code  = TAB_EVT_PEN_LIFTED;
			} else if (type_code == 0x80) { // Pen events
				event->evt_class = TAB_EVT_PEN;
				event->evt_code = (report->pen_buttons & 0x01) ? TAB_EVT_PEN_DOWN : TAB_EVT_PEN_HOVER;
				
				event->pen.coord.x = (report->x_high << 16) | (report->x_middle << 8) | report->x_low;
				event->pen.coord.y = (report->y_high << 16) | (report->y_middle << 8) | report->y_low;
				
				event->pen.pressure = (report->pressure_high << 8) | report->pressure_low;
				
				event->pen.tilt.x = report->x_tilt;
				event->pen.tilt.y = report->y_tilt;
				
				event->pen.buttons = report->pen_buttons & 0b00000111;
			} else if (type_code == 0xe0) { // Face button events
				event->evt_class = TAB_EVT_KEY;
				
				// Pen Y coordinate encodes the button press information
				// In every face button press event, X = 257, don't know why
				if (report->x_low == 0x01 && report->x_middle == 0x01) {
					uint32_t btns = (report->y_high << 16) | (report->y_middle << 8) | report->y_low;
					
					event->evt_code = (btns == 0) ? TAB_EVT_KEY_UP : TAB_EVT_KEY_DOWN;
					
					event->key.keys = btns;
				} else {
					event->evt_class = TAB_EVT_IGNORE;
				}
			} else { // What
				event->evt_class = TAB_EVT_ERROR;
			}
			
			//print_event(event);
		}
		
		goto cleanup;
	} else if (err == LIBUSB_ERROR_TIMEOUT) {
		event->evt_class = TAB_EVT_PEN;
		event->evt_code  = TAB_EVT_PEN_NOT_PRESENT;
		
		goto cleanup;
	}

cleanup:
	return err;
}
