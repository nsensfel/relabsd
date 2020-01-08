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

#include <relabsd/config/parameters.h>

/******************************************************************************/
/**** LOCAL FUNCTIONS *********************************************************/
/******************************************************************************/

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
   pthread_mutex_unlock(&(server->mutex));

   /* This also closes 'socket' */
   (void) fclose(socket_as_file);

   return 0;
}
