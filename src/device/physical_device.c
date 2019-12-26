/**** RELABSD *****************************************************************/
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

/**** LIBEVDEV ****************************************************************/
#include <libevdev/libevdev.h>

/**** RELABSD *****************************************************************/
#include <relabsd/debug.h>

/*
 * Ensures that the input device has enabled the EV_REL axes mentioned
 * in the configuration file.
 *
 * Returns -1 on (fatal) error,
 *         0 all configured axes are accounted for.
 */
static int check_for_axes
(
   const struct libevdev * const dev,
   const struct relabsd_config * const conf
)
{
   int i, device_is_valid;
   unsigned int rel_code;

   device_is_valid = 1;

   for (i = RELABSD_VALID_AXES_COUNT; i --> 0;)
   {
      if (conf->axis[i].enabled)
      {
         rel_code = relabsd_axis_to_rel((enum relabsd_axis) i);

         if (!libevdev_has_event_code(dev, EV_REL, rel_code))
         {
            RELABSD_FATAL
            (
               "Input device has no relative %s axis, yet the configuration "
               "file asks to convert it.",
               relabsd_axis_to_name((enum relabsd_axis) i)
            );

            device_is_valid = 0;
         }
      }
   }

   return (device_is_valid - 1);
}

/*
 * Ensures that the input device is compatible with the config file.
 *
 * Returns -1 on (fatal) error,
 *         0 is the device is compatible.
 */
static int device_is_compatible
(
   const struct libevdev * const dev,
   const struct relabsd_config * const conf
)
{
   if (!libevdev_has_event_type(dev, EV_REL))
   {
      RELABSD_S_FATAL("Input device has no relative axis.");

      return -1;
   }

   if (check_for_axes(dev, conf) < 0)
   {
      return -1;
   }

   return 0;
}

int relabsd_physical_device_open
(
   struct relabsd_physical_device * const input,
   const struct relabsd_config * const conf
)
{
   RELABSD_S_DEBUG(RELABSD_DEBUG_PROGRAM_FLOW, "Opening input device...");

   input->fd = open(conf->input_file, O_RDONLY);

   if (input->fd < 0)
   {
      RELABSD_FATAL
      (
         "Could not open device '%s' in read only mode: %s.",
         conf->input_file,
         strerror(errno)
      );

      return -1;
   }

   if
   (
      libevdev_new_from_fd(input->fd, &(input->dev)) < 0
   )
   {
      RELABSD_FATAL
      (
         "libevdev could not open '%s': %s.",
         conf->input_file,
         strerror(errno)
      );

      close(input->fd);

      return -1;
   }

   if (device_is_compatible(input->dev, conf) < 0)
   {
      return -1;
   }

   return 0;
}

void relabsd_physical_device_close (const struct relabsd_physical_device * const input)
{
   RELABSD_S_DEBUG(RELABSD_DEBUG_PROGRAM_FLOW, "Closing input device...");

   libevdev_free(input->dev);
   close(input->fd);
}

int relabsd_physical_device_read
(
   const struct relabsd_physical_device * const input,
   unsigned int * const input_type,
   unsigned int * const input_code,
   int * const input_value
)
{
   int rc;
   struct input_event event;

   /*
   if (libevdev_has_event_pending(input->dev) == 0)
   {
      return -1;
   }
   */
   rc =
      libevdev_next_event
      (
         input->dev,
         (LIBEVDEV_READ_FLAG_NORMAL | LIBEVDEV_READ_FLAG_BLOCKING),
         &event
      );

   /* TODO: Look into LIBEVDEV_READ_STATUS_SYNC, handle it. */
   /*
   if (rc == LIBEVDEV_READ_STATUS_SYNC)
   {
      handle_syn_dropped(input->dev);
   }
   else
   */
   if (rc != LIBEVDEV_READ_STATUS_SUCCESS)
   {
      RELABSD_WARNING("[INPUT] Could not get next event: %s.", strerror(-rc));

      return -1;
   }

   RELABSD_DEBUG
   (
      RELABSD_DEBUG_REAL_EVENTS,
      "Valid event received: {type = %s; code = %s; value = %d}.",
       libevdev_event_type_get_name(event.type),
       libevdev_event_code_get_name(event.type, event.code),
       event.value
   );

   *input_type = event.type;
   *input_code = event.code;
   *input_value = event.value;

   return 0;
}

int relabsd_physical_device_wait_for_next_event
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

   errno = old_errno;

   return ready_fds;
}
