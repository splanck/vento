#ifndef VENTO_PATH_UTILS_H
#define VENTO_PATH_UTILS_H

#include <limits.h>
#if defined(_POSIX_VERSION) && !defined(_WIN32)
#include <stdlib.h>
#endif

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

char *vento_realpath(const char *path, char *resolved_path);

#endif
