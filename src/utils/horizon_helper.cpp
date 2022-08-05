#include "utils/horizon_helper.h"

namespace horizon {

namespace utils {

std::string readFile(const char *filePath, FileType type) {
    std::ifstream file;
    switch (type) {
    case FileType::TEXT:
        file = std::ifstream{filePath};
        break;
    case FileType::BINARY:
        file = std::ifstream{filePath, std::ios::binary};
        break;
    default:
        throw std::runtime_error("Invalid enum!");
    }
    if (!file.is_open()) {
        throw std::runtime_error(std::string("Failed to open file: ") + filePath);
    }
    return std::string((std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>()));
}

} // namespace utils

} // namespace horizon
