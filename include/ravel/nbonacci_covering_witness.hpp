// nbonacci_covering_witness.hpp
//
// Header for the n-bonacci covering-witness computation.  This
// header exposes the core logic (SignInt bareiss-like Laplace
// expansion, sign enumeration, simplicity sort) as a reusable
// function that the existing C++ covering-witness enumerator, the
// new C++ proof probe, and a Lua binding all share.

#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace ravel {

// One covering witness for the n-bonacci numeration at length L.
// Computed by the C++ covering-witness search: it picks the
// simplest (n-1)-pin, sign-assigned, free-parameter assignment
// that satisfies the box (all values in [-1, 1]) and cover (every
// window has a ±1 entry) properties, with the search restricted
// to the (n-1)-pin strategy (per the doc's documented route in
// docs/NBONACCI_CODE_MECHANISM.md).
struct covering_witness_t {
    long long n;          // dimension
    long long L;          // sequence length = n + number of windows
    std::vector<long long> indices;          // (n-1) pin indices
    std::vector<long long> signs;            // (n-1) pin signs, all ±1
    std::vector<std::string> free_params;   // (n-1) rationals a_1..a_{n-1}
    std::vector<std::string> sequence;       // (n + L) rationals a_0..a_{n+L-1}
};

// Compute the simplest (lowest simplicity score) covering witness
// at length L (default L = n + 1, the SAT witness length).  Returns
// std::nullopt if no valid witness exists.  Internally uses exact
// rational arithmetic (Bareiss-like Laplace expansion over a
// SignInt = (num, denom) reduced to coprime form).
std::optional<covering_witness_t> compute_simplest_covering_witness(
    std::size_t n, std::size_t L);

// Check box: all free_params and sequence values in [-1, 1].
// Returns a JSON-style error string ("") if OK, or a description
// of the first violation.
std::string check_box(const covering_witness_t& w);

// Check cover: every window t in 0..L has some i in 0..n-1 with
// |a_{t+i}| = 1.  Returns "" if OK, else description.
std::string check_cover(const covering_witness_t& w);

}  // namespace ravel
