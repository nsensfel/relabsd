cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
# 3.0 introduces line continuation.

project("relabsd")

include(FindPkgConfig)

# ${SRC_FILES} is recursively defined in the subdirectories.
# Each subdirectory adds only the source files that are present at its level.
add_subdirectory(src)
add_executable(relabsd ${SRC_FILES})

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

# Highest debug level is defined here to be able to access it in CMake.
set(RELABSD_HIGHEST_DEBUG_LEVEL 100)
target_compile_definitions(
   relabsd
   PUBLIC
   "-DRELABSD_HIGHEST_DEBUG_LVL=${RELABSD_HIGHEST_DEBUG_LEVEL}"
)
message(
   STATUS
   "[CONST] Highest debug level set to ${RELABSD_HIGHEST_DEBUG_LEVEL}."
)

set(
   RELABSD_DEBUG_LEVEL
   "0"
   CACHE
   INTEGER
   "Debug verbosity level[0-${RELABSD_HIGHEST_DEBUG_LEVEL}]."
)
target_compile_definitions(
   relabsd
   PUBLIC
   "-DRELABSD_DEBUG_LVL=${RELABSD_DEBUG_LEVEL}"
)
message(STATUS "[OPTION] Debug level set to ${RELABSD_DEBUG_LEVEL}.")

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
