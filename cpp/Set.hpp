#pragma once
#include <iostream>
#include "Array.hpp"
#include "Hashing.hpp"
#include "Value.hpp"

class Set {
    Array slots;
    Value empty;
    Value deleted;
    int cap;
    int used;
    int count;

    void fill(int n) {
        slots.clear();
        for (int i = 0; i < n; ++i) slots.pushBack(empty);
    }

    int home(const Value& x) const { return static_cast<int>(hashSeeded(x, 0) % static_cast<uint64_t>(cap)); }

    int findIndex(const Value& x) const {
        int i = home(x);
        while (true) {
            Value s = slots.get(i);
            if (s == empty) return -1;
            if (s != deleted && s == x) return i;
            i = (i + 1) % cap;
        }
    }

    void place(const Value& x) {
        int i = home(x);
        while (true) {
            Value s = slots.get(i);
            if (s == empty) { ++used; break; }
            if (s == deleted) break;
            i = (i + 1) % cap;
        }
        slots.set(i, x);
        ++count;
    }

    void rehash(int newCap) {
        Array old(slots.length() > 0 ? slots.length() : 4);
        for (int i = 0; i < cap; ++i) old.pushBack(slots.get(i));
        int oldCap = cap;
        cap = newCap;
        used = 0;
        count = 0;
        fill(newCap);
        for (int i = 0; i < oldCap; ++i) {
            Value s = old.get(i);
            if (s != empty && s != deleted) place(s);
        }
    }

public:
    Set() : slots(8), empty(std::string("\0empty", 6)), deleted(std::string("\0deleted", 8)), cap(8), used(0), count(0) {
        fill(8);
    }

    bool add(const Value& x) {
        if (contains(x)) return false;
        if ((used + 1) * 10 > cap * 7) rehash((count + 1) * 2 > cap ? cap * 2 : cap);
        place(x);
        return true;
    }

    bool remove(const Value& x) {
        int i = findIndex(x);
        if (i < 0) return false;
        slots.set(i, deleted);
        --count;
        return true;
    }

    bool contains(const Value& x) const { return findIndex(x) >= 0; }
    int length() const { return count; }

    void elements(Array& out) const {
        for (int i = 0; i < cap; ++i) {
            Value s = slots.get(i);
            if (s != empty && s != deleted) out.pushBack(s);
        }
    }

    void print() const {
        Array items;
        elements(items);
        std::cout << '{';
        for (int i = 0; i < items.length(); ++i) {
            if (i > 0) std::cout << ", ";
            std::cout << items.get(i);
        }
        std::cout << "}\n";
    }
};
