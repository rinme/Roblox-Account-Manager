#include "ram/account.h"

#include <algorithm>

namespace ram {

Account::Account(const std::string& security_token)
    : security_token(security_token) {}

bool Account::set_alias(const std::string& value) {
    if (value.size() > kMaxAliasLength) return false;
    alias_ = value;
    return true;
}

bool Account::set_description(const std::string& value) {
    if (value.size() > kMaxDescriptionLength) return false;
    description_ = value;
    return true;
}

bool Account::set_password(const std::string& value) {
    if (value.size() > kMaxPasswordLength) return false;
    password_ = value;
    return true;
}

nlohmann::json Account::to_json() const {
    nlohmann::json j;
    j["Valid"] = valid;
    j["SecurityToken"] = security_token;
    j["Username"] = username;
    j["UserID"] = user_id;
    j["BrowserTrackerID"] = browser_tracker_id;
    j["Group"] = group;
    j["Alias"] = alias_;
    j["Description"] = description_;
    j["Password"] = password_;
    j["Fields"] = fields;

    auto to_epoch_ms = [](const std::chrono::system_clock::time_point& tp) {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
                   tp.time_since_epoch())
            .count();
    };
    j["LastUse"] = to_epoch_ms(last_use);
    j["LastAttemptedRefresh"] = to_epoch_ms(last_attempted_refresh);

    return j;
}

Account Account::from_json(const nlohmann::json& j) {
    Account acc;
    if (j.contains("Valid")) acc.valid = j["Valid"].get<bool>();
    if (j.contains("SecurityToken"))
        acc.security_token = j["SecurityToken"].get<std::string>();
    if (j.contains("Username"))
        acc.username = j["Username"].get<std::string>();
    if (j.contains("UserID")) acc.user_id = j["UserID"].get<int64_t>();
    if (j.contains("BrowserTrackerID"))
        acc.browser_tracker_id = j["BrowserTrackerID"].get<std::string>();
    if (j.contains("Group")) acc.group = j["Group"].get<std::string>();
    if (j.contains("Alias"))
        acc.set_alias(j["Alias"].get<std::string>());
    if (j.contains("Description"))
        acc.set_description(j["Description"].get<std::string>());
    if (j.contains("Password"))
        acc.set_password(j["Password"].get<std::string>());
    if (j.contains("Fields"))
        acc.fields = j["Fields"].get<std::map<std::string, std::string>>();

    auto from_epoch_ms = [](int64_t ms) {
        return std::chrono::system_clock::time_point(
            std::chrono::milliseconds(ms));
    };
    if (j.contains("LastUse"))
        acc.last_use = from_epoch_ms(j["LastUse"].get<int64_t>());
    if (j.contains("LastAttemptedRefresh"))
        acc.last_attempted_refresh =
            from_epoch_ms(j["LastAttemptedRefresh"].get<int64_t>());

    return acc;
}

}  // namespace ram
