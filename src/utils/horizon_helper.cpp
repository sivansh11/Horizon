#include "utils/horizon_helper.h"

namespace horizon {

namespace utils {

std::vector<char> readFile(const char *filePath, FileType type) {
    std::ifstream file;
    switch (type) {
    case FileType::TEXT:
        file = std::ifstream{filePath, std::ios::ate};
        break;
    case FileType::BINARY:
        file = std::ifstream{filePath, std::ios::ate | std::ios::binary};
        break;
    default:
        throw std::runtime_error("Invalid enum!");
    }
    if (!file.is_open()) {
        throw std::runtime_error(std::string("Failed to open file: ") + filePath);
    }
    size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();
    return buffer;
}

} // namespace utils

} // namespace horizon
