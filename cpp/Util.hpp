#pragma once
#include <iostream>
#include <iterator>
#include <string>
#include "Array.hpp"
#include "Value.hpp"

std::string readStdin() {
    return std::string(std::istreambuf_iterator<char>(std::cin), std::istreambuf_iterator<char>());
}

bool parseInts(const std::string& text, Array& out) {
    size_t i = 0;
    while (i < text.size()) {
        char c = text[i];
        bool neg = c == '-' && i + 1 < text.size() && isdigit(static_cast<unsigned char>(text[i + 1]));
        if (isdigit(static_cast<unsigned char>(c)) || neg) {
            size_t j = neg ? i + 1 : i;
            long long x = 0;
            while (j < text.size() && isdigit(static_cast<unsigned char>(text[j]))) {
                x = x * 10 + (text[j] - '0');
                if (x > 2147483648LL) return false;
                ++j;
            }
            if (neg) x = -x;
            if (x > 2147483647LL || x < -2147483647LL - 1) return false;
            out.pushBack(Value(static_cast<int>(x)));
            i = j;
        } else {
            ++i;
        }
    }
    return true;
}