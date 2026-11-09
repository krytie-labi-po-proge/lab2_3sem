#pragma once
#include <iostream>
#include "Value.hpp"

class SinglyList {
    struct Node {
        Value data;
        Node* next;
        Node(const Value& x) : data(x), next(nullptr) {}
    };

    Node* head;
    Node* tail;
    int size;

public:
    SinglyList() : head(nullptr), tail(nullptr), size(0) {}
    ~SinglyList() { clear(); }

    void pushFront(const Value& x) {
        Node* n = new Node(x);
        n->next = head;
        head = n;
        if (!tail) tail = n;
        ++size;
    }

    void pushBack(const Value& x) {
        Node* n = new Node(x);
        if (!tail) head = tail = n;
        else { tail->next = n; tail = n; }
        ++size;
    }

    void insertBefore(const Value& value, const Value& x) {
        if (!head) return;
        if (head->data == value) { pushFront(x); return; }
        Node* p = head;
        while (p->next && p->next->data != value) p = p->next;
        if (!p->next) return;
        Node* n = new Node(x);
        n->next = p->next;
        p->next = n;
        ++size;
    }

    void insertAfter(const Value& value, const Value& x) {
        Node* p = head;
        while (p && p->data != value) p = p->next;
        if (!p) return;
        Node* n = new Node(x);
        n->next = p->next;
        p->next = n;
        if (p == tail) tail = n;
        ++size;
    }

    Value popFront() {
        if (!head) return Value();
        Node* p = head;
        Value ret = p->data;
        head = head->next;
        delete p;
        --size;
        if (!head) tail = nullptr;
        return ret;
    }

    Value popBack() {
        if (!head) return Value();
        if (head == tail) {
            Value ret = head->data;
            delete head;
            head = tail = nullptr;
            size = 0;
            return ret;
        }
        Node* p = head;
        Value ret = tail->data;
        while (p->next != tail) p = p->next;
        delete tail;
        tail = p;
        tail->next = nullptr;
        --size;
        return ret;
    }

    bool find(const Value& x) const {
        Node* p = head;
        while (p) {
            if (p->data == x) return true;
            p = p->next;
        }
        return false;
    }

    void remove(const Value& x) {
        if (!head) return;
        if (head->data == x) { popFront(); return; }
        Node* p = head;
        while (p->next && p->next->data != x) p = p->next;
        if (!p->next) return;
        Node* q = p->next;
        p->next = q->next;
        if (q == tail) tail = p;
        delete q;
        --size;
    }

    void removeBefore(const Value& value) {
        if (!head || !head->next || head->data == value) return;
        if (head->next->data == value) { popFront(); return; }
        Node* p = head;
        while (p->next->next && p->next->next->data != value) p = p->next;
        if (!p->next->next) return;
        Node* q = p->next;
        p->next = q->next;
        delete q;
        --size;
    }

    void removeAfter(const Value& value) {
        Node* p = head;
        while (p && p->data != value) p = p->next;
        if (!p || !p->next) return;
        Node* q = p->next;
        p->next = q->next;
        if (q == tail) tail = p;
        delete q;
        --size;
    }

    Value get(int index) const {
        if (index < 0 || index >= size) return Value();
        Node* p = head;
        for (int i = 0; i < index; ++i) p = p->next;
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

    void clear() { while (head) popFront(); }
};
