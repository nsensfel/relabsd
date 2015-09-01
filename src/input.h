#ifndef RELABSD_INPUT_H
#define RELABSD_INPUT_H

#include <libevdev/libevdev.h>

struct relabsd_input
{
   struct libevdev * dev;
   int fd;
};

int relabsd_input_open
(
   struct relabsd_input * const input,
   const char * const filename
);

void relabsd_input_close (const struct relabsd_input * const input);

int relabsd_input_read
(
   const struct relabsd_input * const input,
   unsigned int * const input_type,
   unsigned int * const input_code,
   int * const input_value
);

#endif
