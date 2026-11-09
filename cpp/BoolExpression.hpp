#pragma once
#include <iostream>
#include <sstream>
#include <string>

#include "Stack.hpp"
#include "Util.hpp"
#include "Value.hpp"

namespace bool_expression {

int prec(char op) {
    switch (op) {
        case '!': return 3;
        case '&': return 2;
        case '|':
        case '^': return 1;
        default: return 0;
    }
}

bool apply(char op, Stack& vals) {
    if (op == '!') {
        if (vals.length() < 1) return false;
        int a = vals.pop().asInt();
        vals.push(Value(1 - a));
        return true;
    }
    if (vals.length() < 2) return false;
    int b = vals.pop().asInt();
    int a = vals.pop().asInt();
    int r;
    switch (op) {
        case '&': r = a & b; break;
        case '|': r = a | b; break;
        case '^': r = a ^ b; break;
        default: return false;
    }
    vals.push(Value(r));
    return true;
}

bool eval(const std::string& expr, int& result) {
    Stack vals, ops;
    bool expectOperand = true;
    for (char c : expr) {
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') continue;
        if (c == '0' || c == '1') {
            if (!expectOperand) return false;
            vals.push(Value(c - '0'));
            expectOperand = false;
        } else if (c == '!' || c == '(') {
            if (!expectOperand) return false;
            ops.push(Value(c));
        } else if (c == ')') {
            if (expectOperand) return false;
            while (true) {
                if (ops.length() == 0) return false;
                char t = ops.pop().asChar();
                if (t == '(') break;
                if (!apply(t, vals)) return false;
            }
            expectOperand = false;
        } else if (c == '&' || c == '|' || c == '^') {
            if (expectOperand) return false;
            while (ops.length() > 0) {
                char t = ops.top().asChar();
                if (t == '(' || prec(t) < prec(c)) break;
                ops.pop();
                if (!apply(t, vals)) return false;
            }
            ops.push(Value(c));
            expectOperand = true;
        } else {
            return false;
        }
    }
    if (expectOperand) return false;
    while (ops.length() > 0) {
        char t = ops.pop().asChar();
        if (t == '(') return false;
        if (!apply(t, vals)) return false;
    }
    if (vals.length() != 1) return false;
    result = vals.top().asInt();
    return true;
}

int evaluate_stdin(int argc, char* argv[]) {
    std::string text;   
    bool hasQuery = false;

    for (int i = 1; i < argc; ++i){
        if (std::string(argv[i]) == "--query" && i+1 < argc) {
            text = argv[i+1];
            hasQuery = true;
        }
    }

    if (!hasQuery){
        printf("Нету аргумента --query");
        return 0;
    }

    int r;
    if (eval(text, r)) std::cout << r << '\n';
    else std::cout << "ERROR\n";
    return 0;
}

}