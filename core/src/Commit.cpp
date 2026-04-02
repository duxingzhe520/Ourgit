#include "../include/Commit.hpp"

Commit::Commit(const std::string& author, const std::string& message, const std::string& treeHash, const std::vector<std::string>& parentCommitHash) {
    this->time = getStdTime();
    this->author = author;
    this->message = message;
    this->treeHash = treeHash;
    this->parentCommitHash = parentCommitHash;
}

std::string Commit::getStdTime() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

const std::string& Commit::getTreeHash() {
    return treeHash;
}

const std::string& Commit::getMessage() {
    return message;
}

bool Commit::isMerge() {
    return treeHash.size() > 1;
}

std::vector<std::string>& Commit::getParents() {
    return parentCommitHash;
}