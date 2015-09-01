#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include <libevdev/libevdev.h>

#include "error.h"

#include "input.h"

static int check_for_axis
(
   const struct libevdev * const dev,
   const char * axis_name,
   const unsigned int axis_id
)
{
   if (!libevdev_has_event_code(dev, EV_REL, axis_id))
   {
      _FATAL("Input device has no %s axis.", axis_name);

      return -1;
   }

   return 0;
}

static int device_is_compatible (const struct libevdev * const dev)
{
   if (!libevdev_has_event_type(dev, EV_REL))
   {
      _S_FATAL("Input device has no relative axis.");

      return -1;
   }

   if
   (
      (check_for_axis(dev, "X", REL_X) < 0)
      | (check_for_axis(dev, "Y", REL_Y) < 0)
      | (check_for_axis(dev, "Z", REL_Z) < 0)
      | (check_for_axis(dev, "RX", REL_RX) < 0)
      | (check_for_axis(dev, "RY", REL_RY) < 0)
      | (check_for_axis(dev, "RZ", REL_RZ) < 0)
   )
   {
      return -1;
   }

   return 0;
}

int relabsd_input_open
(
   struct relabsd_input * const input,
   const char * const filename
)
{
   input->fd = open(filename, O_RDONLY);

   if (input->fd < 0)
   {
      _FATAL
      (
         "Could not open device %s in read only mode:",
         filename,
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
         filename,
         strerror(errno)
      );

      close(input->fd);

      return -1;
   }

   if (device_is_compatible(input->dev) < 0)
   {
      _FATAL("%s is not compatible with relabsd.", filename);

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

   /*if (rc == LIBEVDEV_READ_STATUS_SYNC)
   {
      handle_syn_dropped(input->dev);
   }
   else*/ if (rc != LIBEVDEV_READ_STATUS_SUCCESS)
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
