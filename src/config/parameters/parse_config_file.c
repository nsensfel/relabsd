/**** POSIX *******************************************************************/
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include <sys/time.h>

/**** RELABSD *****************************************************************/
#include <relabsd/config.h>
#include <relabsd/debug.h>

#include <relabsd/config/parameters.h>

#include <relabsd/device/axis.h>

#include <relabsd/util/string.h>


/* TODO: move this to relabsd/config.h */
#ifndef RELABSD_OPTION_MAX_SIZE
   #define RELABSD_OPTION_MAX_SIZE 64
#endif

/******************************************************************************/
/**** LOCAL FUNCTIONS *********************************************************/
/******************************************************************************/

/*
 * Returns -1 on error,
 *          0 on EOF,
 *          1 on newline.
 */
static int read_axis_options
(
   FILE file [const restrict static 1],
   const char axis_name [const restrict static 1],
   struct relabsd_axis axis [const restrict static 1]
)
{
   char option[(RELABSD_OPTION_MAX_SIZE + 1)];
   int i;
   char c;

   option[RELABSD_OPTION_MAX_SIZE] = '\0';

   i = 0;

   while (i <= RELABSD_OPTION_MAX_SIZE)
   {
      c = (char) getc(file);

      if (c == EOF)
      {
         if (ferror(file))
         {
            RELABSD_FATAL
            (
               "Reading error while parsing an option in the configuration file"
               " (axis '%s').",
               axis_name
            );
         }
         else
         {
            RELABSD_FATAL
            (
               "End of file reached while parsing an option in the"
               " configuration file (axis '%s').",
               axis_name
            );
         }

         return -1;
      }

      switch (c)
      {
         case ' ':
         case '\t':
            break;

         case ',':
            option[i] = '\0';
            /* We parsed a new option and there is a least another. */
            (void)
               relabsd_axis_enable_option_from_name(option, axis_name, axis);

            i = 0;

            break;

         case '\n':
            option[i] = '\0';
            (void)
               relabsd_axis_enable_option_from_name(option, axis_name, axis);

            return 1;

         case EOF:
            option[i] = '\0';
            (void)
               relabsd_axis_enable_option_from_name(option, axis_name, axis);

            return 0;

         default:
            option[i] = c;
            i++;

            break;
      }
   }

   RELABSD_FATAL
   (
      "[CONFIG] Option name '%s[...]' (axis '%s') is too long (%d chars max).",
      option,
      axis_name,
      RELABSD_OPTION_MAX_SIZE
   );

   return -1;
}

static int parse_timeout_configuration_line
(
   FILE file [const restrict static 1],
   struct relabsd_parameters parameters [const static 1]
)
{
   int read_count;
   int timeout_msec;

   errno = 0;

   read_count = fscanf(file, "%d", &timeout_msec);

   if (read_count == EOF)
   {
      if (errno == 0)
      {
         RELABSD_S_FATAL
         (
            "Unexpected end of file while reading the timeout parameter in the"
            " configuration file."
         );
      }
      else
      {
         RELABSD_FATAL
         (
            "An error occured while reading the timeout parameter in the"
            " configuration file: %s.",
            strerror(errno)
         );
      }

      return -1;
   }
   else if (read_count < 1)
   {
      RELABSD_S_FATAL
      (
         "Invalid parameter count for the timeout option in the configuration"
         " file."
      );

      return -1;
   }
   else if (timeout_msec < 0)
   {
      RELABSD_FATAL
      (
         "Invalid value the timeout option in the configuration file (%d).",
         timeout_msec
      );

      return -1;
   }

   relabsd_parameters_set_timeout(timeout_msec, parameters);

   return 1;
}

/*
 * Returns -1 on (fatal) error,
 *          0 on succes.
 */
static int parse_axis_configuration_line
(
   FILE file [const restrict static 1],
   const char axis_name [const restrict static 1],
   struct relabsd_parameters parameters [const static 1]
)
{
   int read_count;
   enum relabsd_axis_name axis_index;
   struct relabsd_axis * axis;

   axis_index = relabsd_axis_parse_name(axis_name);

   if (axis_index == RELABSD_UNKNOWN)
   {
      if
      (
         RELABSD_IS_PREFIX("TO", axis_name)
         || RELABSD_IS_PREFIX("to", axis_name)
      )
      {
         return parse_timeout_configuration_line(file, parameters);
      }

      RELABSD_FATAL("Unknown axis '%s' in the configuration file.", axis_name);

      return -1;

   }

   axis = (parameters->axes + axis_index);

   errno = 0;

   read_count =
      fscanf
      (
         file,
         "%d%d%d%d%d",
         &(axis->min),
         &(axis->max),
         &(axis->fuzz),
         &(axis->flat),
         &(axis->resolution)
      );

   if (read_count == EOF)
   {
      if (errno == 0)
      {
         RELABSD_FATAL
         (
            "Unexpected end of file while reading the '%s' axis' parameters in"
            " the configuration file.",
            axis_name
         );
      }
      else
      {
         RELABSD_FATAL
         (
            "An error occured while reading the '%s' axis' parameters in the"
            " configuration file: %s.",
            axis_name,
            strerror(errno)
         );
      }

      return -1;
   }
   else if (read_count < 5)
   {
      RELABSD_FATAL
      (
         "Invalid parameter count for the '%s' axis in the configuration file.",
         axis_name
      );

      return -1;
   }

   relabsd_axis_enable(axis);

   return read_axis_options(file, axis_name, axis);
}

/*
 * Returns -1 on (fatal) error,
 *          0 on EOF,
 *          1 on newline.
 */
static int read_config_line
(
   FILE file [const restrict static 1],
   const char prefix [const restrict static 1],
   struct relabsd_parameters parameters [const restrict static 1]
)
{
   if (RELABSD_IS_PREFIX("#", prefix))
   {
      return relabsd_util_reach_next_line_or_eof(file);
   }

   return parse_axis_configuration_line(file, prefix, parameters);
}

/******************************************************************************/
/**** EXPORTED FUNCTIONS ******************************************************/
/******************************************************************************/
int relabsd_parameters_parse_config_file
(
   const char filename [const restrict static 1],
   struct relabsd_parameters parameters [const restrict static 1]
)
{
   FILE * file;
   char buffer[(RELABSD_CONF_AXIS_CODE_SIZE + 1)];
   int continue_reading;

   buffer[RELABSD_CONF_AXIS_CODE_SIZE] = '\0';

   errno = 0;
   file = fopen(filename, "r");

   if (file == (FILE *) NULL)
   {
      RELABSD_FATAL("Could not open file %s: %s.", filename, strerror(errno));

      return -1;
   }

   errno = 0;
   continue_reading = 1;

   while
   (
      (continue_reading == 1)
      &&
      (
         fscanf
         (
            file,
            "%" RELABSD_TO_STRING(RELABSD_CONF_AXIS_CODE_SIZE) "s",
            buffer
         )
         != EOF
      )
   )
   {
      switch (read_config_line(file, buffer, parameters))
      {
         case 1:
            /* Everything is going well. */
            break;

         case 0:
            /* EOF reached. */
            continue_reading = 0;
            break;

         case -1:
            /* A fatal error occured. */
            fclose(file);
            return -1;
      }
   }

   if (errno != 0)
   {
      /* An error happened in the while loop condition. */
      RELABSD_FATAL
      (
         "[CONFIG] Error while reading file: %s, last read '%s'.",
         strerror(errno),
         buffer
      );

      fclose(file);

      return -1;
   }

   fclose(file);

   return 0;
}
