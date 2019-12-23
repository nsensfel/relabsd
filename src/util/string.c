/**** POSIX *******************************************************************/
#include <stdlib.h>

/******************************************************************************/
/**** LOCAL FUNCTIONS *********************************************************/
/******************************************************************************/

/******************************************************************************/
/**** EXPORTED FUNCTIONS ******************************************************/
/******************************************************************************/
int relabsd_util_parse_int
(
   const char string [const restrict static 1],
   const int min,
   const int max,
   int output [const restrict static 1]
)
{
   char * invalid_char; /* may become an alias of string. */
   long int buffer;

   buffer = strtol(string, &invalid_char, 10);

   if ((invalid_char[0] != '\0') || (string[0] == '\0'))
   {
      return -1;
   }

   if ((buffer < ((long int) min)) || (buffer > ((long int) max)))
   {
      return -2;
   }

   *output = ((int) buffer);
   return 0;
}
