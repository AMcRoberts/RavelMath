#pragma once

#include <cstddef>
#include <sstream>
#include <string>
#include <vector>

#include "ravel/proof/twisted_bellman_transport.hpp"
#include "ravel/proof/twisted_dimension_extension.hpp"

namespace ravel::proof {

// Composite structural certificate for one adjacent-dimensional competitor
// family.  It deliberately records each independently derived premise so the
// proof engine can explain an obstruction instead of returning a flat FAIL.
struct AdjacentTwistedRenewalProof {
    std::size_t lower_dimension = 0;
    std::size_t upper_dimension = 0;
    std::size_t competitor_edges = 0;
    std::size_t competitor_maximum_path = 0;
    std::size_t core_maximum_path = 0;
    bool competitor_paths_complete = false;
    bool competitor_profiles_exact = false;
    bool core_shadow_total = false;
    bool core_paths_complete = false;
    bool shared_role_embedding_total = false;
    bool predicted_core_twist_proved = false;
    TwistedBellmanTransportProof bellman;
    bool proved = false;
    std::string obstruction;
};

inline AdjacentTwistedRenewalProof derive_adjacent_twisted_renewal(
    std::size_t lower_dimension,
    std::size_t competitor_edges,
    std::size_t competitor_maximum_path,
    std::size_t core_maximum_path,
    bool competitor_paths_complete,
    bool competitor_profiles_exact,
    bool core_shadow_total,
    bool core_paths_complete,
    bool shared_role_embedding_total,
    const PhaseBellmanGrammar& lower_grammar,
    const PhaseBellmanGrammar& upper_macro_grammar,
    const std::vector<std::size_t>& shared_role_embedding,
    const std::vector<int>& shared_sheet_correction,
    std::size_t max_block = 256) {
    AdjacentTwistedRenewalProof out;
    out.lower_dimension = lower_dimension;
    out.upper_dimension = lower_dimension + 1;
    out.competitor_edges = competitor_edges;
    out.competitor_maximum_path = competitor_maximum_path;
    out.core_maximum_path = core_maximum_path;
    out.competitor_paths_complete = competitor_paths_complete;
    out.competitor_profiles_exact = competitor_profiles_exact;
    out.core_shadow_total = core_shadow_total;
    out.core_paths_complete = core_paths_complete;
    out.shared_role_embedding_total = shared_role_embedding_total;

    const auto core_twist =
        derive_twisted_predicted_core_extension(lower_dimension);
    out.predicted_core_twist_proved = core_twist.proved;

    if (!out.competitor_paths_complete)
        out.obstruction = "competitor macro-path substitution is incomplete";
    else if (!out.competitor_profiles_exact)
        out.obstruction = "competitor macro branch profiles are not exact";
    else if (!out.core_shadow_total)
        out.obstruction = "predicted-core zero-append shadow is not total";
    else if (!out.core_paths_complete)
        out.obstruction = "predicted-core macro-path substitution is incomplete";
    else if (!out.shared_role_embedding_total)
        out.obstruction = "shared competitor/core role embedding is ambiguous";
    else if (!out.predicted_core_twist_proved)
        out.obstruction = core_twist.obstruction.empty()
            ? "predicted-core twisted extension did not close"
            : core_twist.obstruction;
    else {
        out.bellman = derive_twisted_bellman_transport(
            lower_grammar, lower_grammar,
            upper_macro_grammar, upper_macro_grammar,
            shared_role_embedding, shared_role_embedding,
            shared_sheet_correction, shared_sheet_correction,
            max_block);
        if (!out.bellman.proved)
            out.obstruction = out.bellman.obstruction;
    }

    out.proved = out.competitor_paths_complete &&
        out.competitor_profiles_exact && out.core_shadow_total &&
        out.core_paths_complete && out.shared_role_embedding_total &&
        out.predicted_core_twist_proved && out.bellman.proved;
    return out;
}

inline std::string render_adjacent_twisted_renewal_report(
    const AdjacentTwistedRenewalProof& p) {
    std::ostringstream out;
    out << "ADJACENT_TWISTED_RENEWAL\n";
    out << "dimensions=" << p.lower_dimension << "->"
        << p.upper_dimension << "\n";
    out << "competitor_edges=" << p.competitor_edges << "\n";
    out << "competitor_maximum_path=" << p.competitor_maximum_path << "\n";
    out << "core_maximum_path=" << p.core_maximum_path << "\n";
    out << "competitor_paths_complete="
        << (p.competitor_paths_complete ? "true" : "false") << "\n";
    out << "competitor_profiles_exact="
        << (p.competitor_profiles_exact ? "true" : "false") << "\n";
    out << "core_shadow_total="
        << (p.core_shadow_total ? "true" : "false") << "\n";
    out << "core_paths_complete="
        << (p.core_paths_complete ? "true" : "false") << "\n";
    out << "shared_role_embedding_total="
        << (p.shared_role_embedding_total ? "true" : "false") << "\n";
    out << "predicted_core_twist_proved="
        << (p.predicted_core_twist_proved ? "true" : "false") << "\n";
    out << "competitor_profiles_checked="
        << p.bellman.competitor_profiles_checked << "\n";
    out << "core_profiles_checked=" << p.bellman.core_profiles_checked << "\n";
    out << "renewal_block=" << p.bellman.renewal_block << "\n";
    out << "proved=" << (p.proved ? "true" : "false") << "\n";
    if (!p.obstruction.empty()) out << "obstruction=" << p.obstruction << "\n";
    return out.str();
}

inline std::string render_adjacent_twisted_renewal_lean() {
    return R"LEAN(import Mathlib

namespace RavelPlayground

/-- Adjacent-dimensional renewal is the composition of complete competitor
    macro-path transport, twisted predicted-core transport, exact branch-profile
    relabelling, and the lower strict Bellman deficit. -/
theorem adjacent_twisted_renewal
    (competitorPaths competitorProfiles corePaths coreTwist profileCommutes : Prop)
    (hCompetitorPaths : competitorPaths)
    (hCompetitorProfiles : competitorProfiles)
    (hCorePaths : corePaths)
    (hCoreTwist : coreTwist)
    (hProfileCommutes : profileCommutes) :
    competitorPaths ∧ competitorProfiles ∧ corePaths ∧ coreTwist ∧ profileCommutes := by
  exact ⟨hCompetitorPaths, hCompetitorProfiles, hCorePaths, hCoreTwist,
    hProfileCommutes⟩

/-- Once the macro Bellman values are identified with the lower values, the
    strict renewal inequality is inherited without a new spectral argument. -/
theorem adjacent_strict_loss
    (lowerCompetitor lowerCore upperCompetitor upperCore : Nat)
    (hc : lowerCompetitor = upperCompetitor)
    (hk : lowerCore = upperCore)
    (h : lowerCompetitor < lowerCore) :
    upperCompetitor < upperCore := by
  omega

end RavelPlayground
)LEAN";
}

} // namespace ravel::proof
