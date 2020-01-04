/**** POSIX *******************************************************************/
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

/**** RELABSD *****************************************************************/
#include <relabsd/debug.h>

/******************************************************************************/
/**** LOCAL FUNCTIONS *********************************************************/
/******************************************************************************/
static int RELABSD_INTERRUPTION_PIPES[2];
static int RELABSD_RUN = 1;

static void interrupt (int unused_mandatory_parameter __attribute__((unused)))
{
   RELABSD_RUN = 0;

   errno = 0;

   if (write(RELABSD_INTERRUPTION_PIPES[0], (void *) "!", (size_t) 1) == -1)
   {
      RELABSD_ERROR
      (
         "Unable to signal the interruption to the conversion thread."
         " Interruption should still occur following the next input from the"
         " physical device. Error: %s.",
         strerror(errno)
      );
   }
}

/******************************************************************************/
/**** EXPORTED FUNCTIONS ******************************************************/
/******************************************************************************/
int relabsd_server_keep_running (void)
{
   return RELABSD_RUN;
}

void relabsd_server_interrupt (void)
{
   interrupt(0);
}

int relabsd_server_initialize_signal_handlers (void)
{
   RELABSD_S_DEBUG(RELABSD_DEBUG_PROGRAM_FLOW, "Setting signal handlers.");

   errno = 0;

   if (pipe(RELABSD_INTERRUPTION_PIPES) == -1)
   {
      RELABSD_FATAL
      (
         "Unable to create an unnamed pipe for the interruption handling: %s",
         strerror(errno)
      );

      return -1;
   }

   if (signal(SIGINT, interrupt) == SIG_ERR)
   {
      RELABSD_S_FATAL("Unable to set the SIGINT signal handler.");

      (void) close(RELABSD_INTERRUPTION_PIPES[0]);
      (void) close(RELABSD_INTERRUPTION_PIPES[1]);

      return -1;
   }

   return 0;
}

void relabsd_server_finalize_signal_handlers (void)
{
   (void) close(RELABSD_INTERRUPTION_PIPES[0]);
   (void) close(RELABSD_INTERRUPTION_PIPES[1]);
}

int relabsd_server_get_interruption_file_descriptor (void)
{
   return RELABSD_INTERRUPTION_PIPES[1];
}
