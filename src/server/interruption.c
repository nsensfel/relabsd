static int RELABSD_RUN = 1;

static void interrupt (int unused_mandatory_parameter)
{
   RELABSD_RUN = 0;

   RELABSD_S_WARNING("Interrupted, will exit at the next input device event.");
}

static int set_signal_handlers ()
{
   RELABSD_S_DEBUG(RELABSD_DEBUG_PROGRAM_FLOW, "Setting signal handlers.");

   if (signal(SIGINT, interrupt) == SIG_ERR)
   {
      RELABSD_S_FATAL("Unable to set the SIGINT signal handler.");

      return -1;
   }

   return 0;
}

