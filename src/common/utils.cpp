#include "utils.h"
#include <fstream>
#include <iomanip>
#include <sstream>
#include <openssl/sha.h>
#include <vector>

namespace filesync {

namespace utils {

std::string CalculateSHA256(const std::string& file_path) {
    std::ifstream file(file_path, std::ios::binary);
    if (!file) {
        return "";
    }

    SHA256_CTX sha256;
    SHA256_Init(&sha256);

    const int buffer_size = 32768;
    std::vector<char> buffer(buffer_size);

    while (file.read(buffer.data(), buffer_size)) {
        SHA256_Update(&sha256, buffer.data(), file.gcount());
    }
    SHA256_Update(&sha256, buffer.data(), file.gcount());

    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_Final(hash, &sha256);

    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }

    return ss.str();
}

int64_t GetFileSize(const std::string& file_path) {
    std::ifstream file(file_path, std::ios::binary | std::ios::ate);
    if (!file) {
        return -1;
    }
    return file.tellg();
}

} // namespace utils

} // namespace filesync
