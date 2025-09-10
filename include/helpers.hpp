#pragma once

#include <algorithm>
#include <bit>

#include "common.hpp"

static inline void prefetchL2(const void *p) noexcept {
  // explicitly instruct processor to prefetch data into cache
  // addr, r/w, expected temporal locality
  // ETL of 2 loads data into L2 or higher
  __builtin_prefetch(p, 0, 2);
}

// splitmix64
// https://stackoverflow.com/questions/664014/what-integer-hash-function-are-good-that-accepts-an-integer-hash-key
static inline u64 splitmix64(u64 x) {
  x += 0x9e3779b97f4a7c15ULL;
  x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
  x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
  return x ^ (x >> 31);
}

static inline u64 nextPow2(u64 x) { return std::bit_ceil(x); }

static inline double clampDouble(double x) {
  return std::clamp(x, static_cast<double>(0.01), static_cast<double>(1.0));
}
