#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#include <libevdev/libevdev-uinput.h>

#include "error.h"
#include "axis.h"
#include "config.h"

#include "relabsd_device.h"

/*
   LIBEVDEV_UINPUT_OPEN_MANAGED is not defined on my machines.
   It is not my place to define it, so I'll avoid the issue by defining my own
   constant.
*/
#ifndef LIBEVDEV_UINPUT_OPEN_MANAGED
   #pragma message  "[WARNING] libevdev did not define "\
      "LIBEVDEV_UINPUT_OPEN_MANAGED, using value '-2' instead."
   #define RELABSD_UINPUT_OPEN_MANAGED -2
#else
   #define RELABSD_UINPUT_OPEN_MANAGED LIBEVDEV_UINPUT_OPEN_MANAGED
#endif

static void replace_rel_axes
(
   struct relabsd_device * const dev,
   const struct relabsd_config * const config
)
{
   int i;
   struct input_absinfo absinfo;
   unsigned int abs_code, rel_code;

   for (i = RELABSD_VALID_AXES_COUNT; i --> 0;)
   {
      if (config->axis[i].enabled)
      {
         rel_code = relabsd_axis_to_rel((enum relabsd_axis) i);
         abs_code = relabsd_axis_to_abs((enum relabsd_axis) i);

         relabsd_config_get_absinfo(config, (enum relabsd_axis) i, &absinfo);
         libevdev_disable_event_code(dev->dev, EV_REL, rel_code);
         libevdev_enable_event_code(dev->dev, EV_ABS, abs_code, &absinfo);
      }
   }

}

int relabsd_device_create
(
   struct relabsd_device * const dev,
   const struct relabsd_config * const config
)
{
   int fd;

   fd = open(config->input_file, O_RDONLY);

   if (fd < 0)
   {
      _FATAL
      (
         "Could not open device '%s' in read only mode: %s.",
         config->input_file,
         strerror(errno)
      );

      return -1;
   }

   if (libevdev_new_from_fd(fd, &(dev->dev)) < 0)
   {
      _FATAL
      (
         "libevdev could not open '%s': '%s'.",
         config->input_file,
         strerror(errno)
      );

      close(fd);

      return -1;
   }

   libevdev_set_name(dev->dev, config->device_name);

   libevdev_enable_event_type(dev->dev, EV_ABS);

   replace_rel_axes(dev, config);

   if
   (
       libevdev_uinput_create_from_device
       (
         dev->dev,
         /* See top of the file. */
         RELABSD_UINPUT_OPEN_MANAGED,
         &(dev->uidev)
       )
       < 0
   )
   {
      _FATAL("Could not create relabsd device: %s.", strerror(errno));

      libevdev_free(dev->dev);

      close(fd);

      return -1;
   }

   close(fd);

   return 0;
}

void relabsd_device_destroy (const struct relabsd_device * const dev)
{
   libevdev_uinput_destroy(dev->uidev);
   libevdev_free(dev->dev);
}

int relabsd_device_write_evdev_event
(
   const struct relabsd_device * const dev,
   unsigned int const type,
   unsigned int const code,
   int const value
)
{
   /* OPTIMIZE: Should we really send 'EV_SYN' after every event? */
   if
   (
      (libevdev_uinput_write_event(dev->uidev, type, code, value) == 0)
      && (libevdev_uinput_write_event(dev->uidev, EV_SYN, SYN_REPORT, 0) == 0)
   )
   {
      return 0;
   }

   return -1;
}
