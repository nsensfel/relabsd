#include <libevdev/libevdev.h>

#include "pervasive.h"

#include "axis.h"

/*
 * Implementation note: _IS_PREFIX, as its name implies, is checking for a
 * prefix, not an equal value. This could cause issues if there were axes
 * with name prefixed by another axis name.
 */
enum relabsd_axis relabsd_axis_name_to_enum (const char * const name)
{
   if (_IS_PREFIX("X", name))
   {
      return RELABSD_X;
   }
   else if (_IS_PREFIX("Y", name))
   {
      return RELABSD_Y;
   }
   else if (_IS_PREFIX("Z", name))
   {
      return RELABSD_Z;
   }
   else if (_IS_PREFIX("RX", name))
   {
      return RELABSD_RX;
   }
   else if (_IS_PREFIX("RY", name))
   {
      return RELABSD_RY;
   }
   else if (_IS_PREFIX("RZ", name))
   {
      return RELABSD_RZ;
   }

   return RELABSD_UNKNOWN;
}

char * relabsd_axis_enum_to_name (enum relabsd_axis const e)
{
   switch (e)
   {
      case RELABSD_X:
         return "X";

      case RELABSD_Y:
         return "Y";

      case RELABSD_Z:
         return "Z";

      case RELABSD_RX:
         return "RX";

      case RELABSD_RY:
         return "RY";

      case RELABSD_RZ:
         return "RZ";

      case RELABSD_UNKNOWN:
         return "??";
   }
}

enum relabsd_axis relabsd_axis_convert_evdev_rel
(
   unsigned int const rel_code,
   unsigned int * const abs_code
)
{
   switch (rel_code)
   {
      case REL_X:
         *abs_code = ABS_X;
         return RELABSD_X;

      case REL_Y:
         *abs_code = ABS_Y;
         return RELABSD_Y;

      case REL_Z:
         *abs_code = ABS_Z;
         return RELABSD_Z;

      case REL_RX:
         *abs_code = ABS_RX;
         return RELABSD_RX;

      case REL_RY:
         *abs_code = ABS_RY;
         return RELABSD_RY;

      case REL_RZ:
         *abs_code = ABS_RZ;
         return RELABSD_RZ;

      default:
         return RELABSD_UNKNOWN;
   }
}
