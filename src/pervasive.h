#ifndef RELABSD_PERVASIVE_H
#define RELABSD_PERVASIVE_H

#include <string.h>

#define __TO_STRING(x) #x
#define _TO_STRING(x) __TO_STRING(x)

#define _ISOLATE(a) do {a} while (0)

/* strncmp stops at '\0' and strlen does not count '\0'. */
#define _IS_PREFIX(a, b) (strncmp(a, b, strlen(a)) == 0)

#endif
