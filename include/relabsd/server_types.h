#pragma once

/**** POSIX *******************************************************************/
#include <pthread.h>

/**** RELABSD *****************************************************************/
#include <relabsd/config/parameters_types.h>

#include <relabsd/device/physical_device_types.h>
#include <relabsd/device/virtual_device_types.h>

struct relabsd_server
{
   pthread_mutex_t mutex;
   pthread_t communication_thread;
   struct relabsd_parameters parameters;
   struct relabsd_physical_device physical_device;
   struct relabsd_virtual_device virtual_device;
};
