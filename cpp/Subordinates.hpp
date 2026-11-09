#pragma once
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <string>

#include "Array.hpp"
#include "ChainTable.hpp"
#include "CuckooTable.hpp"
#include "HashMapLike.hpp"
#include "Queue.hpp"
#include "Util.hpp"
#include "Value.hpp"

namespace subordinates {

void replaceAll(std::string& text, const std::string& from, const std::string& to) {
    size_t pos = 0;
    while ((pos = text.find(from, pos)) != std::string::npos) {
        text.replace(pos, from.size(), to);
        pos += to.size();
    }
}

int idOf(HashMapLike& ids, Array& names, const std::string& name) {
    Value key{name};
    std::optional<Value> v = ids.get(key);
    if (v) return v->asInt();
    int id = names.length();
    names.pushBack(key);
    ids.put(key, Value(id));
    return id;
}

int count_stdin(int argc, char* argv[]) {
    bool useCuckoo = false;
    bool hasQuery = false;
    std::string text;
    for (int i = 1; i < argc; ++i){
        if (std::string(argv[i]) == "--cuckoo") useCuckoo = true;
        if (std::string(argv[i]) == "--query" && i+1 < argc) {
            text = argv[i+1];
            hasQuery = true;
        }
    }

    if (!hasQuery){
        printf("Нету аргумента --query");
        return 0;
    }

    std::unique_ptr<HashMapLike> ids;
    if (useCuckoo) ids.reset(new CuckooTable(16));
    else ids.reset(new ChainTable(16, true));

    for (char& c : text) {
        if (std::string("{}\"',;()").find(c) != std::string::npos) c = ' ';
    }
    replaceAll(text, "«", " ");
    replaceAll(text, "»", " ");
    std::istringstream in(text);
    Array names, mgr;
    std::string a, b;
    while (in >> a) {
        if (!(in >> b)) {
            std::cout << "ERROR: неверный формат\n";
            return 0;
        }
        int e = idOf(*ids, names, a);
        int m = idOf(*ids, names, b);
        while (mgr.length() < names.length()) mgr.pushBack(Value(-1));
        int cur = mgr.get(e).asInt();
        if (cur != -1 && cur != m) {
            std::cout << "ERROR: у сотрудника " << a << " несколько менеджеров\n";
            return 0;
        }
        mgr.set(e, Value(m));
    }
    int n = names.length();
    Array deg, cnt;
    for (int i = 0; i < n; ++i) {
        deg.pushBack(Value(0));
        cnt.pushBack(Value(0));
    }
    for (int i = 0; i < n; ++i) {
        int m = mgr.get(i).asInt();
        if (m != -1 && m != i) deg.set(m, Value(deg.get(m).asInt() + 1));
    }
    Queue q;
    for (int i = 0; i < n; ++i)
        if (deg.get(i).asInt() == 0) q.push(Value(i));
    int processed = 0;
    while (q.length() > 0) {
        int v = q.pop().asInt();
        ++processed;
        int m = mgr.get(v).asInt();
        if (m != -1 && m != v) {
            cnt.set(m, Value(cnt.get(m).asInt() + cnt.get(v).asInt() + 1));
            int d = deg.get(m).asInt() - 1;
            deg.set(m, Value(d));
            if (d == 0) q.push(Value(m));
        }
    }
    if (processed < n) {
        std::cout << "ERROR: в иерархии есть цикл\n";
        return 0;
    }

    Array order;
    for (int i = 0; i < n; ++i) order.pushBack(Value(i));
    for (int i = 1; i < n; ++i) {
        int key = order.get(i).asInt();
        std::string keyName = names.get(key).toString();
        int j = i - 1;
        while (j >= 0 && names.get(order.get(j).asInt()).toString() > keyName) {
            order.set(j + 1, order.get(j));
            --j;
        }
        order.set(j + 1, Value(key));
    }
    for (int k = 0; k < n; ++k) {
        int id = order.get(k).asInt();
        if (k) std::cout << ", ";
        std::cout << names.get(id) << '-' << cnt.get(id).asInt();
    }
    std::cout << '\n';
    return 0;
}

}