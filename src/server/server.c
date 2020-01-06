/**** POSIX *******************************************************************/
#include <string.h>

/**** RELABSD *****************************************************************/
#include <relabsd/config.h>
#include <relabsd/debug.h>
#include <relabsd/server.h>

#include <relabsd/config/parameters.h>

#include <relabsd/device/physical_device.h>
#include <relabsd/device/virtual_device.h>

/******************************************************************************/
/**** LOCAL FUNCTIONS *********************************************************/
/******************************************************************************/
static int initialize
(
   struct relabsd_server server [const restrict static 1]
)
{
   int err;

   if
   (
      relabsd_physical_device_open
      (
         relabsd_parameters_get_physical_device_file_name
         (
            &(server->parameters)
         ),
         &(server->physical_device)
      )
      < 0
   )
   {
      return -1;
   }

   if
   (
      relabsd_virtual_device_create_from
      (
         &(server->parameters),
         &(server->virtual_device)
      )
      < 0
   )
   {
      relabsd_physical_device_close(&(server->physical_device));

      return -2;
   }

   err =
      pthread_mutex_init(&(server->mutex), (const pthread_mutexattr_t *) NULL);

   if (err != 0)
   {
      RELABSD_FATAL
      (
         "Could not initialize the server's mutex: %s.",
         strerror(err)
      );
   }

   if
   (
      (
         relabsd_parameters_get_communication_node_name(&(server->parameters))
         != ((char *) NULL)
      )
      && (relabsd_server_create_communication_thread(server) < 0)
   )
   {
      relabsd_virtual_device_destroy(&(server->virtual_device));
      relabsd_physical_device_close(&(server->physical_device));

      return -4;
   }

   return 0;
}

static void finalize (struct relabsd_server server [const static 1])
{
   if
   (
      relabsd_parameters_get_communication_node_name(&(server->parameters))
      != ((char *) NULL)
   )
   {
      relabsd_server_join_communication_thread(server);
   }

   relabsd_virtual_device_destroy(&(server->virtual_device));
   relabsd_physical_device_close(&(server->physical_device));

   (void) pthread_mutex_destroy(&(server->mutex));
}

/******************************************************************************/
/**** EXPORTED FUNCTIONS ******************************************************/
/******************************************************************************/
int relabsd_server_main
(
   const int argc,
   const char * const argv [const static argc],
   struct relabsd_parameters parameters [const restrict static 1]
)
{
   struct relabsd_server server;

   server.parameters = *parameters;

   RELABSD_S_DEBUG(RELABSD_DEBUG_PROGRAM_FLOW, "Started server mode.");

   if (relabsd_parameters_parse_options(argc, argv, &(server.parameters)) < 0)
   {
      return -1;
   }

   if
   (
      relabsd_parameters_get_run_as_daemon(&(server.parameters))
      && (relabsd_server_create_daemon() < 0)
   )
   {
      return -2;
   }

   (void) initialize(&server);

   (void) relabsd_server_conversion_loop(&server);

   finalize(&server);

   RELABSD_S_DEBUG(RELABSD_DEBUG_PROGRAM_FLOW, "Completed server mode.");

   return 0;
}
