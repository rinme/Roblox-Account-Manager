#include "ram/ini_file.h"

#include <algorithm>
#include <fstream>
#include <stdexcept>

namespace ram {

// --- IniSection ---

IniSection::IniSection(const std::string& name) : name_(name) {}

std::string IniSection::get(const std::string& name) const {
    auto it = properties_.find(name);
    if (it != properties_.end()) {
        return it->second.value;
    }
    return "";
}

template <>
int IniSection::get_as<int>(const std::string& name) const {
    auto val = get(name);
    if (val.empty()) return 0;
    try {
        return std::stoi(val);
    } catch (...) {
        return 0;
    }
}

template <>
double IniSection::get_as<double>(const std::string& name) const {
    auto val = get(name);
    if (val.empty()) return 0.0;
    try {
        return std::stod(val);
    } catch (...) {
        return 0.0;
    }
}

template <>
bool IniSection::get_as<bool>(const std::string& name) const {
    auto val = get(name);
    if (val.empty()) return false;
    // Case-insensitive comparison
    std::string lower = val;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    return lower == "true" || lower == "1" || lower == "yes";
}

template <>
std::string IniSection::get_as<std::string>(const std::string& name) const {
    return get(name);
}

bool IniSection::exists(const std::string& name) const {
    return properties_.find(name) != properties_.end();
}

void IniSection::set(const std::string& name, const std::string& value,
                     const std::string& comment) {
    if (value.empty()) {
        remove_property(name);
        return;
    }

    auto it = properties_.find(name);
    if (it == properties_.end()) {
        order_.push_back(name);
        properties_[name] = {name, value, comment};
    } else {
        it->second.value = value;
        if (!comment.empty()) {
            it->second.comment = comment;
        }
    }
}

void IniSection::remove_property(const std::string& name) {
    auto it = properties_.find(name);
    if (it != properties_.end()) {
        properties_.erase(it);
        order_.erase(
            std::remove(order_.begin(), order_.end(), name), order_.end());
    }
}

std::vector<IniProperty> IniSection::properties() const {
    std::vector<IniProperty> result;
    result.reserve(order_.size());
    for (const auto& key : order_) {
        auto it = properties_.find(key);
        if (it != properties_.end()) {
            result.push_back(it->second);
        }
    }
    return result;
}

// --- IniFile ---

IniFile::IniFile() = default;

IniFile::IniFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open INI file: " + path);
    }
    load(file);
}

IniFile::IniFile(std::istream& stream) { load(stream); }

void IniFile::load(std::istream& stream) {
    IniSection* current_section = nullptr;
    std::string line;

    while (std::getline(stream, line)) {
        // Trim whitespace
        size_t start = line.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) continue;
        size_t end = line.find_last_not_of(" \t\r\n");
        line = line.substr(start, end - start + 1);

        // Skip empty lines
        if (line.empty()) continue;

        // Skip comments
        if (line[0] == ';' || line[0] == '#') continue;

        // Section header
        if (line.front() == '[' && line.back() == ']') {
            std::string section_name = line.substr(1, line.size() - 2);
            // Support old theme name
            if (section_name == "RBX Alt Manager") {
                section_name = "Roblox Account Manager";
            }
            if (sections_.find(section_name) == sections_.end()) {
                section_order_.push_back(section_name);
                sections_.emplace(section_name, IniSection(section_name));
            }
            current_section = &sections_.at(section_name);
            continue;
        }

        // Key=Value pair
        if (current_section != nullptr) {
            auto eq_pos = line.find('=');
            if (eq_pos == std::string::npos) continue;

            std::string key = line.substr(0, eq_pos);
            std::string value = line.substr(eq_pos + 1);

            // Trim key and value
            auto trim = [](std::string& s) {
                size_t s2 = s.find_first_not_of(" \t");
                size_t e2 = s.find_last_not_of(" \t");
                if (s2 == std::string::npos) {
                    s.clear();
                } else {
                    s = s.substr(s2, e2 - s2 + 1);
                }
            };
            trim(key);
            trim(value);

            if (!key.empty() && !value.empty()) {
                current_section->set(key, value);
            }
        }
    }
}

IniSection& IniFile::section(const std::string& name) {
    auto it = sections_.find(name);
    if (it == sections_.end()) {
        section_order_.push_back(name);
        sections_.emplace(name, IniSection(name));
        return sections_.at(name);
    }
    return it->second;
}

bool IniFile::has_section(const std::string& name) const {
    return sections_.find(name) != sections_.end();
}

void IniFile::remove_section(const std::string& name) {
    auto it = sections_.find(name);
    if (it != sections_.end()) {
        sections_.erase(it);
        section_order_.erase(
            std::remove(section_order_.begin(), section_order_.end(), name),
            section_order_.end());
    }
}

std::vector<IniSection> IniFile::sections() const {
    std::vector<IniSection> result;
    result.reserve(section_order_.size());
    for (const auto& name : section_order_) {
        auto it = sections_.find(name);
        if (it != sections_.end()) {
            result.push_back(it->second);
        }
    }
    return result;
}

void IniFile::save(const std::string& path) const {
    std::ofstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot write INI file: " + path);
    }
    save(file);
}

void IniFile::save(std::ostream& stream) const {
    for (const auto& name : section_order_) {
        auto it = sections_.find(name);
        if (it == sections_.end()) continue;

        const auto& sec = it->second;
        auto props = sec.properties();
        if (props.empty()) continue;

        if (!sec.comment().empty()) {
            stream << comment_char_ << " " << sec.comment() << "\n";
        }

        stream << "[" << sec.name() << "]\n";

        for (const auto& prop : props) {
            if (!prop.comment.empty()) {
                stream << comment_char_ << " " << prop.comment << "\n";
            }
            if (write_spacing_) {
                stream << prop.name << " = " << prop.value << "\n";
            } else {
                stream << prop.name << "=" << prop.value << "\n";
            }
        }

        stream << "\n";
    }
}

std::string IniFile::to_string() const {
    std::ostringstream oss;
    save(oss);
    return oss.str();
}

}  // namespace ram
