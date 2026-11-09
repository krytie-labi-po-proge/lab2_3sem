#pragma once
#include <chrono>
#include <cstdint>
#include <cstdio>

#include "ChainTable.hpp"
#include "CuckooTable.hpp"
#include "Value.hpp"

namespace hash_benchmark {

using Clock = std::chrono::steady_clock;

Value key(uint64_t i) {
    return Value(static_cast<int>(static_cast<uint32_t>((i * 2654435761ULL) & 0xFFFFFFFFULL)));
}

double perOp(Clock::time_point start, uint64_t n) {
    return std::chrono::duration<double, std::nano>(Clock::now() - start).count() / static_cast<double>(n);
}

volatile long long sink = 0;

void chainExperiment() {
    std::printf("Метод цепочек: m = 65536 цепочек, автоперехеширование выключено\n");
    std::printf("%6s %8s %10s %10s %9s %10s %9s %9s\n", "alpha", "n", "ins,ns", "find+,ns", "probes+", "find-,ns", "probes-", "maxchain");
    const int m = 65536;
    const double alphas[] = {0.25, 0.5, 0.75, 1.0, 2.0, 4.0};
    for (double a : alphas) {
        uint64_t n = static_cast<uint64_t>(a * m);
        ChainTable t(m, false);
        auto st = Clock::now();
        for (uint64_t i = 0; i < n; ++i) t.put(key(i), Value(static_cast<int>(i)));
        double tIns = perOp(st, n);

        t.probes = 0;
        st = Clock::now();
        for (uint64_t i = 0; i < n; ++i) { auto r = t.get(key(i)); sink += r ? 1 : 0; }
        double tHit = perOp(st, n);
        double pHit = static_cast<double>(t.probes) / n;

        t.probes = 0;
        st = Clock::now();
        for (uint64_t i = n; i < 2 * n; ++i) { auto r = t.get(key(i)); sink += r ? 1 : 0; }
        double tMiss = perOp(st, n);
        double pMiss = static_cast<double>(t.probes) / n;

        std::printf("%6.2f %8llu %10.1f %10.1f %9.2f %10.1f %9.2f %9d\n", a, (unsigned long long)n, tIns, tHit, pHit, tMiss, pMiss, t.longestChain());
    }
    std::printf("\n");
}

void cuckooExperiment() {
    std::printf("Хеширование кукушки: две таблицы по 32768 ячеек\n");
    std::printf("%6s %8s %10s %10s %9s %10s %9s %12s\n", "alpha", "n", "ins,ns", "find+,ns", "probes+", "find-,ns", "probes-", "kicks/ins");
    const int m1 = 32768;
    const double loads[] = {0.1, 0.2, 0.3, 0.4};
    for (double a : loads) {
        uint64_t n = static_cast<uint64_t>(a * 2.0 * m1);
        CuckooTable t(m1);
        auto st = Clock::now();
        bool failed = false;
        for (uint64_t i = 0; i < n; ++i) {
            if (!t.tryInsert(key(i), Value(static_cast<int>(i)))) { failed = true; break; }
        }
        double tIns = perOp(st, n);
        if (failed) {
            std::printf("%6.2f %8llu  failure: kick cycle detected\n", a, (unsigned long long)n);
            continue;
        }
        t.probes = 0;
        st = Clock::now();
        for (uint64_t i = 0; i < n; ++i) { auto r = t.get(key(i)); sink += r ? 1 : 0; }
        double tHit = perOp(st, n);
        double pHit = static_cast<double>(t.probes) / n;

        t.probes = 0;
        st = Clock::now();
        for (uint64_t i = n; i < 2 * n; ++i) { auto r = t.get(key(i)); sink += r ? 1 : 0; }
        double tMiss = perOp(st, n);
        double pMiss = static_cast<double>(t.probes) / n;

        std::printf("%6.2f %8llu %10.1f %10.1f %9.2f %10.1f %9.2f %12.2f\n", a, (unsigned long long)n, tIns, tHit, pHit, tMiss, pMiss, static_cast<double>(t.kicks) / n);
    }
    std::printf("\n");
}

void cuckooCapacity() {
    std::printf("Хеширование кукушки: загрузка в момент первого цикла\n");
    std::printf("%12s %12s\n", "cells/table", "load");
    const int sizes[] = {1024, 8192, 65536};
    for (int m1 : sizes) {
        const int trials = 5;
        double sum = 0;
        for (int tr = 0; tr < trials; ++tr) {
            CuckooTable t(m1, 0x12345678ULL + tr * 7919ULL, 0x9abcdef0ULL + tr * 104729ULL);
            uint64_t base = static_cast<uint64_t>(tr) * 10000000ULL;
            for (uint64_t i = 0;; ++i) {
                if (!t.tryInsert(key(base + i), Value(0))) break;
            }
            sum += static_cast<double>(t.len()) / (2.0 * m1);
        }
        std::printf("%12d %12.3f\n", m1, sum / trials);
    }
    std::printf("\n");
}

void cuckooGrowth() {
    std::printf("Хеширование кукушки: вставка 200000 ключей с автоматическим перестроением\n");
    CuckooTable t(1024);
    auto st = Clock::now();
    for (uint64_t i = 0; i < 200000; ++i) t.put(key(i), Value(static_cast<int>(i)));
    double total = perOp(st, 200000);
    bool ok = true;
    for (uint64_t i = 0; i < 200000; ++i)
        if (!t.get(key(i))) ok = false;
    std::printf("элементов: %zu, ячеек: %zu, загрузка: %.3f, перестроений: %u, вставка: %.1f нс/оп, все ключи найдены: %s\n",
                t.len(), t.capacity(), static_cast<double>(t.len()) / t.capacity(), t.rehashes, total, ok ? "true" : "false");
}

int run_all(int, char*[]) {
    chainExperiment();
    cuckooExperiment();
    cuckooCapacity();
    cuckooGrowth();
    return 0;
}

}