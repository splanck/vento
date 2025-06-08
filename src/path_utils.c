#include "path_utils.h"
#include <string.h>
#if defined(_POSIX_VERSION) && !defined(_WIN32)
#include <stdlib.h>
#endif

char *vento_realpath(const char *path, char *resolved_path) {
#if defined(_POSIX_VERSION) && !defined(_WIN32)
    return realpath(path, resolved_path);
#else
    if (!path || !resolved_path)
        return NULL;
    strncpy(resolved_path, path, PATH_MAX - 1);
    resolved_path[PATH_MAX - 1] = '\0';
    return resolved_path;
#endif
}
