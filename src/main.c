/**** RELABSD *****************************************************************/
#include <relabsd/client.h>
#include <relabsd/config.h>
#include <relabsd/debug.h>
#include <relabsd/server.h>

#include <relabsd/config/parameters.h>

/******************************************************************************/
/**** LOCAL FUNCTIONS *********************************************************/
/******************************************************************************/

/******************************************************************************/
/**** EXPORTED FUNCTIONS ******************************************************/
/******************************************************************************/
int main (int argc, char ** argv)
{
   int retval;
   struct relabsd_parameters params;

   RELABSD_S_DEBUG(RELABSD_DEBUG_PROGRAM_FLOW, "relabsd started.");

   if (relabsd_parameters_parse_execution_mode(argc, argv, &params) < 0)
   {
      RELABSD_S_DEBUG(RELABSD_DEBUG_PROGRAM_FLOW, "relabsd crashing");

      return -1;
   }

   switch (relabsd_parameters_get_execution_mode(&params))
   {
      case RELABSD_PARAMETERS_CLIENT_MODE:
         retval = relabsd_client_main(argc, argv, &params);
         break;

      case RELABSD_PARAMETERS_RUN_SERVER_MODE:
         retval = relabsd_server_main(argc, argv, &params);
         break;
   }

   RELABSD_S_DEBUG(RELABSD_DEBUG_PROGRAM_FLOW, "relabsd terminating.");

   return retval;
}
