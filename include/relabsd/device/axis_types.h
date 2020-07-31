#pragma once

/* Number of axes that can be configured. */
#define RELABSD_AXIS_VALID_AXES_COUNT 8
#define RELABSD_AXIS_FLAGS_COUNT 4

/*
 * C enumerations are always int, and the standard does specify that it starts
 * at zero and increases from there, unless otherwise specified in the
 * declaration.
 */
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

enum relabsd_axis_flag
{
   RELABSD_DIRECT,
   RELABSD_REAL_FUZZ,
   RELABSD_FRAMED,
   RELABSD_NOT_ABS
};

struct relabsd_axis
{
   int min;
   int max;
   int fuzz;
   int flat;
   int resolution;

   int is_enabled;
   int previous_value;
   int flags[RELABSD_AXIS_FLAGS_COUNT];
   int attributes_were_modified;
   enum relabsd_axis_name convert_to;
};
