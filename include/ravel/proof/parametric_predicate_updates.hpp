#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "ravel/proof/symbolic_radius_one_controller.hpp"

namespace ravel::proof {

/*
 * Dimension-free update algebra for the predicate language used by the
 * first-return boundary abstraction.
 *
 * The controller transition is the shift/append map
 *
 *     T_t(x_0,...,x_{n-1}) = (x_1,...,x_{n-1},t).
 *
 * Local statistics therefore obey a boundary-flux identity: occurrences
 * wholly inside the retained window survive; occurrences crossing the old
 * left boundary disappear; occurrences ending at the appended tail appear.
 * This file records those identities as executable derivations.
 */

struct LocalWordUpdateProof {
    SymbolicControllerState source;
    std::int64_t tail = 0;
    SymbolicControllerState target;
    bool shift_append_replay = false;
    bool count_equations = false;
    bool adjacency_equations = false;
    bool factor_equations = false;
    bool period_equations = false;
    bool endpoint_equations = false;
    bool valid = false;
    std::string failure;
};

inline std::int64_t indicator(bool b) { return b ? 1 : 0; }

inline std::size_t symbol_count(const SymbolicControllerState& x,
                                std::int64_t a) {
    return static_cast<std::size_t>(std::count(x.begin(), x.end(), a));
}

inline std::size_t change_count(const SymbolicControllerState& x) {
    std::size_t out = 0;
    for (std::size_t i = 1; i < x.size(); ++i) out += x[i - 1] != x[i];
    return out;
}

inline std::size_t equal_adjacency_count(const SymbolicControllerState& x) {
    if (x.empty()) return 0;
    return (x.size() - 1) - change_count(x);
}

inline bool starts_with(const SymbolicControllerState& x,
                        const SymbolicControllerState& word) {
    return word.size() <= x.size() &&
           std::equal(word.begin(), word.end(), x.begin());
}

inline bool ends_with(const SymbolicControllerState& x,
                      const SymbolicControllerState& word) {
    return word.size() <= x.size() &&
           std::equal(word.rbegin(), word.rend(), x.rbegin());
}

inline std::size_t factor_count(const SymbolicControllerState& x,
                                const SymbolicControllerState& word) {
    if (word.empty()) return x.size() + 1;
    if (word.size() > x.size()) return 0;
    std::size_t out = 0;
    for (std::size_t i = 0; i + word.size() <= x.size(); ++i)
        out += std::equal(word.begin(), word.end(), x.begin() + i);
    return out;
}

inline std::size_t period_defect_count(const SymbolicControllerState& x,
                                       std::size_t period) {
    if (period == 0) throw std::invalid_argument("period zero");
    std::size_t out = 0;
    for (std::size_t i = period; i < x.size(); ++i)
        out += x[i] != x[i - period];
    return out;
}

inline SymbolicControllerState suffix_with_tail(
    const SymbolicControllerState& x,
    std::size_t kept_source_symbols,
    std::int64_t tail) {
    SymbolicControllerState out;
    const auto keep = std::min(kept_source_symbols, x.size());
    out.insert(out.end(), x.end() - static_cast<std::ptrdiff_t>(keep), x.end());
    out.push_back(tail);
    return out;
}

inline LocalWordUpdateProof derive_local_word_update_proof(
    const SymbolicControllerState& x,
    std::int64_t tail,
    std::size_t max_factor_length = 4,
    std::size_t max_period = 4) {
    LocalWordUpdateProof p;
    p.source = x;
    p.tail = tail;
    if (x.empty()) { p.failure = "empty state"; return p; }
    p.target = append_tail(x, tail);
    p.shift_append_replay = p.target.size() == x.size();
    for (std::size_t i = 0; i + 1 < x.size(); ++i)
        p.shift_append_replay &= p.target[i] == x[i + 1];
    p.shift_append_replay &= p.target.back() == tail;

    p.endpoint_equations =
        p.target.front() == (x.size() == 1 ? tail : x[1]) &&
        p.target.back() == tail;

    p.count_equations = true;
    for (const auto a : {-1LL, 0LL, 1LL}) {
        const auto lhs = static_cast<std::int64_t>(symbol_count(p.target, a));
        const auto rhs = static_cast<std::int64_t>(symbol_count(x, a))
                       - indicator(x.front() == a) + indicator(tail == a);
        p.count_equations &= lhs == rhs;
    }

    if (x.size() == 1) {
        p.adjacency_equations = change_count(p.target) == 0;
    } else {
        const auto change_rhs = static_cast<std::int64_t>(change_count(x))
            - indicator(x[0] != x[1]) + indicator(x.back() != tail);
        const auto equal_rhs = static_cast<std::int64_t>(equal_adjacency_count(x))
            - indicator(x[0] == x[1]) + indicator(x.back() == tail);
        p.adjacency_equations =
            static_cast<std::int64_t>(change_count(p.target)) == change_rhs &&
            static_cast<std::int64_t>(equal_adjacency_count(p.target)) == equal_rhs;
    }

    p.factor_equations = true;
    for (std::size_t length = 1; length <= std::min(max_factor_length, x.size()); ++length) {
        std::size_t words = 1;
        for (std::size_t i = 0; i < length; ++i) words *= 3;
        for (std::size_t code = 0; code < words; ++code) {
            auto q = code;
            SymbolicControllerState word(length);
            for (std::size_t i = 0; i < length; ++i) {
                word[length - 1 - i] = static_cast<std::int64_t>(q % 3) - 1;
                q /= 3;
            }
            const auto old_count = static_cast<std::int64_t>(factor_count(x, word));
            const auto new_count = static_cast<std::int64_t>(factor_count(p.target, word));
            const auto exits = indicator(starts_with(x, word));
            const auto incoming_window = suffix_with_tail(x, length - 1, tail);
            const auto enters = indicator(incoming_window == word);
            p.factor_equations &= new_count == old_count - exits + enters;
        }
    }

    p.period_equations = true;
    for (std::size_t period = 1; period <= std::min(max_period, x.size()); ++period) {
        const auto old_defects = static_cast<std::int64_t>(period_defect_count(x, period));
        const auto new_defects = static_cast<std::int64_t>(period_defect_count(p.target, period));
        if (period >= x.size()) {
            p.period_equations &= new_defects == 0;
        } else {
            const auto exits = indicator(x[period] != x[0]);
            const auto enters = indicator(tail != x[x.size() - period]);
            p.period_equations &= new_defects == old_defects - exits + enters;
        }
    }

    p.valid = p.shift_append_replay && p.endpoint_equations &&
              p.count_equations && p.adjacency_equations &&
              p.factor_equations && p.period_equations;
    if (!p.valid) p.failure = "one or more shift-register update identities failed";
    return p;
}

struct PhaseUpdateProof {
    std::size_t dimension = 0;
    std::size_t remaining = 0;
    std::size_t target_remaining = 0;
    std::size_t source_phase = 0;
    std::size_t target_phase = 0;
    bool exact_decrement = false;
    bool modular_update = false;
    bool valid = false;
};

inline PhaseUpdateProof derive_phase_update_proof(std::size_t dimension,
                                                   std::size_t remaining) {
    PhaseUpdateProof p;
    p.dimension = dimension;
    p.remaining = remaining;
    if (dimension == 0 || remaining == 0) return p;
    p.target_remaining = remaining - 1;
    p.source_phase = remaining % (dimension + 1);
    p.target_phase = p.target_remaining % (dimension + 1);
    p.exact_decrement = p.target_remaining + 1 == remaining;
    p.modular_update = p.target_phase ==
        (p.source_phase + dimension) % (dimension + 1);
    p.valid = p.exact_decrement && p.modular_update;
    return p;
}

inline std::string render_parametric_update_explanation() {
    std::ostringstream o;
    o << "PARAMETRIC_PREDICATE_UPDATE_ALGEBRA\n";
    o << "controller_map=T_t(x0,...,x[n-1])=(x1,...,x[n-1],t)\n";
    o << "symbol_count'=symbol_count-[x0=a]+[t=a]\n";
    o << "changes'=changes-[x0!=x1]+[x[n-1]!=t]\n";
    o << "factor_count_w'=factor_count_w-[prefix=w]+[new_suffix=w]\n";
    o << "period_defects_p'=period_defects_p-[x[p]!=x[0]]+[t!=x[n-p]]\n";
    o << "phase'=(phase-1) mod (n+1)\n";
    o << "explanation=all selected word predicates are finite-window observables of a shift register; their updates are discrete boundary-flux laws\n";
    return o.str();
}

} // namespace ravel::proof
