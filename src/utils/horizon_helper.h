#ifndef UTILS_HORIZON_HELPER_H
#define UTILS_HORIZON_HELPER_H

#include "core/debug.h"

#include <fstream>
#include <vector>

namespace horizon {

namespace utils {

enum FileType {
    TEXT,
    BINARY
};

std::vector<char> readFile(const char *filePath, FileType type);

} // namespace utils

} // namespace horizon

#endif