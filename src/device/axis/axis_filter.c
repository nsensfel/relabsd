/**** POSIX *******************************************************************/
#include <stdlib.h>
#include <limits.h>

/**** RELABSD *****************************************************************/
#include <relabsd/device/axis.h>

/******************************************************************************/
/**** LOCAL FUNCTIONS *********************************************************/
/******************************************************************************/
static int direct_filter
(
   struct relabsd_axis axis [const restrict static 1],
   int value [const restrict static 1]
)
{
   if (abs(*value - axis->previous_value) <= axis->fuzz)
   {
      if (axis->flags[RELABSD_REAL_FUZZ])
      {
         axis->previous_value = *value;
      }

      return -1;
   }

   if (*value < axis->min)
   {
      *value = axis->min;
   }
   else if (*value > axis->max)
   {
      *value = axis->max;
   }
   else if (abs(*value) <= axis->flat)
   {
      *value = 0;
   }

   if (*value == axis->previous_value)
   {
      return -1;
   }

   axis->previous_value = *value;

   return 1;
}

static int rel_to_abs_filter
(
   struct relabsd_axis axis [const restrict static 1],
   int value [const restrict static 1]
)
{
   long int guard;

   guard = (((long int) axis->previous_value) + ((long int) *value));

   if (guard < ((long int) INT_MIN))
   {
      guard = ((long int) INT_MIN);
   }
   else if (guard > ((long int) INT_MAX))
   {
      guard = ((long int) INT_MAX);
   }

   *value = (int) guard;

   if (axis->flags[RELABSD_FRAMED])
   {
      if (*value < axis->min)
      {
         *value = axis->min;
      }
      else if (*value > axis->max)
      {
         *value = axis->max;
      }

      if (*value == axis->previous_value)
      {
         return 0;
      }

      axis->previous_value = *value;

      return 1;
   }
   else
   {
      if (*value == axis->previous_value)
      {
         return 0;
      }

      axis->previous_value = *value;

      if ((*value < axis->min) || (*value > axis->max))
      {
         return 0;
      }
      else
      {
         return 1;
      }
   }
}

/******************************************************************************/
/**** EXPORTED FUNCTIONS ******************************************************/
/******************************************************************************/
int relabsd_axis_filter_new_value
(
   struct relabsd_axis axis [const restrict static 1],
   int value [const restrict static 1]
)
{
   if (!(axis->is_enabled))
   {
      return 0;
   }

   if (axis->flags[RELABSD_INVERT])
   {
      *value = -(*value);
   }

   if (axis->flags[RELABSD_NOT_ABS])
   {
      return 1;
   }

   if (axis->flags[RELABSD_DIRECT])
   {
      return direct_filter(axis, value);
   }
   else
   {
      return rel_to_abs_filter(axis, value);
   }
}
