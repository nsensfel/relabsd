/**** POSIX *******************************************************************/
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include <sys/time.h>

/**** RELABSD *****************************************************************/
#include <relabsd/debug.h>

#include <relabsd/config/config_file.h>

#ifndef RELABSD_OPTION_MAX_SIZE
   #define RELABSD_OPTION_MAX_SIZE 64
#endif

/******************************************************************************/
/**** LOCAL FUNCTIONS *********************************************************/
/******************************************************************************/
/*
 * Returns -1 on (fatal) error,
 *          0 on EOF,
 *          1 on newline.
 */
static int reach_next_line_or_eof (FILE f [const restrict static 1])
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
      RELABSD_S_FATAL
      (
         "[CONFIG] Error while attempting to reach EOF or next line: %s."
      );

      return -1;
   }

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

      RELABSD_DEBUG
      (
         RELABSD_DEBUG_CONFIG,
         "Axis '%s' enabled option 'direct'.",
         relabsd_axis_to_name(axis)
      );

      if (conf->axis[axis].option[RELABSD_FRAMED_OPTION])
      {
         RELABSD_WARNING
         (
            "[CONFIG] Axis '%s': using option 'direct' discards option"
            "'framed'.",
            relabsd_axis_to_name(axis)
         );
      }
   }
   else if (strcmp(name, "real_fuzz") == 0)
   {
      conf->axis[axis].option[RELABSD_REAL_FUZZ_OPTION] = 1;

      RELABSD_DEBUG
      (
         RELABSD_DEBUG_CONFIG,
         "Axis '%s' enabled option 'real_fuzz'.",
         relabsd_axis_to_name(axis)
      );
   }
   else if (strcmp(name, "framed") == 0)
   {
      conf->axis[axis].option[RELABSD_FRAMED_OPTION] = 1;

      RELABSD_DEBUG
      (
         RELABSD_DEBUG_CONFIG,
         "Axis '%s' enabled option 'framed'.",
         relabsd_axis_to_name(axis)
      );

      if (conf->axis[axis].option[RELABSD_DIRECT_OPTION])
      {
         RELABSD_WARNING
         (
            "[CONFIG] Axis '%s': using option 'direct' discards option"
            "'framed'.",
            relabsd_axis_to_name(axis)
         );
      }
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

static int parse_timeout_option
(
   struct relabsd_config * const conf,
   const char * const param
)
{
   int timeout_msec;
   const int prev_errno = errno;

   conf->enable_timeout = 1;

   errno = 0;

   timeout_msec = atoi(param);

   if (timeout_msec <= 0)
   {
      RELABSD_FATAL
      (
         "Illegal value for timeout \"%d\": accepted range is [1, %d].",
         timeout_msec,
         INT_MAX
      );

      return -1;
   }

   memset((void *) &(conf->timeout), 0, sizeof(struct timeval));

   conf->timeout.tv_sec = (time_t) (timeout_msec / 1000);

   conf->timeout.tv_usec =
      (
         ((suseconds_t) timeout_msec)
         * ((suseconds_t) 1000)
      );

   return 0;
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

   RELABSD_DEBUG
   (
      RELABSD_DEBUG_CONFIG,
      "Axis '%s': {min = %d; max = %d; fuzz = %d; flat = %d; resolution = %d}",
      buffer,
      conf->axis[axis].min,
      conf->axis[axis].max,
      conf->axis[axis].fuzz,
      conf->axis[axis].flat,
      conf->axis[axis].resolution
   );

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
   char buffer[(RELABSD_CONF_AXIS_CODE_SIZE + 1)];
   int continue_reading, prev_errno;

   buffer[RELABSD_CONF_AXIS_CODE_SIZE] = '\0';

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

   while
   (
      (continue_reading == 1)
      &&
      (
         fscanf
         (
            f,
            "%" RELABSD_TO_STRING(RELABSD_CONF_AXIS_CODE_SIZE) "s",
            buffer
         )
         != EOF
      )
   )
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

static void print_usage
(
   const char * const exec
)
{
   RELABSD_FATAL
   (
      "USAGE: %s input_device config_file [<OPTION>+]\n"
      "<OPTION>:\n"
      "\t [-n | --name] <relabsd_device_name>: Names the virtual device.\n"
      "\t [-t | --timeout] <timeout_in_ms>: Resets all enabled axes to zero"
      " <timeout_in_ms> milliseconds after the last event. Range for"
      " <timeout_in_ms>: [1, %d].\n"
      "Alternatively, the previous usage is still supported:\n"
      "\t%s input_device config_file [<relabsd_device_name>]\n"
      "However, with that usage, <relabsd_device_name> cannot start with '-'.",
      exec,
      INT_MAX,
      exec
   );
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
   if (argc < 3)
   {
      print_usage(argv[0]);

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

static int parse_options
(
   struct relabsd_config * const conf,
   int const argc,
   char * const * const argv
)
{
   int i;

   if ((argc == 4) && !RELABSD_IS_PREFIX("-", argv[3]))
   {
      /* Old usage */

      RELABSD_S_DEBUG(RELABSD_DEBUG_CONFIG, "Old usage detected.");

      conf->device_name = argv[3];

      RELABSD_DEBUG
      (
         RELABSD_DEBUG_CONFIG,
         "Virtual device name param set to '%s'.",
         conf->device_name
      );

      return 0;
   }

   conf->device_name = NULL;
   conf->enable_timeout = 0;

   for (i = 3; i < argc; ++i)
   {
      if
      (
         RELABSD_STRING_EQUALS("-n", argv[i])
         || RELABSD_STRING_EQUALS("--name", argv[i])
      )
      {
         i += 1;

         if (i >= argc)
         {
            RELABSD_S_FATAL
            (
               "Missing value for parameter \"--name\"."
            );

            print_usage(argv[0]);

            return -1;
         }

         conf->device_name = argv[i];
      }
      else if
      (
         RELABSD_STRING_EQUALS("-t", argv[i])
         || RELABSD_STRING_EQUALS("--timeout", argv[i])
      )
      {
         i += 1;

         if (i >= argc)
         {
            RELABSD_S_FATAL
            (
               "Missing value for parameter \"--timeout\"."
            );

            print_usage(argv[0]);

            return -1;
         }

         if (parse_timeout_option(conf, argv[i]) < 0)
         {
            return -1;
         }
      }
      else
      {
         RELABSD_FATAL
         (
            "Unknown parameter \"%s\".",
            argv[i]
         );

         print_usage(argv[0]);

         return -1;
      }
   }

   return 0;
}

int relabsd_config_parse
(
   struct relabsd_config * const conf,
   int const argc,
   char * const * const argv
)
{
   RELABSD_S_DEBUG(RELABSD_DEBUG_PROGRAM_FLOW, "Parsing config & params...");

   if (check_usage(argc, argv) < 0)
   {
      return -1;
   }

   if (parse_options(conf, argc, argv) < 0)
   {
      return -1;
   }

   conf->input_file = argv[1];

   RELABSD_DEBUG
   (
      RELABSD_DEBUG_CONFIG,
      "Using configuration file '%s'.",
      conf->input_file
   );

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

/******************************************************************************/
/**** EXPORTED FUNCTIONS ******************************************************/
/******************************************************************************/
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
