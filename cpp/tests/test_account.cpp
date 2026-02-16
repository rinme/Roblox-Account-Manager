#include <gtest/gtest.h>

#include "ram/account.h"

TEST(AccountTest, DefaultConstruction) {
    ram::Account acc;
    EXPECT_FALSE(acc.valid);
    EXPECT_EQ(acc.security_token, "");
    EXPECT_EQ(acc.username, "");
    EXPECT_EQ(acc.user_id, 0);
    EXPECT_EQ(acc.group, "Default");
    EXPECT_TRUE(acc.fields.empty());
    EXPECT_EQ(acc.alias(), "");
    EXPECT_EQ(acc.description(), "");
    EXPECT_EQ(acc.password(), "");
}

TEST(AccountTest, ConstructWithToken) {
    ram::Account acc("test_cookie_123");
    EXPECT_EQ(acc.security_token, "test_cookie_123");
}

TEST(AccountTest, AliasMaxLength) {
    ram::Account acc;

    // Should accept alias up to 50 chars
    std::string short_alias(50, 'a');
    EXPECT_TRUE(acc.set_alias(short_alias));
    EXPECT_EQ(acc.alias(), short_alias);

    // Should reject alias > 50 chars
    std::string long_alias(51, 'a');
    EXPECT_FALSE(acc.set_alias(long_alias));
    EXPECT_EQ(acc.alias(), short_alias);  // unchanged
}

TEST(AccountTest, DescriptionMaxLength) {
    ram::Account acc;

    // Should accept description up to 5000 chars
    std::string short_desc(5000, 'a');
    EXPECT_TRUE(acc.set_description(short_desc));
    EXPECT_EQ(acc.description(), short_desc);

    // Should reject description > 5000 chars
    std::string long_desc(5001, 'a');
    EXPECT_FALSE(acc.set_description(long_desc));
    EXPECT_EQ(acc.description(), short_desc);  // unchanged
}

TEST(AccountTest, PasswordMaxLength) {
    ram::Account acc;

    // Should accept password up to 5000 chars
    std::string short_pass(5000, 'p');
    EXPECT_TRUE(acc.set_password(short_pass));
    EXPECT_EQ(acc.password(), short_pass);

    // Should reject password > 5000 chars
    std::string long_pass(5001, 'p');
    EXPECT_FALSE(acc.set_password(long_pass));
    EXPECT_EQ(acc.password(), short_pass);  // unchanged
}

TEST(AccountTest, Sorting) {
    ram::Account a, b;
    a.group = "Alpha";
    b.group = "Beta";

    EXPECT_TRUE(a < b);
    EXPECT_FALSE(b < a);
}

TEST(AccountTest, SerializeToJson) {
    ram::Account acc("my_security_token");
    acc.valid = true;
    acc.username = "TestUser";
    acc.user_id = 12345;
    acc.group = "VIP";
    acc.set_alias("MyAlias");
    acc.set_description("My description");
    acc.fields["note"] = "hello";

    auto j = acc.to_json();

    EXPECT_EQ(j["Valid"], true);
    EXPECT_EQ(j["SecurityToken"], "my_security_token");
    EXPECT_EQ(j["Username"], "TestUser");
    EXPECT_EQ(j["UserID"], 12345);
    EXPECT_EQ(j["Group"], "VIP");
    EXPECT_EQ(j["Alias"], "MyAlias");
    EXPECT_EQ(j["Description"], "My description");
    EXPECT_EQ(j["Fields"]["note"], "hello");
}

TEST(AccountTest, DeserializeFromJson) {
    nlohmann::json j;
    j["Valid"] = true;
    j["SecurityToken"] = "cookie_abc";
    j["Username"] = "Player1";
    j["UserID"] = 67890;
    j["Group"] = "Staff";
    j["Alias"] = "P1";
    j["Description"] = "Test desc";
    j["Password"] = "secret";
    j["Fields"] = {{"key1", "val1"}, {"key2", "val2"}};
    j["LastUse"] = 1000000;
    j["LastAttemptedRefresh"] = 2000000;

    auto acc = ram::Account::from_json(j);

    EXPECT_TRUE(acc.valid);
    EXPECT_EQ(acc.security_token, "cookie_abc");
    EXPECT_EQ(acc.username, "Player1");
    EXPECT_EQ(acc.user_id, 67890);
    EXPECT_EQ(acc.group, "Staff");
    EXPECT_EQ(acc.alias(), "P1");
    EXPECT_EQ(acc.description(), "Test desc");
    EXPECT_EQ(acc.password(), "secret");
    EXPECT_EQ(acc.fields.size(), 2);
    EXPECT_EQ(acc.fields["key1"], "val1");
}

TEST(AccountTest, RoundTripSerialization) {
    ram::Account original("token123");
    original.valid = true;
    original.username = "TestUser";
    original.user_id = 99999;
    original.group = "Group1";
    original.set_alias("Alias1");
    original.set_description("Desc1");
    original.set_password("Pass1");
    original.fields["custom"] = "data";

    auto j = original.to_json();
    auto restored = ram::Account::from_json(j);

    EXPECT_EQ(restored.valid, original.valid);
    EXPECT_EQ(restored.security_token, original.security_token);
    EXPECT_EQ(restored.username, original.username);
    EXPECT_EQ(restored.user_id, original.user_id);
    EXPECT_EQ(restored.group, original.group);
    EXPECT_EQ(restored.alias(), original.alias());
    EXPECT_EQ(restored.description(), original.description());
    EXPECT_EQ(restored.password(), original.password());
    EXPECT_EQ(restored.fields, original.fields);
}

TEST(AccountTest, EmptyAlias) {
    ram::Account acc;
    EXPECT_TRUE(acc.set_alias(""));
    EXPECT_EQ(acc.alias(), "");
}

TEST(AccountTest, DeserializePartialJson) {
    nlohmann::json j;
    j["Username"] = "OnlyName";

    auto acc = ram::Account::from_json(j);
    EXPECT_EQ(acc.username, "OnlyName");
    EXPECT_FALSE(acc.valid);
    EXPECT_EQ(acc.user_id, 0);
    EXPECT_EQ(acc.group, "Default");
}
