#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <set>
#include <numeric>
#include <string>
#include <vector>

#include "ravel/proof/strict_shell_pump.hpp"

namespace ravel::proof {

struct CarryPathWitness {
    std::vector<IntegerState> states;
    std::vector<std::int64_t> digits;
    bool replay = false;
};

inline CarryPathWitness replay_carry_path(
    std::vector<IntegerState> states,
    std::vector<std::int64_t> digits,
    std::int64_t digit_min = -1,
    std::int64_t digit_max = 1) {
    CarryPathWitness p{std::move(states), std::move(digits), false};
    if (p.states.empty() || p.states.size() != p.digits.size() + 1) return p;
    p.replay = true;
    for (std::size_t k = 0; k < p.digits.size(); ++k) {
        p.replay &= p.digits[k] >= digit_min && p.digits[k] <= digit_max;
        p.replay &= nbonacci_step(p.states[k], p.digits[k]) == p.states[k + 1];
    }
    return p;
}

struct SynchronizedZeroExpansionCertificate {
    std::size_t dimension = 0;
    std::size_t length = 0;
    CarryPathWitness left;
    CarryPathWitness right;
    std::vector<IntegerState> difference_states;
    std::vector<std::int64_t> difference_digits;
    std::size_t distinct_prefix_states = 0;
    bool equal_start = false;
    bool equal_end = false;
    bool paths_replay = false;
    bool difference_replay = false;
    bool zero_boundary = false;
    bool alphabet_bounded_by_two = false;
    bool exact_zero_expansion = false;
    std::string obstruction;
};

/** Subtract two equal-length carry paths with common endpoints.
 *
 * Linearity gives z_{k+1}=T(z_k, d_k-e_k).  Common endpoints make the
 * difference path run from zero to zero, so the digit-difference word is an
 * exact finite zero expansion over {-2,-1,0,1,2}.
 */
inline SynchronizedZeroExpansionCertificate derive_synchronized_zero_expansion(
    CarryPathWitness left, CarryPathWitness right) {
    SynchronizedZeroExpansionCertificate c;
    c.left = std::move(left);
    c.right = std::move(right);
    c.paths_replay = c.left.replay && c.right.replay;
    if (!c.paths_replay || c.left.states.size() != c.right.states.size() ||
        c.left.digits.size() != c.right.digits.size() || c.left.states.empty()) {
        c.obstruction = "paths do not replay with equal lengths";
        return c;
    }
    c.dimension = c.left.states.front().size();
    c.length = c.left.digits.size();
    c.equal_start = c.left.states.front() == c.right.states.front();
    c.equal_end = c.left.states.back() == c.right.states.back();
    c.difference_replay = true;
    c.alphabet_bounded_by_two = true;
    for (std::size_t k = 0; k < c.left.states.size(); ++k) {
        IntegerState z(c.dimension, 0);
        for (std::size_t i = 0; i < c.dimension; ++i)
            z[i] = c.left.states[k][i] - c.right.states[k][i];
        c.difference_states.push_back(std::move(z));
    }
    for (std::size_t k = 0; k < c.length; ++k) {
        const auto delta = c.left.digits[k] - c.right.digits[k];
        c.difference_digits.push_back(delta);
        c.alphabet_bounded_by_two &= std::llabs(delta) <= 2;
        c.difference_replay &=
            nbonacci_step(c.difference_states[k], delta) ==
            c.difference_states[k + 1];
    }
    c.zero_boundary = c.equal_start && c.equal_end &&
        std::all_of(c.difference_states.front().begin(),
                    c.difference_states.front().end(), [](auto x){ return x == 0; }) &&
        std::all_of(c.difference_states.back().begin(),
                    c.difference_states.back().end(), [](auto x){ return x == 0; });
    std::set<IntegerState> unique(c.difference_states.begin(), c.difference_states.end());
    c.distinct_prefix_states = unique.size();
    c.exact_zero_expansion = c.paths_replay && c.difference_replay &&
        c.zero_boundary && c.alphabet_bounded_by_two;
    if (!c.exact_zero_expansion)
        c.obstruction = "synchronized path difference is not an exact zero expansion";
    return c;
}

} // namespace ravel::proof

namespace ravel::proof {

inline CarryPathWitness repeat_closed_carry_path(
    const CarryPathWitness& loop, std::size_t repetitions) {
    CarryPathWitness out;
    if (!loop.replay || loop.states.empty() ||
        loop.states.front() != loop.states.back() || repetitions == 0)
        return out;
    out.states.push_back(loop.states.front());
    for (std::size_t r = 0; r < repetitions; ++r) {
        out.digits.insert(out.digits.end(), loop.digits.begin(), loop.digits.end());
        out.states.insert(out.states.end(), loop.states.begin() + 1, loop.states.end());
    }
    out.replay = true;
    for (std::size_t k = 0; k < out.digits.size(); ++k)
        out.replay &= nbonacci_step(out.states[k], out.digits[k]) == out.states[k + 1];
    return out;
}

struct SynchronizedLoopDifferenceCertificate {
    std::size_t common_length = 0;
    CarryPathWitness left_repeated;
    CarryPathWitness right_repeated;
    SynchronizedZeroExpansionCertificate zero_expansion;
    bool loops_share_basepoint = false;
    bool synchronized = false;
    bool nontrivial = false;
    bool valid = false;
    std::string obstruction;
};

/** Equalize two closed loop lengths by repetition and subtract them.
 * This is the exact branch-synchronization operation used by the universal
 * Condition-F reduction. */
inline SynchronizedLoopDifferenceCertificate
 derive_synchronized_loop_difference(
    const CarryPathWitness& left, const CarryPathWitness& right) {
    SynchronizedLoopDifferenceCertificate c;
    if (!left.replay || !right.replay || left.states.empty() || right.states.empty() ||
        left.states.front() != left.states.back() ||
        right.states.front() != right.states.back() ||
        left.states.front() != right.states.front() ||
        left.digits.empty() || right.digits.empty()) {
        c.obstruction = "inputs are not nonempty closed loops at a common basepoint";
        return c;
    }
    c.loops_share_basepoint = true;
    const auto a = left.digits.size();
    const auto b = right.digits.size();
    const auto g = std::gcd(a, b);
    c.common_length = a / g * b;
    c.left_repeated = repeat_closed_carry_path(left, c.common_length / a);
    c.right_repeated = repeat_closed_carry_path(right, c.common_length / b);
    c.synchronized = c.left_repeated.replay && c.right_repeated.replay &&
        c.left_repeated.digits.size() == c.right_repeated.digits.size();
    if (!c.synchronized) {
        c.obstruction = "loop repetition failed to synchronize";
        return c;
    }
    c.zero_expansion = derive_synchronized_zero_expansion(
        c.left_repeated, c.right_repeated);
    c.nontrivial = c.left_repeated.digits != c.right_repeated.digits;
    c.valid = c.zero_expansion.exact_zero_expansion && c.nontrivial;
    if (!c.valid)
        c.obstruction = c.nontrivial ? c.zero_expansion.obstruction
                                     : "synchronized loops are identical";
    return c;
}

} // namespace ravel::proof
