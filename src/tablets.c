#include <string.h>
#include <tablets.h>

tablet_device_t *known_devices[] = {
	&kamvas_13,
	NULL // Last element is a null to indicate the end of the array
};

tablet_device_t *find_tablet_by_reference(const char *ref) {
	for (tablet_device_t *tablet = *known_devices; tablet != NULL; tablet++) {
		if (strncmp(ref, tablet->reference_name, 32) == 0) {
			return tablet;
		}
	}
	
	return NULL;
}
