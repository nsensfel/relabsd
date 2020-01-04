#pragma once

#include <relabsd/client_types.h>

#include <relabsd/config/parameters.h>

int relabsd_client_main
(
   const int argc,
   const char * const argv [const static argc],
   struct relabsd_parameters parameters [const restrict static 1]
);
