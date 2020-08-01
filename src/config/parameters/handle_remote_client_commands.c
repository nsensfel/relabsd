/**** POSIX *******************************************************************/
/*
 * To get the POSIX 'getline' function.
 * We don't know what POSIX version is set by default.
 */
#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**** RELABSD *****************************************************************/
#include <relabsd/debug.h>
#include <relabsd/server.h>

#include <relabsd/util/string.h>

#include <relabsd/config/parameters.h>

/******************************************************************************/
/**** TYPES *******************************************************************/
/******************************************************************************/
struct relabsd_parameters_client_input
{
   FILE * socket_as_file;
   ssize_t size;
   char * buffer;
   size_t buffer_size;
};

/******************************************************************************/
/**** LOCAL FUNCTIONS *********************************************************/
/******************************************************************************/
static int get_next_argument
(
   struct relabsd_parameters_client_input input [const restrict static 1]
)
{
   errno = 0;
   input->size =
      getline
      (
         &(input->buffer),
         &(input->buffer_size),
         input->socket_as_file
      );

   if (input->size < 1)
   {
      RELABSD_ERROR
      (
         "Unable to read line from client socket: %s.",
         strerror(errno)
      );

      return -1;
   }

   /*
    * size is "not including the terminating null byte", so it's the '\n'
    * character.
    */
   input->buffer[(input->size - 1)] = '\0';

   return 0;
}

static void initialize_client_input
(
   FILE socket_as_file [const static 1],
   struct relabsd_parameters_client_input input [const restrict static 1]
)
{
   input->socket_as_file = socket_as_file;
   input->size = 0;
   input->buffer = (char *) NULL;
   input->buffer_size = 0;
}

static void finalize_client_input
(
   struct relabsd_parameters_client_input input [const restrict static 1]
)
{
   free((void *) input->buffer);

   input->buffer = (char *) NULL;
   input->buffer_size = 0;
}

static int handle_timeout_change
(
   struct relabsd_parameters_client_input input [const restrict static 1],
   struct relabsd_parameters parameters [const restrict static 1]
)
{
   int timeout_msec;

   if (get_next_argument(input) < 0)
   {
      RELABSD_S_ERROR("Could not get timeout value from client.");

      return -1;
   }

   if (relabsd_util_parse_int(input->buffer, 0, INT_MAX, &timeout_msec) < 0)
   {
      RELABSD_S_ERROR("Invalid timeout value from client.");

      return -1;
   }

   relabsd_parameters_set_timeout(timeout_msec, parameters);

   return 0;
}

static int handle_name_change
(
   struct relabsd_parameters_client_input input [const restrict static 1],
   struct relabsd_parameters parameters [const restrict static 1]
)
{
   const char * device_name;

   if (get_next_argument(input) < 0)
   {
      RELABSD_S_ERROR("Could not get device name value from client.");

      return -1;
   }

   device_name =
      (const char *) calloc((size_t) (input->size - 1), sizeof(char));

   if (device_name == (const char *) NULL)
   {
      RELABSD_S_ERROR
      (
         "Could not allocate memory to store the device name requested by"
         " the client."
      );

      return -1;
   }

   (void) memcpy
   (
      (void *) device_name,
      (const void *) input->buffer,
      (size_t) (input->size - 1)
   );

   if (parameters->device_name_was_modified)
   {
      free((void *) parameters->device_name);
   }

   parameters->device_name = device_name;
   parameters->device_name_was_modified = 1;

   return 0;
}

static int handle_axis_mod
(
   struct relabsd_parameters_client_input input [const restrict static 1],
   struct relabsd_parameters parameters [const static 1]
)
{
   enum relabsd_axis_name axis_name;
   int * value_to_modify;
   int min_value;
   int input_value;

   if (get_next_argument(input) < 0)
   {
      RELABSD_S_ERROR("Could not get name of axis to modify from client.");

      return -1;
   }

   axis_name = relabsd_axis_parse_name(input->buffer);

   if (axis_name == RELABSD_UNKNOWN)
   {
      RELABSD_ERROR
      (
         "Client requested modification on unknown axis \"%s\".",
         input->buffer
      );

      return -1;
   }

   if (get_next_argument(input) < 0)
   {
      RELABSD_S_ERROR("Could not get parameter of axis to modify from client.");

      return -1;
   }

   if (RELABSD_STRING_EQUALS("min", input->buffer))
   {
      value_to_modify = &(parameters->axes[axis_name].min);
      min_value = INT_MIN;
   }
   else if (RELABSD_STRING_EQUALS("max", input->buffer))
   {
      value_to_modify = &(parameters->axes[axis_name].max);
      min_value = INT_MIN;
   }
   else if (RELABSD_STRING_EQUALS("fuzz", input->buffer))
   {
      value_to_modify = &(parameters->axes[axis_name].fuzz);
      min_value = 0;
   }
   else if (RELABSD_STRING_EQUALS("flat", input->buffer))
   {
      value_to_modify = &(parameters->axes[axis_name].flat);
      min_value = 0;
   }
   else if (RELABSD_STRING_EQUALS("resolution", input->buffer))
   {
      value_to_modify = &(parameters->axes[axis_name].resolution);
      min_value = 0;
   }
   else
   {
      RELABSD_ERROR
      (
         "Client requested modification on unknown axis parameter \"%s\".",
         input->buffer
      );

      return -1;
   }

   if (get_next_argument(input) < 0)
   {
      RELABSD_S_ERROR
      (
         "Could not get new value for axis modification requested by client."
      );

      return -1;
   }

   if
   (
      relabsd_util_parse_int
      (
         (input->buffer + 1),
         min_value,
         INT_MAX,
         &input_value
      )
      < 0
   )
   {
      RELABSD_ERROR
      (
         "Invalid value \"%s\"for axis modification requested by client.",
         (input->buffer + 1)
      );

      return -1;
   }

   switch (input->buffer[0])
   {
      case '=':
         *value_to_modify = input_value;
         break;

      case '-':
         if (input_value == INT_MIN)
         {
            input_value = INT_MAX;
         }
         else
         {
            input_value = -input_value;
         }
         /* fall through */
      case '+':
         if
         (
            (
               (input_value > 0)
               && (*value_to_modify > (INT_MAX - input_value))
            )
            ||
            (
               (input_value < 0)
               && (*value_to_modify < (min_value - input_value))
            )
         )
         {
            RELABSD_S_WARNING
            (
               "Client request would make axis parameter over/underflow."
            );

            *value_to_modify = (input_value < 0) ? min_value : INT_MAX;
         }
         else
         {
            *value_to_modify += input_value;
         }
         break;
   }

   parameters->axes[axis_name].previous_value = 0;
   parameters->axes[axis_name].attributes_were_modified = 1;

   return 0;
}

static int handle_option_toggle
(
   struct relabsd_parameters_client_input input [const restrict static 1],
   struct relabsd_parameters parameters [const restrict static 1]
)
{
   enum relabsd_axis_name axis_name;

   if (get_next_argument(input) < 0)
   {
      RELABSD_S_ERROR("Could not get name of axis to modify from client.");

      return -1;
   }

   axis_name = relabsd_axis_parse_name(input->buffer);

   if (axis_name == RELABSD_UNKNOWN)
   {
      RELABSD_ERROR
      (
         "Client requested modification on unknown axis \"%s\".",
         input->buffer
      );

      return -1;
   }

   if (get_next_argument(input) < 0)
   {
      RELABSD_S_ERROR("Could not get option of axis to modify from client.");

      return -1;
   }

   if (RELABSD_STRING_EQUALS("framed", input->buffer))
   {
      parameters->axes[axis_name].flags[RELABSD_FRAMED] ^= 1;
   }
   else if (RELABSD_STRING_EQUALS("direct", input->buffer))
   {
      parameters->axes[axis_name].flags[RELABSD_DIRECT] ^= 1;
   }
   else if (RELABSD_STRING_EQUALS("real_fuzz", input->buffer))
   {
      parameters->axes[axis_name].flags[RELABSD_REAL_FUZZ] ^= 1;
   }
   else if (RELABSD_STRING_EQUALS("invert", input->buffer))
   {
      parameters->axes[axis_name].flags[RELABSD_INVERT] ^= 1;
   }
   else if (RELABSD_STRING_EQUALS("not_abs", input->buffer))
   {
      parameters->axes[axis_name].flags[RELABSD_NOT_ABS] ^= 1;
   }
   else if (RELABSD_STRING_EQUALS("enable", input->buffer))
   {
      parameters->axes[axis_name].is_enabled ^= 1;
   }
   else if (RELABSD_IS_PREFIX("convert_to=", input->buffer))
   {
      relabsd_axis_enable_option_from_name
      (
         input->buffer,
         relabsd_axis_name_to_string(axis_name),
         (parameters->axes + axis_name)
      );
   }
   else
   {
      RELABSD_ERROR
      (
         "Client requested toggle of unknown axis option \"%s\".",
         input->buffer
      );

      return -1;
   }

   parameters->axes[axis_name].previous_value = 0;

   return 0;
}

static int handle_inputs
(
   struct relabsd_parameters_client_input input [const restrict static 1],
   struct relabsd_parameters parameters [const restrict static 1]
)
{
   for (;;)
   {
      if (get_next_argument(input) < 0)
      {
         RELABSD_S_ERROR("Could not get next client command.");

         return -1;
      }

      if ((input->buffer[0] == '\n') || (input->buffer[0] == '\0'))
      {
         return 0;
      }
      else if
      (
         RELABSD_STRING_EQUALS("-q", input->buffer)
         || RELABSD_STRING_EQUALS("--quit", input->buffer)
      )
      {
         relabsd_server_interrupt();
      }
      else if
      (
         RELABSD_STRING_EQUALS("-t", input->buffer)
         || RELABSD_STRING_EQUALS("--timeout", input->buffer)
      )
      {
         if (handle_timeout_change(input, parameters) < 0)
         {
            return -1;
         }
      }
      else if
      (
         RELABSD_STRING_EQUALS("-n", input->buffer)
         || RELABSD_STRING_EQUALS("--name", input->buffer)
      )
      {
         if (handle_name_change(input, parameters) < 0)
         {
            return -1;
         }
      }
      else if
      (
         RELABSD_STRING_EQUALS("-m", input->buffer)
         || RELABSD_STRING_EQUALS("--mod-axis", input->buffer)
      )
      {
         if (handle_axis_mod(input, parameters) < 0)
         {
            return -1;
         }
      }
      else if
      (
         RELABSD_STRING_EQUALS("-o", input->buffer)
         || RELABSD_STRING_EQUALS("--toggle-option", input->buffer)
      )
      {
         if (handle_option_toggle(input, parameters) < 0)
         {
            return -1;
         }
      }
      else
      {
         RELABSD_ERROR("Unknown client command \"%s\"", input->buffer);

         return -1;
      }
   }
}

/******************************************************************************/
/**** EXPORTED FUNCTIONS ******************************************************/
/******************************************************************************/
int relabsd_parameters_handle_remote_client
(
   FILE socket_as_file [const static 1],
   struct relabsd_parameters parameters [const restrict static 1]
)
{
   struct relabsd_parameters_client_input input;

   initialize_client_input(socket_as_file, &input);

   (void) handle_inputs(&input, parameters);

   finalize_client_input(&input);

   return 0;
}
