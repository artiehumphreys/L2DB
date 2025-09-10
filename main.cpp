#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include "common.hpp"
#include "linear_hash.hpp"

static inline u64 rand_u64() {
  return (static_cast<u64>(std::rand()) << 32) ^ static_cast<u64>(std::rand());
}

int main(int argc, char **argv) {
  double secs = 5.0;
  u64 l2_mib = 12;
  double table_frac = 0.5;
  bool random_access = false;
  u8 seed = 1;

  for (int i = 1; i < argc; ++i) {
    const std::string &s = argv[i];
    if (s == "--random") {
      random_access = true;
      continue;
    }
    if (i + 1 < argc) {
      const std::string &next = argv[++i];
      if (s == "--table_frac") {
        table_frac = std::stod(next);
      }
      if (s == "--l2-mib") {
        l2_mib = std::stoull(next);
      }
      if (s == "--secs") {
        secs = std::stod(next);
      }
    }
  }

  std::srand(seed);

  LinearHash ht(l2_mib * 1024ull * 1024ull, table_frac, 0.70);

  const uint64_t target = static_cast<uint64_t>(ht.maxItems() * 0.9);
  for (u64 i = 1; i <= target; ++i)
    if (!ht.put(i, 3 * i + 1))
      break;

  std::vector<u64> keys;
  keys.reserve(1 << 20);
  for (size_t i = 0; i < keys.capacity(); ++i)
    keys.push_back((rand_u64() % target) + 1);

  // cache warmup
  for (uint64_t i = 1; i <= target; i += 16)
    (void)ht.get(i);

  const auto deadline =
      std::chrono::steady_clock::now() + std::chrono::duration<double>(secs);
  u64 ops = 0, hits = 0;
  u64 ki = 0;
  while (std::chrono::steady_clock::now() < deadline) {
    for (int k = 0; k < 16; ++k) {
      u64 key =
          random_access ? keys[ki++ & (keys.size() - 1)] : (ops % target) + 1;
      if (k == 15)
        key ^= 0xDEADBEEF; // force a miss
      hits += ht.get(key).has_value();
      ++ops;
    }
  }

  std::cout << "items=" << target << " ops=" << ops << " time=" << secs << "s "
            << "throughput=" << (ops / (1e6 * secs)) << " Mops/s "
            << "app_hit%=" << (100.0 * hits / ops) << "\n";
  return 0;
}
