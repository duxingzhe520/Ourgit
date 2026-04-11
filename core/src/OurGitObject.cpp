#include "../include/OurGitObject.hpp"
#include <fstream>
#include <filesystem>
#include <sstream>
#include <iomanip>
#include <cstdint>

// ── Self-contained SHA-1 ──────────────────────────────────────────────────────

static uint32_t leftRotate(uint32_t val, uint32_t n) {
    return (val << n) | (val >> (32 - n));
}

static std::string sha1(const std::string& input) {
    uint32_t h0 = 0x67452301u;
    uint32_t h1 = 0xEFCDAB89u;
    uint32_t h2 = 0x98BADCFEu;
    uint32_t h3 = 0x10325476u;
    uint32_t h4 = 0xC3D2E1F0u;

    std::string msg = input;
    uint64_t bitLen = static_cast<uint64_t>(input.size()) * 8;

    // Padding: append 0x80 then zeros until length ≡ 56 (mod 64)
    msg += static_cast<char>(0x80);
    while (msg.size() % 64 != 56)
        msg += static_cast<char>(0x00);

    // Append original length as 64-bit big-endian
    for (int i = 7; i >= 0; --i)
        msg += static_cast<char>((bitLen >> (i * 8)) & 0xFF);

    // Process each 512-bit (64-byte) chunk
    for (size_t i = 0; i < msg.size(); i += 64) {
        uint32_t w[80];
        for (int j = 0; j < 16; ++j) {
            w[j] = (static_cast<uint8_t>(msg[i + j*4])     << 24)
                 | (static_cast<uint8_t>(msg[i + j*4 + 1]) << 16)
                 | (static_cast<uint8_t>(msg[i + j*4 + 2]) << 8)
                 |  static_cast<uint8_t>(msg[i + j*4 + 3]);
        }
        for (int j = 16; j < 80; ++j)
            w[j] = leftRotate(w[j-3] ^ w[j-8] ^ w[j-14] ^ w[j-16], 1);

        uint32_t a = h0, b = h1, c = h2, d = h3, e = h4;
        for (int j = 0; j < 80; ++j) {
            uint32_t f, k;
            if (j < 20) {
                f = (b & c) | (~b & d);
                k = 0x5A827999u;
            } else if (j < 40) {
                f = b ^ c ^ d;
                k = 0x6ED9EBA1u;
            } else if (j < 60) {
                f = (b & c) | (b & d) | (c & d);
                k = 0x8F1BBCDCu;
            } else {
                f = b ^ c ^ d;
                k = 0xCA62C1D6u;
            }
            uint32_t temp = leftRotate(a, 5) + f + e + k + w[j];
            e = d; d = c; c = leftRotate(b, 30); b = a; a = temp;
        }
        h0 += a; h1 += b; h2 += c; h3 += d; h4 += e;
    }

    std::ostringstream oss;
    oss << std::hex << std::setfill('0')
        << std::setw(8) << h0
        << std::setw(8) << h1
        << std::setw(8) << h2
        << std::setw(8) << h3
        << std::setw(8) << h4;
    return oss.str();
}

// ── OurGitObject methods ──────────────────────────────────────────────────────

std::string OurGitObject::serialize() {
    return "";
}

std::string OurGitObject::getType() {
    return "";
}

std::string OurGitObject::getHash() {
    return sha1(serialize());
}

void OurGitObject::save() {
    namespace fs = std::filesystem;
    const fs::path objDir = ".OurGit/objects";
    fs::create_directories(objDir);
    std::ofstream f(objDir / getHash());
    f << serialize();
}
