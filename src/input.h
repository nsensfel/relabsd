#ifndef RELABSD_INPUT_H
#define RELABSD_INPUT_H

#include <libevdev/libevdev.h>

#include "config.h"

struct relabsd_input
{
   struct libevdev * dev;
   int fd;
};

/*
 * Returns -1 on (fatal) error,
 *         0  on success.
 *
 * 'input' does not need to be initialized, as the function will to that for
 * you (on success).
 * On success, 'input' will need to be closed.
 */
int relabsd_input_open
(
   struct relabsd_input * const input,
   const struct relabsd_config * const conf
);

void relabsd_input_close (const struct relabsd_input * const input);

/*
 * Returns -1 on (warning) error,
 *         0 on successful read.
 *
 * The 'input_*' parameters do not need to be initialized, as the function will
 * do that for you (on success).
 * Requires 'input' to be open.
 */
int relabsd_input_read
(
   const struct relabsd_input * const input,
   unsigned int * const input_type,
   unsigned int * const input_code,
   int * const input_value
);

#endif
