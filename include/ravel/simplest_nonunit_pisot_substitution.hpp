#pragma once
#include <cstdint>
#include <vector>
namespace ravel {
// x^2-2x-2, beta = 1+sqrt(3) ~ 2.7320508. The simplest possible
// non-unit Pisot substitution: derived, not searched for, from
// `beta_in_one_two_forces_qrs.hpp`'s own argument taken to its
// necessary conclusion. Every Pisot number has a forced first digit
// t_1 = floor(beta); a terminating expansion's LAST digit equals
// |norm(beta)| exactly (`pisot_norm_from_last_digit.hpp`); so the
// shortest possible non-unit digit sequence is length 2 with
// t_2 = 2 (the smallest non-unit value), which forces t_1 >= t_2 = 2
// for admissibility -- i.e. digits (2,2), uniquely minimal in both
// length and magnitude among non-unit cases. No search over
// candidate polynomials was needed to arrive at this example.
inline std::vector<std::vector<std::int8_t>> simplest_nonunit_pisot_rule(){
 return {{0,0,1},{0,0}};
}
inline std::vector<long long> simplest_nonunit_pisot_minpoly_coefficients(){return {-2,-2,1};} // x^2-2x-2, low-degree-first
inline std::vector<long long> simplest_nonunit_pisot_greedy_digits(){return {2,2};}
}
