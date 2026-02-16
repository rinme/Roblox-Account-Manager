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

    acc.valid = j.value("Valid", false);
    acc.security_token = j.value("SecurityToken", std::string{});
    acc.username = j.value("Username", std::string{});
    acc.user_id = j.value("UserID", int64_t{0});
    acc.browser_tracker_id = j.value("BrowserTrackerID", std::string{});
    acc.group = j.value("Group", std::string{"Default"});

    acc.set_alias(j.value("Alias", std::string{}));
    acc.set_description(j.value("Description", std::string{}));
    acc.set_password(j.value("Password", std::string{}));

    if (j.contains("Fields")) {
        try {
            acc.fields =
                j["Fields"].get<std::map<std::string, std::string>>();
        } catch (const nlohmann::json::exception&) {
        }
    }

    auto from_epoch_ms = [](int64_t ms) {
        return std::chrono::system_clock::time_point(
            std::chrono::milliseconds(ms));
    };
    acc.last_use = from_epoch_ms(j.value("LastUse", int64_t{0}));
    acc.last_attempted_refresh =
        from_epoch_ms(j.value("LastAttemptedRefresh", int64_t{0}));

    return acc;
}

}  // namespace ram
