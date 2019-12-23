#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include "pervasive.h"
#include "error.h"
#include "config.h"
#include "input.h"
#include "relabsd_device.h"

static int RELABSD_RUN = 1;

static void interrupt (int unused_mandatory_parameter)
{
   RELABSD_RUN = 0;

   RELABSD_S_WARNING("Interrupted, will exit at the next input device event.");
}

static void handle_relative_axis_event
(
   struct relabsd_config * const conf,
   const struct relabsd_device * const dev,
   unsigned int const input_type,
   unsigned int const input_code,
   int value
)
{
   unsigned int abs_code;
   enum relabsd_axis rad_code;

   rad_code = relabsd_axis_convert_evdev_rel(input_code, &abs_code);

   switch (relabsd_config_filter(conf, rad_code, &value))
   {
      case -1:
         /* 'conf' doesn't want the event to be transmitted. */
         break;

      case 0:
         /* 'conf' wants the event to be transmitted as is. */
         relabsd_device_write_evdev_event(dev, input_type, input_code, value);
         break;

      case 1:
         /* 'conf' allows the value to be emitted */
         relabsd_device_write_evdev_event(dev, EV_ABS, abs_code, value);
         break;
   }
}

static void convert_input
(
   struct relabsd_config * const conf,
   struct relabsd_input * const input,
   const struct relabsd_device * const dev
)
{
   unsigned int input_type, input_code;
   int value;

   RELABSD_S_DEBUG(RELABSD_DEBUG_PROGRAM_FLOW, "Handling input events...");

   input->timed_out = 1;

   while (RELABSD_RUN == 1)
   {
      if (conf->enable_timeout)
      {
         switch (relabsd_input_wait_for_next_event(input, conf))
         {
            case 1:
               input->timed_out = 0;
               break;

            case 0:
               relabsd_device_set_axes_to_zero(dev, conf);
               input->timed_out = 1;
               break;

            case -1:
               continue;
         }
      }

      if (relabsd_input_read(input, &input_type, &input_code, &value) < 0)
      {
         /*
          * The next event should not be retransmitted, or some kind of error
          * happened.
          */
         /* TODO: error handling. */
         continue;
      }

      if (input_type == EV_REL)
      {
         /* We might have to convert the event. */
         handle_relative_axis_event(conf, dev, input_type, input_code, value);
      }
      else
      {
         /* Any other event is retransmitted as is. */
         relabsd_device_write_evdev_event(dev, input_type, input_code, value);
      }
   }
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

/*
int main (int argc, char ** argv)
{
   struct relabsd_config conf;
   struct relabsd_input input;
   struct relabsd_device dev;

   RELABSD_S_DEBUG(RELABSD_DEBUG_PROGRAM_FLOW, "relabsd started.");

   if (set_signal_handlers() < 0)
   {
      return -1;
   }

   if (relabsd_config_parse(&conf, argc, argv) < 0)
   {
      return -2;
   }

   if (relabsd_input_open(&input, &conf) < 0)
   {
      return -3;
   }

   if (relabsd_device_create(&dev, &conf) < 0)
   {
      return -4;
   }

   convert_input(&conf, &input, &dev);

   RELABSD_S_DEBUG(RELABSD_DEBUG_PROGRAM_FLOW, "Terminating...");

   relabsd_device_destroy(&dev);
   relabsd_input_close(&input);

   RELABSD_S_DEBUG(RELABSD_DEBUG_PROGRAM_FLOW, "Done.");

   return 0;
}
*/
