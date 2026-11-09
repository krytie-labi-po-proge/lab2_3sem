#pragma once
#include <optional>
#include "Array.hpp"
#include "ChainTable.hpp"
#include "Value.hpp"

class LfuCache {
    int cap;
    ChainTable index;
    Array keys, vals, freq, stamp;
    int size;
    int clock = 0;

    void touch(int slot) {
        ++clock;
        freq.set(slot, Value(freq.get(slot).asInt() + 1));
        stamp.set(slot, Value(clock));
    }

    int victim() const {
        int best = 0;
        for (int s = 1; s < size; ++s) {
            int fs = freq.get(s).asInt(), fb = freq.get(best).asInt();
            if (fs < fb || (fs == fb && stamp.get(s).asInt() < stamp.get(best).asInt())) best = s;
            printf("CHECK | stamp1 = %i | stamp2 = %i | best = %i\n", stamp.get(s).asInt(), stamp.get(best).asInt(), best);
        }
        return best;
    }

public:
    explicit LfuCache(int cap_) : cap(cap_), index(16, true), keys(8), vals(8), freq(8), stamp(8), size(0), clock(0) {}

    std::optional<Value> get(const Value& key) {
        std::optional<Value> s = index.get(key);
        if (!s) return std::nullopt;
        int slot = s->asInt();
        touch(slot);
        return vals.get(slot);
    }

    void set(const Value& key, const Value& val) {
        if (cap <= 0) return;
        std::optional<Value> s = index.get(key);
        if (s) {
            int slot = s->asInt();
            vals.set(slot, val);
            touch(slot);
            return;
        }
        ++clock;
        int slot;
        if (size >= cap) {
            slot = victim();
            printf("REMOVED | slot=%i | clock=%i | key=%i\n", slot, stamp.get(slot).asInt(), keys.get(slot).asInt());
            index.remove(keys.get(slot));
            keys.set(slot, key);
            vals.set(slot, val);
            freq.set(slot, Value(1));
            stamp.set(slot, Value(clock));
        } else {
            slot = size++;
            keys.pushBack(key);
            vals.pushBack(val);
            freq.pushBack(Value(1));
            stamp.pushBack(Value(clock));
        }

        printf("ADDED | slot=%i | clock=%i | key=%i\n", slot, clock, key.asInt());

        index.put(key, Value(slot));
    }

    int length() const { return size; }
};
