#ifndef TABLETS_H
#define TABLETS_H

#include <libusb.h>

#include <tablets/kamvas13.h>

extern tablet_device_t *known_devices[];

tablet_device_t *find_tablet_by_reference(const char *ref);

#endif
