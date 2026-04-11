#include "../core/include/Repository.hpp"
#include <iostream>
#include <string>
#include <vector>

/**
 * @author sry & yrc
 *
 * OurGit CLI
 * ----------
 * Usage:
 *   ourgit init   <author>           — initialise a new repository
 *   ourgit add    <file> [<file>...]  — stage one or more files
 *   ourgit commit -m <message>        — create a commit
 *   ourgit status                     — show staging area
 *   ourgit log                        — show commit history
 *   ourgit help                       — print this help text
 */

static void printUsage() {
    std::cout <<
        "Usage:\n"
        "  ourgit init   <author>            initialise a new repository\n"
        "  ourgit add    <file> [<file>...]   stage one or more files\n"
        "  ourgit commit -m <message>         create a commit\n"
        "  ourgit status                      show current staging area\n"
        "  ourgit log                         show commit history\n"
        "  ourgit help                        show this help text\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage();
        return 1;
    }

    std::string cmd = argv[1];

    // ── ourgit init <author> ──────────────────────────────────────────────────
    if (cmd == "init") {
        if (argc < 3) {
            std::cerr << "Usage: ourgit init <author>\n";
            return 1;
        }
        std::string author = argv[2];
        return Repository::init(author) ? 0 : 1;
    }

    // ── ourgit add <file> [<file>...] ─────────────────────────────────────────
    if (cmd == "add") {
        if (argc < 3) {
            std::cerr << "Usage: ourgit add <file> [<file>...]\n";
            return 1;
        }
        if (argc == 3) {
            return Repository::add(std::string(argv[2])) ? 0 : 1;
        }
        // Multiple files
        std::vector<std::string> paths;
        for (int i = 2; i < argc; ++i)
            paths.emplace_back(argv[i]);
        return Repository::add(paths) ? 0 : 1;
    }

    // ── ourgit commit -m <message> ────────────────────────────────────────────
    if (cmd == "commit") {
        if (argc < 4 || std::string(argv[2]) != "-m") {
            std::cerr << "Usage: ourgit commit -m <message>\n";
            return 1;
        }
        std::string message = argv[3];
        return Repository::commit(message) ? 0 : 1;
    }

    // ── ourgit status ─────────────────────────────────────────────────────────
    if (cmd == "status") {
        Repository::status();
        return 0;
    }

    // ── ourgit log ────────────────────────────────────────────────────────────
    if (cmd == "log") {
        Repository::log();
        return 0;
    }

    // ── ourgit help ───────────────────────────────────────────────────────────
    if (cmd == "help") {
        printUsage();
        return 0;
    }

    std::cerr << "ourgit: '" << cmd << "' is not a known command.\n\n";
    printUsage();
    return 1;
}
