#pragma once
#include <iostream>
#include "Value.hpp"

class Queue {
    struct Node {
        Value data;
        Node* next;
        Node(const Value& x) : data(x), next(nullptr) {}
    };

    Node* first;
    Node* last;
    int size;

public:
    Queue() : first(nullptr), last(nullptr), size(0) {}
    ~Queue() { clear(); }

    void push(const Value& x) {
        Node* n = new Node(x);
        if (!last) first = last = n;
        else { last->next = n; last = n; }
        ++size;
    }

    Value pop() {
        if (!first) return Value();
        Node* p = first;
        Value x = p->data;
        first = first->next;
        if (!first) last = nullptr;
        delete p;
        --size;
        return x;
    }

    Value front() const {
        return first ? first->data : Value();
    }

    int length() const { return size; }

    void print() const {
        Node* p = first;
        while (p) {
            std::cout << p->data << ' ';
            p = p->next;
        }
        std::cout << '\n';
    }

    void clear() { while (first) pop(); }
};
