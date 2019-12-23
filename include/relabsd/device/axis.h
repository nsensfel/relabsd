#pragma once

/* Number of axes that can be configured. */
#define RELABSD_AXIS_AXES_COUNT 8

enum relabsd_axis_name
{
   RELABSD_X,
   RELABSD_Y,
   RELABSD_Z,
   RELABSD_RX,
   RELABSD_RY,
   RELABSD_RZ,
   RELABSD_WHEEL,
   RELABSD_MISC,
   RELABSD_UNKNOWN
};

struct relabsd_axis
{
   int min;
   int max;
   int fuzz;
   int flat;
   int resolution;
   int flags;
};

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

/*
 * Gives an string representation of an relabsd_axis.
 * "??" is returned for RELABSD_UNKNOWN.
 * Returned values should be coherent with the configuration file syntax.
 */
const char * relabsd_axis_name_to_string (const enum relabsd_axis_name e);
