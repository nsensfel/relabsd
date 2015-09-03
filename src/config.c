#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "error.h"
#include "pervasive.h"
#include "axis.h"
#include "config.h"

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
 * Returns -1 on error,
 *          0 on EOF,
 *          1 on newline.
 */
static int reach_next_line_or_eof (FILE * const f)
{
   int prev_errno;
   char c;

   prev_errno = errno;

   errno = 0;

   c = getc(f);

   while ((c != '\n') && c != EOF)
   {
      c = getc(f);
   }

   if (errno != 0)
   {
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

   return 0;
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
      if (parse_axis_configuration_line(conf, f, prefix) < 0)
      {
         /* Fatal error. */
         return -1;
      }
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
      conf->device_name = "relabsd device";
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

   if (abs(*value - conf->axis[axis].previous_value) <= conf->axis[axis].fuzz)
   {
#ifdef RELABSD_REAL_FUZZ
      conf->axis[axis].previous_value = *value;
#endif

      return -1;
   }

   if (*value < conf->axis[axis].min)
   {
      *value = conf->axis[axis].min;
   }
   else if (*value > conf->axis[axis].max)
   {
      *value = conf->axis[axis].max;
   }
   else if (abs(*value) <= conf->axis[axis].flat)
   {
      *value = 0;

      /*
       * As long as the 'fuzz' test is done prior the 'flat' one, moving around
       * in the 'flat' zone won't trigger useless '0' value events.
       */
   }

   /* TODO: handle conf->axis[axis].resolution */

   conf->axis[axis].previous_value = *value;

   return 1;
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
