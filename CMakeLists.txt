cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

project("relabsd")

include(FindPkgConfig)

add_subdirectory(src)

# ${SRC_FILES} is recursively defined in the subdirectories.
# Each subdirectory only adds the source files that are present at its level.

add_executable(relabsd ${SRC_FILES})

pkg_search_module(LIBEVDEV REQUIRED libevdev)
include_directories(${LIBEVDEV_INCLUDE_DIRS})
target_link_libraries(relabsd ${LIBEVDEV_LIBRARIES})
