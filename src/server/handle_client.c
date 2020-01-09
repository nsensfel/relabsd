/**** POSIX *******************************************************************/
/*
 * To get the POSIX 'getline' function.
 * We don't know what POSIX version is set by default.
 */
#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/**** RELABSD *****************************************************************/
#include <relabsd/debug.h>
#include <relabsd/server.h>

#include <relabsd/device/virtual_device.h>
#include <relabsd/device/axis.h>

#include <relabsd/config/parameters.h>

/******************************************************************************/
/**** LOCAL FUNCTIONS *********************************************************/
/******************************************************************************/

static void propagate_changes
(
   struct relabsd_server server [const static 1]
)
{
   struct relabsd_axis * axis;
   int i;
   int virtual_device_is_dirty;

   virtual_device_is_dirty = 0;

   for (i = 0; i < RELABSD_AXIS_VALID_AXES_COUNT; ++i)
   {
      axis =
         relabsd_parameters_get_axis
         (
            (enum relabsd_axis_name) i,
            &(server->parameters)
         );

      if (relabsd_axis_attributes_are_dirty(axis))
      {
         (void) relabsd_virtual_device_update_axis_absinfo
         (
            (enum relabsd_axis_name) i,
            axis,
            &(server->virtual_device)
         );

         relabsd_axis_set_attributes_are_dirty(0, axis);

         virtual_device_is_dirty = 1;
      }
   }

   if (relabsd_parameters_device_name_is_dirty(&(server->parameters)))
   {
      (void) relabsd_virtual_device_rename
      (
         &(server->parameters),
         &(server->virtual_device)
      );

      relabsd_parameters_clean_device_name(&(server->parameters));
      virtual_device_is_dirty = 1;
   }

   if (virtual_device_is_dirty)
   {
      (void) relabsd_virtual_device_recreate(&(server->virtual_device));
   }
}

/******************************************************************************/
/**** EXPORTED FUNCTIONS ******************************************************/
/******************************************************************************/
int relabsd_server_handle_client
(
   const int socket,
   struct relabsd_server server [const static 1]
)
{
   FILE * socket_as_file;

   errno = 0;
   socket_as_file = fdopen(socket, "r");

   if (socket_as_file == ((FILE *) NULL))
   {
      RELABSD_ERROR
      (
         "Unable to open client socket as a FILE: %s.",
         strerror(errno)
      );

      (void) close(socket);

      return -1;
   }

   pthread_mutex_lock(&(server->mutex));
   (void) relabsd_parameters_handle_remote_client
   (
      socket_as_file,
      &(server->parameters)
   );
   propagate_changes(server);
   pthread_mutex_unlock(&(server->mutex));

   /* This also closes 'socket' */
   (void) fclose(socket_as_file);

   return 0;
}
