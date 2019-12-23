#pragma once

#define RELABSD_ISOLATE(a) do {a} while (0)

#define RELABSD__TO_STRING(x) #x
#define RELABSD_TO_STRING(x) RELABSD__TO_STRING(x)
