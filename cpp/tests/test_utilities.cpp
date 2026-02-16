#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

#include "ram/utilities.h"

// --- MD5 Tests ---

TEST(UtilitiesTest, MD5EmptyString) {
    EXPECT_EQ(ram::md5(""), "D41D8CD98F00B204E9800998ECF8427E");
}

TEST(UtilitiesTest, MD5HelloWorld) {
    EXPECT_EQ(ram::md5("Hello, World!"), "65A8E27D8879283831B664BD8B7F0AD4");
}

TEST(UtilitiesTest, MD5Test) {
    // Matches the C# behavior: MD5("test") = uppercase hex
    EXPECT_EQ(ram::md5("test"), "098F6BCD4621D373CADE4E832627B4F6");
}

TEST(UtilitiesTest, MD5LongString) {
    std::string long_str(1000, 'A');
    std::string hash = ram::md5(long_str);
    EXPECT_EQ(hash.size(), 32);
    // Should be consistent
    EXPECT_EQ(hash, ram::md5(long_str));
}

// --- SHA-256 Tests ---

TEST(UtilitiesTest, FileSHA256NonExistent) {
    std::string hash = ram::file_sha256("/nonexistent/path/file.txt");
    EXPECT_EQ(hash,
              "E3B0C44298FC1C149AFBF4C8996FB92427AE41E4649B934CA495991B7852B855");
}

TEST(UtilitiesTest, FileSHA256EmptyFile) {
    auto tmp = std::filesystem::temp_directory_path() / "ram_test_empty.bin";
    {
        std::ofstream f(tmp, std::ios::binary);
    }

    std::string hash = ram::file_sha256(tmp.string());
    // SHA-256 of empty input
    EXPECT_EQ(hash,
              "E3B0C44298FC1C149AFBF4C8996FB92427AE41E4649B934CA495991B7852B855");

    std::filesystem::remove(tmp);
}

TEST(UtilitiesTest, FileSHA256KnownContent) {
    auto tmp = std::filesystem::temp_directory_path() / "ram_test_sha256.txt";
    {
        std::ofstream f(tmp);
        f << "hello";
    }

    std::string hash = ram::file_sha256(tmp.string());
    // SHA-256 of "hello"
    EXPECT_EQ(hash,
              "2CF24DBA5FB0A30E26E83B2AC5B9E29E1B161E5C1FA7425E73043362938B9824");

    std::filesystem::remove(tmp);
}

// --- Clamp Tests ---

TEST(UtilitiesTest, ClampInt) {
    EXPECT_EQ(ram::clamp(5, 0, 10), 5);
    EXPECT_EQ(ram::clamp(-1, 0, 10), 0);
    EXPECT_EQ(ram::clamp(15, 0, 10), 10);
    EXPECT_EQ(ram::clamp(0, 0, 10), 0);
    EXPECT_EQ(ram::clamp(10, 0, 10), 10);
}

TEST(UtilitiesTest, ClampDouble) {
    EXPECT_DOUBLE_EQ(ram::clamp(0.5, 0.0, 1.0), 0.5);
    EXPECT_DOUBLE_EQ(ram::clamp(-0.1, 0.0, 1.0), 0.0);
    EXPECT_DOUBLE_EQ(ram::clamp(1.5, 0.0, 1.0), 1.0);
}

// --- JSON Parse Tests ---

TEST(UtilitiesTest, TryParseJsonValid) {
    auto result = ram::try_parse_json<int>("42");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 42);
}

TEST(UtilitiesTest, TryParseJsonInvalid) {
    auto result = ram::try_parse_json<int>("not json");
    EXPECT_FALSE(result.has_value());
}

TEST(UtilitiesTest, TryParseJsonString) {
    auto result = ram::try_parse_json<std::string>("\"hello\"");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, "hello");
}

TEST(UtilitiesTest, TryParseJsonObject) {
    auto result = ram::try_parse_json<std::map<std::string, int>>(
        "{\"a\": 1, \"b\": 2}");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->at("a"), 1);
    EXPECT_EQ(result->at("b"), 2);
}

// --- Roblox Tick Tests ---

TEST(UtilitiesTest, ToRobloxTickEpoch) {
    auto epoch = std::chrono::system_clock::time_point{};
    EXPECT_DOUBLE_EQ(ram::to_roblox_tick(epoch), 0.0);
}

TEST(UtilitiesTest, ToRobloxTickNonZero) {
    auto tp = std::chrono::system_clock::time_point(
        std::chrono::seconds(1000) + std::chrono::milliseconds(500));
    double tick = ram::to_roblox_tick(tp);
    EXPECT_DOUBLE_EQ(tick, 1000.5);
}

// --- Recursive Delete Tests ---

TEST(UtilitiesTest, RecursiveDeleteCreatesAndDeletes) {
    auto tmp =
        std::filesystem::temp_directory_path() / "ram_test_recursive_delete";
    std::filesystem::create_directories(tmp / "subdir");
    {
        std::ofstream f(tmp / "file.txt");
        f << "data";
    }
    {
        std::ofstream f(tmp / "subdir" / "nested.txt");
        f << "nested";
    }

    EXPECT_TRUE(std::filesystem::exists(tmp));
    EXPECT_TRUE(ram::recursive_delete(tmp.string()));
    EXPECT_FALSE(std::filesystem::exists(tmp));
}

TEST(UtilitiesTest, RecursiveDeleteNonExistent) {
    EXPECT_TRUE(ram::recursive_delete("/tmp/nonexistent_path_ram_test_xyz"));
}
