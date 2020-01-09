/**** POSIX *******************************************************************/
#include <string.h>

/**** LIBEVDEV ****************************************************************/
#include <libevdev/libevdev.h>

/**** RELABSD *****************************************************************/
#include <relabsd/device/axis.h>

/******************************************************************************/
/**** LOCAL FUNCTIONS *********************************************************/
/******************************************************************************/

/******************************************************************************/
/**** EXPORTED FUNCTIONS ******************************************************/
/******************************************************************************/
void relabsd_axis_initialize
(
   struct relabsd_axis axis [const restrict static 1]
)
{
   (void) memset(axis, 0, sizeof(struct relabsd_axis));
}

void relabsd_axis_to_absinfo
(
   const struct relabsd_axis axis [const restrict static 1],
   struct input_absinfo absinfo [const restrict static 1]
)
{
   absinfo->value = (__s32) 0;
   absinfo->minimum = (__s32) axis->min;
   absinfo->maximum = (__s32) axis->max;
   absinfo->fuzz = (__s32) axis->fuzz;
   absinfo->flat = (__s32) axis->flat;
   absinfo->resolution = (__s32) axis->resolution;
}

void relabsd_axis_enable
(
   struct relabsd_axis axis [const restrict static 1]
)
{
   axis->is_enabled = 1;
}

int relabsd_axis_is_enabled
(
   const struct relabsd_axis axis [const restrict static 1]
)
{
   return axis->is_enabled;
}

int relabsd_axis_attributes_are_dirty
(
   const struct relabsd_axis axis [const restrict static 1]
)
{
   return axis->attributes_were_modified;
}

void relabsd_axis_set_attributes_are_dirty
(
   const int val,
   struct relabsd_axis axis [const restrict static 1]
)
{
   axis->attributes_were_modified = val;
}
