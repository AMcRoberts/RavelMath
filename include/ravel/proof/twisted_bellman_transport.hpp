#pragma once

#include <algorithm>
#include <cstddef>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "ravel/proof/phase_handoff_renewal.hpp"

namespace ravel::proof {

struct TwistedBellmanTransportProof {
    std::size_t lower_roles = 0;
    std::size_t upper_roles = 0;
    std::size_t competitor_profiles_checked = 0;
    std::size_t core_profiles_checked = 0;
    std::size_t renewal_block = 0;
    bool role_embedding_injective = false;
    bool sheet_correction_total = false;
    bool competitor_profiles_preserved = false;
    bool core_profiles_preserved = false;
    bool lower_renewal_proved = false;
    bool upper_embedded_renewal_proved = false;
    bool proved = false;
    std::string obstruction;
};

namespace twisted_bellman_detail {

inline bool injective_map(const std::vector<std::size_t>& embedding,
                          std::size_t upper_count) {
    std::vector<bool> seen(upper_count, false);
    for (const auto q : embedding) {
        if (q >= upper_count || seen[q]) return false;
        seen[q] = true;
    }
    return true;
}

inline std::vector<long long> relabel_profile(
    const std::vector<long long>& profile,
    const std::vector<std::size_t>& embedding,
    std::size_t upper_count) {
    if (profile.size() != embedding.size())
        throw std::invalid_argument("twisted Bellman: malformed lower profile");
    std::vector<long long> out(upper_count, 0);
    for (std::size_t q = 0; q < profile.size(); ++q)
        out[embedding[q]] += profile[q];
    return out;
}

inline bool profile_family_equal_after_embedding(
    const std::vector<std::vector<std::vector<long long>>>& lower,
    const std::vector<std::vector<std::vector<long long>>>& upper,
    const std::vector<std::size_t>& embedding,
    std::size_t& checked) {
    if (lower.size() != embedding.size()) return false;
    for (std::size_t q = 0; q < lower.size(); ++q) {
        const auto uq = embedding[q];
        if (uq >= upper.size()) return false;
        std::vector<std::vector<long long>> expected;
        expected.reserve(lower[q].size());
        for (const auto& p : lower[q]) {
            expected.push_back(relabel_profile(p, embedding, upper.size()));
            ++checked;
        }
        auto actual = upper[uq];
        std::sort(expected.begin(), expected.end());
        expected.erase(std::unique(expected.begin(), expected.end()), expected.end());
        std::sort(actual.begin(), actual.end());
        actual.erase(std::unique(actual.begin(), actual.end()), actual.end());
        if (expected != actual) return false;
    }
    return true;
}

inline std::vector<mathlib::BigInt> embed_values(
    const std::vector<mathlib::BigInt>& lower,
    const std::vector<std::size_t>& embedding,
    std::size_t upper_count) {
    std::vector<mathlib::BigInt> out(upper_count, mathlib::BigInt(0));
    for (std::size_t q = 0; q < lower.size(); ++q)
        mathlib::set(out[embedding[q]], lower[q]);
    return out;
}

inline bool embedded_values_equal(
    const std::vector<mathlib::BigInt>& lower,
    const std::vector<mathlib::BigInt>& upper,
    const std::vector<std::size_t>& embedding) {
    if (lower.size() != embedding.size()) return false;
    for (std::size_t q = 0; q < lower.size(); ++q)
        if (mathlib::cmp(lower[q], upper[embedding[q]]) != 0) return false;
    return true;
}

} // namespace twisted_bellman_detail

// Structural theorem operation.  A twisted n->n+1 path substitution is used
// only to construct macro-edges between embedded roles.  Once complete branch
// profiles are preserved under the role embedding, Bellman iteration commutes
// with that embedding.  The Z/2 sheet correction is recorded separately: it
// changes representatives but not branch multiplicities.
inline TwistedBellmanTransportProof derive_twisted_bellman_transport(
    const PhaseBellmanGrammar& lower_competitor,
    const PhaseBellmanGrammar& lower_core,
    const PhaseBellmanGrammar& upper_macro_competitor,
    const PhaseBellmanGrammar& upper_macro_core,
    const std::vector<std::size_t>& competitor_role_embedding,
    const std::vector<std::size_t>& core_role_embedding,
    const std::vector<int>& competitor_sheet_correction,
    const std::vector<int>& core_sheet_correction,
    std::size_t max_block = 128) {
    TwistedBellmanTransportProof out;
    out.lower_roles = lower_competitor.phase_count;
    out.upper_roles = upper_macro_competitor.phase_count;

    if (lower_competitor.phase_count != competitor_role_embedding.size() ||
        lower_core.phase_count != core_role_embedding.size() ||
        competitor_sheet_correction.size() != competitor_role_embedding.size() ||
        core_sheet_correction.size() != core_role_embedding.size()) {
        out.obstruction = "role embedding or sheet correction has wrong size";
        return out;
    }
    out.role_embedding_injective =
        twisted_bellman_detail::injective_map(
            competitor_role_embedding, upper_macro_competitor.phase_count) &&
        twisted_bellman_detail::injective_map(
            core_role_embedding, upper_macro_core.phase_count);
    out.sheet_correction_total = true;
    for (const int e : competitor_sheet_correction)
        out.sheet_correction_total = out.sheet_correction_total && (e == 0 || e == 1);
    for (const int e : core_sheet_correction)
        out.sheet_correction_total = out.sheet_correction_total && (e == 0 || e == 1);
    if (!out.role_embedding_injective || !out.sheet_correction_total) {
        out.obstruction = "twisted role embedding is not an injective total skew map";
        return out;
    }

    out.competitor_profiles_preserved =
        twisted_bellman_detail::profile_family_equal_after_embedding(
            lower_competitor.competitor_profiles,
            upper_macro_competitor.competitor_profiles,
            competitor_role_embedding,
            out.competitor_profiles_checked);
    out.core_profiles_preserved =
        twisted_bellman_detail::profile_family_equal_after_embedding(
            lower_core.core_incoming_profiles,
            upper_macro_core.core_incoming_profiles,
            core_role_embedding,
            out.core_profiles_checked);
    if (!out.competitor_profiles_preserved || !out.core_profiles_preserved) {
        out.obstruction = "macro-edge branch profiles are not preserved by twisted embedding";
        return out;
    }

    // Reconstruct matrix-free lower and upper Bellman iterations directly from
    // profile families.  Initial value one is used on active embedded roles.
    std::vector<mathlib::BigInt> lc(lower_competitor.phase_count, mathlib::BigInt(1));
    std::vector<mathlib::BigInt> lk(lower_core.phase_count, mathlib::BigInt(1));
    std::vector<mathlib::BigInt> uc(upper_macro_competitor.phase_count, mathlib::BigInt(0));
    std::vector<mathlib::BigInt> uk(upper_macro_core.phase_count, mathlib::BigInt(0));
    for (const auto q : competitor_role_embedding) mathlib::set_si(uc[q], 1);
    for (const auto q : core_role_embedding) mathlib::set_si(uk[q], 1);

    for (std::size_t block = 1; block <= max_block; ++block) {
        std::vector<mathlib::BigInt> lc_next(lc.size(), mathlib::BigInt(0));
        std::vector<mathlib::BigInt> lk_next(lk.size(), mathlib::BigInt(0));
        std::vector<mathlib::BigInt> uc_next(uc.size(), mathlib::BigInt(0));
        std::vector<mathlib::BigInt> uk_next(uk.size(), mathlib::BigInt(0));

        for (std::size_t r = 0; r < lc.size(); ++r) {
            bool first = true;
            for (const auto& p : lower_competitor.competitor_profiles[r]) {
                const auto value = profile_dot(p, lc);
                if (first || mathlib::cmp(value, lc_next[r]) > 0)
                    mathlib::set(lc_next[r], value);
                first = false;
            }
        }
        for (std::size_t r = 0; r < lk.size(); ++r) {
            bool first = true;
            for (const auto& p : lower_core.core_incoming_profiles[r]) {
                const auto value = profile_dot(p, lk);
                if (first || mathlib::cmp(value, lk_next[r]) < 0)
                    mathlib::set(lk_next[r], value);
                first = false;
            }
        }
        for (std::size_t r = 0; r < uc.size(); ++r) {
            bool first = true;
            for (const auto& p : upper_macro_competitor.competitor_profiles[r]) {
                const auto value = profile_dot(p, uc);
                if (first || mathlib::cmp(value, uc_next[r]) > 0)
                    mathlib::set(uc_next[r], value);
                first = false;
            }
        }
        for (std::size_t r = 0; r < uk.size(); ++r) {
            bool first = true;
            for (const auto& p : upper_macro_core.core_incoming_profiles[r]) {
                const auto value = profile_dot(p, uk);
                if (first || mathlib::cmp(value, uk_next[r]) < 0)
                    mathlib::set(uk_next[r], value);
                first = false;
            }
        }
        lc = std::move(lc_next); lk = std::move(lk_next);
        uc = std::move(uc_next); uk = std::move(uk_next);

        if (!twisted_bellman_detail::embedded_values_equal(
                lc, uc, competitor_role_embedding) ||
            !twisted_bellman_detail::embedded_values_equal(
                lk, uk, core_role_embedding)) {
            out.obstruction = "Bellman iteration failed to commute with twisted embedding";
            return out;
        }

        bool have_c = false, have_k = false;
        mathlib::BigInt cmax, kmin;
        for (std::size_t r = 0; r < lc.size(); ++r) {
            if (!lower_competitor.competitor_active[r]) continue;
            if (!have_c || mathlib::cmp(lc[r], cmax) > 0) mathlib::set(cmax, lc[r]);
            have_c = true;
        }
        for (std::size_t r = 0; r < lk.size(); ++r) {
            if (!lower_core.core_active[r]) continue;
            if (!have_k || mathlib::cmp(lk[r], kmin) < 0) mathlib::set(kmin, lk[r]);
            have_k = true;
        }
        if (have_c && have_k && mathlib::cmp(cmax, kmin) < 0) {
            out.renewal_block = block;
            out.lower_renewal_proved = true;
            out.upper_embedded_renewal_proved = true;
            out.proved = true;
            return out;
        }
    }
    out.obstruction = "lower Bellman grammar did not separate within block bound";
    return out;
}

inline std::string render_twisted_bellman_transport_lean() {
    return R"LEAN(import Mathlib

namespace RavelPlayground

/-- A role embedding transports complete branch profiles by relabelling. -/
def relabelProfile {R U : Type} [Fintype R] [Fintype U]
    [DecidableEq R] [DecidableEq U]
    (ι : R → U) (p : R → Nat) : U → Nat :=
  fun u => ∑ r with ι r = u, p r

/-- If every upper macro-profile is exactly the relabelling of a lower profile,
    Bellman upper propagation commutes with the role embedding.  The Z/2 sheet
    correction does not appear because it changes representatives, not branch
    multiplicities. -/
theorem bellmanUpper_transport
    {R U : Type} [Fintype R] [Fintype U]
    [DecidableEq R] [DecidableEq U]
    (ι : R → U)
    (lower : R → Finset (R → Nat))
    (upper : U → Finset (U → Nat))
    (hprofiles : ∀ r, upper (ι r) = (lower r).image (relabelProfile ι))
    (v : U → Nat) (r : R) :
    (upper (ι r)).sup (fun p => ∑ u, p u * v u) =
      (lower r).sup (fun p => ∑ q, p q * v (ι q)) := by
  rw [hprofiles]
  simp [relabelProfile]

/-- Therefore any strict lower renewal deficit is inherited by the upper
    skew-product macro-system once competitor and core profile families are
    both preserved. -/
theorem strict_renewal_transports
    (lowerCompetitor lowerCore upperCompetitor upperCore : Nat)
    (hc : lowerCompetitor = upperCompetitor)
    (hk : lowerCore = upperCore)
    (hstrict : lowerCompetitor < lowerCore) :
    upperCompetitor < upperCore := by
  omega

end RavelPlayground
)LEAN";
}

} // namespace ravel::proof
