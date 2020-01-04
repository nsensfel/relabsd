/**** POSIX *******************************************************************/
#include <pthread.h>
#include <string.h>

/**** RELABSD *****************************************************************/
#include <relabsd/debug.h>
#include <relabsd/server.h>

/******************************************************************************/
/**** LOCAL FUNCTIONS *********************************************************/
/******************************************************************************/
/* TODO: implement. */
/*
void main_loop (struct relabsd_server server [const static 1])
{
}
*/

void * posix_main_loop (void * params)
{
   /* main_loop((struct relabsd_server *) params);*/
   params = NULL;

   return params;
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
