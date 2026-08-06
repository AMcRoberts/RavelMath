#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <limits>
#include <set>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "ravel/proof/cyclic_splice_compactness.hpp"
#include "ravel/proof/symbolic_radius_one_controller.hpp"

namespace ravel::proof {

struct PeriodicWordControllerCycleProof {
    std::size_t dimension = 0;
    std::vector<std::int64_t> word;
    std::vector<SymbolicControllerState> states;
    OneLapControllerRelation one_lap;
    CyclicOneLapRelationProof cycle;
    bool replayed = false;
    std::string failure;
};

inline std::vector<SymbolicControllerState>
nonzero_radius_one_controller_states(std::size_t n) {
    if (n == 0) throw std::invalid_argument("controller dimension must be positive");
    std::vector<SymbolicControllerState> result;
    std::size_t count = 1;
    for (std::size_t i = 0; i < n; ++i) count *= 3;
    result.reserve(count - 1);
    for (std::size_t code = 0; code < count; ++code) {
        std::size_t q = code;
        SymbolicControllerState state(n, 0);
        bool nonzero = false;
        for (std::size_t i = 0; i < n; ++i) {
            state[i] = static_cast<std::int64_t>(q % 3) - 1;
            q /= 3;
            nonzero |= state[i] != 0;
        }
        if (nonzero) result.push_back(std::move(state));
    }
    return result;
}

inline PeriodicWordControllerCycleProof prove_periodic_word_controller_cycle(
    std::size_t dimension,
    const std::vector<std::int64_t>& word) {
    PeriodicWordControllerCycleProof proof;
    proof.dimension = dimension;
    proof.word = word;
    if (dimension == 0 || word.empty()) {
        proof.failure = "dimension and word must be nonempty";
        return proof;
    }
    for (const auto d : word) {
        if (d < -1 || d > 1) {
            proof.failure = "base word is not ternary";
            return proof;
        }
    }

    proof.states = nonzero_radius_one_controller_states(dimension);
    std::map<SymbolicControllerState, std::size_t> index;
    for (std::size_t i = 0; i < proof.states.size(); ++i)
        index.emplace(proof.states[i], i);

    proof.one_lap.state_count = proof.states.size();
    proof.one_lap.successors.resize(proof.states.size());
    for (std::size_t i = 0; i < proof.states.size(); ++i) {
        std::set<SymbolicControllerState> frontier{proof.states[i]};
        for (const auto digit : word) {
            std::set<SymbolicControllerState> next;
            for (const auto& state : frontier) {
                const auto successors = symbolic_successors(state, digit);
                next.insert(successors.begin(), successors.end());
            }
            frontier = std::move(next);
            if (frontier.empty()) break;
        }
        for (const auto& target : frontier) {
            const auto found = index.find(target);
            if (found == index.end()) {
                proof.failure = "word transition escaped the radius-one boundary state space";
                return proof;
            }
            proof.one_lap.successors[i].push_back(found->second);
        }
    }

    proof.cycle = find_cyclic_one_lap_orbit(proof.one_lap);
    proof.replayed = proof.cycle.valid;
    if (!proof.replayed) proof.failure = proof.cycle.failure;
    return proof;
}

struct PeriodicWordCycleAudit {
    std::size_t dimension = 0;
    std::size_t max_length = 0;
    std::size_t words_checked = 0;
    std::size_t words_with_cycle = 0;
    std::size_t maximum_period_laps = 0;
    std::vector<std::int64_t> first_counterexample;
    bool all_passed = false;
};

inline PeriodicWordCycleAudit audit_periodic_word_controller_cycles(
    std::size_t dimension,
    std::size_t max_length) {
    PeriodicWordCycleAudit audit;
    audit.dimension = dimension;
    audit.max_length = max_length;
    std::vector<std::int64_t> word;
    const auto visit = [&](auto&& self, std::size_t remaining) -> bool {
        if (remaining == 0) {
            ++audit.words_checked;
            const auto proof = prove_periodic_word_controller_cycle(dimension, word);
            if (!proof.cycle.valid) {
                audit.first_counterexample = word;
                return false;
            }
            ++audit.words_with_cycle;
            audit.maximum_period_laps = std::max(
                audit.maximum_period_laps,
                proof.cycle.closed_orbit.empty()
                    ? std::size_t{0}
                    : proof.cycle.closed_orbit.size() - 1);
            return true;
        }
        for (const auto digit : {-1LL, 0LL, 1LL}) {
            word.push_back(digit);
            if (!self(self, remaining - 1)) return false;
            word.pop_back();
        }
        return true;
    };
    for (std::size_t length = 1; length <= max_length; ++length) {
        word.clear();
        if (!visit(visit, length)) break;
    }
    audit.all_passed = audit.first_counterexample.empty() &&
        audit.words_checked == audit.words_with_cycle;
    return audit;
}

inline std::string render_periodic_word_cycle_audit(
    const PeriodicWordCycleAudit& audit) {
    std::string counterexample;
    for (std::size_t i = 0; i < audit.first_counterexample.size(); ++i) {
        if (i) counterexample += ',';
        counterexample += std::to_string(audit.first_counterexample[i]);
    }
    return "PERIODIC_WORD_CONTROLLER_CYCLE_AUDIT\n"
        "dimension=" + std::to_string(audit.dimension) + "\n"
        "max_length=" + std::to_string(audit.max_length) + "\n"
        "words_checked=" + std::to_string(audit.words_checked) + "\n"
        "words_with_cycle=" + std::to_string(audit.words_with_cycle) + "\n"
        "maximum_period_laps=" + std::to_string(audit.maximum_period_laps) + "\n"
        "all_passed=" + std::string(audit.all_passed ? "true" : "false") + "\n"
        "first_counterexample=" + counterexample + "\n";
}

} // namespace ravel::proof
