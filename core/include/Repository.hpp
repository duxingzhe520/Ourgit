#include "Blob.hpp"
#include "Tree.hpp"
#include "Commit.hpp"
#include "Index.hpp"
#include <string>
#include <vector>
#include <iostream>

class Repository {
    public :
        /** Current Working Directory */
        const static std::string CWD;

        /** .OurGit's path */
        const static std::string OUR_GIT_DIRECTORY;

        /** Initialize OUR_GIT_DIRECTORY, CommitZero, IndexZero. 
         * @author yrc
        */
        static bool init(const std::string& author);

        /** TODO: */
        static bool add(const std::vector<std::string>& paths);

        /** Create a blob, update the ref to the blob.
         * @author yrc
         */
        static bool add(const std::string& path);

        /** Index --> Tree --> hashcode存到commit里，再算commit的哈希值，别忘了改HEAD存的哈希值 
         * @author sry
        */
        static bool commit(const std::string& message);

        /** 暂时随便输出一些存的东西  all to test
         * @author yrc
        */
        static void status();

        /** 暂时把所有commit都输出即可 all to test
         * @author sry
        */
        static void log();

    private :
        static std::string& getCurrentCommitHash();

        static std::string& getAuthor();

        static Index& getCurrentIndex();

        static void changeHEAD(std::string newCommitHash);
};