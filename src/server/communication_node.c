/**** POSIX *******************************************************************/
#include <sys/socket.h>
#include <sys/un.h>

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

/**** RELABSD *****************************************************************/
#include <relabsd/debug.h>

/******************************************************************************/
/**** LOCAL FUNCTIONS *********************************************************/
/******************************************************************************/

static int create_socket (int result [const restrict static 1])
{
   errno = 0;

   *result = socket(AF_UNIX, SOCK_STREAM, 0);

   if (*result == -1)
   {
      RELABSD_FATAL
      (
         "Unable to create server socket: %s.",
         strerror(errno)
      );

      return -1;
   }

   return 0;
}

static int bind_socket
(
   const char socket_name [const restrict static 1],
   const int socket
)
{
   struct sockaddr_un addr;


   memset(&addr, 0, sizeof(struct sockaddr_un));

   addr.sun_family = AF_UNIX;

   strncpy
   (
      (void *) addr.sun_path,
      (const void *) socket_name,
      (sizeof(addr.sun_path) - 1)
   );

   errno = 0;

   if
   (
      bind
      (
         socket,
         (const struct sockaddr *) &addr,
         (socklen_t) sizeof(struct sockaddr_un)
      )
      == -1
   )
   {
      RELABSD_FATAL
      (
         "Unable to bind communication socket to %s: %s.",
         socket_name,
         strerror(errno)
      );

      return -1;
   }

   return 0;
}

static int set_socket_to_unblocking (const int socket)
{
   int current_flags;

   errno = 0;
   current_flags = fcntl(socket, F_GETFD);

   if (current_flags == -1)
   {
      RELABSD_FATAL
      (
         "Unable to get communication socket properties: %s.",
         strerror(errno)
      );

      return -1;
   }

   errno = 0;
   current_flags = fcntl(socket, F_SETFD, (current_flags | O_NONBLOCK));

   if (current_flags == -1)
   {
      RELABSD_FATAL
      (
         "Unable to set communication socket properties: %s.",
         strerror(errno)
      );

      return -2;
   }

   return 0;
}

static int set_socket_as_listener (const int socket)
{
   errno = 0;

   if (listen(socket, 0) == -1)
   {
      RELABSD_FATAL
      (
         "Unable to set server socket properties: %s.",
         strerror(errno)
      );

      return -1;
   }

   return 0;
}

/******************************************************************************/
/**** EXPORTED FUNCTIONS ******************************************************/
/******************************************************************************/
int relabsd_server_create_communication_node
(
   const char socket_name [const restrict static 1],
   int socket [const restrict static 1]
)
{
   if (create_socket(socket) < 0)
   {
      return -1;
   }

   if (bind_socket(socket_name, *socket) < 0)
   {
      /* TODO: err message. */
      (void) close(*socket);

      return -1;
   }

   /*
   if (set_socket_to_unblocking(*socket) < 0)
   {
      /* TODO: err message. *//*
      (void) close(*socket);

      return -1;
   }
   */

   if (set_socket_as_listener(*socket) < 0)
   {
      /* TODO: err message. */
      (void) close(*socket);

      return -1;
   }

   return 0;
}
