#pragma once
#include <iostream>
#include <string>

#include "Array.hpp"
#include "Set.hpp"
#include "Util.hpp"
#include "Value.hpp"

namespace subset_split {

const long long MAX_SUM = 20000000;

std::string subsetText(const Array& items, const Set& subset) {
    std::string s = "{";
    bool first = true;
    for (int i = 0; i < items.length(); ++i) {
        if (subset.contains(items.get(i))) {
            if (!first) s += ", ";
            s += items.get(i).toString();
            first = false;
        }
    }
    return s + "}";
}

int split_stdin(int argc, char* argv[]) {
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

    Array raw;
    if (!parseInts(text, raw)) {
        std::cout << "ERROR: число вне диапазона\n";
        return 0;
    }

    Set set;
    Array items;
    long long total = 0;
    for (int i = 0; i < raw.length(); ++i) {
        Value v = raw.get(i);
        int x = v.asInt();
        if (x <= 0) {
            std::cout << "ERROR: элементы множества должны быть натуральными числами\n";
            return 0;
        }
        if (set.add(v)) {
            items.pushBack(v);
            total += x;
        }
    }
    int n = items.length();
    if (n == 0) {
        std::cout << "ERROR: пустое множество\n";
        return 0;
    }
    if (total > MAX_SUM) {
        std::cout << "ERROR: сумма элементов превышает " << MAX_SUM << '\n';
        return 0;
    }

    int half = static_cast<int>(total / 2);
    Array reach, from;
    for (int i = 0; i <= half; ++i) {
        reach.pushBack(Value(0));
        from.pushBack(Value(-1));
    }
    reach.set(0, Value(1));
    for (int idx = 0; idx < n; ++idx) {
        int a = items.get(idx).asInt();
        for (int s = half; s >= a; --s) {
            if (reach.get(s).asInt() == 0 && reach.get(s - a).asInt() == 1) {
                reach.set(s, Value(1));
                from.set(s, Value(idx));
            }
        }
    }
    int best = half;
    while (reach.get(best).asInt() == 0) --best;

    Array chosen;
    for (int i = 0; i < n; ++i) chosen.pushBack(Value(0));
    for (int s = best; s > 0;) {
        int i = from.get(s).asInt();
        chosen.set(i, Value(1));
        s -= items.get(i).asInt();
    }

    Set larger, smaller;
    for (int i = 0; i < n; ++i) {
        if (chosen.get(i).asInt() == 1) smaller.add(items.get(i));
        else larger.add(items.get(i));
    }

    long long big = total - best;
    std::cout << "Подмножество 1: " << subsetText(items, larger) << " (сумма = " << big << ")\n";
    std::cout << "Подмножество 2: " << subsetText(items, smaller) << " (сумма = " << best << ")\n";
    std::cout << "Разница сумм: " << big - best << '\n';
    return 0;
}

}