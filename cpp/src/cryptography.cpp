#include "ram/cryptography.h"

#include <algorithm>
#include <cstring>

#if RAM_HAS_LIBSODIUM
#include <sodium.h>
#endif

namespace ram {

// "Roblox Account Manager created by ic3w0lf22 @ github.com ........."
const std::vector<uint8_t> kRAMHeader = {
    82,  111, 98,  108, 111, 120, 32,  65,  99,  99,  111, 117, 110,
    116, 32,  77,  97,  110, 97,  103, 101, 114, 32,  99,  114, 101,
    97,  116, 101, 100, 32,  98,  121, 32,  105, 99,  51,  119, 48,
    108, 102, 50,  50,  32,  64,  32,  103, 105, 116, 104, 117, 98,
    46,  99,  111, 109, 32,  46,  46,  46,  46,  46,  46,  46};

bool has_ram_header(const std::vector<uint8_t>& data) {
    if (data.size() < kRAMHeader.size()) return false;
    return std::equal(kRAMHeader.begin(), kRAMHeader.end(), data.begin());
}

std::vector<uint8_t> encrypt(const std::string& content,
                             const std::vector<uint8_t>& password) {
#if RAM_HAS_LIBSODIUM
    if (content.empty()) return {};

    if (sodium_init() < 0) return {};

    // Generate salt for Argon2
    uint8_t salt[crypto_pwhash_SALTBYTES];
    randombytes_buf(salt, sizeof(salt));

    // Derive key using Argon2
    uint8_t key[crypto_secretbox_KEYBYTES];
    if (crypto_pwhash(key, sizeof(key), reinterpret_cast<const char*>(password.data()),
                      password.size(), salt, crypto_pwhash_OPSLIMIT_MODERATE,
                      crypto_pwhash_MEMLIMIT_MODERATE,
                      crypto_pwhash_ALG_DEFAULT) != 0) {
        return {};
    }

    // Generate nonce
    uint8_t nonce[crypto_secretbox_NONCEBYTES];
    randombytes_buf(nonce, sizeof(nonce));

    // Encrypt
    size_t ciphertext_len = content.size() + crypto_secretbox_MACBYTES;
    std::vector<uint8_t> ciphertext(ciphertext_len);
    crypto_secretbox_easy(ciphertext.data(),
                          reinterpret_cast<const uint8_t*>(content.data()),
                          content.size(), nonce, key);

    // Securely clear the derived key from memory after use
    sodium_memzero(key, sizeof(key));

    // Build output: Header | Salt | Nonce | Ciphertext
    std::vector<uint8_t> output;
    output.reserve(kRAMHeader.size() + sizeof(salt) + sizeof(nonce) +
                   ciphertext_len);
    output.insert(output.end(), kRAMHeader.begin(), kRAMHeader.end());
    output.insert(output.end(), salt, salt + sizeof(salt));
    output.insert(output.end(), nonce, nonce + sizeof(nonce));
    output.insert(output.end(), ciphertext.begin(), ciphertext.end());

    return output;
#else
    (void)content;
    (void)password;
    return {};
#endif
}

std::vector<uint8_t> decrypt(const std::vector<uint8_t>& encrypted,
                             const std::vector<uint8_t>& password) {
#if RAM_HAS_LIBSODIUM
    if (!has_ram_header(encrypted)) return {};

    if (sodium_init() < 0) return {};

    size_t offset = kRAMHeader.size();
    size_t min_size =
        offset + crypto_pwhash_SALTBYTES + crypto_secretbox_NONCEBYTES +
        crypto_secretbox_MACBYTES;
    if (encrypted.size() < min_size) return {};

    // Extract salt
    const uint8_t* salt = encrypted.data() + offset;
    offset += crypto_pwhash_SALTBYTES;

    // Extract nonce
    const uint8_t* nonce = encrypted.data() + offset;
    offset += crypto_secretbox_NONCEBYTES;

    // Extract ciphertext
    size_t ciphertext_len = encrypted.size() - offset;
    const uint8_t* ciphertext = encrypted.data() + offset;

    // Derive key
    uint8_t key[crypto_secretbox_KEYBYTES];
    if (crypto_pwhash(key, sizeof(key), reinterpret_cast<const char*>(password.data()),
                      password.size(), salt, crypto_pwhash_OPSLIMIT_MODERATE,
                      crypto_pwhash_MEMLIMIT_MODERATE,
                      crypto_pwhash_ALG_DEFAULT) != 0) {
        return {};
    }

    // Decrypt
    size_t plaintext_len = ciphertext_len - crypto_secretbox_MACBYTES;
    std::vector<uint8_t> plaintext(plaintext_len);
    if (crypto_secretbox_open_easy(plaintext.data(), ciphertext, ciphertext_len,
                                   nonce, key) != 0) {
        sodium_memzero(key, sizeof(key));
        return {};
    }

    sodium_memzero(key, sizeof(key));
    return plaintext;
#else
    (void)encrypted;
    (void)password;
    return {};
#endif
}

}  // namespace ram
