#ifndef RELABSD_CONFIG_H
#define RELABSD_CONFIG_H

#include <libevdev/libevdev.h>

#include "axis.h"

struct relabsd_config_axis
{
   int enabled;
   int previous_value;
   int min;
   int max;
   int fuzz;
   int flat;
   int resolution;
};

/*
 * There is no relabsd_config_free function, so be careful about using dynamic
 * memory for input_file or device_name.
 */
struct relabsd_config
{
   const char * input_file;
   const char * device_name;
   struct relabsd_config_axis axis[RELABSD_VALID_AXES_COUNT];
};

/*
 * Parses the invocation parameters and the config file into 'conf'.
 *
 * Returns -1 on (fatal) error,
 *          0 on successfully parsed config.
 *
 * 'conf' does not need to be initialized, as the function will only write to
 * it.
 * As one would expect, argc is the number of elements in argv.
 */
int relabsd_config_parse
(
   struct relabsd_config * const conf,
   int const argc,
   char * const * const argv
);

/*
 * This function aims at preventing us from emitting values that are incoherent
 * with our REV_ABS axis configuration, such as the axis' minimum or maximum
 * values.
 *
 * Returns 1 if 'conf' allows the value to be emitted,
 *         0 if 'conf' wants the event to be transmitted as is.
 *         -1 if 'conf' doesn't want the event to be transmitted.
 *
 * If the return value is 0, this function will not have altered the value at
 * 'value'. Otherwise, this function can have altered it to match its
 * requirements.
 * If the return value is either 0 or -1, the 'previous_value' of the axis
 * has been updated.
 */
int relabsd_config_filter
(
   struct relabsd_config * const conf,
   enum relabsd_axis const axis,
   int * const value
);

/*
 * Copies all the ABS event parameters of 'axis' into 'absinfo'.
 * 'absinfo' does not need to be initialized, as the function will only write to
 * it.
 */
void relabsd_config_get_absinfo
(
   const struct relabsd_config * const conf,
   enum relabsd_axis const axis,
   struct input_absinfo * const absinfo
);

#endif
