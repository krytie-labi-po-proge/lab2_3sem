#pragma once
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "Set.hpp"
#include "Value.hpp"

namespace set_cli {

struct Entry {
    std::string name;
    Set set;
    Entry* next;
    Entry(const std::string& n, Entry* nx) : name(n), next(nx) {}
};

class Registry {
    Entry* head = nullptr;
public:
    ~Registry() {
        while (head) { Entry* n = head->next; delete head; head = n; }
    }
    Set* find(const std::string& name) {
        for (Entry* e = head; e; e = e->next)
            if (e->name == name) return &e->set;
        return nullptr;
    }
    Set& getOrCreate(const std::string& name) {
        Set* s = find(name);
        if (s) return *s;
        head = new Entry(name, head);
        return head->set;
    }
};

bool fail(bool loading, const std::string& msg) {
    if (!loading) std::cerr << "ERROR: " << msg << '\n';
    return false;
}

bool execute(Registry& reg, const std::string& query, bool loading) {
    std::istringstream in(query);
    std::string parts[4];
    int n = 0;
    while (n < 4 && in >> parts[n]) ++n;
    if (n == 0) return false;
    const std::string& cmd = parts[0];
    if (cmd == "SETADD") {
        if (n != 3) return fail(loading, "SETADD <имя> <значение>");
        return reg.getOrCreate(parts[1]).add(Value::parse(parts[2]));
    }
    if (cmd == "SETDEL") {
        if (n != 3) return fail(loading, "SETDEL <имя> <значение>");
        Set* s = reg.find(parts[1]);
        if (!s) return fail(loading, "множество не найдено");
        return s->remove(Value::parse(parts[2]));
    }
    if (cmd == "SET_AT") {
        if (n != 3) return fail(loading, "SET_AT <имя> <значение>");
        Set* s = reg.find(parts[1]);
        bool present = s != nullptr && s->contains(Value::parse(parts[2]));
        if (!loading) std::cout << (present ? "TRUE" : "FALSE") << '\n';
        return false;
    }
    if (cmd == "SETLEN") {
        if (n != 2) return fail(loading, "SETLEN <имя>");
        Set* s = reg.find(parts[1]);
        if (!s) return fail(loading, "множество не найдено");
        if (!loading) std::cout << s->length() << '\n';
        return false;
    }
    if (cmd == "PRINT") {
        if (n != 2) return fail(loading, "PRINT <имя>");
        Set* s = reg.find(parts[1]);
        if (!s) return fail(loading, "множество не найдено");
        if (!loading) s->print();
        return false;
    }
    return fail(loading, "неизвестная команда");
}

int run_command(int argc, char* argv[]) {
    std::string file, query;
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "--file" && i + 1 < argc) file = argv[++i];
        else if (a == "--query" && i + 1 < argc) query = argv[++i];
    }
    if (file.empty() || query.empty()) {
        printf("Должны быть аргументы --file и --query\n");
        return 0;
    }
    Registry reg;
    std::ifstream in(file);
    std::string line;
    while (std::getline(in, line)) execute(reg, line, true);
    in.close();
    if (execute(reg, query, false)) {
        std::ofstream out(file, std::ios::app);
        out << query << '\n';
    }
    return 0;
}

}