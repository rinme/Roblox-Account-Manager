#include "ram/utilities.h"

#include <array>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>

// Portable MD5 and SHA-256 implementations
// Using simple public domain implementations for cross-platform support

namespace {

// --- MD5 Implementation (RFC 1321) ---
// Based on public domain implementation

struct MD5Context {
    uint32_t state[4];
    uint64_t count;
    uint8_t buffer[64];
};

constexpr uint32_t md5_f(uint32_t x, uint32_t y, uint32_t z) {
    return (x & y) | (~x & z);
}
constexpr uint32_t md5_g(uint32_t x, uint32_t y, uint32_t z) {
    return (x & z) | (y & ~z);
}
constexpr uint32_t md5_h(uint32_t x, uint32_t y, uint32_t z) {
    return x ^ y ^ z;
}
constexpr uint32_t md5_i(uint32_t x, uint32_t y, uint32_t z) {
    return y ^ (x | ~z);
}
constexpr uint32_t rotate_left(uint32_t x, int n) {
    return (x << n) | (x >> (32 - n));
}

static const uint32_t md5_t_table[64] = {
    0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
    0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
    0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
    0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
    0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
    0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
    0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
    0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
    0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
    0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
    0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
    0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
    0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
    0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
    0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
    0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
};

static const int md5_shift[64] = {
    7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
    5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20,
    4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
    6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21
};

void md5_init(MD5Context& ctx) {
    ctx.state[0] = 0x67452301;
    ctx.state[1] = 0xefcdab89;
    ctx.state[2] = 0x98badcfe;
    ctx.state[3] = 0x10325476;
    ctx.count = 0;
}

void md5_transform(MD5Context& ctx, const uint8_t block[64]) {
    uint32_t a = ctx.state[0], b = ctx.state[1];
    uint32_t c = ctx.state[2], d = ctx.state[3];
    uint32_t m[16];

    for (int i = 0; i < 16; i++) {
        m[i] = static_cast<uint32_t>(block[i * 4]) |
               (static_cast<uint32_t>(block[i * 4 + 1]) << 8) |
               (static_cast<uint32_t>(block[i * 4 + 2]) << 16) |
               (static_cast<uint32_t>(block[i * 4 + 3]) << 24);
    }

    for (int i = 0; i < 64; i++) {
        uint32_t f_val, g;
        if (i < 16) {
            f_val = md5_f(b, c, d);
            g = i;
        } else if (i < 32) {
            f_val = md5_g(b, c, d);
            g = (5 * i + 1) % 16;
        } else if (i < 48) {
            f_val = md5_h(b, c, d);
            g = (3 * i + 5) % 16;
        } else {
            f_val = md5_i(b, c, d);
            g = (7 * i) % 16;
        }

        uint32_t temp = d;
        d = c;
        c = b;
        b = b + rotate_left(a + f_val + md5_t_table[i] + m[g], md5_shift[i]);
        a = temp;
    }

    ctx.state[0] += a;
    ctx.state[1] += b;
    ctx.state[2] += c;
    ctx.state[3] += d;
}

void md5_update(MD5Context& ctx, const uint8_t* data, size_t len) {
    size_t index = ctx.count % 64;
    ctx.count += len;

    size_t i = 0;
    if (index) {
        size_t part_len = 64 - index;
        if (len >= part_len) {
            std::memcpy(ctx.buffer + index, data, part_len);
            md5_transform(ctx, ctx.buffer);
            i = part_len;
        } else {
            std::memcpy(ctx.buffer + index, data, len);
            return;
        }
    }

    for (; i + 64 <= len; i += 64) {
        md5_transform(ctx, data + i);
    }

    if (i < len) {
        std::memcpy(ctx.buffer, data + i, len - i);
    }
}

void md5_final(MD5Context& ctx, uint8_t digest[16]) {
    uint8_t padding[64] = {0x80};
    uint64_t bits = ctx.count * 8;
    size_t index = ctx.count % 64;
    size_t pad_len = (index < 56) ? (56 - index) : (120 - index);

    md5_update(ctx, padding, pad_len);

    uint8_t bits_buf[8];
    for (int i = 0; i < 8; i++) {
        bits_buf[i] = static_cast<uint8_t>(bits >> (i * 8));
    }
    md5_update(ctx, bits_buf, 8);

    for (int i = 0; i < 4; i++) {
        digest[i * 4] = static_cast<uint8_t>(ctx.state[i]);
        digest[i * 4 + 1] = static_cast<uint8_t>(ctx.state[i] >> 8);
        digest[i * 4 + 2] = static_cast<uint8_t>(ctx.state[i] >> 16);
        digest[i * 4 + 3] = static_cast<uint8_t>(ctx.state[i] >> 24);
    }
}

// --- SHA-256 Implementation ---

struct SHA256Context {
    uint32_t state[8];
    uint64_t count;
    uint8_t buffer[64];
};

static const uint32_t sha256_k[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
    0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
    0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
    0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
    0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
    0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

constexpr uint32_t rotr(uint32_t x, int n) {
    return (x >> n) | (x << (32 - n));
}
constexpr uint32_t sha256_ch(uint32_t x, uint32_t y, uint32_t z) {
    return (x & y) ^ (~x & z);
}
constexpr uint32_t sha256_maj(uint32_t x, uint32_t y, uint32_t z) {
    return (x & y) ^ (x & z) ^ (y & z);
}
constexpr uint32_t sha256_sigma0(uint32_t x) {
    return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22);
}
constexpr uint32_t sha256_sigma1(uint32_t x) {
    return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25);
}
constexpr uint32_t sha256_gamma0(uint32_t x) {
    return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3);
}
constexpr uint32_t sha256_gamma1(uint32_t x) {
    return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10);
}

void sha256_init(SHA256Context& ctx) {
    ctx.state[0] = 0x6a09e667;
    ctx.state[1] = 0xbb67ae85;
    ctx.state[2] = 0x3c6ef372;
    ctx.state[3] = 0xa54ff53a;
    ctx.state[4] = 0x510e527f;
    ctx.state[5] = 0x9b05688c;
    ctx.state[6] = 0x1f83d9ab;
    ctx.state[7] = 0x5be0cd19;
    ctx.count = 0;
}

void sha256_transform(SHA256Context& ctx, const uint8_t block[64]) {
    uint32_t w[64];
    for (int i = 0; i < 16; i++) {
        w[i] = (static_cast<uint32_t>(block[i * 4]) << 24) |
               (static_cast<uint32_t>(block[i * 4 + 1]) << 16) |
               (static_cast<uint32_t>(block[i * 4 + 2]) << 8) |
               static_cast<uint32_t>(block[i * 4 + 3]);
    }
    for (int i = 16; i < 64; i++) {
        w[i] = sha256_gamma1(w[i - 2]) + w[i - 7] +
               sha256_gamma0(w[i - 15]) + w[i - 16];
    }

    uint32_t a = ctx.state[0], b = ctx.state[1], c = ctx.state[2],
             d = ctx.state[3];
    uint32_t e = ctx.state[4], f = ctx.state[5], g = ctx.state[6],
             h = ctx.state[7];

    for (int i = 0; i < 64; i++) {
        uint32_t t1 =
            h + sha256_sigma1(e) + sha256_ch(e, f, g) + sha256_k[i] + w[i];
        uint32_t t2 = sha256_sigma0(a) + sha256_maj(a, b, c);
        h = g;
        g = f;
        f = e;
        e = d + t1;
        d = c;
        c = b;
        b = a;
        a = t1 + t2;
    }

    ctx.state[0] += a;
    ctx.state[1] += b;
    ctx.state[2] += c;
    ctx.state[3] += d;
    ctx.state[4] += e;
    ctx.state[5] += f;
    ctx.state[6] += g;
    ctx.state[7] += h;
}

void sha256_update(SHA256Context& ctx, const uint8_t* data, size_t len) {
    size_t index = ctx.count % 64;
    ctx.count += len;

    size_t i = 0;
    if (index) {
        size_t part_len = 64 - index;
        if (len >= part_len) {
            std::memcpy(ctx.buffer + index, data, part_len);
            sha256_transform(ctx, ctx.buffer);
            i = part_len;
        } else {
            std::memcpy(ctx.buffer + index, data, len);
            return;
        }
    }

    for (; i + 64 <= len; i += 64) {
        sha256_transform(ctx, data + i);
    }

    if (i < len) {
        std::memcpy(ctx.buffer, data + i, len - i);
    }
}

void sha256_final(SHA256Context& ctx, uint8_t digest[32]) {
    uint8_t padding[64] = {0x80};
    uint64_t bits = ctx.count * 8;
    size_t index = ctx.count % 64;
    size_t pad_len = (index < 56) ? (56 - index) : (120 - index);

    sha256_update(ctx, padding, pad_len);

    uint8_t bits_buf[8];
    for (int i = 0; i < 8; i++) {
        bits_buf[i] = static_cast<uint8_t>(bits >> ((7 - i) * 8));
    }
    sha256_update(ctx, bits_buf, 8);

    for (int i = 0; i < 8; i++) {
        digest[i * 4] = static_cast<uint8_t>(ctx.state[i] >> 24);
        digest[i * 4 + 1] = static_cast<uint8_t>(ctx.state[i] >> 16);
        digest[i * 4 + 2] = static_cast<uint8_t>(ctx.state[i] >> 8);
        digest[i * 4 + 3] = static_cast<uint8_t>(ctx.state[i]);
    }
}

std::string to_hex_upper(const uint8_t* data, size_t len) {
    std::ostringstream oss;
    oss << std::uppercase << std::hex << std::setfill('0');
    for (size_t i = 0; i < len; i++) {
        oss << std::setw(2) << static_cast<int>(data[i]);
    }
    return oss.str();
}

}  // namespace

namespace ram {

std::string md5(const std::string& input) {
    MD5Context ctx;
    md5_init(ctx);
    md5_update(ctx, reinterpret_cast<const uint8_t*>(input.data()),
               input.size());
    uint8_t digest[16];
    md5_final(ctx, digest);
    return to_hex_upper(digest, 16);
}

std::string file_sha256(const std::string& filename) {
    if (!std::filesystem::exists(filename)) {
        return "E3B0C44298FC1C149AFBF4C8996FB92427AE41E4649B934CA495991B7852B855";
    }

    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        return "E3B0C44298FC1C149AFBF4C8996FB92427AE41E4649B934CA495991B7852B855";
    }

    SHA256Context ctx;
    sha256_init(ctx);

    std::array<uint8_t, 8192> buf{};
    while (file.read(reinterpret_cast<char*>(buf.data()), buf.size()) ||
           file.gcount() > 0) {
        sha256_update(ctx, buf.data(), static_cast<size_t>(file.gcount()));
    }

    uint8_t digest[32];
    sha256_final(ctx, digest);
    return to_hex_upper(digest, 32);
}

double to_roblox_tick(
    const std::chrono::system_clock::time_point& time_point) {
    auto duration = time_point.time_since_epoch();
    auto seconds =
        std::chrono::duration_cast<std::chrono::seconds>(duration).count();
    auto ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    double fractional = static_cast<double>(ms % 1000) / 1000.0;
    return static_cast<double>(seconds) + fractional;
}

bool recursive_delete(const std::string& path) {
    std::error_code ec;
    std::filesystem::remove_all(path, ec);
    return !ec;
}

}  // namespace ram
