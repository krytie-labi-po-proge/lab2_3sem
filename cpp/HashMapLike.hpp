#pragma once
#include <cstddef>
#include <optional>
#include "Value.hpp"

class HashMapLike {
public:
    virtual ~HashMapLike() {}
    virtual void put(const Value& k, const Value& v) = 0;
    virtual std::optional<Value> get(const Value& k) = 0;
    virtual bool remove(const Value& k) = 0;
    virtual size_t len() const = 0;
};