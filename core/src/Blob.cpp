#include "../include/Blob.hpp"
#include <sstream>
#include <stdexcept>

/**
 * Serialization format:
 *   Blob\n
 *   <byte_length>\n
 *   <raw_content>
 *
 * Storing the byte length allows content that itself contains newlines to be
 * round-tripped without ambiguity.
 */

std::string Blob::serialize() {
    std::ostringstream oss;
    oss << "Blob\n" << content.size() << "\n" << content;
    return oss.str();
}

Blob Blob::deserialize(const std::string& data) {
    // Line 1: "Blob"
    size_t pos1 = data.find('\n');
    if (pos1 == std::string::npos)
        throw std::invalid_argument("Blob::deserialize: malformed data");

    // Line 2: byte length
    size_t pos2 = data.find('\n', pos1 + 1);
    if (pos2 == std::string::npos)
        throw std::invalid_argument("Blob::deserialize: missing length line");

    size_t len = std::stoul(data.substr(pos1 + 1, pos2 - pos1 - 1));

    // Remainder: raw content (exactly len bytes)
    return Blob(data.substr(pos2 + 1, len));
}
