#pragma once

#include <libevdev/libevdev.h>

struct relabsd_physical_device
{
   struct libevdev * libevdev;
   int file;
   int is_late;
};
