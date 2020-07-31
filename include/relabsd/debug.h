#pragma once

#include <stdio.h>

#include <relabsd/util/macro.h>

#ifndef RELABSD_DEBUG_PROGRAM_FLOW
   #define RELABSD_DEBUG_PROGRAM_FLOW 1
#endif

#ifndef RELABSD_DEBUG_CONFIG
   #define RELABSD_DEBUG_CONFIG 1
#endif

#ifndef RELABSD_DEBUG_REAL_EVENTS
   #define RELABSD_DEBUG_REAL_EVENTS 1
#endif

#ifndef RELABSD_DEBUG_VIRTUAL_EVENTS
   #define RELABSD_DEBUG_VIRTUAL_EVENTS 1
#endif

#ifndef RELABSD_ENABLE_WARNINGS_OUTPUT
#define RELABSD_ENABLE_WARNINGS_OUTPUT              1
#endif
#ifndef RELABSD_ENABLE_RUNTIME_ERRORS_OUTPUT
#define RELABSD_ENABLE_RUNTIME_ERRORS_OUTPUT        1
#endif
#ifndef RELABSD_ENABLE_PROGRAMMING_ERRORS_OUTPUT
#define RELABSD_ENABLE_PROGRAMMING_ERRORS_OUTPUT    1
#endif
#ifndef RELABSD_ENABLE_FATAL_ERROR_OUTPUT
#define RELABSD_ENABLE_FATAL_ERROR_OUTPUT           1
#endif

#ifdef RELABSD_ENABLE_ERROR_LOCATION
   #define RELABSD_LOCATION "[" __FILE__ "][" RELABSD_TO_STRING(__LINE__) "]"
#else
   #define RELABSD_LOCATION ""
#endif

#define RELABSD_PRINT_STDERR(symbol, str, ...)\
   fprintf(stderr, "[" symbol "]" RELABSD_LOCATION " " str "\n", __VA_ARGS__);

/*
 * Given that we use preprocessor contants as flags, we can expect the compilers
 * to remove the test condition for disabled flags. No need to be shy about
 * allowing many debug options.
 */

#define RELABSD_DEBUG(flag, str, ...)\
   RELABSD_ISOLATE\
   (\
      if (flag)\
      {\
         RELABSD_PRINT_STDERR("D", str, __VA_ARGS__);\
      }\
   )


#define RELABSD_WARNING(str, ...)\
   RELABSD_ISOLATE\
   (\
      if (RELABSD_ENABLE_WARNINGS_OUTPUT)\
      {\
         RELABSD_PRINT_STDERR("W", str, __VA_ARGS__);\
      }\
   )

#define RELABSD_ERROR(str, ...)\
   RELABSD_ISOLATE\
   (\
      if (RELABSD_ENABLE_RUNTIME_ERRORS_OUTPUT)\
      {\
         RELABSD_PRINT_STDERR("E", str, __VA_ARGS__);\
      }\
   )

#define RELABSD_PROG_ERROR(str, ...)\
   RELABSD_ISOLATE\
   (\
      if (RELABSD_ENABLE_PROGRAMMING_ERRORS_OUTPUT)\
      {\
         RELABSD_PRINT_STDERR("P", str, __VA_ARGS__);\
      }\
   )

#define RELABSD_FATAL(str, ...)\
   RELABSD_ISOLATE\
   (\
     if (RELABSD_ENABLE_FATAL_ERROR_OUTPUT)\
      {\
         RELABSD_PRINT_STDERR("F", str, __VA_ARGS__);\
      }\
   )

/* For outputs without dynamic content (static). ******************************/

#define RELABSD_PRINT_S_STDERR(symbol, str)\
   fprintf(stderr, "[" symbol "]" RELABSD_LOCATION " " str "\n");

#define RELABSD_S_DEBUG(flag, str)\
   RELABSD_ISOLATE\
   (\
      if (flag)\
      {\
         RELABSD_PRINT_S_STDERR("D", str);\
      }\
   )

#define RELABSD_S_WARNING(str)\
   RELABSD_ISOLATE\
   (\
      if (RELABSD_ENABLE_WARNINGS_OUTPUT)\
      {\
         RELABSD_PRINT_S_STDERR("W", str);\
      }\
   )

#define RELABSD_S_ERROR(str)\
   RELABSD_ISOLATE\
   (\
      if (RELABSD_ENABLE_RUNTIME_ERRORS_OUTPUT)\
      {\
         RELABSD_PRINT_S_STDERR("E", str);\
      }\
   )

#define RELABSD_S_PROG_ERROR(str)\
   RELABSD_ISOLATE\
   (\
      if (RELABSD_ENABLE_PROGRAMMING_ERRORS_OUTPUT)\
      {\
         RELABSD_PRINT_S_STDERR("P", str);\
      }\
   )

#define RELABSD_S_FATAL(str)\
   RELABSD_ISOLATE\
   (\
     if (RELABSD_ENABLE_FATAL_ERROR_OUTPUT)\
      {\
         RELABSD_PRINT_S_STDERR("F", str);\
      }\
   )
