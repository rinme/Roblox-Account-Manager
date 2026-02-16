#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ram {

/// RAM file header used to identify encrypted files.
extern const std::vector<uint8_t> kRAMHeader;

/// Encrypt content using libsodium (XSalsa20-Poly1305 with Argon2 KDF).
/// Returns encrypted bytes:
///   RAMHeader | Salt(crypto_pwhash_SALTBYTES) |
///   Nonce(crypto_secretbox_NONCEBYTES) | Ciphertext.
/// Returns an empty vector if libsodium is not available or on error.
std::vector<uint8_t> encrypt(const std::string& content,
                             const std::vector<uint8_t>& password);

/// Decrypt content that was encrypted with the encrypt function.
/// Returns decrypted bytes, or empty vector on error.
std::vector<uint8_t> decrypt(const std::vector<uint8_t>& encrypted,
                             const std::vector<uint8_t>& password);

/// Check if the encrypted data has a valid RAM header.
bool has_ram_header(const std::vector<uint8_t>& data);

}  // namespace ram
