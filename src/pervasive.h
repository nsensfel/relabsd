#ifndef RELABSD_PERVASIVE_H
#define RELABSD_PERVASIVE_H

#include <string.h>

#define RELABSD__TO_STRING(x) #x
#define RELABSD_TO_STRING(x) RELABSD__TO_STRING(x)

#define RELABSD_ISOLATE(a) do {a} while (0)

/* strncmp stops at '\0' and strlen does not count '\0'. */
#define RELABSD_IS_PREFIX(a, b) (strncmp(a, b, strlen(a)) == 0)

#endif
