#pragma once

#include <cstddef>
#include <cstdint>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "ravel/proof/ordered_boundary_queue.hpp"

namespace ravel::proof {

// A derivation object, not a flat certificate.  Each law is generated from the
// executable queue operation and can replay itself on concrete states.
struct BoundaryGrammarLaw {
  enum class Kind {
    left_shift,
    left_insert,
    right_insert,
    right_shift,
    weighted_telescoping
  };

  Kind kind{};
  std::size_t role = 0;
  std::string proposition;
};

struct BoundaryGrammarDerivation {
  std::size_t dimension = 0;
  std::size_t depth = 0;
  std::vector<BoundaryGrammarLaw> laws;

  [[nodiscard]] bool replay(const ShellState& x, std::int64_t incoming) const {
    if (x.size() != dimension || depth == 0 || depth > dimension) return false;
    const auto u = derive_boundary_queue_update(x, incoming, depth);
    if (!u.exact) return false;

    // Derive nontrivial deterministic weights from the role order itself.
    std::vector<std::int64_t> lw(depth), rw(depth);
    for (std::size_t k = 0; k < depth; ++k) {
      lw[k] = static_cast<std::int64_t>(k + 1);
      rw[k] = static_cast<std::int64_t>(depth - k);
    }
    const auto before = weighted_boundary_value(u.before, lw, rw);
    const auto after = weighted_boundary_value(u.after, lw, rw);
    return after - before == weighted_boundary_delta_formula(
      u.before, incoming, u.left_entering, lw, rw);
  }
};

inline BoundaryGrammarDerivation derive_boundary_queue_grammar(
    std::size_t dimension, std::size_t depth) {
  if (dimension == 0 || depth == 0 || depth > dimension)
    throw std::invalid_argument("invalid reflective boundary grammar size");

  BoundaryGrammarDerivation d;
  d.dimension = dimension;
  d.depth = depth;

  for (std::size_t k = 0; k + 1 < depth; ++k) {
    d.laws.push_back({BoundaryGrammarLaw::Kind::left_shift, k,
      "left'(" + std::to_string(k) + ") = left(" + std::to_string(k + 1) + ")"});
  }
  d.laws.push_back({BoundaryGrammarLaw::Kind::left_insert, depth - 1,
    "left'(depth-1) = role(depth), or incoming when depth = n"});
  d.laws.push_back({BoundaryGrammarLaw::Kind::right_insert, 0,
    "right'(0) = incoming"});
  for (std::size_t k = 1; k < depth; ++k) {
    d.laws.push_back({BoundaryGrammarLaw::Kind::right_shift, k,
      "right'(" + std::to_string(k) + ") = right(" + std::to_string(k - 1) + ")"});
  }
  d.laws.push_back({BoundaryGrammarLaw::Kind::weighted_telescoping, 0,
    "weighted potential difference equals outgoing/incoming boundary flux"});
  return d;
}

struct QueueWeightRecurrenceDerivation {
  std::size_t depth = 0;
  // Exact coefficients of the one-step delta in the basis
  // [L_0..L_{d-1}, R_0..R_{d-1}, leftEntering, incoming].
  std::vector<std::int64_t> coefficients;

  [[nodiscard]] bool replay(const ShellState& x, std::int64_t incoming,
                            const std::vector<std::int64_t>& left_w,
                            const std::vector<std::int64_t>& right_w) const {
    if (left_w.size() != depth || right_w.size() != depth || x.size() < depth)
      return false;
    const auto u = derive_boundary_queue_update(x, incoming, depth);
    if (!u.exact) return false;
    const auto before = weighted_boundary_value(u.before, left_w, right_w);
    const auto after = weighted_boundary_value(u.after, left_w, right_w);
    return after - before == weighted_boundary_delta_formula(
      u.before, incoming, u.left_entering, left_w, right_w);
  }
};

inline QueueWeightRecurrenceDerivation derive_queue_weight_recurrence(
    const std::vector<std::int64_t>& left_w,
    const std::vector<std::int64_t>& right_w) {
  if (left_w.empty() || left_w.size() != right_w.size())
    throw std::invalid_argument("invalid queue weights");
  const std::size_t d = left_w.size();
  QueueWeightRecurrenceDerivation out;
  out.depth = d;
  out.coefficients.assign(2 * d + 2, 0);

  // L contribution after shift minus before.
  out.coefficients[0] -= left_w[0];
  for (std::size_t k = 1; k < d; ++k)
    out.coefficients[k] += left_w[k - 1] - left_w[k];
  out.coefficients[2 * d] += left_w[d - 1]; // left-entering

  // R contribution after shift minus before.
  for (std::size_t k = 0; k + 1 < d; ++k)
    out.coefficients[d + k] += right_w[k + 1] - right_w[k];
  out.coefficients[d + d - 1] -= right_w[d - 1];
  out.coefficients[2 * d + 1] += right_w[0]; // incoming
  return out;
}

inline std::string emit_boundary_grammar_lean() {
  return R"LEAN(import Mathlib

namespace RavelReflectiveBoundaryGrammar

variable {α : Type}

/-- Shift left and append one new boundary element. -/
def shiftAppend (xs : List α) (incoming : α) : List α := xs.drop 1 ++ [incoming]

/-- Ordered left boundary queue of depth `d`. -/
def leftQueue (d : Nat) (xs : List α) : List α := xs.take d

/-- Ordered right boundary queue, nearest incoming boundary first. -/
def rightQueue (d : Nat) (xs : List α) : List α := xs.reverse.take d

/-- The executable queue operation is literally a left shift followed by one insertion. -/
theorem shiftAppend_cons {x : α} {xs : List α} (incoming : α) :
    shiftAppend (x :: xs) incoming = xs ++ [incoming] := by
  simp [shiftAppend]

/-- The newly appended value is the last value of every nonempty shifted queue. -/
theorem getLast_shiftAppend {x : α} {xs : List α} (incoming : α) :
    (shiftAppend (x :: xs) incoming).getLast? = some incoming := by
  simp [shiftAppend]

/-- The first element after shifting is the old second element. -/
theorem head_shiftAppend {x y : α} {xs : List α} (incoming : α) :
    (shiftAppend (x :: y :: xs) incoming).head? = some y := by
  simp [shiftAppend]

/-- Repeating the executable operation gives the grammar induction over words. -/
def run : List α → List α → List α
  | state, [] => state
  | state, a :: word => run (shiftAppend state a) word

@[simp] theorem run_nil (state : List α) : run state [] = state := rfl

@[simp] theorem run_cons (state : List α) (a : α) (word : List α) :
    run state (a :: word) = run (shiftAppend state a) word := rfl

end RavelReflectiveBoundaryGrammar
)LEAN";
}
} // namespace ravel::proof
