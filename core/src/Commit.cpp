#include "../include/Commit.hpp"
#include <sstream>
#include <stdexcept>
#include <ctime>

/**
 * Serialization format:
 *   Commit\n
 *   author: <author>\n
 *   time: <unix_timestamp>\n
 *   treeHash: <hash>\n
 *   parents: <hash1> <hash2> ...\n
 *   message: <message>\n
 *
 * The message is always the last field so that future multi-line messages can
 * be accommodated by reading until EOF after the "message: " prefix.
 */

Commit::Commit(const std::string& author,
               const std::string& message,
               const std::string& treeHash,
               const std::vector<std::string>& parentCommitHash)
    : author(author),
      message(message),
      treeHash(treeHash),
      parentCommitHash(parentCommitHash)
{
    time = std::to_string(std::time(nullptr));
}

std::string Commit::serialize() {
    std::ostringstream oss;
    oss << "Commit\n";
    oss << "author: "   << author   << "\n";
    oss << "time: "     << time     << "\n";
    oss << "treeHash: " << treeHash << "\n";

    oss << "parents:";
    for (const auto& p : parentCommitHash)
        oss << " " << p;
    oss << "\n";

    oss << "message: " << message << "\n";
    return oss.str();
}

Commit Commit::deserialize(const std::string& data) {
    Commit c;
    std::istringstream iss(data);
    std::string line;

    // Line 1: "Commit"
    std::getline(iss, line);

    auto readValue = [](const std::string& line, const std::string& key) -> std::string {
        return line.substr(key.size());
    };

    while (std::getline(iss, line)) {
        if (line.rfind("author: ", 0) == 0)
            c.author = readValue(line, "author: ");
        else if (line.rfind("time: ", 0) == 0)
            c.time = readValue(line, "time: ");
        else if (line.rfind("treeHash: ", 0) == 0)
            c.treeHash = readValue(line, "treeHash: ");
        else if (line.rfind("parents:", 0) == 0) {
            // "parents: hash1 hash2 ..." or "parents:" (empty)
            std::string rest = line.substr(std::string("parents:").size());
            std::istringstream ps(rest);
            std::string h;
            while (ps >> h)
                c.parentCommitHash.push_back(h);
        } else if (line.rfind("message: ", 0) == 0) {
            c.message = readValue(line, "message: ");
        }
    }
    return c;
}

const std::string& Commit::getTreeHash() {
    return treeHash;
}

const std::string& Commit::getMessage() {
    return message;
}

bool Commit::isMerge() {
    return parentCommitHash.size() > 1;
}

std::vector<std::string>& Commit::getParents() {
    return parentCommitHash;
}
