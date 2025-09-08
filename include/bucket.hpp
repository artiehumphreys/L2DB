#include "common.hpp"

struct Bucket {
  u64 key;
  u64 value;
};

static_assert(CACHELINE_SIZE % sizeof(Bucket) == 0, "Bad Bucket size");
constexpr u8 bucketsPerLine = CACHELINE_SIZE / sizeof(Bucket);
