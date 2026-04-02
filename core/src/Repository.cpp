#include "../include/Repository.hpp"

bool Repository::commit(const std::string& message) {
    // read current branch and commit
    std::string parentCommit = getCurrentCommitHash();

    std::vector<std::string> parentCommitHash = {parentCommit};

    std::string author = getAuthor();

    Index index = getCurrentIndex();

    Tree tree = index.buildTree();

    std::string treeHash = tree.getHash();

    Commit thisCommit(author, message, treeHash, parentCommitHash);

    tree.save();
    thisCommit.save();
    index.clear();

    changeHEAD(thisCommit.getHash());
}

void Repository::log() {
    Commit thisCommit = Commit::deserialize(getCurrentCommitHash());

    std::cout << thisCommit.printOut() << std::endl << std::endl;

    Commit* tmp = &thisCommit;
    while(1) {
        std::cout << tmp->printOut() << std::endl << std::endl;
        // TODO:
    }
}