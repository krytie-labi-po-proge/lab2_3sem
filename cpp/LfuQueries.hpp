#pragma once
#include <cctype>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include "LfuCache.hpp"
#include "Util.hpp"
#include "Value.hpp"

namespace lfu_queries {

int process_stdin(int argc, char* argv[]) {
    int cap, Q;
    std::string query;

    bool hasCap = false;
    bool hasQ = false;
    bool hasQuery = false;

    for (int i = 1; i < argc; ++i) {
        std::string argument(argv[i]);

        if (argument == "--cap") {
            if (i + 1 >= argc) {
                std::cout << "ERROR: после --cap не указано значение\n";
                return 0;
            }

            cap = std::stoi(argv[i + 1]);
            hasCap = true;
            ++i;
        } else if (argument == "--Q") {
            if (i + 1 >= argc) {
                std::cout << "ERROR: после --Q не указано значение\n";
                return 0;
            }

            Q = std::stoi(argv[i + 1]);
            hasQ = true;
            ++i;
        } else if (argument == "--query") {
            if (i + 1 >= argc) {
                std::cout << "ERROR: после --query не указано значение\n";
                return 0;
            }

            query = argv[i + 1];
            hasQuery = true;
            ++i;
        }
    }

    if (!hasCap || !hasQuery || !hasQ) {
        std::cout << "ERROR: необходимо указать --cap, --query и --Q\n";
        return 0;
    }

    std::vector<std::string> tokens;
    std::string cur;
    for (char c : query) {
        if (isspace(static_cast<unsigned char>(c)) || c == ',' || c == '=') {
            if (!cur.empty()) { tokens.push_back(cur); cur.clear(); }
        } else {
            cur += c;
        }
    }
    if (!cur.empty()) tokens.push_back(cur);

    size_t pos = 0;
    LfuCache cache(cap);
    std::string out;
    while (pos < tokens.size()) {
        if (tokens[pos] == "SET") {
            if (pos + 2 >= tokens.size()) break;
            cache.set(Value::parse(tokens[pos + 1]), Value::parse(tokens[pos + 2]));
            pos += 3;
        } else if (tokens[pos] == "GET") {
            if (pos + 1 >= tokens.size()) break;
            std::optional<Value> r = cache.get(Value::parse(tokens[pos + 1]));
            if (!out.empty()) out += ' ';
            out += r ? r->toString() : "-1";
            pos += 2;
        } else {
            ++pos;
        }
    }
    std::cout << out << '\n';
    return 0;
}

}