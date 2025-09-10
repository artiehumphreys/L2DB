#pragma once

#include "common.hpp"

struct Bucket {
  u64 key;
  u64 value;
};

static_assert(CACHELINE_SIZE % sizeof(Bucket) == 0, "Bad Bucket size");
static_assert(sizeof(Bucket) == 16, "Must be 16 bytes");
constexpr u8 bucketsPerLine = CACHELINE_SIZE / sizeof(Bucket);
