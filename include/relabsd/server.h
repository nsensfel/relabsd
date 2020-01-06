#pragma once

#include <relabsd/server_types.h>

int relabsd_server_main
(
   const int argc,
   const char * const argv [const static argc],
   struct relabsd_parameters parameters [const static 1]
);

int relabsd_server_create_daemon (void);

int relabsd_server_keep_running (void);
void relabsd_server_interrupt (void);
int relabsd_server_initialize_signal_handlers (void);
void relabsd_server_finalize_signal_handlers (void);
int relabsd_server_get_interruption_file_descriptor (void);

int relabsd_server_create_communication_thread
(
   struct relabsd_server server [const static 1]
);

int relabsd_server_create_communication_node
(
   const char socket_name [const restrict static 1],
   int socket [const restrict static 1]
);

int relabsd_server_conversion_loop
(
   struct relabsd_server server [const static 1]
);

int relabsd_server_join_communication_thread
(
   struct relabsd_server server [const static 1]
);

int relabsd_server_handle_client
(
   const int socket,
   struct relabsd_server server [const static 1]
);
