#ifndef TABLET_COMMON_H
#define TABLET_COMMON_H

#include <stdlib.h>
#include <libusb.h>

#define CAP_PEN             1
#define CAP_ERASER          2
#define CAP_STYLUS_BTN1     4
#define CAP_STYLUS_BTN2     8
#define CAP_STYLUS_PRESSURE 16
#define CAP_STYLUS_TILT     32

typedef enum {
	TAB_EVT_PEN,
	TAB_EVT_KEY,
	
	TAB_EVT_ERROR,
	TAB_EVT_IGNORE
} tablet_event_class_t;

typedef enum {
	TAB_EVT_PEN_HOVER = 0,
	TAB_EVT_PEN_LIFTED = 1,
	TAB_EVT_PEN_DOWN = 2,
	TAB_EVT_PEN_NOT_PRESENT = 3,
	
	TAB_EVT_KEY_DOWN = 4,
	TAB_EVT_KEY_UP = 5
} tablet_event_code_t;

typedef struct {
	long x, y;
} int_pair_t;

struct tablet_device_s;
typedef struct {
	tablet_event_class_t    evt_class;
	tablet_event_code_t     evt_code;
	struct tablet_device_s *tablet;
	
	union {
		struct {
			int_pair_t coord;
			
			uint32_t   pressure;
			int_pair_t tilt;
			uint8_t    buttons;
		} pen;
		
		struct {
			uint32_t keys;
		} key;
	};
} tablet_event_t;

typedef struct tablet_device_s {
	const char *evdev_name;
	const char *reference_name;
	
	libusb_device_handle            *usb_handle;
	libusb_context                  *usb_ctx;
	uint16_t                         usb_vid, usb_pid;
	uint8_t                          usb_bus, usb_address;
	struct libusb_config_descriptor *config;
	struct libusb_device_descriptor  dev_desc;
	
	int_pair_t resolution;
	
	uint16_t   pressure_max;
	int_pair_t tilt_min;
	int_pair_t tilt_max;
	
	uint32_t capabilities;
	uint32_t keys;
	
	uint32_t poll_interval_us;
	
	int (*initialize)(struct tablet_device_s *, libusb_context *);
	int (*cleanup)(struct tablet_device_s *);
	int (*poll)(struct tablet_device_s *, tablet_event_t *);
	
	void *internal;
	void *user_data;
} tablet_device_t;

void print_event(tablet_event_t *event);

#endif
