#pragma once

#include <chrono>
#include <cstdint>
#include <map>
#include <string>

#include <nlohmann/json.hpp>

namespace ram {

/// Represents a Roblox account with associated metadata.
class Account {
public:
    Account() = default;

    /// Construct an account with the security token (cookie).
    explicit Account(const std::string& security_token);

    // Core account data
    bool valid = false;
    std::string security_token;
    std::string username;
    int64_t user_id = 0;
    std::string browser_tracker_id;
    std::string group = "Default";

    // Timestamps
    std::chrono::system_clock::time_point last_use{};
    std::chrono::system_clock::time_point last_attempted_refresh{};

    // Custom key-value fields
    std::map<std::string, std::string> fields;

    // Alias with max length enforcement
    std::string alias() const { return alias_; }
    bool set_alias(const std::string& value);

    // Description with max length enforcement
    std::string description() const { return description_; }
    bool set_description(const std::string& value);

    // Password with max length enforcement
    std::string password() const { return password_; }
    bool set_password(const std::string& value);

    /// Compare accounts by group name (for sorting).
    bool operator<(const Account& other) const {
        return group < other.group;
    }

    /// Serialize account to JSON.
    nlohmann::json to_json() const;

    /// Deserialize account from JSON.
    static Account from_json(const nlohmann::json& j);

private:
    std::string alias_;
    std::string description_;
    std::string password_;

    static constexpr size_t kMaxAliasLength = 50;
    static constexpr size_t kMaxDescriptionLength = 5000;
    static constexpr size_t kMaxPasswordLength = 5000;
};

}  // namespace ram
