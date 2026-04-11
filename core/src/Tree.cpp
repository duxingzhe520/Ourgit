#include "../include/Tree.hpp"
#include <sstream>
#include <stdexcept>

/**
 * Serialization format:
 *   Tree\n
 *   <num_entries>\n
 *   <mode>\t<type>\t<hash>\t<name>\n   (one line per entry)
 *
 * Fields are tab-separated. File names may contain spaces but must not contain
 * tabs.  The mode field may be left empty ("") for entries where it is unused.
 */

std::string Tree::serialize() {
    std::ostringstream oss;
    oss << "Tree\n" << treeEntries.size() << "\n";
    for (const auto& e : treeEntries)
        oss << e.mode << "\t" << e.type << "\t" << e.hash << "\t" << e.name << "\n";
    return oss.str();
}

Tree Tree::deserialize(const std::string& data) {
    Tree tree;
    std::istringstream iss(data);
    std::string line;

    // Line 1: "Tree"
    if (!std::getline(iss, line) || line != "Tree")
        throw std::invalid_argument("Tree::deserialize: missing Tree header");

    // Line 2: number of entries
    if (!std::getline(iss, line))
        throw std::invalid_argument("Tree::deserialize: missing entry count");
    int n = std::stoi(line);

    for (int i = 0; i < n; ++i) {
        if (!std::getline(iss, line))
            throw std::invalid_argument("Tree::deserialize: unexpected end of data");

        std::istringstream ls(line);
        TreeEntry e;
        std::getline(ls, e.mode, '\t');
        std::getline(ls, e.type, '\t');
        std::getline(ls, e.hash, '\t');
        std::getline(ls, e.name);
        tree.addEntry(e);
    }
    return tree;
}
