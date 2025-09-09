#include <cstdlib>
#include <cstring>
#include <new>
#include <optional>

#include "bucket.hpp"
#include "common.hpp"
#include "linear_hash.hpp"

LinearHash::LinearHash(u64 l2Bytes, double tableFraction = 0.5,
                       double loadFactor = 0.7) {
  tableFraction = clampDouble(tableFraction);
  loadFactor = clampDouble(loadFactor);

  u64 tableBytes = static_cast<u64>(l2Bytes * tableFraction);
  u64 maxBuckets = tableBytes / static_cast<u64>(sizeof(Bucket));

  u64 capacity = nextPow2(maxBuckets);

  // ensure capacity is still a power of two
  while (capacity * static_cast<u64>(sizeof(Bucket)) > tableBytes &&
         capacity > 8)
    capacity >>= 1;

  void *p = nullptr;

  u64 byteCount = capacity * static_cast<u64>(sizeof(Bucket));

  if (posix_memalign(&p, CACHELINE_SIZE, byteCount) != 0 || !p) {
    throw std::bad_alloc();
  }

  std::memset(p, 0, byteCount);

  buckets_ = static_cast<Bucket *>(p);
  capacity_ = capacity;
  // mask for fast modulo with power of two
  mask_ = capacity_ - 1;
  size_ = 0;
}

LinearHash::~LinearHash() {
  std::free(buckets_);
  buckets_ = nullptr;
  capacity_ = mask_ = size_ = 0;
  maxLoadFactor_ = 0.0;
}

bool LinearHash::put(u64 key, u64 value) {
  if (size_ + 1 > maxItems())
    return false;

  u64 index = splitmix64(key) & mask_;
  prefetchL2(&buckets_[index]);

  for (;;) {
    Bucket &b = buckets_[index];
    if (b.key == 0) { // insert
      b.key = key;
      b.value = value;
      ++size_;
      return true;
    }

    if (b.key == key) { // update
      b.value = value;
      return false;
    }

    // collision
    index = (index + 1) & mask_;
    if (index % static_cast<u64>(bucketsPerLine) == 0)
      prefetchL2(&buckets_[index]);
  }
}

std::optional<u64> LinearHash::get(u64 key) {
  u64 index = splitmax64(key) & mask_;

  for (;;) {
    const Bucket &b = buckets_[index];
    if (b.key == 0) // terminate probe
      return std::nullopt;
    if (b.key == key)
      return b.value;

    index = (index + 1) & mask_;
    if (index % static_cast<u64>(bucketsPerLine) == 0)
      prefetchL2(&buckets_[index]);
  }
}
