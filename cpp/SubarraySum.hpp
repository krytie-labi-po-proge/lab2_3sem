#pragma once
#include <iostream>
#include <optional>
#include <sstream>
#include <string>

#include "Array.hpp"
#include "ChainTable.hpp"
#include "Util.hpp"
#include "Value.hpp"

namespace subarray_sum {

std::string subarrayText(const Array& a, int from, int to) {
    std::string s = "[";
    for (int i = from; i < to; ++i) {
        if (i > from) s += ", ";
        s += a.get(i).toString();
    }
    return s + "]";
}

int find_stdin(int argc, char* argv[]) {
    std::string text;
    int target; 
    bool hasQuery = false;
    bool hasTarget = false;

    if (argc < 5){
        printf("Недостаточно аргументов\n");
        return 0;
    }

    for (int i = 1; i < argc; ++i){
        if (std::string(argv[i]) == "--query") {
            text = argv[i+1];
            hasQuery = true;
        }
        if (std::string(argv[i]) == "--target") {
            target = std::stoi(argv[i+1]);
            hasTarget = true;
        }
    }

    if (!hasQuery || !hasTarget){
        printf("Должны быть аргументы --query и --target");
        return 0;
    }

    Array a;
    if (!parseInts(text, a)) {
        std::cout << "ERROR: неверный формат данных\n";
        return 0;
    }
    int n = a.length();

    Array pre;
    pre.pushBack(Value(0));
    long long acc = 0;
    for (int i = 0; i < n; ++i) {
        acc += a.get(i).asInt();
        if (acc > 2147483647LL || acc < -2147483647LL - 1) {
            std::cout << "ERROR: переполнение префиксной суммы\n";
            return 0;
        }
        pre.pushBack(Value(static_cast<int>(acc)));
    }

    ChainTable table(16, true);
    Array prev;
    table.put(Value(0), Value(0));
    prev.pushBack(Value(-1));

    int found = 0;
    for (int j = 1; j <= n; ++j) {
        int pj = pre.get(j).asInt();
        long long need = static_cast<long long>(pj) - target;
        if (need >= -2147483647LL - 1 && need <= 2147483647LL) {
            std::optional<Value> h = table.get(Value(static_cast<int>(need)));
            int cur = h ? h->asInt() : -1;
            while (cur != -1) {
                std::cout << subarrayText(a, cur, j) << "\n";
                ++found;
                cur = prev.get(cur).asInt();
            }
        }
        std::optional<Value> last = table.get(Value(pj));
        prev.pushBack(Value(last ? last->asInt() : -1));
        table.put(Value(pj), Value(j));
    }
    if (found == 0) std::cout << "Подмассивов с суммой " << target << " нет\n";
    else std::cout << "Найдено подмассивов: " << found << '\n';
    return 0;
}

}