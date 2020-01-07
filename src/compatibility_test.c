/**** POSIX *******************************************************************/
#include <stdio.h>

/**** LIBEVDEV ****************************************************************/
#include <libevdev/libevdev.h>

/**** RELABSD *****************************************************************/
#include <relabsd/debug.h>

#include <relabsd/config/parameters.h>

#include <relabsd/device/axis.h>
#include <relabsd/device/physical_device.h>

/******************************************************************************/
/**** LOCAL FUNCTIONS *********************************************************/
/******************************************************************************/
static int test_for_axis_and_print_info
(
   const struct libevdev * const restrict libevdev
)
{
   int i, device_has_rel;
   unsigned int rel_code;

   device_has_rel = 0;

   for (i = 0; i < RELABSD_AXIS_VALID_AXES_COUNT; ++i)
   {
      rel_code = relabsd_axis_name_to_evdev_rel((enum relabsd_axis_name) i);

      if (libevdev_has_event_code(libevdev, EV_REL, rel_code))
      {
         printf
         (
            "Relative axis: %s\n",
            relabsd_axis_name_to_string((enum relabsd_axis_name) i)
         );

         device_has_rel = 1;
      }
   }

   return device_has_rel;
}

/******************************************************************************/
/**** EXPORTED FUNCTIONS ******************************************************/
/******************************************************************************/
int relabsd_compatibility_test_main
(
   const int argc,
   const char * const argv [const static argc],
   struct relabsd_parameters parameters [const restrict static 1]
)
{
   struct relabsd_physical_device device;
   int is_compatible;
   struct libevdev * libevdev;

   if (relabsd_parameters_parse_options(argc, argv, parameters) < 0)
   {
      return -1;
   }

   if
   (
      relabsd_physical_device_open
      (
         relabsd_parameters_get_physical_device_file_name(parameters),
         &device
      )
      < 0
   )
   {
      return -1;
   }

   libevdev = relabsd_physical_device_get_libevdev(&device);
   is_compatible = test_for_axis_and_print_info (libevdev);

   if (!is_compatible)
   {
      return 0;
   }

   if (relabsd_parameters_are_compatible_with(libevdev, parameters))
   {
      return 2;
   }

   return 1;
}
