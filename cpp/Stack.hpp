#pragma once
#include <iostream>
#include "Value.hpp"

class Stack {
    struct Node {
        Value data;
        Node* next;
        Node(const Value& x) : data(x), next(nullptr) {}
    };

    Node* topNode;
    int size;

public:
    Stack() : topNode(nullptr), size(0) {}
    ~Stack() { clear(); }

    void push(const Value& x) {
        Node* n = new Node(x);
        n->next = topNode;
        topNode = n;
        ++size;
    }

    Value pop() {
        if (!topNode) return Value();
        Node* p = topNode;
        Value x = p->data;
        topNode = p->next;
        delete p;
        --size;
        return x;
    }

    Value top() const {
        return topNode ? topNode->data : Value();
    }

    int length() const { return size; }

    void print() const {
        Node* p = topNode;
        while (p) {
            std::cout << p->data << ' ';
            p = p->next;
        }
        std::cout << '\n';
    }

    void clear() { while (topNode) pop(); }
};
