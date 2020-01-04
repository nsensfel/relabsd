/**** POSIX *******************************************************************/
#include <sys/stat.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/**** RELABSD *****************************************************************/
#include <relabsd/debug.h>

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
   int unnamed_pipe[2];
   pid_t proc_id;

   errno = 0;

   if (pipe(unnamed_pipe) == -1)
   {
      RELABSD_FATAL
      (
         "Unable to create an unnamed pipe for the daemon creation process: %s",
         strerror(errno)
      );

      return -1;
   }

   /* 1/ Close all open file descriptors ... **********************************/
   /* None were opened at this point, except the pipe, which is still needed. */

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
      char buffer;

      /* Close the writing end of the pipe, as this process does not use it */
      errno = 0;

      if (close(unnamed_pipe[1]) == -1)
      {
         RELABSD_ERROR
         (
            "Unable to close writing end of an unnamed pipe during the daemon"
            " creation process: %s.",
            strerror(errno)
         );
      }

      /* Awaiting step 14...*/
      errno = 0;

      if (read(unnamed_pipe[0], &buffer, (size_t) 1) == -1)
      {
         RELABSD_ERROR
         (
            "Unable to read from reading end of an unnamed pipe during the "
            " daemon creation process: %s.",
            strerror(errno)
         );
      }

      if (close(unnamed_pipe[0]) == -1)
      {
         RELABSD_ERROR
         (
            "Unable to close reading end of an unnamed pipe during the daemon"
            " creation process: %s.",
            strerror(errno)
         );
      }

      /* 15/ Original process exits *******************************************/
      exit(0);
   }

   /* Close reading on the pipe, as this process does not use it */
   if (close(unnamed_pipe[0]))
   {

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
      errno = 0;

      if (close(unnamed_pipe[1]) == -1)
      {
         RELABSD_ERROR
         (
            "Unable to close writing end of an unnamed pipe during the daemon"
            " creation process: %s.",
            strerror(errno)
         );
      }

      /* 8/ First child process exits *****************************************/
      exit(0);
   }

   /* 9/ /dev/null for standard input/outputs *********************************/
   /* TODO. */

   /* 10/ reset umask to 0 ****************************************************/
   /* Can't fail, returns previous mask. */
   (void) umask(0);

   /* 11/ Set current directory to / ******************************************/
   errno = 0;

   if (chdir("/") == -1)
   {
      // Can't print an error message at that point though...
      RELABSD_FATAL
      (
         "Step 11 of the daemon creation process, fork(), failed: %s.",
         strerror(errno)
      );

      /* TODO: boop main process. */
      return -1;
   }

   /* 12/ lock file using PID to assert single instance ***********************/
   /* Don't want to limit to a single instance. */

   /* 13/ Drop privileges *****************************************************/
   /* We need those. */

   /* 14/ Signal completion ***************************************************/
   errno = 0;

   if (write(unnamed_pipe[0], (void *) "!", (size_t) 1) == -1)
   {
      RELABSD_ERROR
      (
         "Unable to write to writing end of an unnamed pipe during the daemon"
         " creation process: %s.",
         strerror(errno)
      );
   }

   /* Step 15 is done on the very first process. */
   errno = 0;

   if (close(unnamed_pipe[1]) == -1)
   {
      RELABSD_ERROR
      (
         "Unable to close writing end of an unnamed pipe during the daemon"
         " creation process: %s.",
         strerror(errno)
      );
   }

   return 0;
}
