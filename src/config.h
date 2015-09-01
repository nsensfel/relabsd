#ifndef RELABSD_CONFIG_H
#define RELABSD_CONFIG_H

#include <libevdev/libevdev.h>

#include "axis.h"

struct relabsd_config_axis
{
   int min;
   int max;
   int fuzz;
   int flat;
   int resolution;
};

/*
 * There are no relabsd_config_free function so be careful about using dynamic
 * memory for input_file or device_name.
 */
struct relabsd_config
{
   char * input_file;
   char * device_name;
   struct relabsd_config_axis axis[6];
};

int relabsd_config_parse
(
   struct relabsd_config * const conf,
   int const argc,
   char ** const argv
);

int relabsd_config_allows
(
   const struct relabsd_config * const conf,
   enum relabsd_axis const axis,
   int const value
);

void relabsd_config_get_absinfo
(
   const struct relabsd_config * const conf,
   enum relabsd_axis const axis,
   struct input_absinfo * const absinfo
);

#endif
