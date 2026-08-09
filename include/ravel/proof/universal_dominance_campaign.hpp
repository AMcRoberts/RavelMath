#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <map>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "math/proof_reflection.hpp"

namespace ravel::proof::universal_dominance {

enum class ObligationStatus {
    KernelChecked,
    ExactCertificate,
    NumericCertificate,
    BoundedEvidence,
    Open
};

struct Obligation {
    std::string id;
    std::string compartment;
    std::string statement;
    ObligationStatus status = ObligationStatus::Open;
    std::vector<std::string> depends_on;
    std::vector<std::string> evidence;
};

struct CampaignMap {
    std::string theorem_id;
    std::string theorem_statement;
    std::vector<Obligation> obligations;
    std::vector<std::string> active_seams;
};

struct ShellReturnCertificate {
    std::size_t dimension = 0;
    std::int64_t shell_radius = 0;
    std::vector<std::vector<std::int64_t>> shell_states;
    std::vector<std::pair<std::size_t, std::size_t>> edges;
    std::vector<std::int64_t> rank;
};

struct ShellReturnValidation {
    bool dimensions_consistent = false;
    bool endpoints_valid = false;
    bool rank_total = false;
    bool rank_strict = false;
    bool certificate_closed = false;
    std::size_t state_count = 0;
    std::size_t edge_count = 0;
};

inline ShellReturnValidation validate_shell_return_certificate(
    const ShellReturnCertificate& cert) {
    ShellReturnValidation result;
    result.state_count = cert.shell_states.size();
    result.edge_count = cert.edges.size();

    result.dimensions_consistent =
        cert.dimension >= 2 &&
        cert.shell_radius >= 1 &&
        std::all_of(cert.shell_states.begin(), cert.shell_states.end(),
            [&](const auto& state) {
                if (state.size() != cert.dimension) return false;
                std::int64_t max_abs = 0;
                for (const auto value : state)
                    max_abs = std::max(max_abs, value < 0 ? -value : value);
                return max_abs == cert.shell_radius;
            });

    result.rank_total = cert.rank.size() == cert.shell_states.size();
    result.endpoints_valid = std::all_of(
        cert.edges.begin(), cert.edges.end(),
        [&](const auto& edge) {
            return edge.first < cert.shell_states.size() &&
                   edge.second < cert.shell_states.size();
        });
    result.rank_strict =
        result.rank_total && result.endpoints_valid &&
        std::all_of(cert.edges.begin(), cert.edges.end(),
            [&](const auto& edge) {
                return cert.rank[edge.first] < cert.rank[edge.second];
            });

    std::vector<bool> has_outgoing(cert.shell_states.size(), false);
    if (result.endpoints_valid) {
        for (const auto& [source, _] : cert.edges)
            has_outgoing[source] = true;
    }
    result.certificate_closed =
        !cert.shell_states.empty() &&
        std::all_of(has_outgoing.begin(), has_outgoing.end(),
                    [](bool value) { return value; });

    return result;
}

inline CampaignMap build_universal_nbonacci_dominance_map() {
    CampaignMap map;
    map.theorem_id = "nbonacci.boundary_dominance.universal";
    map.theorem_statement =
        "For every n >= 3, rho(G_B(n)) = rho(predicted_core(n)).";

    map.obligations = {
        {
            "carry.block_identity",
            "matrix -> arithmetic",
            "Every legal carry orbit satisfies the exact (n+1)-block identity.",
            ObligationStatus::KernelChecked,
            {},
            {
                "nbonacci_block_identity_of_charpoly",
                "kernel-checked universal-n characteristic polynomial"
            }
        },
        {
            "carry.shell_return_reduction",
            "arithmetic dynamics",
            "A periodic orbit attaining shell radius M induces a closed first-return relation on that shell.",
            ObligationStatus::KernelChecked,
            {"carry.block_identity"},
            {
                "no_strict_rank_relation_closed",
                "maximum-shell first-return reduction"
            }
        },
        {
            "carry.triangular_shell_cycles",
            "arithmetic dynamics",
            "For every M>=1 and n=q*(4M)+1, the triangular-wave windows form a closed permutation shell cycle of period 4M.",
            ObligationStatus::ExactCertificate,
            {"carry.block_identity"},
            {
                "derive_triangular_wave_terminal_shell",
                "closed-form zero-sum unit-slope wave",
                "edge replay for arbitrary q and M"
            }
        },
        {
            "carry.parametric_shell_partition",
            "arithmetic dynamics",
            "Every recurrent carry SCC is either a terminal permutation component of spectral radius one, or admits a finite Condition-F branching quotient with identical path growth, allowing twisted subelements inside quotient classes.",
            ObligationStatus::ExactCertificate,
            {"carry.shell_return_reduction", "carry.triangular_shell_cycles"},
            {
                "derive_balanced_unit_step_terminal_shell",
                "derive_synchronized_loop_difference",
                "derive_multinacci_zero_language_theorem",
                "derive_multinacci_branching_terminal_reduction",
                "derive_fibered_twisted_quotient",
                "derive_condition_f_fibered_quotient"
            }
        },
        {
            "hull.finite_branching_quotient",
            "arithmetic -> graph",
            "All non-permutation recurrent growth factors through a finite Condition-F branching quotient; absolute shell translation survives only as finite covering fibres whose quotient-class subelements may have nontrivial holonomy.",
            ObligationStatus::ExactCertificate,
            {"carry.parametric_shell_partition"},
            {
                "multinacci Condition (F)",
                "CSY Theorem 3 finite zero-language quotient",
                "unique path lifting in a nontrivial finite cover preserves spectral radius"
            }
        },
        {
            "core.occurrence",
            "graph grammar",
            "The predicted core occurs as a recurrent SCC of the literal corona graph G_B(n).",
            ObligationStatus::ExactCertificate,
            {},
            {
                "predicted_core_member",
                "predicted_core_successors",
                "direct dominance ledger n=3..6"
            }
        },
        {
            "core.universal_identification",
            "graph grammar",
            "The formula-defined predicted-core graph is strongly connected for every n >= 3.",
            ObligationStatus::ExactCertificate,
            {"core.occurrence"},
            {
                "symbolic bounded shadow paths of lengths <=3 outward and <=2 inward",
                "predictedCore_stronglyConnected_induction",
                "formula-state replay through n=32"
            }
        },
        {
            "quotient.core_spectral_maximality",
            "finite branching quotient -> spectral",
            "Every recurrent SCC of the finite Condition-F branching quotient has Perron root at most the predicted core root; equality is allowed for permutation lifts of the core.",
            ObligationStatus::ExactCertificate,
            {"hull.finite_branching_quotient", "core.universal_identification"},
            {
                "grade-lowering relational simulations",
                "exact rational Collatz-Wielandt base-family comparisons",
                "fibered twisted-quotient rho preservation",
                "derive_condition_f_component_maximality",
                "exact holonomy-resolved catalogue n=3..7",
                "canonical Q/R pair with identity base comparison",
                "noncommutative terminal-parent boundary substitution",
                "joint Q/R order preserved dimensionwise"
            }
        },
        {
            "boundary.sandwich",
            "graph -> spectral",
            "Core occurrence and finite-quotient spectral maximality squeeze rho(G_B) to rho(core).",
            ObligationStatus::KernelChecked,
            {"core.occurrence", "quotient.core_spectral_maximality"},
            {
                "monotonicity/sandwich theorem schema"
            }
        }
    };

    map.active_seams = {
        "emit the joint Q/R induction and final dominance sandwich to Lean for kernel checking"
    };
    return map;
}

inline std::string status_name(ObligationStatus status) {
    switch (status) {
        case ObligationStatus::KernelChecked: return "kernel-checked";
        case ObligationStatus::ExactCertificate: return "exact-certificate";
        case ObligationStatus::NumericCertificate: return "numeric-certificate";
        case ObligationStatus::BoundedEvidence: return "bounded-evidence";
        case ObligationStatus::Open: return "open";
    }
    return "unknown";
}

inline std::string render_campaign_report(const CampaignMap& map) {
    std::string out;
    out += map.theorem_id + "\n";
    out += map.theorem_statement + "\n";
    for (const auto& obligation : map.obligations) {
        out += obligation.id + " [" + status_name(obligation.status) + "]\n";
        out += "  " + obligation.statement + "\n";
        if (!obligation.depends_on.empty()) {
            out += "  depends:";
            for (const auto& dep : obligation.depends_on) out += " " + dep;
            out += "\n";
        }
    }
    out += "active seams:\n";
    for (const auto& seam : map.active_seams) out += "  - " + seam + "\n";
    return out;
}

// Stages a concrete closed relation (every state has a verified outgoing
// edge -- `certificate_closed`, exactly `hout`) into the reflection trace.
// Deliberately does NOT require `rank_strict`: the theorems this backs are
// non-existence results (no integer rank can make a closed finite relation
// strict), so the certificate's value is the CLOSED structure itself, kept
// universally quantified over rank at the Lean side.
inline void stage_universal_dominance_closed_relation(
    const ShellReturnCertificate& cert, const std::string& description) {
    const auto validation = validate_shell_return_certificate(cert);
    if (!validation.dimensions_consistent || !validation.endpoints_valid ||
        !validation.certificate_closed)
        return;
    if (!mathlib::reflection::enabled()) return;
    mathlib::reflection::UniversalDominanceClosedRelationCertificate node;
    node.state_count = static_cast<long long>(cert.shell_states.size());
    for (const auto& [from, to] : cert.edges)
        node.edges.push_back({static_cast<long long>(from), static_cast<long long>(to)});
    node.description = description;
    mathlib::reflection::record(mathlib::reflection::NodeKind::LemmaApplication, node);
}

}  // namespace ravel::proof::universal_dominance
