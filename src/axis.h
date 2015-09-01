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

enum relabsd_axis relabsd_axis_name_to_enum (char * const name);
char * relabsd_axis_enum_to_name (enum relabsd_axis const e);
enum relabsd_axis relabsd_axis_convert_evdev_rel
(
   unsigned int const rel_code,
   unsigned int * const abs_code
);

#endif
