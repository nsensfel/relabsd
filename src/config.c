#include <errno.h>
#include <string.h>

#include "error.h"
#include "pervasive.h"
#include "axis.h"
#include "config.h"

static int reach_next_line_or_eof (FILE * const f)
{
   char c;
   c = getc(f);

   while ((c != '\n') && c != EOF)
   {
      c = getc(f);
   }

   if (c == EOF)
   {
      return -1;
   }

   return 0;
}

static int all_axis_are_configured (const int * const axis_is_configured)
{
   int i;

   for (i = 0; i < 6; ++i)
   {
      if (axis_is_configured[i] == 0)
      {
         _FATAL
         (
            "[CONFIG] Axis '%s' is not configured.",
            relabsd_axis_enum_to_name((enum relabsd_axis) i)
         );

         return -1;
      }
   }

   return 0;
}

static int parse_axis_configuration_line
(
   struct relabsd_config * const conf,
   FILE * const f,
   int * const axis_is_configured,
   char * const buffer
)
{
   int valc;
   enum relabsd_axis axis;

   axis = relabsd_axis_name_to_enum(buffer);

   if (axis == RELABSD_UNKNOWN)
   {
      _FATAL
      (
         "[CONFIG] Unknown axis '%s'.",
         buffer
      );

      return -1;
   }

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
      _FATAL
      (
         "[CONFIG] Unexpected end of file while reading axis '%s'.",
         buffer
      );

      return -1;
   }
   else if (valc < 5)
   {
      _FATAL
      (
         "[CONFIG] Invalid parameter count for axis '%s'.",
         buffer
      );

      return -1;
   }

   axis_is_configured[axis] = 1;
   return 0;
}

static int read_config_file
(
   struct relabsd_config * const conf,
   char * const filename
)
{
   FILE * f;
   int axis_is_configured[6];
   char buffer[3];
   int test;

   buffer[2] = '\0';

   memset(axis_is_configured, 0, 6 * sizeof(int));

   f = fopen(filename, "r");

   if (f == (FILE *) NULL)
   {
      _FATAL
      (
         "[CONFIG] Could not open file: %s.",
         strerror(errno)
      );

      return -1;
   }

   errno = 0;

   while ((test = fscanf(f, "%2s", buffer)) > 0)
   {
      if (_IS_PREFIX("#", buffer))
      {
         if (reach_next_line_or_eof(f) < 0)
         {
            break;
         }
      }
      else
      {
         if
         (
            parse_axis_configuration_line
            (
               conf,
               f,
               axis_is_configured,
               buffer
            )
            < 0
         )
         {
            break;
         }

         if (reach_next_line_or_eof(f) < 0)
         {
            break;
         }
      }
   }

   if (test < 0 && errno != 0)
   {
      _FATAL
      (
         "[CONFIG] Error while reading file: %s, last read '%s'.",
         strerror(errno),
         buffer
      );

      fclose(f);

      return -1;
   }

   fclose(f);

   if (all_axis_are_configured(axis_is_configured) < 0)
   {
      return -1;
   }

   return 0;
}

int relabsd_config_parse
(
   struct relabsd_config * const conf,
   int const argc,
   char ** const argv
)
{
   if ((argc < 3) || (argc > 4))
   {
      _FATAL
      (
         "Usage: %s input_device config_file [<relabsd_device_name>]",
         argv[0]
      );

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

   if (read_config_file(conf, argv[2]) < 0)
   {
      return -1;
   }

   return 0;
}

int relabsd_config_allows
(
   const struct relabsd_config * const conf,
   enum relabsd_axis const axis,
   int const value
)
{
   return 1;
};

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
