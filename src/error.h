#ifndef RELABSD_ERROR_H
#define RELABSD_ERROR_H

#include <stdio.h>

#include "config.h"
#include "pervasive.h"


#ifndef RELABSD_HIGHEST_DEBUG_LVL
   #define RELABSD_HIGHEST_DEBUG_LVL         100
#endif

#ifndef RELABSD_DEBUG_LEVEL
   #define RELABSD_DEBUG_LVL                 0
#endif

#define RELABSD_ENABLE_WARNINGS_OUTPUT              1
#define RELABSD_ENABLE_RUNTIME_ERRORS_OUTPUT        1
#define RELABSD_ENABLE_PROGRAMMING_ERRORS_OUTPUT    1
#define RELABSD_ENABLE_FATAL_ERROR_OUTPUT           1

#ifdef RELABSD_ENABLE_ERROR_LOCATION
   #define RELABSD_LOCATION "[" __FILE__ "][" RELABSD_TO_STRING(__LINE__) "]"
#else
   #define RELABSD_LOCATION ""
#endif

#define RELABSD_PRINT_STDERR(symbol, str, ...)\
   fprintf(stderr, "[" symbol "]" RELABSD_LOCATION " " str "\n", __VA_ARGS__);

#define RELABSD_DEBUG(level, str, ...)\
   RELABSD_ISOLATE\
   (\
      if (level < RELABSD_DEBUG_LVL)\
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

#define RELABSD_S_DEBUG(level, str)\
   RELABSD_ISOLATE\
   (\
      if (level < RELABSD_DEBUG_LVL)\
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

#endif
