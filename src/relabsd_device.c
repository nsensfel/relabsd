#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>

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

static int rename_device
(
   struct libevdev * const dev,
   const struct relabsd_config * const config
)
{
   size_t new_name_size;
   char * new_name;
   const char * real_name;

   /* +2: One for the \0, one for the space between prefix and 'real_name'. */
   new_name_size = strlen(RELABSD_DEVICE_PREFIX) + 2;

   if (config->device_name == (char *) NULL)
   {
      /* XXX
       * "The name is never NULL but it may be the empty string."
       * I'm assuming that since they use the term 'string', it is \0
       * terminated.
       */
      real_name = libevdev_get_name(dev);
   }
   else
   {
      real_name = config->device_name;
   }

   new_name_size += strlen(real_name);

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
      libevdev_set_name(dev, RELABSD_DEVICE_PREFIX);

      return -1;
   }

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
      libevdev_set_name(dev, RELABSD_DEVICE_PREFIX);

      free((void *) new_name);

      return -1;
   }

   /* This frees whatever came from 'libevdev_get_name'. */
   libevdev_set_name(dev, new_name);
   free((void *) new_name);

   return 0;
}

int relabsd_device_create
(
   struct relabsd_device * const dev,
   const struct relabsd_config * const config
)
{
   int fd;

   RELABSD_S_DEBUG(RELABSD_DEBUG_PROGRAM_FLOW, "Creating virtual device...");

   fd = open(config->input_file, O_RDONLY);

   if (fd < 0)
   {
      RELABSD_FATAL
      (
         "Could not open device '%s' in read only mode: %s.",
         config->input_file,
         strerror(errno)
      );

      return -1;
   }

   if (libevdev_new_from_fd(fd, &(dev->dev)) < 0)
   {
      RELABSD_FATAL
      (
         "libevdev could not open '%s': '%s'.",
         config->input_file,
         strerror(errno)
      );

      close(fd);

      return -1;
   }

   if (rename_device(dev->dev, config) < 0)
   {
      libevdev_free(dev->dev);
      close(fd);

      return -1;
   }

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
      RELABSD_FATAL("Could not create relabsd device: %s.", strerror(errno));

      libevdev_free(dev->dev);

      close(fd);

      return -1;
   }

   dev->fd = fd;

   return 0;
}

void relabsd_device_destroy (const struct relabsd_device * const dev)
{
   RELABSD_S_DEBUG(RELABSD_DEBUG_PROGRAM_FLOW, "Destroying virtual device...");

   libevdev_uinput_destroy(dev->uidev);
   libevdev_free(dev->dev);
   close(dev->fd);
}

int relabsd_device_write_evdev_event
(
   const struct relabsd_device * const dev,
   unsigned int const type,
   unsigned int const code,
   int const value
)
{
   RELABSD_DEBUG
   (
      RELABSD_DEBUG_VIRTUAL_EVENTS,
      "Sending event: {type = %s; code = %s; value = %d}.",
       libevdev_event_type_get_name(type),
       libevdev_event_code_get_name(type, code),
       value
   );

   /*
    * We'll also send the 'EV_SYN' events when we receive them from the input
    * device.
    * OPTIMIZE: prevent 'EV_SYN' from being sent if we haven't sent any new
    *           values. (It might not be worth it though)
    */
   if (libevdev_uinput_write_event(dev->uidev, type, code, value) == 0)
   {
      return 0;
   }

   return -1;
}


void relabsd_device_set_axes_to_zero
(
   const struct relabsd_device * const dev,
   const struct relabsd_config * const config
)
{
   int i;

   for (i = 0; i < RELABSD_VALID_AXES_COUNT; ++i)
   {
      if (config->axis[i].enabled)
      {
         relabsd_device_write_evdev_event
         (
            dev,
            relabsd_axis_to_abs((enum relabsd_axis) i),
            EV_ABS,
            0
         );
      }
   }

   /*
    * Also send a SYN event when the axes have been modified.
    */
   libevdev_uinput_write_event(dev->uidev, EV_SYN, SYN_REPORT, 0);
}

int relabsd_device_wait_next_event
(
   const struct relabsd_device * const dev,
   const struct relabsd_config * const config
)
{
   int ready_fds;
   const int old_errno = errno;
   fd_set ready_to_read;
   struct timeval curr_timeout;

   FD_ZERO(&ready_to_read);
   FD_SET(dev->fd, &ready_to_read);

   /* call to select may alter timeout */
   memcpy
   (
      (void *) &(curr_timeout),
      (const void *) &(config->timeout),
      sizeof(struct timeval)
   );

   errno = 0;

   ready_fds = select
   (
      (dev->fd + 1),
      &ready_to_read,
      (fd_set *) NULL,
      (fd_set *) NULL,
      &(curr_timeout)
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

   errno = old_errno;

   return ready_fds;
}
