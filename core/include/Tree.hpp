#pragma once
#include "OurGitObject.hpp"
#include <vector>

/**
 * @author yrc
 */

struct TreeEntry {
    /** I don't know if we need it.  */
    std::string mode;

    /** The name of the file, if the type is "blob".
     *  The name of the directory, if the type is "tree".*/
    std::string name;

    /** The hashcode.  */
    std::string hash;

    /** "blob" if file, "tree" if directory.  */
    std::string type;
};

class Tree : public OurGitObject {
    std::vector<TreeEntry> treeEntries;

    public:
        Tree() = default;

        std::string serialize() override;

        Tree deserialize(const std::string& data);

        std::string getType() override {return "Tree";};

        void addEntry(const TreeEntry& entry) {treeEntries.push_back(entry);};

        const std::vector<TreeEntry>& getEntries() {return treeEntries;};
};
