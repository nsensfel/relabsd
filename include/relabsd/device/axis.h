#pragma once

/**** LIBEVDEV ****************************************************************/
#include <libevdev/libevdev.h>

/**** RELABSD *****************************************************************/
#include <relabsd/device/axis_types.h>

/******************************************************************************/
/**** EXPORTED FUNCTIONS ******************************************************/
/******************************************************************************/
/*
 * Gives the relabsd_axis and EV_ABS event code equivalent to an EV_REL event
 * code.
 * If the returned relabsd_axis is RELABSD_UNKNOWN, no value is inserted into
 * 'abs_code'.
 */
enum relabsd_axis_name relabsd_axis_name_and_evdev_abs_from_evdev_rel
(
   const unsigned int rel_code,
   unsigned int abs_code [const restrict static 1]
);

/*
 * Returns the EV_REL/EV_ABS equivalent of 'e'.
 * There is no equivalent for RELABSD_UNKNOWN, so 'e' is forbidden from
 * taking this value.
 */
unsigned int relabsd_axis_name_to_evdev_rel (const enum relabsd_axis_name e);
unsigned int relabsd_axis_name_to_evdev_abs (const enum relabsd_axis_name e);

/*
 * Returns the relabsd_axis equivalent of a EV_REL/EV_ABS code.
 */
enum relabsd_axis_name relabsd_axis_name_from_evdev_rel
(
   const unsigned int rel
);

enum relabsd_axis_name relabsd_axis_name_from_evdev_abs
(
   const unsigned int abs
);

/*
 * Returns the relabsd_axis whose name is 'name', according to the configuration
 * file syntax.
 * RELABSD_UNKNOWN is returned for any name that didn't match any other
 * possibility.
 */
enum relabsd_axis_name relabsd_axis_parse_name
(
   const char name [const restrict static 1]
);

/* Same as above, but the string only has to start with the correct name. */
enum relabsd_axis_name relabsd_axis_parse_name_from_prefix
(
   const char name [const restrict static 1]
);

/*
 * Gives an string representation of an relabsd_axis.
 * "??" is returned for RELABSD_UNKNOWN.
 * Returned values should be coherent with the configuration file syntax.
 */
const char * relabsd_axis_name_to_string (const enum relabsd_axis_name e);

/*
 * Returns -1 if the option was discarded (an error has been reported),
 *         0 if the option was successfully parsed.
 */
int relabsd_axis_enable_option_from_name
(
   const char option_name [const restrict static 1],
   const char axis_name [const restrict static 1],
   struct relabsd_axis axis [const restrict static 1]
);

void relabsd_axis_enable (struct relabsd_axis axis [const restrict static 1]);
int relabsd_axis_is_enabled
(
   const struct relabsd_axis axis [const restrict static 1]
);

void relabsd_axis_to_absinfo
(
   const struct relabsd_axis axis [const restrict static 1],
   struct input_absinfo absinfo [const restrict static 1]
);

int relabsd_axis_filter_new_value
(
   struct relabsd_axis axis [const restrict static 1],
   int value [const restrict static 1]
);

void relabsd_axis_initialize
(
   struct relabsd_axis axis [const restrict static 1]
);

enum relabsd_axis_name relabsd_axis_get_convert_to
(
   const struct relabsd_axis axis [const restrict static 1]
);

int relabsd_axis_attributes_are_dirty
(
   const struct relabsd_axis axis [const restrict static 1]
);

void relabsd_axis_set_attributes_are_dirty
(
   const int val,
   struct relabsd_axis axis [const restrict static 1]
);
