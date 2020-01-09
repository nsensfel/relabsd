/**** POSIXS ******************************************************************/
#include <stdlib.h>
#include <string.h>

/**** RELABSD *****************************************************************/
#include <relabsd/config/parameters.h>

#include <relabsd/device/axis.h>

/******************************************************************************/
/**** LOCAL FUNCTIONS *********************************************************/
/******************************************************************************/

/******************************************************************************/
/**** EXPORTED FUNCTIONS ******************************************************/
/******************************************************************************/
void relabsd_parameters_initialize_options
(
   struct relabsd_parameters parameters [const restrict static 1]
)
{
   int i;

   parameters->run_as_daemon = 0;
   parameters->communication_node_name = (const char *) NULL;
   parameters->device_name = (const char *) NULL;
   parameters->physical_device_file_name = (const char *) NULL;
   parameters->configuration_file = (const char *) NULL;
   parameters->device_name_was_modified = 0;

   for (i = 0; i < RELABSD_AXIS_VALID_AXES_COUNT; ++i)
   {
      relabsd_axis_initialize(parameters->axes + i);
   }
}

int relabsd_parameters_get_run_as_daemon
(
   const struct relabsd_parameters parameters [const restrict static 1]
)
{
   return parameters->run_as_daemon;
}

const char * relabsd_parameters_get_communication_node_name
(
   const struct relabsd_parameters parameters [const restrict static 1]
)
{
   return parameters->communication_node_name;
}

const char * relabsd_parameters_get_device_name
(
   const struct relabsd_parameters parameters [const restrict static 1]
)
{
   return parameters->device_name;
}

const char * relabsd_parameters_get_physical_device_file_name
(
   const struct relabsd_parameters parameters [const restrict static 1]
)
{
   return parameters->physical_device_file_name;
}

enum relabsd_parameters_run_mode relabsd_parameters_get_execution_mode
(
   const struct relabsd_parameters parameters [const restrict static 1]
)
{
   return parameters->mode;
}

struct relabsd_axis * relabsd_parameters_get_axis
(
   const enum relabsd_axis_name i,
   struct relabsd_parameters parameters [const restrict static 1]
)
{
   return (parameters->axes + i);
}

void relabsd_parameters_set_timeout
(
   const int timeout_msec,
   struct relabsd_parameters parameters [const restrict static 1]
)
{
   if (timeout_msec == 0)
   {
      parameters->use_timeout = 0;

      return;
   }

   parameters->use_timeout = 1;

   (void) memset((void *) &(parameters->timeout), 0, sizeof(struct timeval));

   /* FIXME: Not sure that's correct. */
   parameters->timeout.tv_sec = (time_t) (timeout_msec / 1000);
   parameters->timeout.tv_usec =
      (
         ((suseconds_t) timeout_msec)
         * ((suseconds_t) 1000)
      );

   return;
}

int relabsd_parameters_use_timeout
(
   const struct relabsd_parameters parameters [const restrict static 1]
)
{
   return parameters->use_timeout;
}

struct timeval relabsd_parameters_get_timeout
(
   const struct relabsd_parameters parameters [const restrict static 1]
)
{
   return parameters->timeout;
}

int relabsd_parameters_device_name_is_dirty
(
   const struct relabsd_parameters parameters [const restrict static 1]
)
{
   return parameters->device_name_was_modified;
}

void relabsd_parameters_clean_device_name
(
   struct relabsd_parameters parameters [const restrict static 1]
)
{
   if (parameters->device_name_was_modified)
   {
      free((void *) parameters->device_name);
      parameters->device_name = (const char *) NULL;
      parameters->device_name_was_modified = 0;
   }
}
