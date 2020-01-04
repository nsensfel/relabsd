/**** POSIX *******************************************************************/
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/un.h>

/**** RELABSD *****************************************************************/
#include <relabsd/client.h>
#include <relabsd/config.h>
#include <relabsd/debug.h>

#include <relabsd/config/parameters.h>

/******************************************************************************/
/**** LOCAL FUNCTIONS *********************************************************/
/******************************************************************************/
static int open_socket
(
   const char socket_name [const restrict static 1],
   FILE * s [const restrict static 1]
)
{
   const int old_errno = errno;
   int fd;
   struct sockaddr_un addr;

   RELABSD_S_DEBUG(RELABSD_DEBUG_PROGRAM_FLOW, "Opening socket to server...");

   errno = 0;

   if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
   {
      RELABSD_FATAL
      (
         "Unable to create socket: %s.",
         strerror(errno)
      );

      errno = old_errno;

      return -1;
   }

   errno = old_errno;

   memset(&addr, 0, sizeof(addr));
   addr.sun_family = AF_UNIX;
   strncpy(addr.sun_path, socket_name, (sizeof(addr.sun_path) - 1));

   errno = 0;

   if (connect(fd, ((struct sockaddr*) &addr), sizeof(addr)) == -1)
   {
      RELABSD_FATAL
      (
         "Unable to connect to address: %s.",
         strerror(errno)
      );

      errno = old_errno;

      close(fd);

      return -1;
   }

   errno = 0;

   *s = fdopen(fd, "w+");

   if (*s == ((FILE *) NULL))
   {
      RELABSD_FATAL
      (
         "Unable to open socket as a file: %s.",
         strerror(errno)
      );

      errno = old_errno;

      close(fd);

      return -1;
   }

   errno = old_errno;

   RELABSD_S_DEBUG(RELABSD_DEBUG_PROGRAM_FLOW, "Opened socket to server.");

   return 0;
}

static int send_commands
(
   const int argc,
   const char * const argv [const restrict static argc],
   FILE socket [const restrict static 1]
)
{
   int i, j;

   RELABSD_S_DEBUG(RELABSD_DEBUG_PROGRAM_FLOW, "Sending commands to server...");

   for (i = 3; i < argc;)
   {

      if (fputs(argv[i], socket) == EOF)
      {
         // TODO: error
      }

      if (relabsd_parameters_argument_count_for(argv[i], &j) < 0)
      {
         RELABSD_FATAL("Unknown option '%s'.", argv[i]);
         relabsd_parameters_print_usage(argv[0]);

         return -1;
      }

      for
      (
         i++;
         ((j > 0) && (i < argc));
         j++, i--
      )
      {
         if (fputc(' ', socket) == EOF)
         {
            // TODO: error
         }
         if (fputs(argv[i], socket) == EOF)
         {
            // TODO: error
         }
      }

      if (fputc('\0', socket) == EOF)
      {
         // TODO: error
      }
   }

   if (fputc('\0', socket) == EOF)
   {
   }

   RELABSD_S_DEBUG(RELABSD_DEBUG_PROGRAM_FLOW, "Sent commands to server.");

   return 0;
}

static int receive_reply
(
   FILE socket [const restrict static 1]
)
{
   /* TODO: implement. */
   (void) fclose(socket);

   return 0;
}

/******************************************************************************/
/**** EXPORTED FUNCTIONS ******************************************************/
/******************************************************************************/
int relabsd_client_main
(
   const int argc,
   const char * const argv [const restrict static argc],
   struct relabsd_parameters parameters [const restrict static 1]
)
{
   FILE * socket;

   RELABSD_S_DEBUG(RELABSD_DEBUG_PROGRAM_FLOW, "Started client mode.");

   if
   (
      open_socket
      (
         relabsd_parameters_get_communication_node_name(parameters),
         &socket
      )
      < 0
   )
   {
      return -1;
   }

   if (send_commands(argc, argv, socket) < 0)
   {
      return -2;
   }

   if (receive_reply(socket) < 0)
   {
      return -3;
   }

   RELABSD_S_DEBUG(RELABSD_DEBUG_PROGRAM_FLOW, "Completed client mode.");

   return 0;
}
