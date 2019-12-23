/**** POSIX *******************************************************************/
#include <sys/types.h>

#include <unistd.h>

/**** RELABSD *****************************************************************/
#include <relabsd/config.h>
#include <relabsd/debug.h>
#include <relabsd/server.h>

#include <relabsd/config/parameters.h>

/******************************************************************************/
/**** LOCAL FUNCTIONS *********************************************************/
/******************************************************************************/

/******************************************************************************/
/**** EXPORTED FUNCTIONS ******************************************************/
/******************************************************************************/
/*
 * Daemon creation function made using the instructions for old-school daemons
 * at https://www.freedesktop.org/software/systemd/man/daemon.html
 * Only meant to be used before things really start. This won't properly create
 * a daemon on an already running server instance with a virtual device and so
 * on...
 */
int relabsd_server_create_daemon (void)
{
   pid_t proc_id;

   /* 1/ Close all open file descriptors ... **********************************/
   /* None were opened at this point. */

   /* 2/ Reset all signal handlers ... ****************************************/
   /* Those were not modified at this point. */

   /* 3/ Reset the signal mask using sigprocmask() ****************************/
   /* Not modified at this point. */

   /* 4/ Sanitize the environment block ... ***********************************/
   /* What? */

   /* 5/ fork() ***************************************************************/
   errno = 0;
   proc_id = fork();

   if (proc_id == ((pid_t) -1))
   {
      RELABSD_FATAL
      (
         "Step 5 of the daemon creation process, fork(), failed: %s.",
         strerror(errno)
      );

      return -1;
   }

   if (proc_id != ((pid_t) 0))
   {
      /* Awaiting step 14...*/
      /* TODO: insert unnamed pipe */
      /* 15/ Original process exits *******************************************/
      exit();
   }

   /* 6/ setsid() *************************************************************/
   errno = 0;

   proc_id = setsid();

   if (proc_id == ((pid_t) -1))
   {
      RELABSD_FATAL
      (
         "Step 6 of the daemon creation process, setsid(), failed: %s.",
         strerror(errno)
      );

      return -1;
   }

   /* 7/ fork() again *********************************************************/
   errno = 0;
   proc_id = fork();

   if (proc_id == ((pid_t) -1))
   {
      RELABSD_FATAL
      (
         "Step 5 of the daemon creation process, fork(), failed: %s.",
         strerror(errno)
      );

      return -1;
   }

   if (proc_id != ((pid_t) 0))
   {
      /* 8/ First child process exits *****************************************/
      exit();
   }

   /* 9/ /dev/null for standard input/outputs *********************************/

   /* 10/ reset umask to 0 ****************************************************/

   /* 11/ Set current directory to / ******************************************/
   errno = 0;

   if (chdir("/") == -1)
   {
      // Can't print an error message at that point though...
      return -1;
   }

   /* 12/ lock file using PID to assert single instance ***********************/
   /* Don't want to limit to a single instance. */

   /* 13/ Drop privileges *****************************************************/

   /* 14/ Signal completion ***************************************************/

   /* Step 15 is done on the very first process. */

   return 0;
}
