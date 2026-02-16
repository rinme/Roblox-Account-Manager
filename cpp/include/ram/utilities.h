#pragma once

#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

namespace ram {

/// Compute MD5 hash of a string and return it as an uppercase hex string.
std::string md5(const std::string& input);

/// Compute SHA-256 hash of a file and return it as an uppercase hex string.
/// Returns the hash of empty input if the file doesn't exist.
std::string file_sha256(const std::string& filename);

/// Clamp a value between min and max.
template <typename T>
T clamp(const T& val, const T& min_val, const T& max_val) {
    return std::max(min_val, std::min(val, max_val));
}

/// Try to parse a JSON string. Returns std::nullopt on failure.
template <typename T>
std::optional<T> try_parse_json(const std::string& json_str) {
    try {
        return nlohmann::json::parse(json_str).get<T>();
    } catch (...) {
        return std::nullopt;
    }
}

/// Convert a time_point to Roblox tick format (seconds since epoch with ms).
double to_roblox_tick(
    const std::chrono::system_clock::time_point& time_point);

/// Recursively delete a directory and all its contents.
bool recursive_delete(const std::string& path);

}  // namespace ram
