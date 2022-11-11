#include "FSUtils.h"
#include "TinySHA1.hpp"
#include <optional>
#include <string>

std::string calculateSHA1(const char *buffer, size_t size) {
    sha1::SHA1 s;
    s.processBytes(buffer, size);
    uint32_t digest[5];
    s.getDigest(digest);
    char tmp[48];
    snprintf(tmp, 45, "%08x%08x%08x%08x%08x", digest[0], digest[1], digest[2], digest[3], digest[4]);
    return tmp;
}

std::optional<std::string> hashFile(const std::string &path) {
    uint8_t *data = nullptr;
    uint32_t size = 0;
    if (LoadFileToMem(path.c_str(), &data, &size) < 0 || data == nullptr) {
        return {};
    }
    std::string result = calculateSHA1(reinterpret_cast<const char *>(data), size);
    free(data);
    return result;
}