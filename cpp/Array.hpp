#pragma once
#include <iostream>
#include "Value.hpp"

class Array {
    Value* data;
    int size;
    int capacity;

    void resize(int c) {
        Value* p = new Value[c];
        for (int i = 0; i < size; ++i)
            p[i] = data[i];
        delete[] data;
        data = p;
        capacity = c;
    }

public:
    Array(int c = 4) : data(new Value[c > 0 ? c : 1]), size(0), capacity(c > 0 ? c : 1) {}
    ~Array() { delete[] data; }

    void pushBack(const Value& x) {
        if (size == capacity) resize(capacity * 2);
        data[size++] = x;
    }

    void insert(int index, const Value& x) {
        if (index < 0 || index > size) return;
        if (size == capacity) resize(capacity * 2);
        for (int i = size; i > index; --i)
            data[i] = data[i - 1];
        data[index] = x;
        ++size;
    }

    Value get(int index) const {
        if (index < 0 || index >= size) return Value();
        return data[index];
    }

    void set(int index, const Value& x) {
        if (index >= 0 && index < size) data[index] = x;
    }

    void remove(int index) {
        if (index < 0 || index >= size) return;
        for (int i = index; i < size - 1; ++i)
            data[i] = data[i + 1];
        --size;
    }

    int length() const { return size; }

    void print() const {
        for (int i = 0; i < size; ++i)
            std::cout << data[i] << ' ';
        std::cout << '\n';
    }

    void clear() { size = 0; }
};