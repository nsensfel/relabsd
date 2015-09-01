#ifndef RELABSD_RELABSD_DEVICE_H
#define RELABSD_RELABSD_DEVICE_H

#include <libevdev/libevdev.h>

#include "config.h"
#include "input.h"

struct relabsd_device
{
   struct libevdev * dev;
   struct libevdev_uinput * uidev;
};

int relabsd_device_create
(
   struct relabsd_device * const dev,
   const struct relabsd_config * const config
);

void relabsd_device_destroy (const struct relabsd_device * const dev);

int relabsd_device_write_evdev_event
(
   const struct relabsd_device * const dev,
   unsigned int const type,
   unsigned int const code,
   int const value
);

#endif
