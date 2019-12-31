/**** RELABSD *****************************************************************/
#include <relabsd/device/axis.h>

/******************************************************************************/
/**** LOCAL FUNCTIONS *********************************************************/
/******************************************************************************/
static int direct_filter
(
   struct relabsd_config_axis * const axis,
   int * const value
)
{
   if (abs(*value - axis->previous_value) <= axis->fuzz)
   {
      if (axis->option[RELABSD_REAL_FUZZ_OPTION])
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
   struct relabsd_config_axis * const axis,
   int * const value
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

   if (axis->option[RELABSD_FRAMED_OPTION])
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
      return;
   }

   /* TODO: handle conf->axis[axis].resolution */

   if (axis->flag[RELABSD_DIRECT_OPTION])
   {
      return direct_filter(axis, value);
   }
   else
   {
      return rel_to_abs_filter(axis, value);
   }
}
