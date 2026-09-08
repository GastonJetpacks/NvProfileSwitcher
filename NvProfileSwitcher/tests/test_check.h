// Minimal shared check macro for the console tests. No framework.
#pragma once
#include <cstdio>

extern int gFailures;
extern int gChecks;

#define CHECK(cond)                                                                 \
    do {                                                                            \
        ++gChecks;                                                                  \
        if (!(cond)) {                                                              \
            ++gFailures;                                                            \
            std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);            \
        }                                                                           \
    } while (0)
