/**** POSIX *******************************************************************/
#include <sys/select.h>

#include <errno.h>
#include <string.h>

/**** RELABSD *****************************************************************/
#include <relabsd/debug.h>
#include <relabsd/server.h>

#include <relabsd/config/parameters.h>

#include <relabsd/device/axis.h>
#include <relabsd/device/physical_device.h>
#include <relabsd/device/virtual_device.h>

/******************************************************************************/
/**** LOCAL FUNCTIONS *********************************************************/
/******************************************************************************/
static void convert_input
(
   struct relabsd_server server [const restrict static 1]
)
{
   unsigned int input_type, input_code;
   int value;

   if
   (
      relabsd_physical_device_read
      (
         &(server->physical_device),
         &input_type,
         &input_code,
         &value
      )
      < 0
   )
   {
      return;
   }

   if (input_type == EV_REL)
   {
      struct relabsd_axis * axis;
      unsigned int abs_code;
      enum relabsd_axis_name axis_name;

      axis_name =
         relabsd_axis_name_and_evdev_abs_from_evdev_rel(input_code, &abs_code);

      if (axis_name == RELABSD_UNKNOWN)
      {
         return;
      }

      axis = relabsd_parameters_get_axis(axis_name, &(server->parameters));
      axis_name = relabsd_axis_get_convert_to(axis);

      if (axis_name != RELABSD_UNKNOWN)
      {
         abs_code = relabsd_axis_name_to_evdev_abs(axis_name);
      }

      switch (relabsd_axis_filter_new_value(axis, &value))
      {
         case -1:
            /* Doesn't want the event to be transmitted. */
            return;

         case 1:
            (void) relabsd_virtual_device_write_evdev_event
            (
               &(server->virtual_device),
               EV_ABS,
               abs_code,
               value
            );

            relabsd_virtual_device_set_has_already_timed_out
            (
               0,
               &(server->virtual_device)
            );
            return;

         case 0:
            (void) relabsd_virtual_device_write_evdev_event
            (
               &(server->virtual_device),
               input_type,
               input_code,
               value
            );
            return;
      }
   }
   else if (input_type == EV_ABS)
   {
      enum relabsd_axis_name axis_name;

      axis_name = relabsd_axis_name_from_evdev_abs(input_code);

      if (axis_name != RELABSD_UNKNOWN)
      {
         struct relabsd_axis * axis;

         axis = relabsd_parameters_get_axis(axis_name, &(server->parameters));
         axis_name = relabsd_axis_get_convert_to(axis);

         if (axis_name != RELABSD_UNKNOWN)
         {
            input_code = relabsd_axis_name_to_evdev_abs(axis_name);
         }
      }

      (void) relabsd_virtual_device_write_evdev_event
      (
         &(server->virtual_device),
         input_type,
         input_code,
         value
      );
   }
   else
   {
      /* Any other event is retransmitted as is. */
      (void) relabsd_virtual_device_write_evdev_event
      (
         &(server->virtual_device),
         input_type,
         input_code,
         value
      );
   }
}

static void reset_axes
(
   struct relabsd_server server [const restrict static 1]
)
{
   relabsd_virtual_device_set_has_already_timed_out
   (
      1,
      &(server->virtual_device)
   );

   relabsd_virtual_device_set_axes_to_zero
   (
      &(server->parameters),
      &(server->virtual_device)
   );
}

static int wait_for_next_event
(
   fd_set ready_to_read [const restrict static 1],
   struct relabsd_server server [const static 1]
)
{
   int ready_fds, physical_device_fd, interruption_fd, highest_fd;

   FD_ZERO(ready_to_read);

   physical_device_fd =
      relabsd_physical_device_get_file_descriptor(&(server->physical_device));

   FD_SET(physical_device_fd, ready_to_read);

   if (relabsd_physical_device_is_late(&(server->physical_device)))
   {
      return 1;
   }

   interruption_fd = relabsd_server_get_interruption_file_descriptor();

   FD_SET(interruption_fd, ready_to_read);

   if (interruption_fd > physical_device_fd)
   {
      highest_fd = interruption_fd;
   }
   {
      highest_fd = physical_device_fd;
   }

   errno = 0;

   if
   (
      relabsd_parameters_use_timeout(&(server->parameters))
      &&
      !relabsd_virtual_device_has_already_timed_out(&(server->virtual_device))
   )
   {
      struct timeval curr_timeout;

      pthread_mutex_lock(&(server->mutex));
      /* call to select may alter timeout */
      curr_timeout = relabsd_parameters_get_timeout(&(server->parameters));
      pthread_mutex_unlock(&(server->mutex));

      ready_fds =
         select
         (
            (highest_fd + 1),
            ready_to_read,
            (fd_set *) NULL,
            (fd_set *) NULL,
            (struct timeval *) &curr_timeout
         );
   }
   else
   {
      ready_fds =
         select
         (
            (highest_fd + 1),
            ready_to_read,
            (fd_set *) NULL,
            (fd_set *) NULL,
            (struct timeval *) NULL
         );
   }

   if (ready_fds == -1)
   {
      RELABSD_ERROR
      (
         "Error while waiting for new input from the physical device: %s.",
         strerror(errno)
      );

      FD_ZERO(ready_to_read);

      return 1;
   }

   /* ready_fds == 0 on timeout */
   return ready_fds;
}

/******************************************************************************/
/**** EXPORTED FUNCTIONS ******************************************************/
/******************************************************************************/
int relabsd_server_conversion_loop
(
   struct relabsd_server server [const static 1]
)
{
   fd_set ready_to_read;

   for (;;)
   {
      switch (wait_for_next_event(&ready_to_read, server))
      {
         case 1:
         case 2:
            if (!relabsd_server_keep_running())
            {
               return 0;
            }

            if
            (
               FD_ISSET
               (
                  relabsd_physical_device_get_file_descriptor
                  (
                     &(server->physical_device)
                  ),
                  &ready_to_read
               )
            )
            {
               pthread_mutex_lock(&(server->mutex));
               convert_input(server);
               pthread_mutex_unlock(&(server->mutex));
            }

            break;

         case 0:
            pthread_mutex_lock(&(server->mutex));
            reset_axes(server);
            pthread_mutex_unlock(&(server->mutex));
            break;
      }
   }
}
