/**** POSIX *******************************************************************/
#include <string.h>

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
/*
 * Returns -1 if the option was discarded (an error has been reported),
 *         0 if the option was successfully parsed.
 */
int relabsd_axis_enable_option_from_name
(
   const char option_name [const restrict static 1],
   const char axis_name [const restrict static 1],
   struct relabsd_axis axis [const restrict static 1]
)
{

   if (RELABSD_IS_PREFIX("direct", option_name))
   {
      axis->flags[RELABSD_DIRECT] = 1;

      if (axis->flags[RELABSD_FRAMED])
      {
         RELABSD_WARNING
         (
            "Option 'direct' on axis '%s' supersedes its 'framed' option.",
            axis_name
         );
      }
   }
   else if (RELABSD_IS_PREFIX("real_fuzz", option_name))
   {
      axis->flags[RELABSD_REAL_FUZZ] = 1;
   }
   else if (RELABSD_IS_PREFIX("framed", option_name))
   {
      axis->flags[RELABSD_FRAMED] = 1;

      if (axis->flags[RELABSD_DIRECT])
      {
         RELABSD_WARNING
         (
            "Option 'direct' on axis '%s' supersedes its 'framed' option.",
            axis_name
         );
      }
   }
   else
   {
      RELABSD_ERROR
      (
         "Unknown option '%s' for axis '%s'.",
         option_name,
         axis_name
      );

      return -1;
   }

   return 0;
}
