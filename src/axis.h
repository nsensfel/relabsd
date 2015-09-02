#ifndef RELABSD_AXIS_H
#define RELABSD_AXIS_H

enum relabsd_axis
{
   RELABSD_X,
   RELABSD_Y,
   RELABSD_Z,
   RELABSD_RX,
   RELABSD_RY,
   RELABSD_RZ,
   RELABSD_UNKNOWN
};

/*
 * Returns the relabsd_axis whose name is 'name', according to the configuration
 * file syntax.
 * RELABSD_UNKNOWN is returned for any name that didn't match any other
 * possibility.
 **/
enum relabsd_axis relabsd_axis_name_to_enum (const char * const name);

/*
 * Gives an string representation of an relabsd_axis.
 * "??" is returned for RELABSD_UNKNOWN.
 * Returned values should be coherent with the configuration file syntax.
 **/
char * relabsd_axis_enum_to_name (enum relabsd_axis const e);

/*
 * Gives the relabsd_axis and EV_ABS event code equivalent to an EV_REL event
 * code.
 * If the returned relabsd_axis is RELABSD_UNKNOWN, no value is inserted into
 * 'abs_code'.
 **/
enum relabsd_axis relabsd_axis_convert_evdev_rel
(
   unsigned int const rel_code,
   unsigned int * const abs_code
);

#endif
