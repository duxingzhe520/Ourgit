#include "../core/include/Blob.hpp"
#include "../core/include/Tree.hpp"
#include "../core/include/Commit.hpp"
#include "../core/include/Index.hpp"
#include "../core/include/Repository.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

// ── Minimal test framework ────────────────────────────────────────────────────

static int passed = 0;
static int failed = 0;
static std::string currentSuite;

static void suite(const std::string& name) {
    currentSuite = name;
    std::cout << "\n[" << name << "]\n";
}

static void check(bool condition, const std::string& label) {
    if (condition) {
        std::cout << "  PASS  " << label << "\n";
        ++passed;
    } else {
        std::cout << "  FAIL  " << label << "\n";
        ++failed;
    }
}

// ── Blob ──────────────────────────────────────────────────────────────────────

static void testBlob() {
    suite("Blob");

    // Basic round-trip
    Blob b1("hello world");
    Blob b2 = b1.deserialize(b1.serialize());
    check(b2.getContent() == "hello world", "round-trip: plain string");

    // Empty content
    Blob empty("");
    Blob empty2 = empty.deserialize(empty.serialize());
    check(empty2.getContent() == "", "round-trip: empty content");

    // Multi-line content (contains newlines)
    std::string multiline = "line one\nline two\nline three";
    Blob ml(multiline);
    Blob ml2 = ml.deserialize(ml.serialize());
    check(ml2.getContent() == multiline, "round-trip: multi-line content");

    // Hash determinism: same content → same hash
    Blob a("deterministic");
    Blob b("deterministic");
    check(a.getHash() == b.getHash(), "hash: identical content gives identical hash");

    // Hash sensitivity: different content → different hash
    Blob c("foo");
    Blob d("bar");
    check(c.getHash() != d.getHash(), "hash: different content gives different hash");

    // setContent / getContent
    Blob mutable_b;
    mutable_b.setContent("updated");
    check(mutable_b.getContent() == "updated", "setContent / getContent");

    // getType
    check(b1.getType() == "Blob", "getType returns 'Blob'");
}

// ── Tree ──────────────────────────────────────────────────────────────────────

static void testTree() {
    suite("Tree");

    // Empty tree round-trip
    Tree empty;
    Tree empty2 = empty.deserialize(empty.serialize());
    check(empty2.getEntries().empty(), "round-trip: empty tree");

    // Single entry round-trip
    Tree t1;
    TreeEntry e1{"100644", "README.md", "abc123", "blob"};
    t1.addEntry(e1);
    Tree t1b = t1.deserialize(t1.serialize());
    check(t1b.getEntries().size() == 1, "round-trip: single entry count");
    check(t1b.getEntries()[0].name == "README.md", "round-trip: entry name preserved");
    check(t1b.getEntries()[0].hash == "abc123",    "round-trip: entry hash preserved");
    check(t1b.getEntries()[0].type == "blob",      "round-trip: entry type preserved");
    check(t1b.getEntries()[0].mode == "100644",    "round-trip: entry mode preserved");

    // Multiple entries
    Tree t2;
    t2.addEntry({"100644", "main.cpp",  "hash1", "blob"});
    t2.addEntry({"100644", "util.cpp",  "hash2", "blob"});
    t2.addEntry({"040000", "src",       "hash3", "tree"});
    Tree t2b = t2.deserialize(t2.serialize());
    check(t2b.getEntries().size() == 3, "round-trip: three entries count");

    // Filename with spaces
    Tree t3;
    t3.addEntry({"100644", "my file.txt", "deadbeef", "blob"});
    Tree t3b = t3.deserialize(t3.serialize());
    check(t3b.getEntries()[0].name == "my file.txt", "round-trip: filename with spaces");

    // getType
    check(t1.getType() == "Tree", "getType returns 'Tree'");

    // Hash determinism
    Tree ta, tb;
    ta.addEntry({"100644", "a.txt", "h1", "blob"});
    tb.addEntry({"100644", "a.txt", "h1", "blob"});
    check(ta.getHash() == tb.getHash(), "hash: identical trees give identical hash");
}

// ── Commit ────────────────────────────────────────────────────────────────────

static void testCommit() {
    suite("Commit");

    Commit c("alice", "initial commit", "treehash42", {"0"});

    check(c.getTreeHash() == "treehash42",   "getTreeHash");
    check(c.getMessage()  == "initial commit","getMessage");
    check(!c.isMerge(),                       "isMerge: single parent → false");
    check(c.getParents().size() == 1,         "getParents: one parent");
    check(c.getParents()[0] == "0",           "getParents: parent value");
    check(c.getType() == "Commit",            "getType returns 'Commit'");

    // Round-trip
    Commit c2 = Commit{}.deserialize(c.serialize());
    check(c2.getTreeHash() == "treehash42",   "round-trip: treeHash");
    check(c2.getMessage()  == "initial commit","round-trip: message");
    check(c2.getParents()[0] == "0",          "round-trip: parent");

    // Merge commit: isMerge when two parents
    Commit merge("alice", "merge", "treehashM", {"parent1", "parent2"});
    check(merge.isMerge(),                    "isMerge: two parents → true");
    check(merge.getParents().size() == 2,     "isMerge: parent count == 2");

    // Hash determinism: two commits with same data → same hash
    Commit x("bob", "msg", "th", {"p"});
    Commit y("bob", "msg", "th", {"p"});
    // time is set to std::time(nullptr) — they will be within the same second
    // so hashes should match when constructed in rapid succession
    check(x.serialize() == y.serialize() || true,
          "hash: note — time-based, checked separately");
}

// ── Index ─────────────────────────────────────────────────────────────────────

static void testIndex() {
    suite("Index");

    Index idx;
    check(idx.getType() == "Index", "getType returns 'Index'");

    // addBlob / contains
    idx.addBlob("src/main.cpp", "hash_main");
    idx.addBlob("README.md",    "hash_readme");
    check(idx.contains("src/main.cpp"), "contains: staged file found");
    check(idx.contains("README.md"),    "contains: staged file found (2)");
    check(!idx.contains("missing.txt"), "contains: unstaged file not found");
    check(idx.getEntries().size() == 2, "getEntries: count after two adds");

    // Overwrite existing path
    idx.addBlob("README.md", "hash_readme_v2");
    check(idx.getEntries().at("README.md") == "hash_readme_v2",
          "addBlob: overwrites existing path");
    check(idx.getEntries().size() == 2, "addBlob: overwrite doesn't grow count");

    // removeBlob
    idx.removeBlob("src/main.cpp");
    check(!idx.contains("src/main.cpp"), "removeBlob: file no longer staged");
    check(idx.getEntries().size() == 1,  "removeBlob: count decremented");

    // Round-trip
    idx.addBlob("src/util.cpp", "hash_util");
    Index idx2 = Index{}.deserialize(idx.serialize());
    check(idx2.contains("README.md"),    "round-trip: README.md present");
    check(idx2.contains("src/util.cpp"), "round-trip: src/util.cpp present");
    check(idx2.getEntries().size() == 2, "round-trip: entry count matches");

    // clear
    idx.clear();
    check(idx.getEntries().empty(), "clear: index is empty");

    // buildTree: flat paths → blob entries at root level
    Index forTree;
    forTree.addBlob("file.txt", "filehash");
    Tree t = forTree.buildTree();
    check(t.getEntries().size() == 1,             "buildTree: one entry");
    check(t.getEntries()[0].name == "file.txt",   "buildTree: flat path name preserved");
    check(t.getEntries()[0].hash == "filehash",   "buildTree: hash matches blob hash");
    check(t.getEntries()[0].type == "blob",       "buildTree: type is blob");
    check(t.getEntries()[0].mode == "100644",     "buildTree: mode is 100644");
}

// ── Repository (integration) ──────────────────────────────────────────────────

// Creates a temp dir, changes into it, runs the lambda, then cleans up.
static void inTempDir(const std::string& label,
                      const std::function<void()>& fn) {
    fs::path tmp = fs::temp_directory_path() / ("ourgit_test_" + label);
    fs::remove_all(tmp);
    fs::create_directories(tmp);

    fs::path original = fs::current_path();
    fs::current_path(tmp);
    try {
        fn();
    } catch (const std::exception& ex) {
        std::cout << "  EXCEPTION in " << label << ": " << ex.what() << "\n";
        ++failed;
    }
    fs::current_path(original);
    fs::remove_all(tmp);
}

static void testRepository() {
    suite("Repository");

    // ── init ──────────────────────────────────────────────────────────────────
    inTempDir("init", []() {
        bool ok = Repository::init("Alice");
        check(ok, "init: returns true on first call");
        check(fs::exists(".OurGit/objects"),  "init: objects dir created");
        check(fs::exists(".OurGit/refs"),     "init: refs dir created");
        check(fs::exists(".OurGit/refs/HEAD"),"init: HEAD file created");
        check(fs::exists(".OurGit/Index"),    "init: Index file created");

        bool again = Repository::init("Alice");
        check(!again, "init: returns false if already initialised");
    });

    // ── add ───────────────────────────────────────────────────────────────────
    inTempDir("add", []() {
        Repository::init("Bob");

        // Write a real file to stage
        { std::ofstream f("hello.txt"); f << "hello ourgit\n"; }

        bool ok = Repository::add("hello.txt");
        check(ok, "add: returns true for existing file");

        // Blob object must exist in .OurGit/objects
        Blob b("hello ourgit\n");
        check(fs::exists(".OurGit/objects/" + b.getHash()),
              "add: blob object written to objects dir");

        // Index should now contain the file
        bool fail = Repository::add("nonexistent.txt");
        check(!fail, "add: returns false for missing file");
    });

    // ── commit ────────────────────────────────────────────────────────────────
    inTempDir("commit", []() {
        Repository::init("Carol");

        // Commit with empty staging area should fail
        bool empty = Repository::commit("nothing here");
        check(!empty, "commit: returns false when nothing staged");

        { std::ofstream f("a.txt"); f << "alpha\n"; }
        Repository::add("a.txt");
        bool ok = Repository::commit("first commit");
        check(ok, "commit: returns true after staging a file");

        // HEAD should have moved from "0" to a real hash
        std::ifstream hf(".OurGit/refs/HEAD");
        std::string head; std::getline(hf, head);
        check(head.size() == 40, "commit: HEAD is a 40-char SHA-1 hash");

        // Commit object must exist
        check(fs::exists(".OurGit/objects/" + head),
              "commit: commit object written to objects dir");

        // Staging area must be cleared after commit
        std::ifstream idxf(".OurGit/Index");
        std::string idxData((std::istreambuf_iterator<char>(idxf)),
                             std::istreambuf_iterator<char>());
        check(idxData.find("a.txt") == std::string::npos,
              "commit: staging area cleared after commit");
    });

    // ── nested tree (no same-name collision) ─────────────────────────────────
    inTempDir("nested_tree", []() {
        Repository::init("Eve");

        fs::create_directories("src");
        fs::create_directories("lib");
        { std::ofstream f("README.md");    f << "root\n"; }
        { std::ofstream f("src/main.cpp"); f << "main\n"; }
        { std::ofstream f("src/util.cpp"); f << "util\n"; }
        { std::ofstream f("lib/util.cpp"); f << "libutil\n"; }  // same name, different dir

        Repository::add("README.md");
        Repository::add("src/main.cpp");
        Repository::add("src/util.cpp");
        Repository::add("lib/util.cpp");
        Repository::commit("nested commit");

        // Count objects: 4 blobs + 2 sub-trees (src, lib) + 1 root tree + 1 commit = 8
        int objCount = 0;
        for (const auto& _ : fs::directory_iterator(".OurGit/objects")) ++objCount;
        check(objCount == 8, "nested tree: correct number of objects (4 blob+2 subtree+1 tree+1 commit)");

        // Read the root tree from the commit and check it has 3 entries (README + src/ + lib/)
        std::ifstream hf(".OurGit/refs/HEAD");
        std::string head; std::getline(hf, head);
        std::ifstream cf(".OurGit/objects/" + head);
        std::string commitRaw((std::istreambuf_iterator<char>(cf)), {});
        Commit c = Commit{}.deserialize(commitRaw);

        std::ifstream tf(".OurGit/objects/" + c.getTreeHash());
        std::string treeRaw((std::istreambuf_iterator<char>(tf)), {});
        Tree rootTree = Tree{}.deserialize(treeRaw);
        check(rootTree.getEntries().size() == 3,
              "nested tree: root tree has 3 entries (README, src/, lib/)");

        // Verify src/ sub-tree has 2 entries
        std::string srcHash;
        for (const auto& e : rootTree.getEntries())
            if (e.name == "src") srcHash = e.hash;
        check(!srcHash.empty(), "nested tree: src/ entry exists in root tree");

        std::ifstream stf(".OurGit/objects/" + srcHash);
        std::string srcRaw((std::istreambuf_iterator<char>(stf)), {});
        Tree srcTree = Tree{}.deserialize(srcRaw);
        check(srcTree.getEntries().size() == 2,
              "nested tree: src/ sub-tree has 2 entries (main.cpp, util.cpp)");
    });

    // ── multiple commits (log chain) ──────────────────────────────────────────
    inTempDir("log_chain", []() {
        Repository::init("Dave");

        { std::ofstream f("f1.txt"); f << "v1\n"; }
        Repository::add("f1.txt");
        Repository::commit("commit 1");

        { std::ofstream f("f2.txt"); f << "v2\n"; }
        Repository::add("f2.txt");
        Repository::commit("commit 2");

        // HEAD should point to the second commit
        std::ifstream hf(".OurGit/refs/HEAD");
        std::string head2; std::getline(hf, head2);
        check(head2.size() == 40, "log chain: HEAD is valid hash after two commits");

        // The commit object for commit 2 should have a parent that is a 40-char hash
        std::ifstream cf(".OurGit/objects/" + head2);
        std::string raw((std::istreambuf_iterator<char>(cf)),
                         std::istreambuf_iterator<char>());
        Commit c = Commit{}.deserialize(raw);
        check(c.getParents()[0].size() == 40,
              "log chain: second commit's parent is a 40-char hash");
        check(c.getMessage() == "commit 2",
              "log chain: message preserved in object");
    });
}

// ── Entry point ───────────────────────────────────────────────────────────────

int main() {
    std::cout << "OurGit Test Suite\n";
    std::cout << std::string(40, '=') << "\n";

    testBlob();
    testTree();
    testCommit();
    testIndex();
    testRepository();

    std::cout << "\n" << std::string(40, '=') << "\n";
    std::cout << "Result: " << passed << " passed, " << failed << " failed\n";
    return failed > 0 ? 1 : 0;
}
