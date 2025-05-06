#include "util.h"

#include <cstdint>
#include <libgen.h>         // dirname
#include <unistd.h>         // readlink
#include <linux/limits.h>   // PATH_MAX

std::string getExePath() {
    char result[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
    const char *path = nullptr;
    if (count != -1) {
        path = dirname(result);
    }
    return std::string(path);
}
