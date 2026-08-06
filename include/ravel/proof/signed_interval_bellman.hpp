#pragma once

#include <algorithm>
#include <cstddef>
#include <map>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "ravel/proof/face_relative_sign_grammar.hpp"
#include "ravel/proof/phase_handoff_renewal.hpp"

namespace ravel::proof {

// Dimension-parametric role: the active face has already been rotated to zero,
// so neither its absolute index nor a coordinate-indexed sign word remains.
struct SignedIntervalRole {
    std::size_t target_offset = 0;
    std::size_t support = 0;
    std::vector<CyclicSignRun> runs;
    bool sign_canonical = false;

    friend bool operator==(const SignedIntervalRole&, const SignedIntervalRole&) = default;
    friend bool operator<(const SignedIntervalRole& a, const SignedIntervalRole& b) {
        return std::tie(a.target_offset, a.support, a.runs, a.sign_canonical) <
               std::tie(b.target_offset, b.support, b.runs, b.sign_canonical);
    }
};

inline SignedIntervalRole forget_absolute_face(const FaceRelativeSignGrammar& g) {
    if (!g.replayed) throw std::invalid_argument("interval role: source grammar not replayed");
    return {g.target_face_offset, g.support, g.runs, g.sign_canonical};
}


struct PhaseIndexedIntervalRole {
    std::size_t dimension = 0;
    std::size_t active_phase = 0;
    SignedIntervalRole interval;

    friend bool operator==(const PhaseIndexedIntervalRole&, const PhaseIndexedIntervalRole&) = default;
    friend bool operator<(const PhaseIndexedIntervalRole& a, const PhaseIndexedIntervalRole& b) {
        return std::tie(a.dimension, a.active_phase, a.interval) <
               std::tie(b.dimension, b.active_phase, b.interval);
    }
};

inline PhaseIndexedIntervalRole derive_phase_indexed_interval_role(
    const FaceRelativeSignGrammar& g) {
    if (!g.replayed) throw std::invalid_argument("phase interval role: source grammar not replayed");
    return {g.dimension, g.active_face, forget_absolute_face(g)};
}


inline std::size_t interval_dimension(const SignedIntervalRole& role) {
    std::size_t n = 0;
    for (const auto& r : role.runs) n += r.length;
    return n;
}

inline SignedIntervalRole rotate_interval_role(
    const SignedIntervalRole& role,
    std::size_t face_advance,
    std::size_t new_target_offset) {
    auto word = run_decode(role.runs);
    if (word.empty()) throw std::invalid_argument("interval role: empty word");
    face_advance %= word.size();
    std::rotate(word.begin(), word.begin() + static_cast<std::ptrdiff_t>(face_advance), word.end());
    SignedIntervalRole out;
    out.target_offset = new_target_offset % word.size();
    out.support = static_cast<std::size_t>(std::count_if(
        word.begin(), word.end(), [](int s) { return s != 0; }));
    out.runs = run_encode(word);
    out.sign_canonical = role.sign_canonical;
    if (out.sign_canonical) {
        const auto neg = negate_runs(out.runs);
        if (neg < out.runs) out.runs = neg;
    }
    return out;
}

// Exact local shift/append update used by the carry-side interval grammar.
inline PhaseIndexedIntervalRole advance_phase_indexed_interval_role(
    const PhaseIndexedIntervalRole& role,
    std::size_t new_active_phase,
    std::size_t face_advance,
    std::size_t new_target_offset) {
    if (role.dimension == 0 || new_active_phase >= role.dimension)
        throw std::invalid_argument("phase interval role: malformed phase");
    return {role.dimension, new_active_phase,
            rotate_interval_role(role.interval, face_advance, new_target_offset)};
}

inline SignedIntervalRole shift_append_interval_role(
    const SignedIntervalRole& role,
    int incoming_sign,
    std::size_t new_target_offset) {
    if (incoming_sign < -1 || incoming_sign > 1)
        throw std::invalid_argument("interval role: bad incoming sign");
    auto word = run_decode(role.runs);
    if (word.empty()) throw std::invalid_argument("interval role: empty word");
    word.erase(word.begin());
    word.push_back(incoming_sign);
    SignedIntervalRole out;
    out.target_offset = new_target_offset % word.size();
    out.support = static_cast<std::size_t>(std::count_if(
        word.begin(), word.end(), [](int s) { return s != 0; }));
    out.runs = run_encode(word);
    out.sign_canonical = role.sign_canonical;
    if (out.sign_canonical) {
        const auto neg = negate_runs(out.runs);
        if (neg < out.runs) out.runs = neg;
    }
    return out;
}

struct IntervalBellmanSystem {
    std::vector<SignedIntervalRole> roles;
    PhaseBellmanGrammar grammar;
    bool absolute_face_free = false;
    bool replayed = false;
};

inline IntervalBellmanSystem derive_signed_interval_bellman_system(
    const std::vector<std::vector<long long>>& competitor,
    const std::vector<SignedIntervalRole>& competitor_roles,
    const std::vector<std::vector<long long>>& core,
    const std::vector<SignedIntervalRole>& core_roles) {
    if (competitor.size() != competitor_roles.size() || core.size() != core_roles.size())
        throw std::invalid_argument("interval Bellman: role size mismatch");
    std::map<SignedIntervalRole, std::size_t> ids;
    auto id = [&](const SignedIntervalRole& role) {
        const auto [it, inserted] = ids.emplace(role, ids.size());
        (void)inserted;
        return it->second;
    };
    std::vector<std::size_t> cr, br;
    cr.reserve(competitor_roles.size());
    br.reserve(core_roles.size());
    for (const auto& r : competitor_roles) cr.push_back(id(r));
    for (const auto& r : core_roles) br.push_back(id(r));
    IntervalBellmanSystem out;
    out.roles.resize(ids.size());
    for (const auto& [role, idx] : ids) out.roles[idx] = role;
    out.grammar = derive_phase_bellman_grammar(
        competitor, cr, core, br, out.roles.size());
    out.absolute_face_free = true;
    out.replayed = out.grammar.replayed;
    return out;
}

struct AffineRenewalLaw {
    long long slope = 0;
    long long intercept = 0;
    std::vector<std::pair<std::size_t, std::size_t>> observations;
    bool exact_on_observations = false;

    std::size_t at(std::size_t n) const {
        const long long value = slope * static_cast<long long>(n) + intercept;
        if (value < 0) throw std::domain_error("renewal law: negative block");
        return static_cast<std::size_t>(value);
    }
};

// Two observations determine the unique affine candidate.  This is theorem
// discovery only; exact_on_observations does not claim the all-n law.
inline AffineRenewalLaw derive_affine_renewal_candidate(
    std::pair<std::size_t, std::size_t> a,
    std::pair<std::size_t, std::size_t> b) {
    if (a.first == b.first) throw std::invalid_argument("renewal law: duplicate dimension");
    const long long dn = static_cast<long long>(b.first) - static_cast<long long>(a.first);
    const long long dl = static_cast<long long>(b.second) - static_cast<long long>(a.second);
    if (dl % dn != 0) throw std::invalid_argument("renewal law: nonintegral affine slope");
    AffineRenewalLaw out;
    out.slope = dl / dn;
    out.intercept = static_cast<long long>(a.second) - out.slope * static_cast<long long>(a.first);
    out.observations = {a, b};
    out.exact_on_observations = out.at(a.first) == a.second && out.at(b.first) == b.second;
    return out;
}

inline std::string render_signed_interval_bellman_lean() {
    return R"LEAN(import Mathlib

namespace RavelPlayground

structure SignRun where
  sign : Int
  length : Nat
  positive : 0 < length

structure SignedIntervalRole where
  targetOffset : Nat
  support : Nat
  runs : List SignRun

/-- Bellman propagation is performed on complete branch profiles, so it keeps
    correlations that are lost by coordinatewise matrix envelopes. -/
def bellmanUpper {R : Type} [Fintype R] [DecidableEq R]
    (profiles : R → Finset (R → Nat)) (u : R → Nat) (r : R) : Nat :=
  (profiles r).sup (fun p => ∑ q, p q * u q)

def bellmanLower {R : Type} [Fintype R] [DecidableEq R]
    (profiles : R → Finset (R → Nat)) (u : R → Nat) (r : R) : Nat :=
  (profiles r).inf' (by simp) (fun p => ∑ q, p q * u q)

/-- A strict interval-role renewal deficit lifts to every represented concrete
    competitor/core state. -/
theorem strict_of_interval_renewal
    {S T R : Type}
    (sr : S → R) (tr : T → R)
    (cw : S → Nat) (kw : T → Nat)
    (upper lower : R → Nat)
    (hu : ∀ s, cw s ≤ upper (sr s))
    (hl : ∀ t, lower (tr t) ≤ kw t)
    (hs : ∀ r q, upper r < lower q) :
    ∀ s t, cw s < kw t := by
  intro s t
  exact lt_of_le_of_lt (hu s) (lt_of_lt_of_le (hs (sr s) (tr t)) (hl t))

/-- The observed blocks at dimensions four and five determine the affine
    discovery candidate L(n)=3*n-7.  A concrete universal proof must still
    establish the renewal inequality at this block for arbitrary n. -/
theorem affine_candidate_4_5 :
    3 * 4 - 7 = 5 ∧ 3 * 5 - 7 = 8 := by decide

end RavelPlayground
)LEAN";
}

} // namespace ravel::proof
