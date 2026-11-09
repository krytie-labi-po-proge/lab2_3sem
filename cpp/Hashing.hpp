#pragma once
#include <cstdint>
#include <string>
#include "Value.hpp"

void feed(uint64_t& h, const unsigned char* p, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        h ^= p[i];
        h *= 0x100000001b3ULL;
    }
}

uint64_t fnv(const Value& v) {
    uint64_t h = 0;

    unsigned char tag = static_cast<unsigned char>(v.kind() + 1);
    h = h * 31 + tag;

    if (v.kind() == 0) {
        uint32_t x = static_cast<uint32_t>(v.asInt());
        h = h * 31 + (x & 255);
        h = h * 31 + ((x >> 8) & 255);
        h = h * 31 + ((x >> 16) & 255);
        h = h * 31 + ((x >> 24) & 255);
    } else if (v.kind() == 1) {
        const std::string& s = v.asStr();
        for (unsigned char c : s) h = h * 31 + c;
    } else {
        h = h * 31 + static_cast<unsigned char>(v.asChar());
    }

    return h;
}

uint64_t mix(uint64_t x) {
    x ^= x >> 30; x *= 0xbf58476d1ce4e5b9ULL;
    x ^= x >> 27; x *= 0x94d049bb133111ebULL;
    x ^= x >> 31;
    return x;
}

uint64_t hashSeeded(const Value& v, uint64_t seed) { return mix(fnv(v) ^ seed); }