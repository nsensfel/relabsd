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

int relabsd_server_conversion_loop
(
   struct relabsd_server server [const static 1]
)
{
   return 0;
}


int wait_for_next_event
(
   const struct relabsd_physical_device * const input,
   const struct relabsd_config * const config
)
{
   int ready_fds;
   const int old_errno = errno;
   fd_set ready_to_read;
   struct timeval curr_timeout;

   FD_ZERO(&ready_to_read);
   FD_SET(input->fd, &ready_to_read);

   /* call to select may alter timeout */
   memcpy
   (
      (void *) &(curr_timeout),
      (const void *) &(config->timeout),
      sizeof(struct timeval)
   );

   errno = 0;

   RELABSD_S_ERROR
   (
      "Waiting for input to be ready..."
   );

   ready_fds = select
   (
      (input->fd + 1),
      &ready_to_read,
      (fd_set *) NULL,
      (fd_set *) NULL,
      (input->timed_out) ? NULL : &(curr_timeout)
   );

   if (errno != 0)
   {
      RELABSD_ERROR
      (
         "Unable to wait for timeout: %s (errno: %d).",
         strerror(errno),
         errno
      );

      if (errno == EINTR)
      {
         /* Signal interruption? */
      }
      else
      {
         /* TODO: error message */
      }

      errno = old_errno;

      return -1;
   }

   if (ready_fds == -1)
   {
      /* TODO: error message */

      RELABSD_S_ERROR
      (
         "Unable to wait for timeout, yet errno was not set to anything."
      );

      errno = old_errno;

      return -1;
   }

   RELABSD_ERROR
   (
      "Input is ready, ready_fds = %d", ready_fds
   );

   return ready_fds;
}
