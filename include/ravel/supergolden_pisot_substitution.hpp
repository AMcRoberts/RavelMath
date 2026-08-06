#pragma once
#include <cstdint>
#include <vector>
namespace ravel {
// Supergolden number: dominant root of x^3 - x^2 - 1 (fourth-smallest
// Pisot number). The rule below is derived, not copied from literature:
// see `derive_supergolden_qrs_closure` in
// `include/ravel/proof/supergolden_qrs_closure.hpp`, which recomputes the
// exact greedy beta-expansion of 1 via `exact_greedy_beta_expansion_of_one`
// and builds this rule via `canonical_beta_substitution_from_digits`. It
// happens to coincide with the substitution commonly called the
// "supergolden" or Narayana-adjacent substitution in the literature; that
// coincidence is evidence corroborating the derivation, not its source.
inline std::vector<std::vector<std::int8_t>> supergolden_pisot_rule(){
 return {{0,1},{2},{0}};
}
inline std::vector<long long> supergolden_pisot_minpoly_coefficients(){return {-1,0,-1,1};} // x^3-x^2-1, low-degree-first
inline std::vector<long long> supergolden_pisot_greedy_digits(){return {1,0,1};}
}
