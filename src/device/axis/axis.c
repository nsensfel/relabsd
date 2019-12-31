/**** POSIX *******************************************************************/
#include <string.h>

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
   struct relabsd_axis axis [const restrict static 1]
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
