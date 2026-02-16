#include <gtest/gtest.h>

#include <sstream>

#include "ram/ini_file.h"

TEST(IniFileTest, EmptyFile) {
    ram::IniFile ini;
    EXPECT_EQ(ini.sections().size(), 0);
    EXPECT_EQ(ini.to_string(), "");
}

TEST(IniFileTest, ParseBasicIni) {
    std::istringstream input(
        "[General]\n"
        "key1=value1\n"
        "key2=value2\n"
        "\n"
        "[Settings]\n"
        "debug=true\n"
        "count=42\n");

    ram::IniFile ini(input);

    EXPECT_TRUE(ini.has_section("General"));
    EXPECT_TRUE(ini.has_section("Settings"));
    EXPECT_FALSE(ini.has_section("NonExistent"));

    EXPECT_EQ(ini.section("General").get("key1"), "value1");
    EXPECT_EQ(ini.section("General").get("key2"), "value2");
    EXPECT_EQ(ini.section("Settings").get("debug"), "true");
    EXPECT_EQ(ini.section("Settings").get("count"), "42");
}

TEST(IniFileTest, ParseWithSpaces) {
    std::istringstream input(
        "[Section]\n"
        "  key1  =  value with spaces  \n"
        "key2 = another value\n");

    ram::IniFile ini(input);

    EXPECT_EQ(ini.section("Section").get("key1"), "value with spaces");
    EXPECT_EQ(ini.section("Section").get("key2"), "another value");
}

TEST(IniFileTest, SkipComments) {
    std::istringstream input(
        "# This is a comment\n"
        "; This is also a comment\n"
        "[Section]\n"
        "# Comment inside section\n"
        "key=value\n");

    ram::IniFile ini(input);

    EXPECT_EQ(ini.section("Section").get("key"), "value");
    EXPECT_EQ(ini.section("Section").size(), 1);
}

TEST(IniFileTest, SkipEmptyLines) {
    std::istringstream input(
        "\n"
        "\n"
        "[Section]\n"
        "\n"
        "key=value\n"
        "\n");

    ram::IniFile ini(input);
    EXPECT_EQ(ini.section("Section").get("key"), "value");
}

TEST(IniFileTest, OldThemeNameMigration) {
    std::istringstream input(
        "[RBX Alt Manager]\n"
        "theme=dark\n");

    ram::IniFile ini(input);

    // Old name should be migrated
    EXPECT_TRUE(ini.has_section("Roblox Account Manager"));
    EXPECT_FALSE(ini.has_section("RBX Alt Manager"));
    EXPECT_EQ(ini.section("Roblox Account Manager").get("theme"), "dark");
}

TEST(IniSectionTest, GetNonExistentProperty) {
    ram::IniSection section("Test");
    EXPECT_EQ(section.get("missing"), "");
    EXPECT_FALSE(section.exists("missing"));
}

TEST(IniSectionTest, SetAndGet) {
    ram::IniSection section("Test");
    section.set("name", "value");

    EXPECT_TRUE(section.exists("name"));
    EXPECT_EQ(section.get("name"), "value");
}

TEST(IniSectionTest, SetEmptyRemoves) {
    ram::IniSection section("Test");
    section.set("name", "value");
    EXPECT_TRUE(section.exists("name"));

    section.set("name", "");
    EXPECT_FALSE(section.exists("name"));
}

TEST(IniSectionTest, UpdateExistingProperty) {
    ram::IniSection section("Test");
    section.set("name", "old_value");
    section.set("name", "new_value");

    EXPECT_EQ(section.get("name"), "new_value");
    EXPECT_EQ(section.size(), 1);
}

TEST(IniSectionTest, RemoveProperty) {
    ram::IniSection section("Test");
    section.set("a", "1");
    section.set("b", "2");

    section.remove_property("a");
    EXPECT_FALSE(section.exists("a"));
    EXPECT_TRUE(section.exists("b"));
    EXPECT_EQ(section.size(), 1);
}

TEST(IniSectionTest, TypedGetAs) {
    ram::IniSection section("Test");
    section.set("count", "42");
    section.set("rate", "3.14");
    section.set("enabled", "true");
    section.set("disabled", "false");

    EXPECT_EQ(section.get_as<int>("count"), 42);
    EXPECT_DOUBLE_EQ(section.get_as<double>("rate"), 3.14);
    EXPECT_TRUE(section.get_as<bool>("enabled"));
    EXPECT_FALSE(section.get_as<bool>("disabled"));
    EXPECT_EQ(section.get_as<std::string>("count"), "42");
}

TEST(IniSectionTest, TypedGetAsDefaults) {
    ram::IniSection section("Test");

    EXPECT_EQ(section.get_as<int>("missing"), 0);
    EXPECT_DOUBLE_EQ(section.get_as<double>("missing"), 0.0);
    EXPECT_FALSE(section.get_as<bool>("missing"));
    EXPECT_EQ(section.get_as<std::string>("missing"), "");
}

TEST(IniFileTest, SaveWithoutSpacing) {
    ram::IniFile ini;
    ini.section("Section").set("key", "value");

    std::string output = ini.to_string();
    EXPECT_NE(output.find("[Section]"), std::string::npos);
    EXPECT_NE(output.find("key=value"), std::string::npos);
}

TEST(IniFileTest, SaveWithSpacing) {
    ram::IniFile ini;
    ini.set_write_spacing(true);
    ini.section("Section").set("key", "value");

    std::string output = ini.to_string();
    EXPECT_NE(output.find("key = value"), std::string::npos);
}

TEST(IniFileTest, SaveWithComments) {
    ram::IniFile ini;
    auto& sec = ini.section("Section");
    sec.set_comment("Section comment");
    sec.set("key", "value", "Property comment");

    std::string output = ini.to_string();
    EXPECT_NE(output.find("# Section comment"), std::string::npos);
    EXPECT_NE(output.find("# Property comment"), std::string::npos);
}

TEST(IniFileTest, RoundTrip) {
    ram::IniFile ini;
    ini.section("General").set("name", "TestApp");
    ini.section("General").set("version", "1.0");
    ini.section("Settings").set("debug", "true");

    std::string serialized = ini.to_string();

    std::istringstream stream(serialized);
    ram::IniFile ini2(stream);

    EXPECT_EQ(ini2.section("General").get("name"), "TestApp");
    EXPECT_EQ(ini2.section("General").get("version"), "1.0");
    EXPECT_EQ(ini2.section("Settings").get("debug"), "true");
}

TEST(IniFileTest, RemoveSection) {
    ram::IniFile ini;
    ini.section("A").set("key", "val");
    ini.section("B").set("key", "val");

    EXPECT_TRUE(ini.has_section("A"));
    ini.remove_section("A");
    EXPECT_FALSE(ini.has_section("A"));
    EXPECT_TRUE(ini.has_section("B"));
}

TEST(IniFileTest, CreateSectionOnAccess) {
    ram::IniFile ini;
    auto& sec = ini.section("NewSection");
    EXPECT_TRUE(ini.has_section("NewSection"));
    EXPECT_EQ(sec.size(), 0);
}

TEST(IniFileTest, PreservesInsertionOrder) {
    ram::IniFile ini;
    ini.section("Zebra").set("z", "1");
    ini.section("Alpha").set("a", "2");
    ini.section("Middle").set("m", "3");

    auto sections = ini.sections();
    ASSERT_EQ(sections.size(), 3);
    EXPECT_EQ(sections[0].name(), "Zebra");
    EXPECT_EQ(sections[1].name(), "Alpha");
    EXPECT_EQ(sections[2].name(), "Middle");
}

TEST(IniFileTest, ValueWithEqualsSign) {
    std::istringstream input(
        "[Section]\n"
        "url=https://example.com?a=1&b=2\n");

    ram::IniFile ini(input);
    EXPECT_EQ(ini.section("Section").get("url"),
              "https://example.com?a=1&b=2");
}
