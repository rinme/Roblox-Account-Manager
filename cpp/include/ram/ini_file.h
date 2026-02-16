#pragma once

#include <map>
#include <string>
#include <sstream>
#include <vector>

namespace ram {

/// Represents a property in an INI file.
struct IniProperty {
    std::string name;
    std::string value;
    std::string comment;
};

/// Represents a section in an INI file.
class IniSection {
public:
    explicit IniSection(const std::string& name = "");

    const std::string& name() const { return name_; }
    void set_name(const std::string& name) { name_ = name; }

    const std::string& comment() const { return comment_; }
    void set_comment(const std::string& comment) { comment_ = comment; }

    /// Get a property value. Returns empty string if not found.
    std::string get(const std::string& name) const;

    /// Get a property value converted to type T.
    template <typename T>
    T get_as(const std::string& name) const;

    /// Check if a property exists.
    bool exists(const std::string& name) const;

    /// Set a property value. If value is empty, removes the property.
    void set(const std::string& name, const std::string& value,
             const std::string& comment = "");

    /// Remove a property by name.
    void remove_property(const std::string& name);

    /// Get all properties.
    std::vector<IniProperty> properties() const;

    /// Return number of properties.
    size_t size() const { return properties_.size(); }

private:
    std::string name_;
    std::string comment_;
    // Use a vector to maintain insertion order plus a map for fast lookup
    std::vector<std::string> order_;
    std::map<std::string, IniProperty> properties_;
};

/// Represents an INI file that can be read from or written to.
class IniFile {
public:
    IniFile();

    /// Load an INI file from a file path.
    explicit IniFile(const std::string& path);

    /// Load an INI file from a stream.
    explicit IniFile(std::istream& stream);

    /// If true, writes extra spacing between property name and value.
    bool write_spacing() const { return write_spacing_; }
    void set_write_spacing(bool v) { write_spacing_ = v; }

    /// The character a comment line begins with. Default '#'.
    char comment_char() const { return comment_char_; }
    void set_comment_char(char c) { comment_char_ = c; }

    /// Get a section by name. Creates it if it doesn't exist.
    IniSection& section(const std::string& name);

    /// Check if a section exists.
    bool has_section(const std::string& name) const;

    /// Remove a section by name.
    void remove_section(const std::string& name);

    /// Get all sections.
    std::vector<IniSection> sections() const;

    /// Save INI content to a file path.
    void save(const std::string& path) const;

    /// Write INI content to a stream.
    void save(std::ostream& stream) const;

    /// Return the INI file content as a string.
    std::string to_string() const;

private:
    void load(std::istream& stream);

    bool write_spacing_ = false;
    char comment_char_ = '#';
    std::vector<std::string> section_order_;
    std::map<std::string, IniSection> sections_;
};

// Template specializations for IniSection::get_as
template <>
int IniSection::get_as<int>(const std::string& name) const;

template <>
double IniSection::get_as<double>(const std::string& name) const;

template <>
bool IniSection::get_as<bool>(const std::string& name) const;

template <>
std::string IniSection::get_as<std::string>(const std::string& name) const;

}  // namespace ram
