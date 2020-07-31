/**** POSIX *******************************************************************/
#include <sys/stat.h>

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/**** LIBEVDEV ****************************************************************/
#include <libevdev/libevdev.h>
#include <libevdev/libevdev-uinput.h>

/**** RELABSD *****************************************************************/
#include <relabsd/config/parameters.h>

#include <relabsd/debug.h>

#include <relabsd/device/axis.h>
#include <relabsd/device/virtual_device.h>

/******************************************************************************/
/**** LOCAL FUNCTIONS *********************************************************/
/******************************************************************************/
static void replace_rel_axes
(
   struct relabsd_parameters parameters [const static 1],
   const struct relabsd_virtual_device device [const restrict static 1]
)
{
   int i;

   for (i = 0; i < RELABSD_AXIS_VALID_AXES_COUNT; i++)
   {
      enum relabsd_axis_name axis_name;
      struct relabsd_axis * axis;

      axis_name = ((enum relabsd_axis_name) i);
      axis = relabsd_parameters_get_axis(axis_name, parameters);

      axis_name = relabsd_axis_get_convert_to(axis);

      if (axis_name == RELABSD_UNKNOWN)
      {
         axis_name = ((enum relabsd_axis_name) i);
      }

      if
      (
         relabsd_axis_is_enabled(axis)
         && !relabsd_axis_has_flag(axis, RELABSD_NOT_ABS)
      )
      {
         (void) relabsd_virtual_device_update_axis_absinfo
         (
            axis_name,
            axis,
            device
         );
      }
   }
}


/******************************************************************************/
/**** EXPORTED FUNCTIONS ******************************************************/
/******************************************************************************/
int relabsd_virtual_device_rename
(
   const struct relabsd_parameters parameters [const restrict static 1],
   const struct relabsd_virtual_device device [const restrict static 1]
)
{
   size_t new_name_size;
   char * new_name;
   const char * real_name;

   /* +2: One for the \0, one for the space between prefix and 'real_name'. */
   new_name_size = strlen(RELABSD_DEVICE_PREFIX) + 2;

   real_name = relabsd_parameters_get_device_name(parameters);

   if (real_name == (const char *) NULL)
   {
      /*
       * "The name is never NULL but it may be the empty string."
       * I'm assuming that since they use the term 'string', it is \0
       * terminated.
       */
      real_name = libevdev_get_name(device->libevdev);
   }

   new_name_size += strlen(real_name);
   errno = 0;
   new_name = (char *) calloc(new_name_size, sizeof(char));

   if (new_name == (char *) NULL)
   {
      RELABSD_ERROR
      (
         "Attempt at allocating memory to create the virtual device's name "
         "failed: %s.",
         strerror(errno)
      );

      /* This frees whatever came from 'libevdev_get_name'. */
      libevdev_set_name(device->libevdev, RELABSD_DEVICE_PREFIX);

      return -1;
   }

   errno = 0;

   if
   (
      snprintf
      (
         new_name,
         new_name_size,
         "%s %s",
         RELABSD_DEVICE_PREFIX,
         real_name
      )
      != ((int) (new_name_size - 1))
   )
   {
      /* This makes for a great message when strerror(errno) returns SUCCESS. */
      RELABSD_ERROR
      (
         "Something unexpected happened while renaming the virtual device: %s.",
         strerror(errno)
      );

      /* This frees whatever came from 'libevdev_get_name'. */
      libevdev_set_name(device->libevdev, RELABSD_DEVICE_PREFIX);

      free((void *) new_name);

      return -1;
   }

   /* This frees whatever came from 'libevdev_get_name'. */
   libevdev_set_name(device->libevdev, new_name);

   /* FIXME: not entirely sure I should be the one to free it. */
   free((void *) new_name);

   return 0;
}

int relabsd_virtual_device_update_axis_absinfo
(
   const enum relabsd_axis_name axis_name,
   const struct relabsd_axis axis [const restrict static 1],
   const struct relabsd_virtual_device device [const restrict static 1]
)
{
   enum relabsd_axis_name target_axis_name;
   struct input_absinfo absinfo;

   relabsd_axis_to_absinfo(axis, &absinfo);

   target_axis_name = relabsd_axis_get_convert_to(axis);

   if (target_axis_name == RELABSD_UNKNOWN)
   {
      target_axis_name = axis_name;
   }

   /*
    * TODO: report failure? 0 on success, -1 otherwise, no cause given.
    * Might want to add an option to see if people want to use the tool to
    * alter existing EV_ABS axes instead of converting from EV_REL to EV_ABS.
    */
   if (!relabsd_axis_has_flag(axis, RELABSD_NOT_ABS))
   {
      (void) libevdev_disable_event_code
      (
         device->libevdev,
         EV_REL,
         relabsd_axis_name_to_evdev_rel(axis_name)
      );

      (void) libevdev_enable_event_code
      (
         device->libevdev,
         EV_ABS,
         relabsd_axis_name_to_evdev_abs(target_axis_name),
         &absinfo
      );
   }

   return 0;
}

int relabsd_virtual_device_create_from
(
   struct relabsd_parameters parameters [const restrict static 1],
   struct relabsd_virtual_device device [const restrict static 1]
)
{
   int err;
   struct libevdev * physical_device_libevdev;
   int physical_device_file;

   RELABSD_S_DEBUG(RELABSD_DEBUG_PROGRAM_FLOW, "Creating virtual device...");

   device->already_timed_out = 0;

   errno = 0;
   physical_device_file =
      open
      (
         relabsd_parameters_get_physical_device_file_name(parameters),
         O_RDONLY
      );

   if (physical_device_file == -1)
   {
      RELABSD_FATAL
      (
         "Could not open physical device '%s' in read only mode: %s.",
         relabsd_parameters_get_physical_device_file_name(parameters),
         strerror(errno)
      );

      return -1;
   }

   err = libevdev_new_from_fd(physical_device_file, &physical_device_libevdev);

   if (err != 0)
   {
      RELABSD_FATAL
      (
         "libevdev could not open physical device '%s': %s.",
         relabsd_parameters_get_physical_device_file_name(parameters),
         strerror(-err)
      );

      (void) close(physical_device_file);

      return -1;
   }

   device->libevdev = physical_device_libevdev;

   /* Not exactly fatal, is it? */
   (void) relabsd_virtual_device_rename(parameters, device);

   libevdev_enable_event_type(physical_device_libevdev, EV_ABS);

   replace_rel_axes(parameters, device);

   err =
      libevdev_uinput_create_from_device
      (
         physical_device_libevdev,
         /* See top of the file. */
         LIBEVDEV_UINPUT_OPEN_MANAGED,
         &(device->uinput_device)
      );

   if (err !=  0)
   {
      RELABSD_FATAL("Could not create uinput device: %s.", strerror(-err));

      libevdev_free(physical_device_libevdev);

      (void) close(physical_device_file);

      return -1;
   }

   /*
    * We only need the physical device's (now modified) profile, not to actually
    * read from it.
    */
   errno = 0;

   if (close(physical_device_file) == -1)
   {
      RELABSD_ERROR("Could not close physical device: %s", strerror(errno));
   }

   RELABSD_S_DEBUG(RELABSD_DEBUG_PROGRAM_FLOW, "Created virtual device.");

   return 0;
}

int relabsd_virtual_device_recreate
(
   struct relabsd_virtual_device device [const restrict static 1]
)
{
   int err;

   RELABSD_S_DEBUG(RELABSD_DEBUG_PROGRAM_FLOW, "Recreating virtual device...");

   libevdev_uinput_destroy(device->uinput_device);

   err =
      libevdev_uinput_create_from_device
      (
         (device->libevdev),
         /* See top of the file. */
         LIBEVDEV_UINPUT_OPEN_MANAGED,
         &(device->uinput_device)
      );

   if (err !=  0)
   {
      RELABSD_FATAL("Could not recreate uinput device: %s.", strerror(-err));

      return -1;
   }

   RELABSD_S_DEBUG(RELABSD_DEBUG_PROGRAM_FLOW, "Recreated virtual device.");

   return 0;
}

void relabsd_virtual_device_destroy
(
   const struct relabsd_virtual_device device [const restrict static 1]
)
{
   RELABSD_S_DEBUG(RELABSD_DEBUG_PROGRAM_FLOW, "Destroying virtual device...");

   libevdev_uinput_destroy(device->uinput_device);
   libevdev_free(device->libevdev);

   RELABSD_S_DEBUG(RELABSD_DEBUG_PROGRAM_FLOW, "Destroyed virtual device.");
}

int relabsd_virtual_device_write_evdev_event
(
   const struct relabsd_virtual_device device [const restrict static 1],
   unsigned int const type,
   unsigned int const code,
   int const value
)
{
   int err;

   RELABSD_DEBUG
   (
      RELABSD_DEBUG_VIRTUAL_EVENTS,
      "Sending event: {type = %s; code = %s; value = %d}.",
      libevdev_event_type_get_name(type),
      libevdev_event_code_get_name(type, code),
      value
   );

   /*
    * "It is the caller's responsibility that any event sequence is terminated
    * with an EV_SYN/SYN_REPORT/0 event. Otherwise, listeners on the device node
    * will not see the events until the next EV_SYN event is posted."
    * We'll simply send the 'EV_SYN' events when we read them from the physical
    * device.
    */
   err = libevdev_uinput_write_event(device->uinput_device, type, code, value);

   if (err != 0)
   {
      RELABSD_ERROR
      (
         "Unable to generate event {type = %s; code = %s; value = %d}: %s.",
         libevdev_event_type_get_name(type),
         libevdev_event_code_get_name(type, code),
         value,
         strerror(-err)
      );

      return -1;
   }

   /*
    * TODO: check if this is needed.
   if (type == EV_KEY)
   {
      libevdev_uinput_write_event
      (
         dev->uidev,
         EV_SYN,
         SYN_REPORT,
         0
      );
   }
   */

   return 0;
}

void relabsd_virtual_device_set_axes_to_zero
(
   struct relabsd_parameters parameters [const static 1],
   const struct relabsd_virtual_device device [const restrict static 1]
)
{
   int i;

   for (i = 0; i < RELABSD_AXIS_VALID_AXES_COUNT; ++i)
   {
      struct relabsd_axis * axis;

      axis =
         relabsd_parameters_get_axis((enum relabsd_axis_name) i, parameters);

      if (relabsd_axis_has_flag(axis, RELABSD_NOT_ABS))
      {
         continue;
      }

      if (relabsd_axis_is_enabled(axis))
      {
         relabsd_virtual_device_write_evdev_event
         (
            device,
            EV_ABS,
            relabsd_axis_name_to_evdev_abs((enum relabsd_axis_name) i),
            0
         );

         axis->previous_value = 0;
      }
   }

   (void) relabsd_virtual_device_write_evdev_event
   (
      device,
      EV_SYN,
      SYN_REPORT,
      0
   );
}

void relabsd_virtual_device_set_has_already_timed_out
(
   const int val,
   struct relabsd_virtual_device device [const restrict static 1]
)
{
   device->already_timed_out = val;
}

int relabsd_virtual_device_has_already_timed_out
(
   const struct relabsd_virtual_device device [const restrict static 1]
)
{
   return device->already_timed_out;
}
