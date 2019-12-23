cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
# 3.0 introduces line continuation.

project("relabsd")

include(FindPkgConfig)

# ${SRC_FILES} is recursively defined in the subdirectories.
# Each subdirectory adds only the source files that are present at its level.
file(GLOB_RECURSE SRC_FILES src/ true *.c)
add_executable(relabsd ${SRC_FILES})

include_directories(include/)

# Language parameters.
enable_language(C)
target_compile_features(relabsd PUBLIC c_variadic_macros)

# We require libevdev.
pkg_search_module(LIBEVDEV REQUIRED libevdev)
include_directories(${LIBEVDEV_INCLUDE_DIRS})
target_link_libraries(relabsd ${LIBEVDEV_LIBRARIES})

# Be loud about dubious code.
if (CMAKE_COMPILER_IS_GNUCC)
   message(STATUS "CMake is using GNUCC. Verbose flags are activated.")
   set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -pedantic -Wextra")
endif (CMAKE_COMPILER_IS_GNUCC)

option(
   RELABSD_DEBUG_PROGRAM_FLOW
   "Adds debug messages to display the program's flow."
   OFF
)
if (RELABSD_DEBUG_PROGRAM_FLOW)
   target_compile_definitions(
      relabsd
      PUBLIC
      "-DRELABSD_DEBUG_PROGRAM_FLOW=1"
   )
   message(STATUS "[OPTION] Debug messages for the program's flow.")
endif (RELABSD_DEBUG_PROGRAM_FLOW)

option(
   RELABSD_DEBUG_CONFIG
   "Adds debug messages to check the program's configuration."
   OFF
)
if (RELABSD_DEBUG_CONFIG)
   target_compile_definitions(
      relabsd
      PUBLIC
      "-DRELABSD_DEBUG_CONFIG=1"
   )
   message(STATUS "[OPTION] Debug messages for the program's configuration.")
endif (RELABSD_DEBUG_CONFIG)

option(
   RELABSD_DEBUG_REAL_EVENTS
   "Adds debug messages to check the real device's events."
   OFF
)
if (RELABSD_DEBUG_REAL_EVENTS)
   target_compile_definitions(
      relabsd
      PUBLIC
      "-DRELABSD_DEBUG_REAL_EVENTS=1"
   )
   message(STATUS "[OPTION] Debug messages for the real device's events.")
endif (RELABSD_DEBUG_REAL_EVENTS)

option(
   RELABSD_DEBUG_VIRTUAL_EVENTS
   "Adds debug messages to check the virtual device's events."
   OFF
)
if (RELABSD_DEBUG_VIRTUAL_EVENTS)
   target_compile_definitions(
      relabsd
      PUBLIC
      "-DRELABSD_DEBUG_VIRTUAL_EVENTS=1"
   )
   message(STATUS "[OPTION] Debug messages for the virtual device's events.")
endif (RELABSD_DEBUG_VIRTUAL_EVENTS)

option(
   RELABSD_ENABLE_ERROR_LOCATION
   "Debug/error messages contain source file and line information."
   OFF
)
if (RELABSD_ENABLE_ERROR_LOCATION)
   target_compile_definitions(relabsd PUBLIC RELABSD_ENABLE_ERROR_LOCATION)
   message(STATUS "[OPTION] Debug/error messages display source file and line.")
else ()
   message(
      STATUS
      "[OPTION] Debug/error messages do not display source file and line."
   )
endif (RELABSD_ENABLE_ERROR_LOCATION)


set(
   RELABSD_OPTION_MAX_SIZE
   "64"
   CACHE
   INTEGER
   "Maximum number of characters in an axis option (name + params)."
)
target_compile_definitions(
   relabsd
   PUBLIC
   "-DRELABSD_OPTION_MAX_SIZE=${RELABSD_OPTION_MAX_SIZE}"
)
message(
   STATUS
   "[OPTION] Axis options can contain up to ${RELABSD_OPTION_MAX_SIZE}\
 characters (name + params)."
)

set(
   RELABSD_DEVICE_PREFIX
   "relabsd:"
   CACHE
   STRING
   "String prefixing the name of the virtual device."
)
target_compile_definitions(
   relabsd
   PUBLIC
   "-DRELABSD_DEVICE_PREFIX=\"${RELABSD_DEVICE_PREFIX}\""
)
message(
   STATUS
   "[OPTION] Virtual devices' names are prefixed by '${RELABSD_DEVICE_PREFIX}'."
)
