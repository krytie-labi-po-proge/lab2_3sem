#pragma once
#include "Array.hpp"
#include "Hashing.hpp"
#include "HashMapLike.hpp"
#include "Value.hpp"

class ChainTable : public HashMapLike {
    Array heads;
    Array keys, vals, next;
    int buckets;
    int freeNode;
    size_t size;
    bool autoResize;

    void fillHeads(int n) {
        heads.clear();
        for (int i = 0; i < n; ++i) heads.pushBack(Value(-1));
    }

    int bucket(const Value& k) const { return static_cast<int>(hashSeeded(k, 0) % static_cast<uint64_t>(buckets)); }

    int findNode(const Value& k) {
        int idx = heads.get(bucket(k)).asInt();
        while (idx != -1) {
            ++probes;
            if (keys.get(idx) == k) return idx;
            idx = next.get(idx).asInt();
        }
        return -1;
    }

    int alloc(const Value& k, const Value& v) {
        if (freeNode != -1) {
            int s = freeNode;
            freeNode = next.get(s).asInt();
            keys.set(s, k);
            vals.set(s, v);
            return s;
        }
        int s = keys.length();
        keys.pushBack(k);
        vals.pushBack(v);
        next.pushBack(Value(-1));
        return s;
    }

    void grow() {
        Array nodes(16);
        for (int b = 0; b < buckets; ++b) {
            int idx = heads.get(b).asInt();
            while (idx != -1) {
                nodes.pushBack(Value(idx));
                idx = next.get(idx).asInt();
            }
        }
        buckets *= 2;
        fillHeads(buckets);
        for (int i = 0; i < nodes.length(); ++i) {
            int idx = nodes.get(i).asInt();
            int b = bucket(keys.get(idx));
            next.set(idx, heads.get(b));
            heads.set(b, Value(idx));
        }
    }

public:
    unsigned long long probes;

    ChainTable(int buckets_ = 16, bool autoResize_ = true)
        : heads(buckets_ < 4 ? 4 : buckets_), keys(16), vals(16), next(16),
          buckets(buckets_ < 4 ? 4 : buckets_), freeNode(-1), size(0), autoResize(autoResize_), probes(0) {
        fillHeads(buckets);
    }

    void put(const Value& k, const Value& v) override {
        int idx = findNode(k);
        if (idx != -1) { vals.set(idx, v); return; }
        int b = bucket(k);
        int slot = alloc(k, v);
        next.set(slot, heads.get(b));
        heads.set(b, Value(slot));
        ++size;
        if (autoResize && size > static_cast<size_t>(buckets)) grow();
    }

    std::optional<Value> get(const Value& k) override {
        int idx = findNode(k);
        if (idx == -1) return std::nullopt;
        return vals.get(idx);
    }

    bool contains(const Value& k) { return findNode(k) != -1; }

    bool remove(const Value& k) override {
        int b = bucket(k);
        int prev = -1;
        int idx = heads.get(b).asInt();
        while (idx != -1) {
            ++probes;
            if (keys.get(idx) == k) {
                int nx = next.get(idx).asInt();
                if (prev == -1) heads.set(b, Value(nx));
                else next.set(prev, Value(nx));
                next.set(idx, Value(freeNode));
                freeNode = idx;
                keys.set(idx, Value());
                vals.set(idx, Value());
                --size;
                return true;
            }
            prev = idx;
            idx = next.get(idx).asInt();
        }
        return false;
    }

    size_t len() const override { return size; }
    int bucketCount() const { return buckets; }

    int longestChain() const {
        int best = 0;
        for (int b = 0; b < buckets; ++b) {
            int len = 0;
            int idx = heads.get(b).asInt();
            while (idx != -1) { ++len; idx = next.get(idx).asInt(); }
            if (len > best) best = len;
        }
        return best;
    }
};