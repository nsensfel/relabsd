#pragma once

#include <sys/time.h>

#include <relabsd/device/axis_types.h>

enum relabsd_parameters_run_mode
{
   RELABSD_PARAMETERS_CLIENT_MODE,
   RELABSD_PARAMETERS_SERVER_MODE,
   RELABSD_PARAMETERS_COMPATIBILITY_TEST_MODE
};

struct relabsd_parameters
{
   int read_argc;
   enum relabsd_parameters_run_mode mode;
   int run_as_daemon;
   const char * communication_node_name;
   const char * device_name;
   const char * physical_device_file_name;
   const char * configuration_file;
   int use_timeout;
   struct timeval timeout;
   struct relabsd_axis axes[RELABSD_AXIS_VALID_AXES_COUNT];
   int device_name_was_modified;
};
