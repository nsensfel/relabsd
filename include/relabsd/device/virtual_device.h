#pragma once

#include <relabsd/config/parameters.h>

#include <relabsd/device/virtual_device_types.h>

/*
 * - Clones the (real) input device.
 * - Adds ABS event support to the clone.
 * - Adds the ABS axis described in 'parameters' to the clone (overwriting if
 *   needed).
 * - Disables the associated REL events from the clone.
 *
 * Returns -1 on (fatal) error,
 *         0  on success.
 *
 * 'device' does not need to be initialized, as the function will to that for
 * you (on success).
 * On success, 'device' will need to be closed.
 * This opens the (real) input device in read only mode to copy its properties.
 */
int relabsd_virtual_device_create_from
(
   struct relabsd_parameters parameters [const restrict static 1],
   struct relabsd_virtual_device device [const restrict static 1]
);

void relabsd_virtual_device_destroy
(
   const struct relabsd_virtual_device device [const restrict static 1]
);

/*
 * Write an event to 'device'. At this time, every event written to 'device' is
 * followed by an EV_SYN event.
 *
 * Returns 0 if both the event and the EV_SYN were successful,
 *         -1 if either failed.
 */
int relabsd_virtual_device_write_evdev_event
(
   const struct relabsd_virtual_device device [const restrict static 1],
   unsigned int const type,
   unsigned int const code,
   int const value
);

/*
 * Send an event for each enabled axis, setting it to zero.
 * An EV_SYN event is sent afterwards.
 */
void relabsd_virtual_device_set_axes_to_zero
(
   struct relabsd_parameters parameters [const restrict static 1],
   const struct relabsd_virtual_device device [const restrict static 1]
);

void relabsd_virtual_device_set_has_already_timed_out
(
   const int val,
   struct relabsd_virtual_device device [const restrict static 1]
);

int relabsd_virtual_device_has_already_timed_out
(
   const struct relabsd_virtual_device device [const restrict static 1]
);

int relabsd_virtual_device_update_axis_absinfo
(
   const enum relabsd_axis_name axis_name,
   const struct relabsd_axis axis [const restrict static 1],
   const struct relabsd_virtual_device device [const restrict static 1]
);

int relabsd_virtual_device_rename
(
   const struct relabsd_parameters parameters [const restrict static 1],
   const struct relabsd_virtual_device device [const restrict static 1]
);

int relabsd_virtual_device_recreate
(
   struct relabsd_virtual_device device [const restrict static 1]
);
