/**** POSIX *******************************************************************/
#include <limits.h>

/**** RELABSD *****************************************************************/
#include <relabsd/config.h>
#include <relabsd/debug.h>

#include <relabsd/device/axis.h>

#include <relabsd/util/string.h>

#include <relabsd/config/parameters.h>

/******************************************************************************/
/**** LOCAL FUNCTIONS *********************************************************/
/******************************************************************************/
static int parse_axis
(
   const int argc,
   const char * const argv [const restrict static argc],
   struct relabsd_axis axes [const static RELABSD_AXIS_VALID_AXES_COUNT]
)
{
   enum relabsd_axis_name axis_index;
   struct relabsd_axis *axis;

   if (argc < 7)
   {
      RELABSD_S_FATAL("7 parameters must be provided for axis definition.");

      return -1;
   }

   axis_index = relabsd_axis_parse_name(argv[0]);

   if (axis_index == RELABSD_UNKNOWN)
   {
      RELABSD_FATAL("Unknown axis \"%s\".", argv[0]);

      return -1;
   }

   axis = (axes + axis_index);

   if (relabsd_util_parse_int(argv[1], INT_MIN, INT_MAX, &(axis->min)) < 0)
   {
      RELABSD_FATAL("Invalid <min> value for axis \"%s\".", argv[0]);

      return -1;
   }

   if (relabsd_util_parse_int(argv[2], INT_MIN, INT_MAX, &(axis->max)) < 0)
   {
      RELABSD_FATAL("Invalid <max> value for axis \"%s\".", argv[0]);

      return -1;
   }

   if (relabsd_util_parse_int(argv[3], INT_MIN, INT_MAX, &(axis->fuzz)) < 0)
   {
      RELABSD_FATAL("Invalid <fuzz> value for axis \"%s\".", argv[0]);

      return -1;
   }

   if (relabsd_util_parse_int(argv[4], INT_MIN, INT_MAX, &(axis->flat)) < 0)
   {
      RELABSD_FATAL("Invalid <flat> value for axis \"%s\".", argv[0]);

      return -1;
   }

   if
   (
      relabsd_util_parse_int(argv[5], INT_MIN, INT_MAX, &(axis->resolution))
      < 0
   )
   {
      RELABSD_FATAL("Invalid <resolution> value for axis \"%s\".", argv[0]);

      return -1;
   }

   return 0;
}

/******************************************************************************/
/**** EXPORTED FUNCTIONS ******************************************************/
/******************************************************************************/
int relabsd_parameters_parse_execution_mode
(
   const int argc,
   const char * const argv [const restrict static argc],
   struct relabsd_parameters parameters [const restrict static 1]
)
{
   RELABSD_S_DEBUG(RELABSD_DEBUG_PROGRAM_FLOW, "Parsing exec mode...");

   if (argc < 3)
   {
      relabsd_parameters_print_usage(argv[0]);

      return -1;
   }

   relabsd_parameters_initialize_options(parameters);

   if
   (
      RELABSD_STRING_EQUALS("-?", argv[1])
      || RELABSD_STRING_EQUALS("--compatibility", argv[1])
   )
   {
      parameters->mode = RELABSD_PARAMETERS_COMPATIBILITY_TEST_MODE;
      parameters->physical_device_file_name = argv[2];
      parameters->read_argc = 2;
   }
   else if
   (
      RELABSD_STRING_EQUALS("-c", argv[1])
      || RELABSD_STRING_EQUALS("--client", argv[1])
   )
   {
      parameters->mode = RELABSD_PARAMETERS_CLIENT_MODE;
      parameters->communication_node_name = argv[2];
      parameters->physical_device_file_name = (const char *) NULL;
      parameters->read_argc = 2;
   }
   else if
   (
      RELABSD_STRING_EQUALS("-s", argv[1])
      || RELABSD_STRING_EQUALS("--server", argv[1])
   )
   {
      parameters->mode = RELABSD_PARAMETERS_SERVER_MODE;
      parameters->communication_node_name = argv[2];
      parameters->physical_device_file_name = argv[3];
      parameters->read_argc = 3;
   }
   else if
   (
      RELABSD_STRING_EQUALS("-1", argv[1])
      || RELABSD_STRING_EQUALS("--self", argv[1])
   )
   {
      parameters->mode = RELABSD_PARAMETERS_SERVER_MODE;
      parameters->communication_node_name = (char *) NULL;
      parameters->physical_device_file_name = argv[2];
      parameters->read_argc = 2;
   }
   else if (argc == 3)
   {
      parameters->mode = RELABSD_PARAMETERS_SERVER_MODE;
      parameters->communication_node_name = (char *) NULL;
      parameters->physical_device_file_name = argv[1];
      parameters->read_argc = 2;

      if (relabsd_parameters_parse_config_file(argv[2], parameters) < 0)
      {
         return -1;
      }
   }
   else if (argc == 4)
   {
      parameters->mode = RELABSD_PARAMETERS_SERVER_MODE;
      parameters->communication_node_name = (char *) NULL;
      parameters->physical_device_file_name = argv[1];

      if (relabsd_parameters_parse_config_file(argv[2], parameters) < 0)
      {
         return -1;
      }

      parameters->device_name = argv[3];
      parameters->read_argc = 3;
   }
   else
   {
      relabsd_parameters_print_usage(argv[0]);

      return -1;
   }

   RELABSD_S_DEBUG(RELABSD_DEBUG_PROGRAM_FLOW, "Parsed exec mode.");

   return 0;
}

int relabsd_parameters_parse_options
(
   const int argc,
   const char * const argv [const restrict static argc],
   struct relabsd_parameters parameters [const restrict static 1]
)
{
   int i;

   RELABSD_S_DEBUG(RELABSD_DEBUG_PROGRAM_FLOW, "Parsing options...");

   /*
    * i = (parameters->read_argc + 1) because reading 2 params is actually
    * reaching the [2] element of the array, since the [0] element is the
    * executable name.
    */
   for (i = (parameters->read_argc + 1); i < argc; ++i)
   {
      if
      (
         RELABSD_STRING_EQUALS("-d", argv[i])
         || RELABSD_STRING_EQUALS("--daemon", argv[i])
      )
      {
         parameters->run_as_daemon = 1;

         if (parameters->communication_node_name == ((char *) NULL))
         {
            RELABSD_S_WARNING
            (
               "Running as a daemon without any communication file."
            );
         }
      }
      else if
      (
         RELABSD_STRING_EQUALS("-n", argv[i])
         || RELABSD_STRING_EQUALS("--name", argv[i])
      )
      {
         if (argc == i)
         {
            RELABSD_FATAL("Missing value for \"%s\" <OPTION>.", argv[i]);

            return -1;
         }

         ++i;
         parameters->device_name = argv[i];
      }
      else if
      (
         RELABSD_STRING_EQUALS("-v", argv[i])
         || RELABSD_STRING_EQUALS("--verbose", argv[i])
      )
      {
         relabsd_debug_toggle_real_event();
         relabsd_debug_toggle_virtual_event();
      }
      else if
      (
         RELABSD_STRING_EQUALS("-vv", argv[i])
         || RELABSD_STRING_EQUALS("--very-verbose", argv[i])
      )
      {
         relabsd_debug_toggle_config();
         relabsd_debug_toggle_program_flow();
         relabsd_debug_toggle_real_event();
         relabsd_debug_toggle_virtual_event();
      }
      else if
      (
         RELABSD_STRING_EQUALS("-t", argv[i])
         || RELABSD_STRING_EQUALS("--timeout", argv[i])
      )
      {
         int timeout;

         if (argc == i)
         {
            RELABSD_FATAL("Missing value for \"%s\" <OPTION>.", argv[i]);
            relabsd_parameters_print_usage(argv[0]);

            return -1;
         }

         ++i;

         if (relabsd_util_parse_int(argv[i], 0, INT_MAX, &timeout) < 0)
         {
            RELABSD_FATAL
            (
               "Invalid value for \"%s\" <OPTION> (valid range is [%d, %d]).",
               argv[i - 1],
               0,
               INT_MAX
            );

            relabsd_parameters_print_usage(argv[0]);

            return -1;
         }

         relabsd_parameters_set_timeout(timeout, parameters);
      }
      else if
      (
         RELABSD_STRING_EQUALS("-a", argv[i])
         || RELABSD_STRING_EQUALS("--axis", argv[i])
      )
      {
         if (argc == i)
         {
            RELABSD_FATAL("Missing values for \"%s\" <OPTION>.", argv[i]);
            relabsd_parameters_print_usage(argv[0]);

            return -1;
         }

         ++i;

         if (parse_axis((argc - i), (argv + i), parameters->axes) < 0)
         {
            relabsd_parameters_print_usage(argv[0]);

            return -1;
         }
      }
      else if
      (
         RELABSD_STRING_EQUALS("-f", argv[i])
         || RELABSD_STRING_EQUALS("--config", argv[i])
      )
      {
         if (argc == i)
         {
            RELABSD_FATAL("Missing value for \"%s\" <OPTION>.", argv[i]);
            relabsd_parameters_print_usage(argv[0]);

            return -1;
         }

         ++i;

         if (relabsd_parameters_parse_config_file(argv[i], parameters) < 0)
         {
            return -1;
         }
      }
      else if
      (
         RELABSD_STRING_EQUALS("-m", argv[i])
         || RELABSD_STRING_EQUALS("--mod-axis", argv[i])
         || RELABSD_STRING_EQUALS("-o", argv[i])
         || RELABSD_STRING_EQUALS("--toggle-option", argv[i])
         ||RELABSD_STRING_EQUALS("-q", argv[i])
         || RELABSD_STRING_EQUALS("--quit", argv[i])
      )
      {
         RELABSD_FATAL("\"%s\" is not available in this mode.", argv[i]);
         relabsd_parameters_print_usage(argv[0]);

         return -1;
      }
      else
      {
         RELABSD_FATAL("Unknown <OPTION> \"%s\".", argv[i]);
         relabsd_parameters_print_usage(argv[0]);

         return -1;
      }
   }

   RELABSD_S_DEBUG(RELABSD_DEBUG_PROGRAM_FLOW, "Parsed options.");

   return 0;
}

int relabsd_parameters_argument_count_for
(
   const char option [const restrict static 1],
   int result [const restrict static 1]
)
{
   if
   (
      RELABSD_STRING_EQUALS("-q", option)
      || RELABSD_STRING_EQUALS("--quit", option)
   )
   {
      *result = 0;
   }
   else if
   (
      RELABSD_STRING_EQUALS("-n", option)
      || RELABSD_STRING_EQUALS("--name", option)
   )
   {
      *result = 1;
   }
   else if
   (
      RELABSD_STRING_EQUALS("-v", option)
      || RELABSD_STRING_EQUALS("--verbose", option)
   )
   {
      *result = 0;
   }
   else if
   (
      RELABSD_STRING_EQUALS("-vv", option)
      || RELABSD_STRING_EQUALS("--very-verbose", option)
   )
   {
      *result = 0;
   }
   else if
   (
      RELABSD_STRING_EQUALS("-t", option)
      || RELABSD_STRING_EQUALS("--timeout", option)
   )
   {
      *result = 1;
   }
   else if
   (
      RELABSD_STRING_EQUALS("-m", option)
      || RELABSD_STRING_EQUALS("--mod-axis", option)
   )
   {
      *result = 3;
   }
   else if
   (
      RELABSD_STRING_EQUALS("-o", option)
      || RELABSD_STRING_EQUALS("--toggle-option", option)
   )
   {
      *result = 2;
   }
   else if
   (
      RELABSD_STRING_EQUALS("-d", option)
      || RELABSD_STRING_EQUALS("--daemon", option)
      || RELABSD_STRING_EQUALS("-f", option)
      || RELABSD_STRING_EQUALS("--config", option)
      || RELABSD_STRING_EQUALS("-a", option)
      || RELABSD_STRING_EQUALS("--axis", option)
   )
   {
      RELABSD_ERROR("\"%s\" is not available in this mode.", option);
      relabsd_parameters_print_usage("relabsd");
   }
   else
   {
      return -1;
   }

   return 0;
}

void relabsd_parameters_print_usage (const char exec [const restrict static 1])
{
   printf
   (
      "USAGES:\n"
         "\t%s <physical_device_file> <config_file>\n"
         "\t%s <physical_device_file> <config_file> <relabsd_device_name>\n"
         "\t\tLegacy usage.\n\n"

         "\t%s [-? | --compatible] <physical_device_file> [<CONF_OPTION>+]\n"
         "\t\tDevice & configuration compatibility test.\n\n"

         "\t%s [-c | --client] <server_file> "
            "[(<CLIENT_OPTION>|<GLOBAL_CONF_OPTION>)+]\n"
         "\t\tSends the commands to a given server instance.\n\n"

         "\t%s [-s | --server] <server_file> <physical_device_file>"
         " [(<SERVER_OPTION>|<CONF_OPTION>)+]\n"
            "\t\tCreates a named server instance.\n\n"

         "\t%s [-1 | --self] <physical_device_file>"
         " [(<SERVER_OPTION>|<CONF_OPTION>)+]\n"
            "\t\tCreates an unnamed server instance.\n\n"

      "<GLOBAL_CONF_OPTION>:\n"
      "\t[-n | --name] <relabsd_device_name>\n"
         "\t\tNames the virtual device.\n\n"

      "\t[-t | --timeout] <timeout_in_ms>\n"
         "\t\tSets a zeroing timeout (0 to disable).\n\n"

      "\t[-v | --verbose]\n"
         "\t\tPrint incoming and outgoing events to stdout.\n\n"

      "\t[-vv | --very-verbose]\n"
         "\t\tPrint program flow, incoming and outgoing events to stdout.\n\n"

      "<CONF_OPTION>:\n"
      "\t<GLOBAL_CONF_OPTION>\n\n"

      "\t[-a | --axis] <name> <min> <max> <fuzz> <flat> <resolution> "
         "<options>\n"
         "\t\t(Re)defines an axis.\n\n"

      "\t[-f | --config] <config_file>\n"
         "\t\tUse the options defined in <config_file>.\n\n"

      "<SERVER_OPTION>:\n"
      "\t[-d | --daemon]\n"
         "\t\tRuns server instance in the background.\n\n"

      "<CLIENT_OPTION>:\n"
      "\t[-q | --quit]\n"
         "\t\tTerminates the targeted server instance.\n\n"

      "\t[-m | --mod-axis] <axis_name> "
         "[min|max|fuzz|flat|resolution] [+|-|=]<value>\n"
         "\t\tModifies an axis.\n\n"

      "\t[-o | --toggle-option] <axis_name> "
         "[direct|real_fuzz|framed|enable|invert|not_abs|convert_to=<axis_name>]\n"
         "\t\tToggles or sets an axis option.\n",
      exec,
      exec,
      exec,
      exec,
      exec,
      exec
   );
}
