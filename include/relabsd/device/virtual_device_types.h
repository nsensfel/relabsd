#pragma once

#include <libevdev/libevdev.h>
#include <libevdev/libevdev-uinput.h>

/*
   LIBEVDEV_UINPUT_OPEN_MANAGED is not defined on my machines.
   It is not my place to define it, so I'll avoid the issue by defining my own
   constant.
*/
#ifndef LIBEVDEV_UINPUT_OPEN_MANAGED
   #pragma message "[WARNING] libevdev did not define " \
      "LIBEVDEV_UINPUT_OPEN_MANAGED, using value '-2' instead."
   #define RELABSD_UINPUT_OPEN_MANAGED -2
#else
   #define RELABSD_UINPUT_OPEN_MANAGED LIBEVDEV_UINPUT_OPEN_MANAGED
#endif

struct relabsd_virtual_device
{
   struct libevdev * libevdev;
   struct libevdev_uinput * uinput_device;
};
