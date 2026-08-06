#pragma once

#include <algorithm>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

#include "math/ball.hpp"
#include "ravel/proof/paired_matrix_dominance.hpp"

namespace ravel::proof {

struct PhaseEnvelope {
    std::size_t phase_count = 0;
    std::vector<std::vector<long long>> competitor_upper;
    std::vector<std::vector<long long>> core_incoming_lower;
    std::vector<bool> competitor_active;
    std::vector<bool> core_active;
    bool replayed = false;
};

struct PhaseRenewalProof {
    PhaseEnvelope envelope;
    std::size_t block_length = 0;
    std::vector<mathlib::BigInt> competitor_phase_walks;
    std::vector<mathlib::BigInt> core_phase_walks;
    mathlib::BigInt competitor_max;
    mathlib::BigInt core_min;
    bool strict_loss = false;
    bool proved = false;
    std::string obstruction;
};

inline PhaseEnvelope derive_phase_envelope(
    const std::vector<std::vector<long long>>& competitor,
    const std::vector<std::size_t>& competitor_phase,
    const std::vector<std::vector<long long>>& core,
    const std::vector<std::size_t>& core_phase,
    std::size_t phase_count) {
    if (competitor.size() != competitor_phase.size() ||
        core.size() != core_phase.size() || phase_count == 0)
        throw std::invalid_argument("phase renewal: malformed phase data");
    for (const auto& row : competitor)
        if (row.size() != competitor.size())
            throw std::invalid_argument("phase renewal: competitor not square");
    for (const auto& row : core)
        if (row.size() != core.size())
            throw std::invalid_argument("phase renewal: core not square");

    PhaseEnvelope out;
    out.phase_count = phase_count;
    out.competitor_upper.assign(phase_count,
        std::vector<long long>(phase_count, 0));
    out.core_incoming_lower.assign(phase_count,
        std::vector<long long>(phase_count, 0));
    out.competitor_active.assign(phase_count, false);
    out.core_active.assign(phase_count, false);
    for (const auto p : competitor_phase) out.competitor_active[p] = true;
    for (const auto p : core_phase) out.core_active[p] = true;

    // Upper envelope for outgoing competitor walks.
    for (std::size_t source = 0; source < competitor.size(); ++source) {
        const auto p = competitor_phase[source];
        std::vector<long long> counts(phase_count, 0);
        for (std::size_t target = 0; target < competitor.size(); ++target)
            counts[competitor_phase[target]] += competitor[source][target];
        for (std::size_t q = 0; q < phase_count; ++q)
            out.competitor_upper[p][q] =
                std::max(out.competitor_upper[p][q], counts[q]);
    }

    // Lower envelope for incoming core walks.  For each target state in phase p,
    // count incoming edges from each predecessor phase q, then retain the minimum.
    std::vector<bool> seen_phase(phase_count, false);
    for (const auto p : core_phase) seen_phase[p] = true;
    for (std::size_t p = 0; p < phase_count; ++p) {
        if (!seen_phase[p]) continue;
        std::vector<long long> minima(phase_count,
            std::numeric_limits<long long>::max());
        for (std::size_t target = 0; target < core.size(); ++target) {
            if (core_phase[target] != p) continue;
            std::vector<long long> counts(phase_count, 0);
            for (std::size_t source = 0; source < core.size(); ++source)
                counts[core_phase[source]] += core[source][target];
            for (std::size_t q = 0; q < phase_count; ++q)
                minima[q] = std::min(minima[q], counts[q]);
        }
        for (std::size_t q = 0; q < phase_count; ++q)
            out.core_incoming_lower[p][q] =
                minima[q] == std::numeric_limits<long long>::max() ? 0 : minima[q];
    }

    // Replay the one-step envelope inequalities exactly.
    bool ok = true;
    for (std::size_t source = 0; source < competitor.size(); ++source) {
        std::vector<long long> counts(phase_count, 0);
        for (std::size_t target = 0; target < competitor.size(); ++target)
            counts[competitor_phase[target]] += competitor[source][target];
        for (std::size_t q = 0; q < phase_count; ++q)
            ok = ok && counts[q] <=
                out.competitor_upper[competitor_phase[source]][q];
    }
    for (std::size_t target = 0; target < core.size(); ++target) {
        std::vector<long long> counts(phase_count, 0);
        for (std::size_t source = 0; source < core.size(); ++source)
            counts[core_phase[source]] += core[source][target];
        for (std::size_t q = 0; q < phase_count; ++q)
            ok = ok && counts[q] >=
                out.core_incoming_lower[core_phase[target]][q];
    }
    out.replayed = ok;
    return out;
}

inline PhaseRenewalProof derive_phase_handoff_renewal(
    const std::vector<std::vector<long long>>& competitor,
    const std::vector<std::size_t>& competitor_phase,
    const std::vector<std::vector<long long>>& core,
    const std::vector<std::size_t>& core_phase,
    std::size_t phase_count,
    std::size_t max_block = 128) {
    PhaseRenewalProof out;
    out.envelope = derive_phase_envelope(
        competitor, competitor_phase, core, core_phase, phase_count);
    if (!out.envelope.replayed) {
        out.obstruction = "phase envelope replay failed";
        return out;
    }
    std::vector<mathlib::BigInt> upper(phase_count, mathlib::BigInt(1));
    std::vector<mathlib::BigInt> lower(phase_count, mathlib::BigInt(1));
    for (std::size_t block = 1; block <= max_block; ++block) {
        upper = exact_matrix_vector_product(out.envelope.competitor_upper, upper);
        lower = exact_matrix_vector_product(out.envelope.core_incoming_lower, lower);
        bool have_upper = false, have_lower = false;
        mathlib::BigInt umax, lmin;
        for (std::size_t p = 0; p < phase_count; ++p) {
            if (out.envelope.competitor_active[p]) {
                if (!have_upper || mathlib::cmp(upper[p], umax) > 0)
                    mathlib::set(umax, upper[p]);
                have_upper = true;
            }
            if (out.envelope.core_active[p]) {
                if (!have_lower || mathlib::cmp(lower[p], lmin) < 0)
                    mathlib::set(lmin, lower[p]);
                have_lower = true;
            }
        }
        if (!have_upper || !have_lower) {
            out.obstruction = "no active competitor/core phases";
            return out;
        }
        if (mathlib::cmp(umax, lmin) < 0) {
            out.block_length = block;
            out.competitor_phase_walks = upper;
            out.core_phase_walks = lower;
            mathlib::set(out.competitor_max, umax);
            mathlib::set(out.core_min, lmin);
            out.strict_loss = true;
            out.proved = true;
            return out;
        }
    }
    out.obstruction = "face-phase envelope did not separate within block bound";
    return out;
}


struct PhaseBellmanGrammar {
    std::size_t phase_count = 0;
    std::vector<std::vector<std::vector<long long>>> competitor_profiles;
    std::vector<std::vector<std::vector<long long>>> core_incoming_profiles;
    std::vector<bool> competitor_active;
    std::vector<bool> core_active;
    bool replayed = false;
};

struct PhaseBellmanRenewalProof {
    PhaseBellmanGrammar grammar;
    std::size_t block_length = 0;
    std::vector<mathlib::BigInt> competitor_upper;
    std::vector<mathlib::BigInt> core_lower;
    mathlib::BigInt competitor_max;
    mathlib::BigInt core_min;
    bool strict_loss = false;
    bool proved = false;
    std::string obstruction;
};

inline void deduplicate_profiles(
    std::vector<std::vector<std::vector<long long>>>& profiles) {
    for (auto& family : profiles) {
        std::sort(family.begin(), family.end());
        family.erase(std::unique(family.begin(), family.end()), family.end());
    }
}

inline PhaseBellmanGrammar derive_phase_bellman_grammar(
    const std::vector<std::vector<long long>>& competitor,
    const std::vector<std::size_t>& competitor_phase,
    const std::vector<std::vector<long long>>& core,
    const std::vector<std::size_t>& core_phase,
    std::size_t phase_count) {
    PhaseBellmanGrammar out;
    out.phase_count = phase_count;
    out.competitor_profiles.resize(phase_count);
    out.core_incoming_profiles.resize(phase_count);
    out.competitor_active.assign(phase_count, false);
    out.core_active.assign(phase_count, false);

    for (std::size_t source = 0; source < competitor.size(); ++source) {
        const auto p = competitor_phase[source];
        out.competitor_active[p] = true;
        std::vector<long long> counts(phase_count, 0);
        for (std::size_t target = 0; target < competitor.size(); ++target)
            counts[competitor_phase[target]] += competitor[source][target];
        out.competitor_profiles[p].push_back(std::move(counts));
    }
    for (std::size_t target = 0; target < core.size(); ++target) {
        const auto p = core_phase[target];
        out.core_active[p] = true;
        std::vector<long long> counts(phase_count, 0);
        for (std::size_t source = 0; source < core.size(); ++source)
            counts[core_phase[source]] += core[source][target];
        out.core_incoming_profiles[p].push_back(std::move(counts));
    }
    deduplicate_profiles(out.competitor_profiles);
    deduplicate_profiles(out.core_incoming_profiles);
    out.replayed = true;
    return out;
}

inline mathlib::BigInt profile_dot(
    const std::vector<long long>& profile,
    const std::vector<mathlib::BigInt>& values) {
    mathlib::BigInt sum(0);
    for (std::size_t q = 0; q < profile.size(); ++q) {
        if (profile[q] == 0) continue;
        mathlib::BigInt term;
        mathlib::mul_si(term, values[q], profile[q]);
        mathlib::add(sum, sum, term);
    }
    return sum;
}

inline PhaseBellmanRenewalProof derive_phase_handoff_bellman_renewal(
    const std::vector<std::vector<long long>>& competitor,
    const std::vector<std::size_t>& competitor_phase,
    const std::vector<std::vector<long long>>& core,
    const std::vector<std::size_t>& core_phase,
    std::size_t phase_count,
    std::size_t max_block = 128) {
    PhaseBellmanRenewalProof out;
    out.grammar = derive_phase_bellman_grammar(
        competitor, competitor_phase, core, core_phase, phase_count);
    std::vector<mathlib::BigInt> upper(phase_count, mathlib::BigInt(1));
    std::vector<mathlib::BigInt> lower(phase_count, mathlib::BigInt(1));
    for (std::size_t block = 1; block <= max_block; ++block) {
        std::vector<mathlib::BigInt> next_upper(phase_count, mathlib::BigInt(0));
        std::vector<mathlib::BigInt> next_lower(phase_count, mathlib::BigInt(0));
        for (std::size_t p = 0; p < phase_count; ++p) {
            if (out.grammar.competitor_active[p]) {
                bool first = true;
                for (const auto& profile : out.grammar.competitor_profiles[p]) {
                    const auto value = profile_dot(profile, upper);
                    if (first || mathlib::cmp(value, next_upper[p]) > 0)
                        mathlib::set(next_upper[p], value);
                    first = false;
                }
            }
            if (out.grammar.core_active[p]) {
                bool first = true;
                for (const auto& profile : out.grammar.core_incoming_profiles[p]) {
                    const auto value = profile_dot(profile, lower);
                    if (first || mathlib::cmp(value, next_lower[p]) < 0)
                        mathlib::set(next_lower[p], value);
                    first = false;
                }
            }
        }
        upper = std::move(next_upper);
        lower = std::move(next_lower);
        bool have_upper = false, have_lower = false;
        mathlib::BigInt umax, lmin;
        for (std::size_t p = 0; p < phase_count; ++p) {
            if (out.grammar.competitor_active[p]) {
                if (!have_upper || mathlib::cmp(upper[p], umax) > 0)
                    mathlib::set(umax, upper[p]);
                have_upper = true;
            }
            if (out.grammar.core_active[p]) {
                if (!have_lower || mathlib::cmp(lower[p], lmin) < 0)
                    mathlib::set(lmin, lower[p]);
                have_lower = true;
            }
        }
        if (have_upper && have_lower && mathlib::cmp(umax, lmin) < 0) {
            out.block_length = block;
            out.competitor_upper = upper;
            out.core_lower = lower;
            mathlib::set(out.competitor_max, umax);
            mathlib::set(out.core_min, lmin);
            out.strict_loss = true;
            out.proved = true;
            return out;
        }
    }
    out.obstruction = "relational face-phase Bellman grammar did not separate";
    return out;
}

inline std::string render_phase_handoff_renewal_lean() {
    return R"LEAN(import Mathlib

namespace RavelPlayground

/-- Once a concrete system and its face-phase abstraction provide pointwise
    upper and lower block-walk bounds, strict phase separation is inherited by
    every concrete competitor/core state pair. -/
theorem concrete_strict_of_phase_bounds
    {S T P : Type}
    (sourcePhase : S → P) (targetPhase : T → P)
    (competitorWalks : S → ℕ) (coreIncomingWalks : T → ℕ)
    (upper lower : P → ℕ)
    (hupper : ∀ s, competitorWalks s ≤ upper (sourcePhase s))
    (hlower : ∀ t, lower (targetPhase t) ≤ coreIncomingWalks t)
    (hstrict : ∀ p q, upper p < lower q) :
    ∀ s t, competitorWalks s < coreIncomingWalks t := by
  intro s t
  exact lt_of_le_of_lt (hupper s)
    (lt_of_lt_of_le (hstrict (sourcePhase s) (targetPhase t)) (hlower t))

/-- A strict block walk deficit is the exact combinatorial input needed by the
    paired-cone construction. -/
structure PhaseRenewalData (P : Type) where
  block : ℕ
  upper : P → ℕ
  lower : P → ℕ
  strict : ∀ p q, upper p < lower q

end RavelPlayground
)LEAN";
}

} // namespace ravel::proof
