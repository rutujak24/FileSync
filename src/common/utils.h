#pragma once

#include <string>
#include <cstdint>

namespace filesync {

namespace utils {

// Calculate SHA256 hash of a file
std::string CalculateSHA256(const std::string& file_path);

// Get size of a file in bytes
int64_t GetFileSize(const std::string& file_path);

} // namespace utils

} // namespace filesync
