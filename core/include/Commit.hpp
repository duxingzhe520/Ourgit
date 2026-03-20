#pragma once
#include "OurGitObject.hpp"
#include <string>
#include <vector>

/**
 * @author sry
 */

class Commit : public OurGitObject {
    std::string time;
    std::string author;
    std::string message;
    std::string treeHash;

    /** It's size > 1 iff the commit is mergeCommit, i.e. this->isMerge() == true.  */
    std::vector<std::string> parentCommitHash;

    public:
        Commit() = default;

        /** When implemented, please call std::time().  */
        Commit(const std::string& author, const std::string& message, const std::string& treeHash, const std::vector<std::string>& parentCommitHash);

        std::string serialize();

        Commit deserialize(const std::string& data);

        std::string getType() {return "Commit";};

        const std::string& getTreeHash();

        const std::string& getMessage();

        bool isMerge();

        std::vector<std::string>& getParents();
};
