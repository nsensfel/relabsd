#pragma once

#include <libevdev/libevdev.h>

struct relabsd_physical_device
{
   struct libevdev * libevdev;
   int file;
   int timed_out;
};

/*
 * Returns -1 on (fatal) error,
 *         0  on success.
 *
 * 'device' does not need to be initialized, as the function will to that for
 * you (on success).
 * On success, 'input' will need to be closed.
 */
int relabsd_physical_device_open
(
   const char filename [const restrict static 1],
   struct relabsd_physical_device device [const restrict static 1]
);

void relabsd_physical_device_close
(
   const struct relabsd_physical_device device [const restrict static 1]
);

/*
 * Returns -1 on (warning) error,
 *         0 on successful read.
 *
 * The 'input_*' parameters do not need to be initialized, as the function will
 * do that for you (on success).
 * Requires 'input' to be open.
 */
int relabsd_physical_device_read
(
   const struct relabsd_physical_device device [const restrict static 1],
   unsigned int input_type [const restrict static 1],
   unsigned int input_code [const restrict static 1],
   int input_value [const restrict static 1]
);

int relabsd_physical_device_is_late
(
   const struct relabsd_physical_device device [const restrict static 1]
);