#pragma once
#include <iostream>
#include "Value.hpp"

class DoublyList {
    struct Node {
        Value data;
        Node* prev;
        Node* next;
        Node(const Value& x) : data(x), prev(nullptr), next(nullptr) {}
    };

    Node* head;
    Node* tail;
    int size;

public:
    DoublyList() : head(nullptr), tail(nullptr), size(0) {}
    ~DoublyList() { clear(); }

    void pushFront(const Value& x) {
        Node* n = new Node(x);
        n->next = head;
        if (head) head->prev = n; else tail = n;
        head = n;
        ++size;
    }

    void pushBack(const Value& x) {
        Node* n = new Node(x);
        n->prev = tail;
        if (tail) tail->next = n; else head = n;
        tail = n;
        ++size;
    }

    Node* findNode(const Value& x) const {
        Node* p = head;
        while (p) {
            if (p->data == x) return p;
            p = p->next;
        }
        return nullptr;
    }

    void insertBefore(const Value& value, const Value& x) {
        Node* p = findNode(value);
        if (!p) return;
        if (p == head) { pushFront(x); return; }
        Node* n = new Node(x);
        n->prev = p->prev;
        n->next = p;
        p->prev->next = n;
        p->prev = n;
        ++size;
    }

    void insertAfter(const Value& value, const Value& x) {
        Node* p = findNode(value);
        if (!p) return;
        if (p == tail) { pushBack(x); return; }
        Node* n = new Node(x);
        n->prev = p;
        n->next = p->next;
        p->next->prev = n;
        p->next = n;
        ++size;
    }

    Value popFront() {
        if (!head) return Value();
        Node* p = head;
        Value ret = p->data;
        head = head->next;
        if (head) head->prev = nullptr; else tail = nullptr;
        delete p;
        --size;
        return ret;
    }

    Value popBack() {
        if (!tail) return Value();
        Node* p = tail;
        Value ret = p->data;
        tail = tail->prev;
        if (tail) tail->next = nullptr; else head = nullptr;
        delete p;
        --size;
        return ret;
    }

    void remove(const Value& x) {
        Node* p = findNode(x);
        if (!p) return;
        if (p == head) { popFront(); return; }
        if (p == tail) { popBack(); return; }
        p->prev->next = p->next;
        p->next->prev = p->prev;
        delete p;
        --size;
    }

    void removeBefore(const Value& value) {
        Node* p = findNode(value);
        if (!p || !p->prev) return;
        Node* q = p->prev;
        if (q == head) { popFront(); return; }
        q->prev->next = p;
        p->prev = q->prev;
        delete q;
        --size;
    }

    void removeAfter(const Value& value) {
        Node* p = findNode(value);
        if (!p || !p->next) return;
        Node* q = p->next;
        if (q == tail) { popBack(); return; }
        p->next = q->next;
        q->next->prev = p;
        delete q;
        --size;
    }

    bool find(const Value& x) const { return findNode(x) != nullptr; }

    Value get(int index) const {
        if (index < 0 || index >= size) return Value();
        if (index < size / 2) {
            Node* p = head;
            for (int i = 0; i < index; ++i) p = p->next;
            return p->data;
        }
        Node* p = tail;
        for (int i = size - 1; i > index; --i) p = p->prev;
        return p->data;
    }

    int length() const { return size; }

    void print() const {
        Node* p = head;
        while (p) {
            std::cout << p->data << ' ';
            p = p->next;
        }
        std::cout << '\n';
    }

    void printReverse() const {
        Node* p = tail;
        while (p) {
            std::cout << p->data << ' ';
            p = p->prev;
        }
        std::cout << '\n';
    }

    void clear() { while (head) popFront(); }
};
