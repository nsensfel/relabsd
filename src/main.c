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

static void interrupt (int signal_id)
{
   RELABSD_RUN = 0;

   _S_WARNING("Interrupted, will exit at the next input device event.");
}

static void handle_relative_axis_event
(
   const struct relabsd_config * const conf,
   const struct relabsd_device * const dev,
   unsigned int const input_type,
   unsigned int const input_code,
   int const value
)
{
   unsigned int abs_code;
   enum relabsd_axis rad_code;

   rad_code = relabsd_axis_convert_evdev_rel(input_code, &abs_code);

   if (rad_code == RELABSD_UNKNOWN)
   {
      /*
       * EV_REL events that do not concern an axis that was explicitly
       * configured are retransmitted as is.
       */
      relabsd_device_write_evdev_event
      (
         dev,
         input_type,
         input_code,
         value
      );
   }
   else if (relabsd_config_allows(conf, rad_code, value))
   {
      /*
       * This filters out events which are inconsistent with 'conf', such as
       * values higher than the axis' configured maximum.
       */
      relabsd_device_write_evdev_event(dev, EV_ABS, abs_code, value);
   }
}

static void convert_input
(
   const struct relabsd_config * const conf,
   const struct relabsd_input * const input,
   const struct relabsd_device * const dev
)
{
   unsigned int input_type, input_code, abs_code;
   int value;

   while (RELABSD_RUN == 1)
   {
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
         relabsd_device_write_evdev_event
         (
            dev,
            input_type,
            input_code,
            value
         );
      }
   }
}

static int set_signal_handlers ()
{
   if (signal(SIGINT, interrupt) == SIG_ERR)
   {
      _S_FATAL("Unable to set the SIGINT signal handler.");

      return -1;
   }

   return 0;
}

int main (int argc, char ** argv)
{
   struct relabsd_config conf;
   struct relabsd_input input;
   struct relabsd_device dev;

   if (set_signal_handlers() < 0)
   {
      return -1;
   }

   if (relabsd_config_parse(&conf, argc, argv) < 0)
   {
      return -2;
   }

   if (relabsd_input_open(&input, conf.input_file) < 0)
   {
      return -3;
   }

   if (relabsd_device_create(&dev, &conf) < 0)
   {
      return -4;
   }

   _S_DEBUG(10, "Converting inputs...");

   convert_input(&conf, &input, &dev);

   _S_DEBUG(10, "Terminating...");

   relabsd_device_destroy(&dev);
   relabsd_input_close(&input);

   return 0;
}
