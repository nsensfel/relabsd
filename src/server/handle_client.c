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

/******************************************************************************/
/**** LOCAL FUNCTIONS *********************************************************/
/******************************************************************************/
static void handle_input
(
   const ssize_t input_size __attribute__((unused)),
   const char input [const static 1] __attribute__((unused)),
   struct relabsd_server server [const static 1] __attribute__((unused))
)
{
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
   /* FIXME: reallocating at every new connection is kind of wasteful. */
   char * input;
   ssize_t input_size;

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

   errno = 0;

   input_size = getline(&input, (size_t *) NULL, socket_as_file);

   if (input_size < 1)
   {
      RELABSD_ERROR
      (
         "Unable to read line from client socket: %s.",
         strerror(errno)
      );

      (void) free((void *) input);

      /* This also closes 'socket' */
      (void) fclose(socket_as_file);

      return -1;
   }

   handle_input(input_size, input, server);

   (void) free((void *) input);

   /* This also closes 'socket' */
   (void) fclose(socket_as_file);

   return 0;
}
