/**** POSIX *******************************************************************/
#include <stdlib.h>
#include <stdio.h>

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

/*
 * Returns -1 on error,
 *          0 on EOF,
 *          1 on newline.
 */
int relabsd_util_reach_next_line_or_eof (FILE f [const restrict static 1])
{
   char c;

   c = (char) getc(f);

   while ((c != '\n') && c != EOF)
   {
      c = (char) getc(f);
   }

   if (ferror(f))
   {
      /*
       * The 'ferror' function's manual specifically states that it does not
       * sets errno. There is no mention of errno in the 'getc' function's
       * either, so I am assuming that errno cannot be used to indicate the
       * error.
       */
      return -1;
   }

   if (c == EOF)
   {
      return 0;
   }

   return 1;
}
