#ifndef RELABSD_ERROR_H
#define RELABSD_ERROR_H

#include <stdio.h>

#include "config.h"
#include "pervasive.h"

#define _HIGHEST_DEBUG_LVL 100

#define _DEBUG_LVL                           _HIGHEST_DEBUG_LVL
#define _ENABLE_WARNINGS_OUTPUT              1
#define _ENABLE_RUNTIME_ERRORS_OUTPUT        1
#define _ENABLE_PROGRAMMING_ERRORS_OUTPUT    1
#define _ENABLE_FATAL_ERROR_OUTPUT           1
#define _ENABLE_ERROR_LOCATION               0

#if _ENABLE_ERROR_LOCATION
   #define _LOCATION "[" __FILE__ "][" _TO_STRING(__LINE__) "]"
#else
   #define _LOCATION ""
#endif

#define _PRINT_STDERR(symbol, str, ...)\
   fprintf(stderr, "[" symbol "]" _LOCATION " " str "\n", __VA_ARGS__);

#define _DEBUG(level, str, ...)\
   _ISOLATE\
   (\
      if (level < _DEBUG_LVL)\
      {\
         _PRINT_STDERR("D", str, __VA_ARGS__);\
      }\
   )


#define _WARNING(str, ...)\
   _ISOLATE\
   (\
      if (_ENABLE_WARNINGS_OUTPUT)\
      {\
         _PRINT_STDERR("W", str, __VA_ARGS__);\
      }\
   )

#define _ERROR(str, ...)\
   _ISOLATE\
   (\
      if (_ENABLE_RUNTIME_ERRORS_OUTPUT)\
      {\
         _PRINT_STDERR("E", str, __VA_ARGS__);\
      }\
   )

#define _PROG_ERROR(str, ...)\
   _ISOLATE\
   (\
      if (_ENABLE_PROGRAMMING_ERRORS_OUTPUT)\
      {\
         _PRINT_STDERR("P", str, __VA_ARGS__);\
      }\
   )

#define _FATAL(str, ...)\
   _ISOLATE\
   (\
     if (_ENABLE_FATAL_ERROR_OUTPUT)\
      {\
         _PRINT_STDERR("F", str, __VA_ARGS__);\
      }\
   )

/* For outputs without dynamic content (static). ******************************/

#define _PRINT_S_STDERR(symbol, str)\
   fprintf(stderr, "[" symbol "]" _LOCATION " " str "\n");

#define _S_DEBUG(level, str)\
   _ISOLATE\
   (\
      if (level < _DEBUG_LVL)\
      {\
         _PRINT_S_STDERR("D", str);\
      }\
   )

#define _S_WARNING(str)\
   _ISOLATE\
   (\
      if (_ENABLE_WARNINGS_OUTPUT)\
      {\
         _PRINT_S_STDERR("W", str);\
      }\
   )

#define _S_ERROR(str)\
   _ISOLATE\
   (\
      if (_ENABLE_RUNTIME_ERRORS_OUTPUT)\
      {\
         _PRINT_S_STDERR("E", str);\
      }\
   )

#define _S_PROG_ERROR(str)\
   _ISOLATE\
   (\
      if (_ENABLE_PROGRAMMING_ERRORS_OUTPUT)\
      {\
         _PRINT_S_STDERR("P", str);\
      }\
   )

#define _S_FATAL(str)\
   _ISOLATE\
   (\
     if (_ENABLE_FATAL_ERROR_OUTPUT)\
      {\
         _PRINT_S_STDERR("F", str);\
      }\
   )

#endif
