/**** LIBEVDEV ****************************************************************/
#include <libevdev/libevdev.h>

/**** RELABSD *****************************************************************/
#include <relabsd/debug.h>

#include <relabsd/util/string.h>

#include <relabsd/device/axis.h>

/******************************************************************************/
/**** LOCAL FUNCTIONS *********************************************************/
/******************************************************************************/

/******************************************************************************/
/**** EXPORTED FUNCTIONS ******************************************************/
/******************************************************************************/
enum relabsd_axis_name relabsd_axis_parse_name
(
   const char name [const restrict static 1]
)
{
   if (RELABSD_STRING_EQUALS("X", name))
   {
      return RELABSD_X;
   }
   else if (RELABSD_STRING_EQUALS("Y", name))
   {
      return RELABSD_Y;
   }
   else if (RELABSD_STRING_EQUALS("Z", name))
   {
      return RELABSD_Z;
   }
   else if (RELABSD_STRING_EQUALS("RX", name))
   {
      return RELABSD_RX;
   }
   else if (RELABSD_STRING_EQUALS("RY", name))
   {
      return RELABSD_RY;
   }
   else if (RELABSD_STRING_EQUALS("RZ", name))
   {
      return RELABSD_RZ;
   }
   else if (RELABSD_STRING_EQUALS("WL", name))
   {
      return RELABSD_WHEEL;
   }
   else if (RELABSD_STRING_EQUALS("MC", name))
   {
      return RELABSD_MISC;
   }

   return RELABSD_UNKNOWN;
}

const char * relabsd_axis_name_to_string (const enum relabsd_axis_name e)
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

   RELABSD_S_PROG_ERROR("relabsd_axis_to_name is missing at least 1 case.");

   return "..";
}

enum relabsd_axis_name relabsd_axis_name_and_evdev_abs_from_evdev_rel
(
   const unsigned int rel_code,
   unsigned int abs_code [const restrict static 1]
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

unsigned int relabsd_axis_name_to_evdev_rel (const enum relabsd_axis_name e)
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
         RELABSD_S_PROG_ERROR
         (
            "relabsd_axis_name_to_evdev_rel(RELABSD_UNKNOWN) is forbidden."
         );
         return REL_MAX;

      default:
         break;
   }

   RELABSD_S_PROG_ERROR
   (
      "relabsd_axis_name_to_evdev_rel is missing at least 1 case."
   );

   return REL_MAX;
}

unsigned int relabsd_axis_name_to_evdev_abs (const enum relabsd_axis_name e)
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
         RELABSD_S_PROG_ERROR
         (
            "relabsd_axis_to_abs(RELABSD_UNKNOWN) is forbidden."
         );
         return ABS_MAX;

      default:
         break;
   }

   RELABSD_S_PROG_ERROR("relabsd_axis_to_abs is missing at least 1 case.");

   return REL_MAX;
}

/*
 * Returns the relabsd_axis equivalent of a EV_REL/EV_ABS code.
 */
enum relabsd_axis_name relabsd_axis_name_from_evdev_rel (const unsigned int rel)
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

enum relabsd_axis_name relabsd_axis_name_from_evdev_abs (const unsigned int abs)
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
