/**** LIBEVDEV ****************************************************************/
#include <libevdev/libevdev.h>

/**** RELABSD *****************************************************************/
#include <relabsd/debug.h>

#include <relabsd/config/parameters.h>

#include <relabsd/device/axis.h>

/******************************************************************************/
/**** LOCAL FUNCTIONS *********************************************************/
/******************************************************************************/

/******************************************************************************/
/**** EXPORTED FUNCTIONS ******************************************************/
/******************************************************************************/
int relabsd_parameters_are_compatible_with
(
   const struct libevdev * const restrict libevdev,
   const struct relabsd_parameters parameters [const restrict static 1]
)
{
   int i, device_is_valid;
   unsigned int rel_code;

   device_is_valid = 1;

   for (i = 0; i < RELABSD_AXIS_VALID_AXES_COUNT; ++i)
   {
      if (relabsd_axis_is_enabled(parameters->axes + i))
      {
         rel_code = relabsd_axis_name_to_evdev_rel((enum relabsd_axis_name) i);

         if (!libevdev_has_event_code(libevdev, EV_REL, rel_code))
         {
            RELABSD_ERROR
            (
               "Input device has no relative %s axis, yet the configuration "
               "file asks to convert it.",
               relabsd_axis_name_to_string((enum relabsd_axis_name) i)
            );

            device_is_valid = 0;
         }
      }
   }

   return device_is_valid;
}
