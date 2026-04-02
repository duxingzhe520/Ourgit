#pragma once
#include "OurGitObject.hpp"
#include <string>

/**
 * @author yrc
 */

class Blob : public OurGitObject {
    std::string content;

    public:
        Blob() = default;
        explicit Blob(const std::string& content) : content(content) {};

        std::string serialize() override;

        Blob deserialize(const std::string& data);

        std::string getType() override {return "Blob";};

        void setContent(const std::string& content) { this->content = content; }; 

        const std::string& getContent() { return content; };
};
