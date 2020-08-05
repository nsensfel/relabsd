
/**** RELABSD *****************************************************************/
#include <relabsd/debug.h>

#ifndef RELABSD_USE_MACRO_DEBUG
/******************************************************************************/
/**** LOCAL FUNCTIONS *********************************************************/
/******************************************************************************/
int RELABSD_DEBUG_PROGRAM_FLOW = 0;
int RELABSD_DEBUG_CONFIG = 0;
int RELABSD_DEBUG_REAL_EVENTS = 0;
int RELABSD_DEBUG_VIRTUAL_EVENTS = 0;

/******************************************************************************/
/**** EXPORTED FUNCTIONS ******************************************************/
/******************************************************************************/
void relabsd_debug_toggle_program_flow (void)
{
   RELABSD_DEBUG_PROGRAM_FLOW = !RELABSD_DEBUG_PROGRAM_FLOW;
}

void relabsd_debug_toggle_config (void)
{
   RELABSD_DEBUG_CONFIG = !RELABSD_DEBUG_CONFIG;
}

void relabsd_debug_toggle_real_event (void)
{
   RELABSD_DEBUG_REAL_EVENTS = !RELABSD_DEBUG_REAL_EVENTS;
}

void relabsd_debug_toggle_virtual_event (void)
{
   RELABSD_DEBUG_VIRTUAL_EVENTS = !RELABSD_DEBUG_VIRTUAL_EVENTS;
}

#endif
