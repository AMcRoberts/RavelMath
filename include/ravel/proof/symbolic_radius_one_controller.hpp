#pragma once

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <set>
#include <stdexcept>
#include <vector>

namespace ravel::proof {

using SymbolicControllerState = std::vector<std::int64_t>;

inline std::int64_t controller_center(
    const SymbolicControllerState& state) {
    if (state.empty()) throw std::invalid_argument("empty controller state");
    std::int64_t value = state.front();
    for (std::size_t i = 1; i < state.size(); ++i) value -= state[i];
    return value;
}

inline bool symbolic_tail_legal(
    const SymbolicControllerState& state,
    std::int64_t base_digit,
    std::int64_t next_tail) {
    if (next_tail < -1 || next_tail > 1) return false;
    return std::llabs(
        next_tail - controller_center(state) + base_digit) <= 1;
}

inline SymbolicControllerState append_tail(
    const SymbolicControllerState& state,
    std::int64_t next_tail) {
    if (state.empty()) throw std::invalid_argument("empty controller state");
    SymbolicControllerState next(state.size(), 0);
    for (std::size_t i = 0; i + 1 < state.size(); ++i)
        next[i] = state[i + 1];
    next.back() = next_tail;
    return next;
}

inline std::set<SymbolicControllerState> symbolic_successors(
    const SymbolicControllerState& state,
    std::int64_t base_digit) {
    std::set<SymbolicControllerState> result;
    for (const auto tail : {-1LL, 0LL, 1LL}) {
        if (!symbolic_tail_legal(state, base_digit, tail)) continue;
        const auto next = append_tail(state, tail);
        bool boundary = false;
        for (const auto value : next) boundary |= std::llabs(value) == 1;
        if (boundary) result.insert(next);
    }
    return result;
}

}  // namespace ravel::proof
