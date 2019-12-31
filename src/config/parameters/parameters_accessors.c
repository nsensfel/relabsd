#include <relabsd/config/parameters.h>

/******************************************************************************/
/**** LOCAL FUNCTIONS *********************************************************/
/******************************************************************************/

/******************************************************************************/
/**** EXPORTED FUNCTIONS ******************************************************/
/******************************************************************************/
void relabsd_parameters_initialize_options
(
   const struct relabsd_parameters parameters [const restrict static 1]
)
{
   parameters->run_as_daemon = 0;
   parameters->communication_node_name = (const char *) NULL;
   parameters->device_name = (const char *) NULL;
   parameters->physical_device_file_name = (const char *) NULL;
   parameters->configuration_file = (const char *) NULL;
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
