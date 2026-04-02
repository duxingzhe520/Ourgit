#include "OurGitObject.hpp"
#include "Tree.hpp"
#include <string>
#include <unordered_map>

/**
 * @author sry
 */

class Index : public OurGitObject {
    /** The key is path, the value is blob's hashcode.  */
    std::unordered_map<std::string, std::string> entries;

    public:
        Index() = default;

        std::string serialize();

        std::string getType();

        static Index deserialize(const std::string& data);

        /** Before this function is called, ensure the Blob has existed.  */
        void addBlob(const std::string& path, const std::string& blobHash);

        /** Before this function is called, ensure the Blob has existed.  */
        void removeBlob(const std::string& path);

        bool contains(const std::string& path);

        Tree& buildTree();

        void clear();

        const std::unordered_map<std::string, std::string>& getEntries();
};
