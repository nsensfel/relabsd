#ifndef RELABSD_AXIS_H
#define RELABSD_AXIS_H

/* Number of axes that can be configured. */
#define RELABSD_VALID_AXES_COUNT 8

enum relabsd_axis
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


/*
 * Gives the relabsd_axis and EV_ABS event code equivalent to an EV_REL event
 * code.
 * If the returned relabsd_axis is RELABSD_UNKNOWN, no value is inserted into
 * 'abs_code'.
 */
enum relabsd_axis relabsd_axis_convert_evdev_rel
(
   unsigned int const rel_code,
   unsigned int * const abs_code
);

/*
 * Returns the EV_REL/EV_ABS equivalent of 'e'.
 * There is no equivalent for RELABSD_UNKNOWN, so 'e' is forbidden from
 * taking this value.
 */
unsigned int relabsd_axis_to_rel (enum relabsd_axis const e);
unsigned int relabsd_axis_to_abs (enum relabsd_axis const e);

/*
 * Returns the relabsd_axis equivalent of a EV_REL/EV_ABS code.
 */
enum relabsd_axis relabsd_axis_from_rel (unsigned int const rel);
enum relabsd_axis relabsd_axis_from_abs (unsigned int const abs);

/*
 * Returns the relabsd_axis whose name is 'name', according to the configuration
 * file syntax.
 * RELABSD_UNKNOWN is returned for any name that didn't match any other
 * possibility.
 */
enum relabsd_axis relabsd_axis_from_name (const char * const name);

/*
 * Gives an string representation of an relabsd_axis.
 * "??" is returned for RELABSD_UNKNOWN.
 * Returned values should be coherent with the configuration file syntax.
 */
char * relabsd_axis_to_name (enum relabsd_axis const e);
#endif
