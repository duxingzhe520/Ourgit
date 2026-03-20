#pragma once
#include <string>

/**
 * @author yrc & sry
 */

//TODO: include more libs when needed.

class OurGitObject {
    public:
        /** Returns the obj's sequence.  */
        virtual std::string serialize();

        /** Returns the obj's type. e.g. Blob, Tree, etc.  */
        virtual std::string getType();

        /** Gets Hashcode, using SHA-1.  */
        std::string getHash();

        /** If any param is needed, add it.  */
        void save();
};
