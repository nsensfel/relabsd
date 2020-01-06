/**** POSIX *******************************************************************/
#include <pthread.h>
#include <string.h>

/**** RELABSD *****************************************************************/
#include <relabsd/debug.h>
#include <relabsd/server.h>

/******************************************************************************/
/**** LOCAL FUNCTIONS *********************************************************/
/******************************************************************************/

static void main_loop (struct relabsd_server server [const static 1])
{
   int communication_socket, current_client_socket;
   int interrupt_fd, highest_fd;
   int ready_fds;
   fd_set ready_to_read;

   if
   (
      relabsd_server_create_communication_node
      (
         relabsd_parameters_get_communication_node_name(&(server->parameters)),
         &communication_socket
      )
      < 0
   )
   {
      relabsd_server_interrupt();

      return;
   }

   interrupt_fd = relabsd_server_get_interruption_file_descriptor();

   if (interrupt_fd > communication_socket)
   {
      highest_fd = (interrupt_fd + 1);
   }
   else
   {
      highest_fd = (communication_socket + 1);
   }

   for (;;)
   {
      FD_ZERO(&ready_to_read);
      FD_SET(communication_socket, &ready_to_read);
      FD_SET(interrupt_fd, &ready_to_read);

      ready_fds =
         select
         (
            highest_fd,
            &ready_to_read,
            (fd_set *) NULL,
            (fd_set *) NULL,
            (struct timeval *) NULL
         );

      /* TODO: select error handling. */

      if (!relabsd_server_keep_running())
      {
         return;
      }

      errno = 0;

      current_client_socket =
         accept
         (
            communication_socket,
            (struct sockaddr *) NULL,
            (socklen_t *) NULL
         );

      if (current_client_socket == -1)
      {
         RELABSD_ERROR
         (
            "Unable to accept on the server's socket: %s.",
            strerror(errno)
         );

         return -1;
      }

      (void) handle_client(current_client_socket, server);
   }
}

static void * posix_main_loop (void * params)
{
   main_loop((struct relabsd_server *) params);

   return NULL;
}

/******************************************************************************/
/**** EXPORTED FUNCTIONS ******************************************************/
/******************************************************************************/
int relabsd_server_create_communication_thread
(
   struct relabsd_server server [const static 1]
)
{
   int err;

   err =
      pthread_create
      (
         &(server->communication_thread),
         (const pthread_attr_t *) NULL,
         posix_main_loop,
         (void *) server
      );

   if (err != 0)
   {
      RELABSD_FATAL
      (
         "Unable to create the communication thread: %s",
         strerror(err)
      );

      return -1;
   }

   return 0;
}

int relabsd_server_join_communication_thread
(
   struct relabsd_server server [const static 1]
)
{
   int err;

   err = pthread_join(server->communication_thread, (void **) NULL);

   if (err != 0)
   {
      RELABSD_FATAL
      (
         "Unable to join with the communication thread: %s",
         strerror(err)
      );

      return -1;
   }

   return 0;
}
