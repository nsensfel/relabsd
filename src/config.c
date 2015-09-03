#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "error.h"
#include "pervasive.h"
#include "axis.h"
#include "config.h"

#ifndef RELABSD_OPTION_MAX_SIZE
   #define RELABSD_OPTION_MAX_SIZE 64
#endif
/*
 * "errno is never set to zero by any system call or library function."
 * This file makes use of this, by setting it to zero and checking if
 * it was modified after calling an function (I'm guessing this is common
 * practice, but I think it's worth explaining).
 * Following the principle of least astonishment, if a function sets errno to
 * zero, it will not return before setting it back either to its previous
 * value or to a arbitrary nonzero value.
 */

/*
 * Returns -1 on (fatal) error,
 *          0 on EOF,
 *          1 on newline.
 */
static int reach_next_line_or_eof (FILE * const f)
{
   int prev_errno;
   char c;

   prev_errno = errno;

   errno = 0;

   c = (char) getc(f);

   while ((c != '\n') && c != EOF)
   {
      c = (char) getc(f);
   }

   if (errno != 0)
   {
      RELABSD_FATAL
      (
         "[CONFIG] Error while attempting to reach EOF or next line: %s.",
         strerror(errno)
      );

      errno = prev_errno;

      return -1;
   }

   errno = prev_errno;

   if (c == EOF)
   {
      return 0;
   }

   return 1;
}

/*
 * Returns -1 if the option was discarded (an error has been reported),
 *         0 if the option was successfully parsed.
 *
 * ('length' - 1) is the number of relevant characters in 'name'.
 * 'name' must support 'length' characters.
 * name[length] will be set to \0, so it does not need to be when calling
 * this function.
 */
static int parse_option
(
   struct relabsd_config * const conf,
   enum relabsd_axis const axis,
   char * const name,
   int const length
)
{
   name[length] = '\0';

   if (strcmp(name, "direct") == 0)
   {
      conf->axis[axis].option[RELABSD_DIRECT_OPTION] = 1;
   }
   else if (strcmp(name, "real_fuzz") == 0)
   {
      conf->axis[axis].option[RELABSD_REAL_FUZZ_OPTION] = 1;
   }
   else if (strcmp(name, "framed") == 0)
   {
      conf->axis[axis].option[RELABSD_FRAMED_OPTION] = 1;
   }
   else
   {
      RELABSD_ERROR
      (
         "[CONFIG] Unknown option '%s' for axis '%s'.",
         name,
         relabsd_axis_to_name(axis)
      );

      return -1;
   }

   return 0;
}

/*
 * Returns -1 on error,
 *          0 on EOF,
 *          1 on newline.
 */
static int read_axis_options
(
   struct relabsd_config * const conf,
   FILE * const f,
   enum relabsd_axis const axis
)
{
   char option[(RELABSD_OPTION_MAX_SIZE + 1)];
   int i, prev_errno;
   char c;

   option[RELABSD_OPTION_MAX_SIZE] = '\0';

   prev_errno = errno;

   errno = 0;

   memset(conf->axis[axis].option, 0, RELABSD_OPTIONS_COUNT * sizeof(int));

   i = 0;

   while (i <= RELABSD_OPTION_MAX_SIZE)
   {
      c = (char) getc(f);

      if ((errno != 0) && (c == EOF))
      {
         RELABSD_FATAL
         (
            "[CONFIG] Reading error while parsing option name (axis '%s'): %s.",
            relabsd_axis_to_name(axis),
            strerror(errno)
         );

         errno = prev_errno;

         return -1;
      }

      switch (c)
      {
         case ' ':
         case '\t':
            break;

         case ',':
            /* We parsed a new option and there is a least another. */
            parse_option(conf, axis, option, i);

            i = 0;

            break;

         case '\n':
            parse_option(conf, axis, option, i);
            errno = prev_errno;

            return 1;

         case EOF:
            parse_option(conf, axis, option, i);
            errno = prev_errno;

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
      relabsd_axis_to_name(axis),
      RELABSD_OPTION_MAX_SIZE
   );

   return -1;
}

/*
 * Returns -1 on (fatal) error,
 *          0 on succes.
 */
static int parse_axis_configuration_line
(
   struct relabsd_config * const conf,
   FILE * const f,
   const char * const buffer
)
{
   int valc, prev_errno;
   enum relabsd_axis axis;

   axis = relabsd_axis_from_name(buffer);

   if (axis == RELABSD_UNKNOWN)
   {
      RELABSD_FATAL
      (
         "[CONFIG] Unknown axis '%s'.",
         buffer
      );

      return -1;
   }

   prev_errno = errno;
   errno = 0;

   valc =
   fscanf
   (
      f,
      "%d %d %d %d %d",
      &(conf->axis[axis].min),
      &(conf->axis[axis].max),
      &(conf->axis[axis].fuzz),
      &(conf->axis[axis].flat),
      &(conf->axis[axis].resolution)
   );

   if (valc == EOF)
   {
      if (errno == 0)
      {
         RELABSD_FATAL
         (
            "[CONFIG] Unexpected end of file while reading axis '%s'.",
            buffer
         );
      }
      else
      {
         RELABSD_FATAL
         (
            "[CONFIG] An error occured while reading axis '%s': %s.",
            buffer,
            strerror(errno)
         );
      }

      errno = prev_errno;

      return -1;
   }
   else if (valc < 5)
   {
      RELABSD_FATAL
      (
         "[CONFIG] Invalid parameter count for axis '%s'.",
         buffer
      );

      errno = prev_errno;

      return -1;
   }

   errno = prev_errno;

   conf->axis[axis].enabled = 1;
   conf->axis[axis].previous_value = 0;

   return read_axis_options(conf, f, axis);
}

/*
 * Returns -1 on (fatal) error,
 *          0 on EOF,
 *          1 on newline.
 */
static int read_config_line
(
   struct relabsd_config * const conf,
   FILE * const f,
   const char * const prefix
)
{
   if (!RELABSD_IS_PREFIX("#", prefix))
   {
      return parse_axis_configuration_line(conf, f, prefix);
   }

   return reach_next_line_or_eof(f);
}

/*
 * Returns -1 on (fatal) error,
 *         0 on success.
 */
static int read_config_file
(
   struct relabsd_config * const conf,
   char * const filename
)
{
   FILE * f;
   char buffer[3];
   int continue_reading, prev_errno;

   buffer[2] = '\0';

   f = fopen(filename, "r");

   if (f == (FILE *) NULL)
   {
      RELABSD_FATAL
      (
         "[CONFIG] Could not open file: %s.",
         strerror(errno)
      );

      return -1;
   }

   prev_errno = errno;
   errno = 0;

   continue_reading = 1;

   while ((continue_reading == 1) && (fscanf(f, "%2s", buffer) != EOF))
   {
      switch (read_config_line(conf, f, buffer))
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
            errno = prev_errno;

            fclose(f);
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

      errno = prev_errno;

      fclose(f);

      return -1;
   }

   errno = prev_errno;

   fclose(f);

   return 0;
}

/*
 * Returns -1 on (fatal) error,
 *          0 on valid usage.
 */
static int check_usage
(
   int const argc,
   char * const * const argv
)
{
   if ((argc < 3) || (argc > 4))
   {
      RELABSD_FATAL
      (
         "Usage: %s input_device config_file [<relabsd_device_name>]",
         argv[0]
      );

      return -1;
   }

   return 0;
}

static void init_axes_config (struct relabsd_config * const conf)
{
   int i;

   for (i = RELABSD_VALID_AXES_COUNT; i --> 0;)
   {
      conf->axis[i].enabled = 0;
   }
}

int relabsd_config_parse
(
   struct relabsd_config * const conf,
   int const argc,
   char * const * const argv
)
{
   if (check_usage(argc, argv) < 0)
   {
      return -1;
   }

   if (argc == 3)
   {
      conf->device_name = NULL;
   }
   else
   {
      conf->device_name = argv[3];
   }

   conf->input_file = argv[1];

   init_axes_config(conf);

   if (read_config_file(conf, argv[2]) < 0)
   {
      return -1;
   }

   return 0;
}

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

int relabsd_config_filter
(
   struct relabsd_config * const conf,
   enum relabsd_axis const axis,
   int * const value
)
{
   if ((axis == RELABSD_UNKNOWN) || !conf->axis[axis].enabled)
   {
      return 0;
   }

   /* TODO: handle conf->axis[axis].resolution */

   if (conf->axis[axis].option[RELABSD_DIRECT_OPTION])
   {
      return direct_filter((conf->axis + axis), value);
   }
   else
   {
      return rel_to_abs_filter((conf->axis + axis), value);
   }
}

void relabsd_config_get_absinfo
(
   const struct relabsd_config * const conf,
   enum relabsd_axis const axis,
   struct input_absinfo * const absinfo
)
{
   absinfo->value = (__s32) 0;
   absinfo->minimum = (__s32) conf->axis[axis].min;
   absinfo->maximum = (__s32) conf->axis[axis].max;
   absinfo->fuzz = (__s32) conf->axis[axis].fuzz;
   absinfo->flat = (__s32) conf->axis[axis].flat;
   absinfo->resolution = (__s32) conf->axis[axis].resolution;
}
