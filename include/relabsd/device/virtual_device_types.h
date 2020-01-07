#pragma once

#include <libevdev/libevdev.h>
#include <libevdev/libevdev-uinput.h>

struct relabsd_virtual_device
{
   int already_timed_out;
   struct libevdev * libevdev;
   struct libevdev_uinput * uinput_device;
};
