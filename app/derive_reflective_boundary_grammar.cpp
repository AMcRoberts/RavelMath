#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "ravel/proof/reflective_boundary_grammar.hpp"

using namespace ravel::proof;

int main(int argc, char** argv) {
  std::size_t max_n = 20;
  std::string lean_path = "lean/generated/reflective_boundary_grammar.lean";
  if (argc > 1) max_n = static_cast<std::size_t>(std::stoul(argv[1]));
  if (argc > 2) lean_path = argv[2];

  std::uint64_t laws = 0;
  for (std::size_t n = 3; n <= max_n; ++n) {
    for (std::size_t d = 1; d <= n; ++d) {
      const auto g = derive_boundary_queue_grammar(n, d);
      laws += g.laws.size();
      std::vector<std::int64_t> lw(d), rw(d);
      for (std::size_t k = 0; k < d; ++k) {
        lw[k] = static_cast<std::int64_t>(k + 1);
        rw[k] = static_cast<std::int64_t>(d - k);
      }
      (void)derive_queue_weight_recurrence(lw, rw);
    }
  }

  std::ofstream out(lean_path);
  if (!out) {
    std::cerr << "cannot open Lean output " << lean_path << "\n";
    return 2;
  }
  out << emit_boundary_grammar_lean();
  std::cout << "REFLECTIVE_BOUNDARY_GRAMMAR max_n=" << max_n
            << " derived_laws=" << laws
            << " lean=" << lean_path << "\n";
  return 0;
}
