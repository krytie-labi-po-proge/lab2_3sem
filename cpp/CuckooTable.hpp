#pragma once
#include "Array.hpp"
#include "Hashing.hpp"
#include "HashMapLike.hpp"
#include "Value.hpp"

class CuckooTable : public HashMapLike {
    static const int MAX_KICKS = 128;
    Array k1, v1, k2, v2;
    int m;
    Value empty;
    uint64_t seed1, seed2;
    size_t size;

    static void fill(Array& a, int n, const Value& x) {
        a.clear();
        for (int i = 0; i < n; ++i) a.pushBack(x);
    }

    int h1(const Value& k) const { return static_cast<int>(hashSeeded(k, seed1) % static_cast<uint64_t>(m)); }
    int h2(const Value& k) const { return static_cast<int>(hashSeeded(k, seed2) % static_cast<uint64_t>(m)); }

    void reset() {
        fill(k1, m, empty);
        fill(k2, m, empty);
        fill(v1, m, Value());
        fill(v2, m, Value());
        size = 0;
    }

    void rebuild(const Value& pk, const Value& pv) {
        Array ks(16), vs(16);
        for (int i = 0; i < m; ++i) {
            Value a = k1.get(i);
            if (a != empty) { ks.pushBack(a); vs.pushBack(v1.get(i)); }
            Value b = k2.get(i);
            if (b != empty) { ks.pushBack(b); vs.pushBack(v2.get(i)); }
        }
        ks.pushBack(pk);
        vs.pushBack(pv);
        size_t total = static_cast<size_t>(ks.length());
        while (total * 5 > static_cast<size_t>(m) * 4) m *= 2;
        int attempt = 0;
        while (true) {
            ++attempt;
            ++rehashes;
            if (attempt % 3 == 0) m *= 2;
            seed1 = mix(seed1 + 0x9E3779B97F4A7C15ULL);
            seed2 = mix(seed2 + 0xD1B54A32D192ED03ULL);
            reset();
            bool ok = true;
            for (int i = 0; i < ks.length(); ++i) {
                if (!tryInsert(ks.get(i), vs.get(i))) { ok = false; break; }
            }
            if (ok) return;
        }
    }

public:
    unsigned long long probes;
    unsigned long long kicks;
    unsigned rehashes;

    explicit CuckooTable(int m_ = 16, uint64_t s1 = 0x9E3779B97F4A7C15ULL, uint64_t s2 = 0xC2B2AE3D27D4EB4FULL)
        : k1(m_ < 4 ? 4 : m_), v1(m_ < 4 ? 4 : m_), k2(m_ < 4 ? 4 : m_), v2(m_ < 4 ? 4 : m_),
          m(m_ < 4 ? 4 : m_), empty(std::string("\0empty", 6)), seed1(s1), seed2(s2), size(0),
          probes(0), kicks(0), rehashes(0) {
        reset();
    }

    std::optional<Value> get(const Value& k) override {
        int i = h1(k);
        ++probes;
        if (k1.get(i) == k) return v1.get(i);
        int j = h2(k);
        ++probes;
        if (k2.get(j) == k) return v2.get(j);
        return std::nullopt;
    }

    bool contains(const Value& k) { return get(k).has_value(); }

    bool tryInsert(Value k, Value v, Value* pk = nullptr, Value* pv = nullptr) {
        int i = h1(k);
        if (k1.get(i) == k) { v1.set(i, v); return true; }
        int j = h2(k);
        if (k2.get(j) == k) { v2.set(j, v); return true; }
        bool first = true;
        for (int step = 0; step < MAX_KICKS; ++step) {
            if (first) {
                int p = h1(k);
                Value oldK = k1.get(p), oldV = v1.get(p);
                k1.set(p, k); v1.set(p, v);
                if (oldK == empty) { ++size; return true; }
                k = oldK; v = oldV;
            } else {
                int p = h2(k);
                Value oldK = k2.get(p), oldV = v2.get(p);
                k2.set(p, k); v2.set(p, v);
                if (oldK == empty) { ++size; return true; }
                k = oldK; v = oldV;
            }
            ++kicks;
            first = !first;
        }
        if (pk) *pk = k;
        if (pv) *pv = v;
        return false;
    }

    void put(const Value& k, const Value& v) override {
        Value pk, pv;
        if (!tryInsert(k, v, &pk, &pv)) rebuild(pk, pv);
    }

    bool remove(const Value& k) override {
        int i = h1(k);
        if (k1.get(i) == k) { k1.set(i, empty); v1.set(i, Value()); --size; return true; }
        int j = h2(k);
        if (k2.get(j) == k) { k2.set(j, empty); v2.set(j, Value()); --size; return true; }
        return false;
    }

    size_t len() const override { return size; }
    size_t capacity() const { return 2 * static_cast<size_t>(m); }
};