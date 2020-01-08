/**** POSIX *******************************************************************/
/*
 * To get the POSIX 'getline' function.
 * We don't know what POSIX version is set by default.
 */
#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**** RELABSD *****************************************************************/
#include <relabsd/debug.h>
#include <relabsd/server.h>

#include <relabsd/util/string.h>

#include <relabsd/config/parameters.h>

/******************************************************************************/
/**** TYPES *******************************************************************/
/******************************************************************************/
struct relabsd_parameters_client_input
{
   FILE * socket_as_file;
   ssize_t size;
   char * buffer;
   size_t buffer_size;
};

/******************************************************************************/
/**** LOCAL FUNCTIONS *********************************************************/
/******************************************************************************/
static int get_next_argument
(
   struct relabsd_parameters_client_input input [const restrict static 1]
)
{
   errno = 0;
   input->size =
      getline
      (
         &(input->buffer),
         &(input->buffer_size),
         input->socket_as_file
      );

   if (input->size < 1)
   {
      RELABSD_ERROR
      (
         "Unable to read line from client socket: %s.",
         strerror(errno)
      );

      return -1;
   }

   input->buffer[(input->size - 1)] = '\0';

   return 0;
}

static void initialize_client_input
(
   FILE socket_as_file [const static 1],
   struct relabsd_parameters_client_input input [const restrict static 1]
)
{
   input->socket_as_file = socket_as_file;
   input->size = 0;
   input->buffer = (char *) NULL;
   input->buffer_size = 0;
}

static void finalize_client_input
(
   struct relabsd_parameters_client_input input [const restrict static 1]
)
{
   free((void *) input->buffer);

   input->buffer = (char *) NULL;
   input->buffer_size = 0;
}

static int handle_timeout_change
(
   struct relabsd_parameters_client_input input [const restrict static 1],
   struct relabsd_parameters parameters [const static 1]
)
{
   int timeout_msec;

   if (get_next_argument(input) < 0)
   {
      RELABSD_S_ERROR("Could not get timeout value from client.");

      return -1;
   }

   if (relabsd_util_parse_int(input->buffer, 0, INT_MAX, &timeout_msec) < 0)
   {
      RELABSD_S_ERROR("Invalid timeout value from client.");

      return -1;
   }

   relabsd_parameters_set_timeout(timeout_msec, parameters);

   return 0;
}

static int handle_inputs
(
   struct relabsd_parameters_client_input input [const restrict static 1],
   struct relabsd_parameters parameters [const static 1]
)
{
   for (;;)
   {
      if (get_next_argument(input) < 0)
      {
         RELABSD_S_ERROR("Could not get next client command.");

         return -1;
      }

      if ((input->buffer[0] == '\n') || (input->buffer[0] == '\0'))
      {
         return 0;
      }
      else if
      (
         RELABSD_STRING_EQUALS("-q", input->buffer)
         || RELABSD_STRING_EQUALS("--quit", input->buffer)
      )
      {
         relabsd_server_interrupt();
      }
      else if
      (
         RELABSD_STRING_EQUALS("-t", input->buffer)
         || RELABSD_STRING_EQUALS("--timeout", input->buffer)
      )
      {
         if (handle_timeout_change(input, parameters) < 0)
         {
            return 0;
         }
      }
      else if
      (
         RELABSD_STRING_EQUALS("-n", input->buffer)
         || RELABSD_STRING_EQUALS("--name", input->buffer)
      )
      {
         /* TODO: implement */
         RELABSD_PROG_ERROR("Unimplemented command \"%s\".", input->buffer);
      }
      else if
      (
         RELABSD_STRING_EQUALS("-m", input->buffer)
         || RELABSD_STRING_EQUALS("--mod-axis", input->buffer)
      )
      {
         /* TODO: implement */
         RELABSD_PROG_ERROR("Unimplemented command \"%s\".", input->buffer);
      }
      else
      {
         RELABSD_ERROR("Unknown client command \"%s\"", input->buffer);

         return -1;
      }
   }
}

/******************************************************************************/
/**** EXPORTED FUNCTIONS ******************************************************/
/******************************************************************************/
int relabsd_parameters_handle_remote_client
(
   FILE socket_as_file [const static 1],
   struct relabsd_parameters parameters [const restrict static 1]
)
{
   struct relabsd_parameters_client_input input;

   initialize_client_input(socket_as_file, &input);

   (void) handle_inputs(&input, parameters);

   finalize_client_input(&input);

   return 0;
}
