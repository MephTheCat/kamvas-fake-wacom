#include <stdio.h>
#include <tablet_common.h>

const char *codes[] = {
	"Pen Hover",
	"Pen Lifted",
	"Pen Down",
	"Pen Not Present",
	"Key Down",
	"Key Up",
	
	"Unpopulated",
	"Unpopulated",
	"Unpopulated",
	"Unpopulated",
	"Unpopulated",
	"Unpopulated",
	"Unpopulated",
	"Unpopulated",
	"Unpopulated",
	"Unpopulated",
	"Unpopulated",
	"Unpopulated",
	"Unpopulated",
	"Unpopulated",
	"Unpopulated",
	"Unpopulated",
	"Unpopulated",
	"Unpopulated",
	"Unpopulated",
	"Unpopulated"
};

void print_event(tablet_event_t *event) {
	if (event->evt_class == TAB_EVT_PEN) {
		printf(
			"Pen Event (%s): C:(%ld, %ld) P:%u T:(%ld, %ld) B:",
			codes[event->evt_code],
			event->pen.coord.x,
			event->pen.coord.y,
			event->pen.pressure,
			event->pen.tilt.x,
			event->pen.tilt.y
		);
		
		for (int i = 7; i >= 0; i--) {
			printf("%c", (event->pen.buttons & (1 << i)) ? '1' : '0');
		}
		
		printf("\n");
	} else if (event->evt_class == TAB_EVT_KEY) {
		printf("Key Event (%s): K:", codes[event->evt_code]);
		
		for (int i = 31; i >= 0; i--) {
			printf("%c", (event->key.keys & (1 << i)) ? '1' : '0');
		}
		
		printf("\n");
	} else if (event->evt_class == TAB_EVT_ERROR) {
		printf("Error event\n");
	} else if (event->evt_class == TAB_EVT_ERROR) {
		printf("Ignore event\n");
	} else {
		printf("Unknown event\n");
	}
}
