#include "../include/Index.hpp"
#include <filesystem>
#include <map>
#include <sstream>
#include <stdexcept>

/**
 * Serialization format:
 *   Index\n
 *   <num_entries>\n
 *   <path>\t<blobHash>\n   (one line per staged file)
 */

std::string Index::getType() {
    return "Index";
}

std::string Index::serialize() {
    std::ostringstream oss;
    oss << "Index\n" << entries.size() << "\n";
    for (const auto& [path, hash] : entries)
        oss << path << "\t" << hash << "\n";
    return oss.str();
}

Index Index::deserialize(const std::string& data) {
    Index idx;
    std::istringstream iss(data);
    std::string line;

    // Line 1: "Index"
    std::getline(iss, line);

    // Line 2: entry count
    std::getline(iss, line);
    int n = std::stoi(line);

    for (int i = 0; i < n; ++i) {
        if (!std::getline(iss, line))
            throw std::invalid_argument("Index::deserialize: unexpected end of data");
        size_t tab = line.find('\t');
        if (tab == std::string::npos)
            throw std::invalid_argument("Index::deserialize: malformed entry line");
        std::string path = line.substr(0, tab);
        std::string hash = line.substr(tab + 1);
        idx.entries[path] = hash;
    }
    return idx;
}

void Index::addBlob(const std::string& path, const std::string& blobHash) {
    entries[path] = blobHash;
}

void Index::removeBlob(const std::string& path) {
    entries.erase(path);
}

bool Index::contains(const std::string& path) {
    return entries.count(path) > 0;
}

void Index::clear() {
    entries.clear();
}

const std::unordered_map<std::string, std::string>& Index::getEntries() {
    return entries;
}

// ── buildTree helpers ─────────────────────────────────────────────────────────

struct DirNode {
    std::map<std::string, std::string> files;    // filename → blobHash
    std::map<std::string, DirNode>     subdirs;  // dirname  → DirNode
};

static void insertPath(DirNode& node,
                       const std::string& path,
                       const std::string& blobHash) {
    size_t slash = path.find('/');
    if (slash == std::string::npos) {
        node.files[path] = blobHash;
    } else {
        std::string head = path.substr(0, slash);
        std::string tail = path.substr(slash + 1);
        insertPath(node.subdirs[head], tail, blobHash);
    }
}

static Tree buildTreeFromNode(const DirNode& node) {
    Tree tree;

    // Files at this level
    for (const auto& [name, hash] : node.files)
        tree.addEntry({"100644", name, hash, "blob"});

    // Sub-directories: recurse, save sub-tree, add its hash as a tree entry
    for (const auto& [name, subdir] : node.subdirs) {
        Tree subtree = buildTreeFromNode(subdir);
        subtree.save();  // persist so we can get a stable hash
        tree.addEntry({"040000", name, subtree.getHash(), "tree"});
    }

    return tree;
}

Tree Index::buildTree() {
    DirNode root;
    for (const auto& [path, blobHash] : entries)
        insertPath(root, path, blobHash);
    return buildTreeFromNode(root);
}
