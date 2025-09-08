#include <optional>

#include "bucket.hpp"
#include "common.hpp"

class LinearHash {
public:
  explicit LinearHash(u64 l2Bytes, double tableFraction = 0.5,
                      double loadFactor = 0.7);

  ~LinearHash();

  LinearHash(const LinearHash &) = delete;
  LinearHash &operator=(const LinearHash &) = delete;
  LinearHash(LinearHash &&) noexcept;
  LinearHash &operator=(LinearHash &&) noexcept;

  u64 size() const { return size_; }
  u64 capacity() const { return capacity_; }
  u64 maxItems() const { return capacity_ * maxLoadFactor_; }

  bool put(u64 key, u64 value);
  std::optional<u64> get(u64 key) const;

private:
  Bucket *buckets_{nullptr};
  u64 capacity_ = 0;
  u64 mask_ = 0;
  u64 size_ = 0;
  double maxLoadFactor_ = 0;
};
