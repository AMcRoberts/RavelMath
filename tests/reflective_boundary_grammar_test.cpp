#include <cstdint>
#include <iostream>
#include <vector>
#include "ravel/proof/reflective_boundary_grammar.hpp"

using namespace ravel::proof;

int main() {
  std::uint64_t checks = 0;
  for (std::size_t n = 3; n <= 20; ++n) {
    for (std::size_t depth = 1; depth <= n; ++depth) {
      const auto grammar = derive_boundary_queue_grammar(n, depth);
      std::vector<std::int64_t> lw(depth), rw(depth);
      for (std::size_t k = 0; k < depth; ++k) {
        lw[k] = static_cast<std::int64_t>((k + 1) * (k + 2));
        rw[k] = static_cast<std::int64_t>((depth - k) * (depth - k + 1));
      }
      const auto recurrence = derive_queue_weight_recurrence(lw, rw);
      for (std::uint64_t mask = 0; mask < 512; ++mask) {
        ShellState x(n);
        for (std::size_t i = 0; i < n; ++i) {
          const auto digit = (mask / (1ULL << (i % 9))) % 3;
          x[i] = static_cast<std::int64_t>(digit) - 1;
        }
        for (std::int64_t incoming = -1; incoming <= 1; ++incoming) {
          if (!grammar.replay(x, incoming) ||
              !recurrence.replay(x, incoming, lw, rw)) {
            std::cerr << "reflective boundary grammar failure n=" << n
                      << " depth=" << depth << "\n";
            return 1;
          }
          ++checks;
        }
      }
    }
  }
  std::cout << "reflective boundary grammar PASS checks=" << checks << "\n";
  return 0;
}
