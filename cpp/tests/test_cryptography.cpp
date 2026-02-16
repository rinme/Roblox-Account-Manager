#include <gtest/gtest.h>

#include "ram/cryptography.h"

TEST(CryptographyTest, RAMHeaderContent) {
    // The header should spell out "Roblox Account Manager created by ic3w0lf22
    // @ github.com ........."
    std::string header_str(ram::kRAMHeader.begin(), ram::kRAMHeader.end());
    EXPECT_EQ(header_str,
              "Roblox Account Manager created by ic3w0lf22 @ github.com "
              ".......");
}

TEST(CryptographyTest, RAMHeaderSize) {
    EXPECT_EQ(ram::kRAMHeader.size(), 64);
}

TEST(CryptographyTest, HasRAMHeaderValid) {
    std::vector<uint8_t> data = ram::kRAMHeader;
    data.push_back(0);
    data.push_back(1);
    EXPECT_TRUE(ram::has_ram_header(data));
}

TEST(CryptographyTest, HasRAMHeaderInvalid) {
    std::vector<uint8_t> data = {0, 1, 2, 3};
    EXPECT_FALSE(ram::has_ram_header(data));
}

TEST(CryptographyTest, HasRAMHeaderTooShort) {
    std::vector<uint8_t> data = {82, 111, 98};
    EXPECT_FALSE(ram::has_ram_header(data));
}

TEST(CryptographyTest, HasRAMHeaderEmpty) {
    std::vector<uint8_t> data;
    EXPECT_FALSE(ram::has_ram_header(data));
}

#if RAM_HAS_LIBSODIUM
TEST(CryptographyTest, EncryptDecryptRoundTrip) {
    std::string content = "Hello, Roblox Account Manager!";
    std::vector<uint8_t> password = {'p', 'a', 's', 's', 'w', 'o', 'r', 'd'};

    auto encrypted = ram::encrypt(content, password);
    ASSERT_FALSE(encrypted.empty());
    EXPECT_TRUE(ram::has_ram_header(encrypted));

    auto decrypted = ram::decrypt(encrypted, password);
    ASSERT_FALSE(decrypted.empty());

    std::string result(decrypted.begin(), decrypted.end());
    EXPECT_EQ(result, content);
}

TEST(CryptographyTest, EncryptEmptyContent) {
    std::vector<uint8_t> password = {'p', 'a', 's', 's'};
    auto encrypted = ram::encrypt("", password);
    EXPECT_TRUE(encrypted.empty());
}

TEST(CryptographyTest, DecryptWrongPassword) {
    std::string content = "Secret data";
    std::vector<uint8_t> password = {'c', 'o', 'r', 'r', 'e', 'c', 't'};
    std::vector<uint8_t> wrong_password = {'w', 'r', 'o', 'n', 'g'};

    auto encrypted = ram::encrypt(content, password);
    ASSERT_FALSE(encrypted.empty());

    auto decrypted = ram::decrypt(encrypted, wrong_password);
    EXPECT_TRUE(decrypted.empty());
}
#else
TEST(CryptographyTest, EncryptWithoutLibsodium) {
    std::vector<uint8_t> password = {'t', 'e', 's', 't'};
    auto result = ram::encrypt("test", password);
    EXPECT_TRUE(result.empty());
}

TEST(CryptographyTest, DecryptWithoutLibsodium) {
    std::vector<uint8_t> data = ram::kRAMHeader;
    std::vector<uint8_t> password = {'t', 'e', 's', 't'};
    auto result = ram::decrypt(data, password);
    EXPECT_TRUE(result.empty());
}
#endif
