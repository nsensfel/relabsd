#include <libevdev/libevdev.h>

#include "pervasive.h"

#include "axis.h"
#include "error.h"

/*
 * Implementation note: _IS_PREFIX, as its name implies, is checking for a
 * prefix, not an equal value. This could cause issues if there were axes
 * with name prefixed by another axis name.
 */
enum relabsd_axis relabsd_axis_from_name (const char * const name)
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
   else if (_IS_PREFIX("WL", name))
   {
      return RELABSD_WHEEL;
   }
   else if (_IS_PREFIX("MC", name))
   {
      return RELABSD_MISC;
   }

   return RELABSD_UNKNOWN;
}

char * relabsd_axis_to_name (enum relabsd_axis const e)
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

      case RELABSD_WHEEL:
         return "WL";

      case RELABSD_MISC:
         return "MC";

      case RELABSD_UNKNOWN:
         return "??";

      default:
         break;
   }

   _S_PROG_ERROR("relabsd_axis_to_name is missing at least 1 case.");

   return "..";
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

      case REL_WHEEL:
         *abs_code = ABS_WHEEL;
         return RELABSD_WHEEL;

      case REL_MISC:
         *abs_code = ABS_MISC;
         return RELABSD_MISC;

      default:
         return RELABSD_UNKNOWN;
   }
}

unsigned int relabsd_axis_to_rel (enum relabsd_axis const e)
{
   switch (e)
   {
      case RELABSD_X:
         return REL_X;

      case RELABSD_Y:
         return REL_Y;

      case RELABSD_Z:
         return REL_Z;

      case RELABSD_RX:
         return REL_RX;

      case RELABSD_RY:
         return REL_RY;

      case RELABSD_RZ:
         return REL_RZ;

      case RELABSD_WHEEL:
         return REL_WHEEL;

      case RELABSD_MISC:
         return REL_MISC;

      case RELABSD_UNKNOWN:
         _S_PROG_ERROR("relabsd_axis_to_rel(RELABSD_UNKNOWN) is forbidden.");
         return REL_MAX;

      default:
         break;
   }

   _S_PROG_ERROR("relabsd_axis_to_rel is missing at least 1 case.");

   return REL_MAX;
}

unsigned int relabsd_axis_to_abs (enum relabsd_axis const e)
{
   switch (e)
   {
      case RELABSD_X:
         return ABS_X;

      case RELABSD_Y:
         return ABS_Y;

      case RELABSD_Z:
         return ABS_Z;

      case RELABSD_RX:
         return ABS_RX;

      case RELABSD_RY:
         return ABS_RY;

      case RELABSD_RZ:
         return ABS_RZ;

      case RELABSD_WHEEL:
         return ABS_WHEEL;

      case RELABSD_MISC:
         return ABS_MISC;

      case RELABSD_UNKNOWN:
         _S_PROG_ERROR("relabsd_axis_to_abs(RELABSD_UNKNOWN) is forbidden.");
         return ABS_MAX;

      default:
         break;
   }

   _S_PROG_ERROR("relabsd_axis_to_rel is missing at least 1 case.");

   return REL_MAX;
}

/*
 * Returns the relabsd_axis equivalent of a EV_REL/EV_ABS code.
 */
enum relabsd_axis relabsd_axis_from_rel (unsigned int const rel)
{
   switch (rel)
   {
      case REL_X:
         return RELABSD_X;

      case REL_Y:
         return RELABSD_Y;

      case REL_Z:
         return RELABSD_Z;

      case REL_RX:
         return RELABSD_RX;

      case REL_RY:
         return RELABSD_RY;

      case REL_RZ:
         return RELABSD_RZ;

      case REL_WHEEL:
         return RELABSD_WHEEL;

      case REL_MISC:
         return RELABSD_MISC;

      default:
         return RELABSD_UNKNOWN;
   }
}
enum relabsd_axis relabsd_axis_from_abs (unsigned int const abs)
{
   switch (abs)
   {
      case ABS_X:
         return RELABSD_X;

      case ABS_Y:
         return RELABSD_Y;

      case ABS_Z:
         return RELABSD_Z;

      case ABS_RX:
         return RELABSD_RX;

      case ABS_RY:
         return RELABSD_RY;

      case ABS_RZ:
         return RELABSD_RZ;

      case ABS_WHEEL:
         return RELABSD_WHEEL;

      case ABS_MISC:
         return RELABSD_MISC;

      default:
         return RELABSD_UNKNOWN;
   }
}
