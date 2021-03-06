#pragma once

#include <string.h>

#include <relabsd/util/macro.h>

/* strncmp stops at '\0' and strlen does not count '\0'. */
#define RELABSD_IS_PREFIX(a, b) (strncmp(a, b, strlen(a)) == 0)

#define RELABSD_STRING_EQUALS(a, b) (strcmp(a, b) == 0)

int relabsd_util_parse_int
(
   const char string [const restrict static 1],
   const int min,
   const int max,
   int output [const restrict static 1]
);

/*
 * Returns -1 on error,
 *          0 on EOF,
 *          1 on newline.
 */
int relabsd_util_reach_next_line_or_eof (FILE f [const restrict static 1]);
