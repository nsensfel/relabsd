/**** POSIX *******************************************************************/
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

/**** LIBEVDEV ****************************************************************/
#include <libevdev/libevdev.h>

/**** RELABSD *****************************************************************/
#include <relabsd/debug.h>

/******************************************************************************/
/**** LOCAL FUNCTIONS *********************************************************/
/******************************************************************************/
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


   return 0;
}

/******************************************************************************/
/**** EXPORTED FUNCTIONS ******************************************************/
/******************************************************************************/
int relabsd_physical_device_compatibility_test
(
   const struct relabsd_physical_device device [const restrict static 1],
   const struct relabsd_parameters parameters [const restrict static 0],
   const int verbose
)
{
   if (!libevdev_has_event_type(device->libevdev, EV_REL))
   {
      RELABSD_S_FATAL("The physical device has no relative axes.");

      return -1;
   }

   if
   (
      (parameters != (const struct relabsd_parameters *) NULL)
      && (check_for_axes(device->libevdev, parameters, verbose) < 0)
   )
   {
      RELABSD_S_FATAL
      (
         "Issue with the physical device's axes (or lack thereof), or their"
         " compatibility with the requested configuration."
      );

      return -2;
   }

   return 0;
}

int relabsd_physical_device_open
(
   const char filename [const restrict static 1],
   struct relabsd_physical_device device [const restrict static 1]
)
{
   int err;

   RELABSD_S_DEBUG(RELABSD_DEBUG_PROGRAM_FLOW, "Opening physical device...");

   errno = 0;
   device->file = open(filename, O_RDONLY);
   device->is_late = 0;

   if (device->file == -1)
   {
      RELABSD_FATAL
      (
         "Could not open physical device '%s' in read only mode: %s.",
         filename,
         strerror(errno)
      );

      return -1;
   }

   err = libevdev_new_from_fd(device->file, &(device->libevdev));

   if (err != 0)
   {
      RELABSD_FATAL
      (
         "libevdev could not open physical device '%s': %s.",
         filename,
         strerror(-err)
      );

      close(device->file);

      return -1;
   }

   return 0;
}

void relabsd_physical_device_close
(
   const struct relabsd_physical_device device [const restrict static 1]
)
{
   RELABSD_S_DEBUG(RELABSD_DEBUG_PROGRAM_FLOW, "Closing input device...");

   libevdev_free(device->libevdev);

   errno = 0;

   if (close(device->file) == -1)
   {
      RELABSD_ERROR
      (
         "Could not properly close the input device: %s.",
         strerror(errno)
      );
   }
}

int relabsd_physical_device_read
(
   const struct relabsd_physical_device device [const restrict static 1],
   unsigned int input_type [const restrict static 1],
   unsigned int input_code [const restrict static 1],
   int input_value [const restrict static 1]
)
{
   int returned_code;
   struct input_event event;

   returned_code =
      libevdev_next_event
      (
         device->libevdev,
         (
            (
               /*
                * If we were already late, reading in NORMAL mode discards all
                * the outdated input events, whereas reading in SYNC mode goes
                * through them in order.
                * TODO: add an option to allow users to drop events when late.
                */
               device->is_late ?
               LIBEVDEV_READ_FLAG_SYNC
               : LIBEVDEV_READ_FLAG_NORMAL
            )
            /* "The fd is not in O_NONBLOCK and a read may block." */
            | LIBEVDEV_READ_FLAG_BLOCKING
         )
         ,
         &event
      );

   switch (returned_code)
   {
      /* Read an actual input. */
      case LIBEVDEV_READ_STATUS_SUCCESS:
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

      /* Code indicating that we are late. */
      case LIBEVDEV_READ_STATUS_SYNC:
         /* There are old input events waiting to be read. */
         device->is_late = 1;
         /*
          * From the documentation, the event we just read was an EV_SYN one,
          * so we don't actually have any input event in hand.
          */

         /* FIXME: Really make sure this cannot recurse a second time. */
         return
            relabsd_physical_device_read
            (
               device,
               input_type,
               input_code,
               input_value
            );

      /* No event to read. */
      case -EAGAIN:
         device->is_late = 0;

         return -1;
   }
}

int relabsd_physical_device_is_late
(
   const struct relabsd_physical_device device [const restrict static 1]
)
{
   return device->is_late;
}
