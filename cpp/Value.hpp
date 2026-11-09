#pragma once
#include <iostream>
#include <string>
#include <variant>
#include <sstream>

class Value {
    std::variant<int, std::string, char> v;
public:
    Value() : v(0) {}
    Value(int x) : v(x) {}
    Value(const std::string& x) : v(x) {}
    Value(const char* x) : v(std::string(x)) {}
    Value(char x) : v(x) {}

    bool operator==(const Value& o) const { return v == o.v; }
    bool operator!=(const Value& o) const { return !(*this == o); }

    friend std::ostream& operator<<(std::ostream& os, const Value& val) {
        std::visit([&os](auto&& x) { os << x; }, val.v);
        return os;
    }


    int kind() const { return static_cast<int>(v.index()); }
    int asInt() const { const int* p = std::get_if<int>(&v); return p ? *p : 0; }
    char asChar() const { const char* p = std::get_if<char>(&v); return p ? *p : '\0'; }
    const std::string& asStr() const {
        static const std::string empty;
        const std::string* p = std::get_if<std::string>(&v);
        return p ? *p : empty;
    }
    std::string toString() const { std::ostringstream o; o << *this; return o.str(); }

    static bool toInt(const std::string& s, int& out) {
        if (s.empty()) return false;
        try {
            size_t pos = 0;
            int x = std::stoi(s, &pos);
            if (pos != s.size()) return false;
            out = x;
            return true;
        } catch (...) {
            return false;
        }
    }

    static Value parse(const std::string& token) {
        int x = 0;
        if (token.size() >= 2 && token[1] == ':') {
            std::string rest = token.substr(2);
            switch (token[0]) {
                case 'i':
                    if (toInt(rest, x)) return Value(x);
                    break;
                case 's':
                    return Value(rest);
                case 'c':
                    if (rest.size() == 1) return Value(rest[0]);
                    break;
            }
        }
        if (toInt(token, x)) return Value(x);
        if (token.size() == 1) return Value(token[0]);
        return Value(token);
    }
};
