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
static void print_usage (const char exec [const restrict static 1])
{
   printf
   (
      "USAGE: %s <MODE> [<OPTION>+]\n\n"

      "<MODE>:\n"
      "\t[-? | --compatible] <physical_device_file>:\n"
         "\t\tDevice compatibility test.\n\n"

      "\t[-c | --client] <server_file>:\n"
         "\t\tSends the commands to a given server instance.\n\n"

      "\t[-s | --server] <server_file> <physical_device_file>:\n"
         "\t\tCreates a named server instance.\n\n"

      "\t[-1 | --self] <physical_device_file>:\n"
         "\t\tCreates a unnamed server instance.\n\n"

      "<OPTION>:\n"
      "\t[-d | --daemon]:\n"
         "\t\tRuns server instance in the background.\n\n"

      "\t[-n | --name] <relabsd_device_name>:\n"
         "\t\tNames the virtual device.\n\n"

      "\t[-t | --timeout] <timeout_in_ms>:\n"
         "\t\tSets a zeroing timeout (0 to disable).\n\n"

      "\t[-a | --axis] <name> <min> <max> <fuzz> <flat> <resolution> "
         "<options>:\n"
         "\t\t(Re)defines an axis.\n\n"

      "\t[-m | --mod-axis] <name> <min> <max> <fuzz> <flat> <resolution> "
         "<signed_options>:\n"
         "\t\tModifies an axis (use + and - signs for the options).\n\n"

      "\t[-f | --config] <config_file>"
         "<options>:\n"
         "\t\t(Re)defines an axis.\n",
      exec
   );
}

static int parse_axis
(
   const int argc,
   const char * argv [const restrict static argc],
   struct relabsd_axis axes [const restrict static RELABSD_AXIS_AXES_COUNT]
)
{
   enum relabsd_axis_name axis_name;
   struct relabsd_axis *axis;

   if (argc < 7)
   {
      RELABSD_S_FATAL("7 parameters must be provided for axis definition.");

      return -1;
   }

   axis_index = relabsd_axis_from_name(argv[0]);

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
   const char * argv [const restrict static argc],
   struct relabsd_parameters params [const restrict static 1]
)
{
   RELABSD_S_DEBUG(RELABSD_DEBUG_PROGRAM_FLOW, "Parsing exec mode...");

   if (argc < 3)
   {
      print_usage(argv[0]);

      return -1;
   }

   if
   (
      RELABSD_STRING_EQUALS("-?", argv[1])
      || RELABSD_STRING_EQUALS("--compatibility", argv[1])
   )
   {
      params->mode = RELABSD_PARAMETERS_COMPATIBILITY_TEST_MODE;
      params->physical_device_name = argv[2];
      params->read_argc = 2;
   }
   else if
   (
      RELABSD_STRING_EQUALS("-c", argv[1])
      || RELABSD_STRING_EQUALS("--client", argv[1])
   )
   {
      params->mode = RELABSD_PARAMETERS_CLIENT_MODE;
      params->communication_node_name = argv[2];
      params->physical_device_name = (char *) NULL;
      params->read_argc = 2;
   }
   else if
   (
      RELABSD_STRING_EQUALS("-s", argv[1])
      || RELABSD_STRING_EQUALS("--server", argv[1])
   )
   {
      params->mode = RELABSD_PARAMETERS_SERVER_MODE;
      params->communication_node_name = argv[2];
      params->physical_device_name = argv[3];
      params->read_argc = 3;
   }
   else if
   (
      RELABSD_STRING_EQUALS("-1", argv[1])
      || RELABSD_STRING_EQUALS("--self", argv[1])
   )
   {
      params->mode = RELABSD_PARAMETERS_SERVER_MODE;
      params->communication_node_name = (char *) NULL;
      params->physical_device_name = argv[2];
      params->read_argc = 2;
   }
   else
   {
      print_usage(argv[0]);

      return -1;
   }

   RELABSD_S_DEBUG(RELABSD_DEBUG_PROGRAM_FLOW, "Parsed exec mode.");

   return 0;
}

int relabsd_parameters_parse_options
(
   const int argc,
   const char * argv [const restrict static argc],
   struct relabsd_parameters params [const restrict static 1]
)
{
   int i;

   RELABSD_S_DEBUG(RELABSD_DEBUG_PROGRAM_FLOW, "Parsing options...");

   set_default_options(params);

   /*
    * i = (params->read_argc + 1) because reading 2 params is actually reaching
    * the [2] element of the array, since the [0] element is the executable
    * name.
    */
   for (i = (params->read_argc + 1); i < argc; ++i)
   {
      if
      (
         RELABSD_STRING_EQUALS("-d", argv[i])
         || RELABSD_STRING_EQUALS("--daemon", argv[i])
      )
      {
         params->run_as_daemon = 1;

         if (params->node == ((char *) NULL))
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
         params->name = argv[i];
      }
      else if
      (
         RELABSD_STRING_EQUALS("-t", argv[i])
         || RELABSD_STRING_EQUALS("--timeout", argv[i])
      )
      {
         if (argc == i)
         {
            RELABSD_FATAL("Missing value for \"%s\" <OPTION>.", argv[i]);
            print_usage(argv[0]);

            return -1;
         }

         ++i;

         if
         (
            relabsd_util_parse_int(argv[i], 0, INT_MAX, &(params->timeout))
            < 0
         )
         {
            RELABSD_FATAL
            (
               "Invalid value for \"%s\" <OPTION> (valid range is [%d, %d]).",
               argv[i - 1],
               0,
               INT_MAX
            );

            print_usage(argv[0]);

            return -1;
         }
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
            print_usage(argv[0]);

            return -1;
         }

         ++i;

         if (parse_axis((argc - i), (argv + i), params->axes) < 0)
         {
            print_usage(argv[0]);

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
            print_usage(argv[0]);

            return -1;
         }

         ++i;
         params->config_file = argv[i];
      }
      else
      {
         RELABSD_FATAL("Unknown <OPTION> \"%s\".", argv[i]);
         print_usage(argv[0]);

         return -1;
      }
   }

   RELABSD_S_DEBUG(RELABSD_DEBUG_PROGRAM_FLOW, "Parsed options.");

   return 0;
}
