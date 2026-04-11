#include "../include/Repository.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace fs = std::filesystem;

// ── Static member definitions ────────────────────────────────────────────────

const std::string Repository::CWD = fs::current_path().string();
const std::string Repository::OUR_GIT_DIRECTORY = CWD + "/.OurGit";

// ── Internal helpers (file-system I/O) ───────────────────────────────────────

static std::string readFile(const fs::path& p) {
    std::ifstream f(p);
    if (!f) throw std::runtime_error("Cannot open file: " + p.string());
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

static void writeFile(const fs::path& p, const std::string& content) {
    fs::create_directories(p.parent_path());
    std::ofstream f(p);
    if (!f) throw std::runtime_error("Cannot write file: " + p.string());
    f << content;
}

static Index loadIndex() {
    fs::path indexPath = ".OurGit/Index";
    if (!fs::exists(indexPath)) return Index{};
    return Index{}.deserialize(readFile(indexPath));
}

static void saveIndex(Index& idx) {
    writeFile(".OurGit/Index", idx.serialize());
}

static std::string readHead() {
    fs::path headPath = ".OurGit/refs/HEAD";
    if (!fs::exists(headPath)) return "0";
    std::string h = readFile(headPath);
    // strip trailing newline
    while (!h.empty() && (h.back() == '\n' || h.back() == '\r'))
        h.pop_back();
    return h;
}

static void writeHead(const std::string& hash) {
    writeFile(".OurGit/refs/HEAD", hash + "\n");
}

static std::string readAuthor() {
    fs::path p = ".OurGit/refs/author";
    if (!fs::exists(p)) return "unknown";
    std::string a = readFile(p);
    while (!a.empty() && (a.back() == '\n' || a.back() == '\r'))
        a.pop_back();
    return a;
}

// ── Repository operations ────────────────────────────────────────────────────

bool Repository::init(const std::string& author) {
    if (fs::exists(".OurGit")) {
        std::cout << "Repository already initialised.\n";
        return false;
    }

    fs::create_directories(".OurGit/objects");
    fs::create_directories(".OurGit/refs");

    writeFile(".OurGit/refs/HEAD",   "0\n");
    writeFile(".OurGit/refs/branch", "main\n");
    writeFile(".OurGit/refs/author", author + "\n");

    // Save an empty Index
    Index emptyIdx;
    saveIndex(emptyIdx);

    std::cout << "Initialised empty OurGit repository (author: " << author << ")\n";
    return true;
}

bool Repository::add(const std::string& path) {
    if (!fs::exists(path)) {
        std::cerr << "error: '" << path << "' did not match any files\n";
        return false;
    }

    // Create and persist the blob
    std::string content = readFile(path);
    Blob blob(content);
    blob.save();                         // writes to .OurGit/objects/<hash>

    // Update the staging index
    Index idx = loadIndex();
    idx.addBlob(path, blob.getHash());
    saveIndex(idx);

    std::cout << "Staged: " << path << " (" << blob.getHash() << ")\n";
    return true;
}

bool Repository::add(const std::vector<std::string>& paths) {
    bool ok = true;
    for (const auto& p : paths)
        ok = add(p) && ok;
    return ok;
}

bool Repository::commit(const std::string& message) {
    Index idx = loadIndex();
    if (idx.getEntries().empty()) {
        std::cout << "Nothing to commit (staging area is empty).\n";
        return false;
    }

    // Build tree from index and save it
    Tree tree = idx.buildTree();
    tree.save();

    std::string parentHash = readHead();
    std::string author     = readAuthor();

    Commit c(author, message, tree.getHash(), {parentHash});
    c.save();

    // Advance HEAD to the new commit
    writeHead(c.getHash());

    // Clear the staging area
    idx.clear();
    saveIndex(idx);

    std::cout << "[main " << c.getHash().substr(0, 7) << "] " << message << "\n";
    return true;
}

void Repository::status() {
    std::cout << "On branch main\n\n";
    Index idx = loadIndex();
    const auto& entries = idx.getEntries();
    if (entries.empty()) {
        std::cout << "Nothing to commit (staging area is empty).\n";
        return;
    }
    std::cout << "Changes to be committed:\n";
    for (const auto& [path, hash] : entries)
        std::cout << "    " << path << "  (" << hash.substr(0, 7) << ")\n";
}

void Repository::log() {
    std::string hash = readHead();
    if (hash == "0") {
        std::cout << "No commits yet.\n";
        return;
    }

    int count = 0;
    while (hash != "0") {
        fs::path objPath = fs::path(".OurGit/objects") / hash;
        if (!fs::exists(objPath)) {
            std::cerr << "error: object " << hash << " not found\n";
            break;
        }

        std::string raw = readFile(objPath);
        Commit c = Commit{}.deserialize(raw);

        // Print header line then all key-value fields from the serialized form
        std::cout << "commit " << hash << "\n";
        {
            std::istringstream iss(raw);
            std::string line;
            std::getline(iss, line); // skip "Commit" type line
            while (std::getline(iss, line))
                std::cout << "    " << line << "\n";
        }
        std::cout << "\n";

        // Follow the first parent
        auto& parents = c.getParents();
        hash = parents.empty() ? "0" : parents[0];
        ++count;
    }
    std::cout << count << " commit(s) total.\n";
}
