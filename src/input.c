#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include <libevdev/libevdev.h>

#include "error.h"
#include "axis.h"
#include "config.h"

#include "input.h"

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
            _FATAL
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
      _S_FATAL("Input device has no relative axis.");

      return -1;
   }

   if (check_for_axes(dev, conf) < 0)
   {
      return -1;
   }

   return 0;
}

int relabsd_input_open
(
   struct relabsd_input * const input,
   const struct relabsd_config * const conf
)
{
   input->fd = open(conf->input_file, O_RDONLY);

   if (input->fd < 0)
   {
      _FATAL
      (
         "Could not open device %s in read only mode:",
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
      _FATAL
      (
         "libevdev could not open %s:",
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

void relabsd_input_close (const struct relabsd_input * const input)
{
   libevdev_free(input->dev);
   close(input->fd);
}

int relabsd_input_read
(
   const struct relabsd_input * const input,
   unsigned int * const input_type,
   unsigned int * const input_code,
   int * const input_value
)
{
   int rc;
   struct input_event event;

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
      _WARNING("[INPUT] Could not get next event: %s.", strerror(-rc));

      return -1;
   }

   _DEBUG
   (
      90,
      "[INPUT] Valid event received: {type = %s; code = %s; value = %d}.",
       libevdev_event_type_get_name(event.type),
       libevdev_event_code_get_name(event.type, event.code),
       event.value
   );

   *input_type = event.type;
   *input_code = event.code;
   *input_value = event.value;

   return 0;
}
