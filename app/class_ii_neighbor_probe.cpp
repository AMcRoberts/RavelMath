// First fixed-light fiber experiment for the Class-II family.
//
// Adjacent unequal-letter swaps preserve the incidence matrix exactly,
// so beta and the contracting splitting are classified once per a and
// reused across the center and its three word-order neighbors.

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <iterator>
#include <map>
#include <set>
#include <string_view>
#include <tuple>
#include <vector>

#include "ravel/barge.hpp"
#include "ravel/class_ii_neighbor_family.hpp"
#include "ravel/class_ii_neighbor2_pruning.hpp"
#include "ravel/contact_boundary.hpp"
#include "ravel/d_cont_check.hpp"
#include "ravel/graph_divisor.hpp"
#include "ravel/spectral.hpp"
#include "ravel/substitution.hpp"
#include "ravel/substitution_neighborhood.hpp"
#include "ravel/survey.hpp"

using namespace ravel;

namespace {

using Matrix = std::vector<std::vector<long long>>;

void print_coefficients(const std::vector<long long>& coefficients);

void print_sparse_matrix(
        std::size_t a,
        std::size_t neighbor,
        std::size_t rank,
        const Matrix& matrix) {
    for (std::size_t source = 0; source < matrix.size(); ++source) {
        for (std::size_t target = 0; target < matrix.size(); ++target) {
            if (matrix[source][target] != 0) {
                std::printf(
                    "RECURRENT_EDGE,%zu,%zu,%zu,%zu,%zu,%lld\n",
                    a, neighbor, rank, source, target,
                    matrix[source][target]);
            }
        }
    }
}

void analyze_regular_shell_controls(std::size_t neighbor) {
    const long long minimum_parameter = neighbor == 2 ? 2 : 1;
    const long long offset = neighbor == 0 ? 3 : (neighbor == 1 ? 2 : 1);
    const auto controls_at = [&](long long parameter, long long slack) {
        const long long a = parameter + offset + slack;
        return class_ii_neighbor_transition_controls(
            neighbor, a,
            class_ii_neighbor_regular_shell_states(
                neighbor, a, parameter));
    };
    const auto base = controls_at(minimum_parameter, 0);
    const auto parameter_step = controls_at(minimum_parameter + 1, 0);
    const auto slack_step = controls_at(minimum_parameter, 1);
    if (base.size() != parameter_step.size()
            || base.size() != slack_step.size()) {
        std::printf("SHELL_CONTROL_ORDER_MISMATCH,%zu\n", neighbor);
        return;
    }
    std::size_t fixed = 0;
    std::set<std::array<long long, 3>> variable_forms;
    for (std::size_t i = 0; i < base.size(); ++i) {
        const long long dp = parameter_step[i] - base[i];
        const long long ds = slack_step[i] - base[i];
        const bool nonnegative =
            base[i] >= 0 && dp >= 0 && ds >= 0;
        const bool nonpositive =
            base[i] <= 0 && dp <= 0 && ds <= 0;
        if (nonnegative || nonpositive) {
            ++fixed;
        } else {
            variable_forms.insert({base[i], dp, ds});
        }
    }
    std::printf(
        "SHELL_CONTROLS,%zu,total=%zu,fixed=%zu,variable=%zu,forms=%zu\n",
        neighbor, base.size(), fixed, base.size() - fixed,
        variable_forms.size());
    for (const auto& form : variable_forms) {
        std::printf(
            "SHELL_VARIABLE_FORM,%zu,%lld,%lld,%lld\n",
            neighbor, form[0], form[1], form[2]);
    }
}

FiniteSubstitution class_ii(std::size_t a) {
    FiniteSubstitution result(3);
    result[0].assign(a, 0);
    result[0].push_back(1);
    result[0].push_back(2);
    result[1].assign(a, 0);
    result[1].push_back(2);
    result[2] = {0};
    return result;
}

Matrix incidence(const FiniteSubstitution& substitution) {
    Matrix matrix(3, std::vector<long long>(3, 0));
    for (std::size_t image = 0; image < 3; ++image) {
        for (const auto letter : substitution[image]) {
            ++matrix[static_cast<std::size_t>(letter)][image];
        }
    }
    return matrix;
}

ContactBoundaryReport report(
        const FiniteSubstitution& substitution,
        double beta,
        double b2) {
    SubstitutionRule rule(substitution);
    const auto subst = make_substitution<3>(rule, beta);
    const auto candidates = search_D_cont<3>(subst, 2);
    std::vector<std::tuple<long long, std::vector<long long>, long long>>
        d_cont;
    for (const auto& candidate : candidates) {
        d_cont.emplace_back(
            candidate.i,
            std::vector<long long>(
                candidate.x.begin(), candidate.x.end()),
            candidate.j);
    }
    ContactBoundaryLimits limits;
    limits.max_corona_rounds = 12;
    return compute_contact_boundary_dispatch(
        rule, beta, b2, d_cont, limits);
}

CoronaTrace<3> corona_trace(
        const FiniteSubstitution& substitution, double beta) {
    const Substitution<3> subst =
        make_substitution<3>(SubstitutionRule(substitution), beta);
    const auto seeds = search_D_cont<3>(subst, 2);
    std::vector<ANode<3>> d_cont;
    for (const auto& seed : seeds) {
        ANode<3> node;
        node.i = seed.i;
        node.j = seed.j;
        node.x = seed.x;
        d_cont.push_back(node);
    }
    const auto pre_contact = backward_closure<3>(subst, d_cont);
    const auto induced = induced_restricted_edges<3>(subst, pre_contact);
    const std::set<ANode<3>> pre_set(
        pre_contact.begin(), pre_contact.end());
    const auto reduced = red_anode<3>(pre_set, induced);
    std::set<SNode<3>> contact;
    for (const auto& node : reduced.first)
        contact.insert(to_simple<3>(node));
    return algorithm2_trace<3>(
        subst, contact, CoronaConnectorPolicy::fixed_signed_contact,
        50, 0, CoronaEdgeArithmetic::exact_rational);
}

struct CoreFingerprint {
    std::size_t recurrent_components = 0;
    std::size_t dominant_core_size = 0;
    std::vector<long long> dominant_charpoly;
    std::vector<std::size_t> original_indices;
};

CoreFingerprint core_fingerprint(const ContactBoundaryReport& value) {
    const auto graph = WeightedDigraph::from_dense(value.gb_matrix);
    CoreFingerprint result;
    for (const auto& component : tarjan_scc(graph)) {
        if (is_recurrent_scc(graph, component)) {
            ++result.recurrent_components;
        }
    }
    auto [core, original_indices] =
        extract_dominant_recurrent_core(graph);
    result.dominant_core_size = core.n;
    result.original_indices = std::move(original_indices);
    Matrix dense(core.n, std::vector<long long>(core.n, 0));
    for (std::size_t source = 0; source < core.n; ++source) {
        for (const auto& [target, weight] : core.out_adj[source]) {
            dense[source][target] += weight;
        }
    }
    result.dominant_charpoly = charpoly_int(dense);
    return result;
}

void dump_core_nodes(
        std::size_t a,
        std::size_t neighbor,
        const ContactBoundaryReport& value,
        const CoreFingerprint& core) {
    for (std::size_t rank = 0; rank < core.original_indices.size(); ++rank) {
        const auto& node =
            value.boundary_nodes[core.original_indices[rank]];
        const auto& x = std::get<1>(node);
        std::printf(
            "CORE,%zu,%zu,%zu,%lld,%lld,%lld,%lld,%lld\n",
            a, neighbor, rank, std::get<0>(node),
            x[0], x[1], x[2], std::get<2>(node));
    }
}

void dump_recurrent_components(
        std::size_t a,
        std::size_t neighbor,
        const ContactBoundaryReport& value) {
    const auto graph = WeightedDigraph::from_dense(value.gb_matrix);
    std::size_t recurrent_rank = 0;
    for (auto component : tarjan_scc(graph)) {
        if (!is_recurrent_scc(graph, component)) continue;
        std::sort(component.begin(), component.end());
        Matrix dense(
            component.size(),
            std::vector<long long>(component.size(), 0));
        for (std::size_t source = 0; source < component.size(); ++source) {
            for (std::size_t target = 0; target < component.size(); ++target) {
                dense[source][target] =
                    value.gb_matrix[component[source]][component[target]];
            }
        }
        std::printf(
            "RECURRENT_POLY,%zu,%zu,%zu,%zu,",
            a, neighbor, recurrent_rank, component.size());
        print_coefficients(charpoly_int(dense));
        std::printf("\n");
        for (const auto original_index : component) {
            const auto& node = value.boundary_nodes[original_index];
            const auto& x = std::get<1>(node);
            std::printf(
                "RECURRENT,%zu,%zu,%zu,%lld,%lld,%lld,%lld,%lld\n",
                a, neighbor, recurrent_rank, std::get<0>(node),
                x[0], x[1], x[2], std::get<2>(node));
        }
        ++recurrent_rank;
    }
}

std::vector<std::vector<long long>> recurrent_polynomials(
        const ContactBoundaryReport& value) {
    const auto graph = WeightedDigraph::from_dense(value.gb_matrix);
    std::vector<std::vector<long long>> result;
    for (auto component : tarjan_scc(graph)) {
        if (!is_recurrent_scc(graph, component)) continue;
        std::sort(component.begin(), component.end());
        Matrix dense(
            component.size(),
            std::vector<long long>(component.size(), 0));
        for (std::size_t source = 0; source < component.size(); ++source) {
            for (std::size_t target = 0; target < component.size(); ++target) {
                dense[source][target] =
                    value.gb_matrix[component[source]][component[target]];
            }
        }
        result.push_back(charpoly_int(dense));
    }
    return result;
}

std::vector<std::set<SNode<3>>> recurrent_component_node_sets(
        const ContactBoundaryReport& value) {
    const auto graph = WeightedDigraph::from_dense(value.gb_matrix);
    std::vector<std::set<SNode<3>>> result;
    for (const auto& component : tarjan_scc(graph)) {
        if (!is_recurrent_scc(graph, component)) continue;
        std::set<SNode<3>> state_set;
        for (const auto original_index : component) {
            const auto& tuple = value.boundary_nodes[original_index];
            SNode<3> node;
            node.i = std::get<0>(tuple);
            node.j = std::get<2>(tuple);
            const auto& x = std::get<1>(tuple);
            for (std::size_t coordinate = 0; coordinate < 3; ++coordinate) {
                node.x[coordinate] = x[coordinate];
            }
            state_set.insert(node);
        }
        result.push_back(std::move(state_set));
    }
    return result;
}

std::set<SNode<3>> core_node_set(
        const ContactBoundaryReport& value,
        const CoreFingerprint& core) {
    std::set<SNode<3>> result;
    for (const auto original_index : core.original_indices) {
        const auto& tuple = value.boundary_nodes[original_index];
        SNode<3> node;
        node.i = std::get<0>(tuple);
        node.j = std::get<2>(tuple);
        const auto& x = std::get<1>(tuple);
        for (std::size_t coordinate = 0; coordinate < 3; ++coordinate) {
            node.x[coordinate] = x[coordinate];
        }
        result.insert(node);
    }
    return result;
}

std::set<SNode<3>> boundary_node_set(
        const ContactBoundaryReport& value) {
    std::set<SNode<3>> result;
    for (const auto& tuple : value.boundary_nodes) {
        SNode<3> node;
        node.i = std::get<0>(tuple);
        node.j = std::get<2>(tuple);
        const auto& x = std::get<1>(tuple);
        for (std::size_t coordinate = 0; coordinate < 3; ++coordinate)
            node.x[coordinate] = x[coordinate];
        result.insert(node);
    }
    return result;
}

Matrix catalog_adjacency(
        const FiniteSubstitution& substitution,
        double beta,
        const std::set<SNode<3>>& state_set) {
    const Substitution<3> subst =
        make_substitution<3>(SubstitutionRule(substitution), beta);
    const std::vector<SNode<3>> states(state_set.begin(), state_set.end());
    std::map<SNode<3>, std::size_t> index;
    for (std::size_t i = 0; i < states.size(); ++i) index[states[i]] = i;
    Matrix result(states.size(), std::vector<long long>(states.size(), 0));
    for (std::size_t source = 0; source < states.size(); ++source) {
        for (const auto& [target, labels] :
             simple_forward_targets_exact<3>(subst, states[source])) {
            (void)labels;
            const auto found = index.find(target);
            if (found != index.end()) {
                ++result[source][found->second];
            }
        }
    }
    return result;
}

Matrix affine_matrix_value(
        const Matrix& at_three,
        const Matrix& first_difference,
        long long a) {
    Matrix result = at_three;
    for (std::size_t row = 0; row < result.size(); ++row) {
        for (std::size_t column = 0; column < result.size(); ++column) {
            result[row][column] +=
                (a - 3) * first_difference[row][column];
        }
    }
    return result;
}

Matrix matrix_difference(const Matrix& right, const Matrix& left) {
    Matrix result = right;
    for (std::size_t row = 0; row < result.size(); ++row) {
        for (std::size_t column = 0; column < result.size(); ++column) {
            result[row][column] -= left[row][column];
        }
    }
    return result;
}

void print_coefficients(const std::vector<long long>& coefficients) {
    std::printf("[");
    for (std::size_t i = 0; i < coefficients.size(); ++i) {
        std::printf("%s%lld", i == 0 ? "" : ",", coefficients[i]);
    }
    std::printf("]");
}

CoreFingerprint print_report(
        std::size_t a,
        const char* label,
        const ContactBoundaryReport& value) {
    const auto core = core_fingerprint(value);
    std::printf(
        "a=%zu %-10s C=%zu GB=%zu rounds=%d "
        "scc=%zu core=%zu lambda=%.12g %s%s cp=",
        a, label, value.contact_size, value.boundary_size,
        value.convergence_rounds, core.recurrent_components,
        core.dominant_core_size, value.boundary_eigenvalue,
        value.converged ? "converged" : "INCOMPLETE",
        value.corona_capped ? " [corona cap]" : "");
    print_coefficients(core.dominant_charpoly);
    std::printf("\n");
    return core;
}

std::vector<long long> expected_neighbor_polynomial(
        std::size_t neighbor, long long a) {
    return class_ii_neighbor_expected_polynomial(neighbor, a);
}

}  // namespace

int main(int argc, char** argv) {
    const bool dump_affine =
        argc == 2 && std::string_view(argv[1]) == "--dump-affine";
    const bool dump_recurrent =
        argc == 2 && std::string_view(argv[1]) == "--dump-recurrent";
    const bool dump_recurrent_matrices =
        argc == 2
        && std::string_view(argv[1]) == "--dump-recurrent-matrices";
    const bool analyze_shell_controls =
        argc == 2
        && std::string_view(argv[1]) == "--analyze-shell-controls";
    const bool compare_center =
        argc == 2 && std::string_view(argv[1]) == "--compare-center";
    const bool dump_center_difference =
        argc == 2
        && std::string_view(argv[1]) == "--dump-center-difference";
    const bool compare_corona =
        argc == 2 && std::string_view(argv[1]) == "--compare-corona";
    const bool dump_corona_difference =
        argc == 2
        && std::string_view(argv[1]) == "--dump-corona-difference";
    const bool dump_extension_witnesses =
        argc == 2
        && std::string_view(argv[1]) == "--dump-extension-witnesses";
    if (analyze_shell_controls) {
        for (std::size_t neighbor = 0; neighbor < 3; ++neighbor)
            analyze_regular_shell_controls(neighbor);
        const auto core =
            class_ii_neighbor_dominant_core_states(2);
        const auto extension =
            class_ii_neighbor2_fixed_extension_states();
        std::vector<SNode<3>> overlap;
        std::set_intersection(
            core.begin(), core.end(), extension.begin(), extension.end(),
            std::back_inserter(overlap));
        std::printf(
            "NEIGHBOR2_CORE_FIXED_EXTENSION_OVERLAP,%zu,%zu,%zu\n",
            overlap.size(), core.size(), extension.size());
        return 0;
    }
    bool ok = true;
    std::array<Matrix, 3> adjacency_at_three;
    std::array<Matrix, 3> adjacency_difference;
    for (std::size_t a = 2; a <= 8; ++a) {
        const auto center = class_ii(a);
        const auto spectral = classify_matrix_spectral(incidence(center));
        const auto center_report =
            report(center, spectral.beta, spectral.b2);
        (void)print_report(a, "center", center_report);
        ok = ok && center_report.converged;

        const auto neighbors = adjacent_swap_neighbors(center);
        ok = ok && neighbors.size() == 3;
        for (std::size_t index = 0; index < neighbors.size(); ++index) {
            const auto neighbor_report = report(
                neighbors[index].substitution, spectral.beta, spectral.b2);
            char label[64];
            std::snprintf(
                label, sizeof(label), "swap%zu[%zu:%zu]", index,
                neighbors[index].image, neighbors[index].position);
            const auto core = print_report(a, label, neighbor_report);
            if (compare_center) {
                const auto center_nodes = boundary_node_set(center_report);
                const auto neighbor_nodes = boundary_node_set(neighbor_report);
                std::vector<SNode<3>> intersection;
                std::set_intersection(
                    center_nodes.begin(), center_nodes.end(),
                    neighbor_nodes.begin(), neighbor_nodes.end(),
                    std::back_inserter(intersection));
                std::printf(
                    "CENTER_OVERLAP,%zu,%zu,%zu,%zu,%zu\n",
                    a, index, intersection.size(),
                    center_nodes.size() - intersection.size(),
                    neighbor_nodes.size() - intersection.size());
            }
            if (dump_center_difference && index == 2 && a >= 3) {
                const auto center_nodes = boundary_node_set(center_report);
                const auto neighbor_nodes = boundary_node_set(neighbor_report);
                std::vector<SNode<3>> difference;
                std::set_difference(
                    neighbor_nodes.begin(), neighbor_nodes.end(),
                    center_nodes.begin(), center_nodes.end(),
                    std::back_inserter(difference));
                for (const auto& node : difference) {
                    std::printf(
                        "CENTER_DIFFERENCE,%zu,%lld,%lld,%lld,%lld,%lld\n",
                        a, node.i, node.x[0], node.x[1], node.x[2], node.j);
                }
            }
            if (index == 2 && a >= 3) {
                const auto center_trace = corona_trace(center, spectral.beta);
                const auto neighbor_trace =
                    corona_trace(neighbors[index].substitution, spectral.beta);
                ok = ok
                    && center_trace.signed_contact
                        == build_signed_contact_set<3>(
                            class_ii_contact_set())
                    && neighbor_trace.signed_contact
                        == class_ii_neighbor2_signed_contact_set();
                const std::size_t rounds = std::max(
                    center_trace.layers.size(), neighbor_trace.layers.size());
                for (std::size_t round = 0; round < rounds; ++round) {
                    const auto& center_layer =
                        center_trace.layers[
                            std::min(round, center_trace.layers.size() - 1)]
                            .nodes;
                    const auto& neighbor_layer =
                        neighbor_trace.layers[
                            std::min(round, neighbor_trace.layers.size() - 1)]
                            .nodes;
                    if (a >= 4 && round + 1 >= 3) {
                        const auto expected_center =
                            class_ii_center_layer_candidate(
                                a, round + 1);
                        if (center_layer != expected_center) {
                            std::printf(
                                "CENTER LAYER GRAMMAR MISMATCH "
                                "a=%zu round=%zu actual=%zu expected=%zu\n",
                                a, round + 1, center_layer.size(),
                                expected_center.size());
                        }
                        ok = ok && center_layer == expected_center;
                    }
                    if (round + 1 >= 3) {
                        const auto interface_states =
                            class_ii_neighbor2_center_interface_states();
                        ok = ok && std::includes(
                            center_layer.begin(), center_layer.end(),
                            interface_states.begin(), interface_states.end());
                    }
                    std::vector<SNode<3>> difference;
                    std::set_difference(
                        neighbor_layer.begin(), neighbor_layer.end(),
                        center_layer.begin(), center_layer.end(),
                        std::back_inserter(difference));
                    const bool contains = std::includes(
                        neighbor_layer.begin(), neighbor_layer.end(),
                        center_layer.begin(), center_layer.end());
                    const std::size_t round_number = round + 1;
                    if (a >= 5 && round_number >= 4
                            && round_number <= a + 1) {
                        std::vector<std::set<SNode<3>>> expected_pruning;
                        if (round_number
                                < static_cast<std::size_t>(a - 1)) {
                            expected_pruning =
                                class_ii_neighbor2_interior_pruning_ranks(
                                    round_number);
                        } else if (round_number
                                   == static_cast<std::size_t>(a - 1)) {
                            expected_pruning =
                                class_ii_neighbor2_penultimate_pruning_ranks(
                                    static_cast<long long>(a));
                        } else if (round_number
                                   == static_cast<std::size_t>(a)) {
                            expected_pruning =
                                class_ii_neighbor2_terminal_pruning_ranks(
                                    static_cast<long long>(a));
                        } else {
                            expected_pruning =
                                class_ii_neighbor2_fixed_pruning_ranks(
                                    static_cast<long long>(a));
                        }
                        const auto& actual_pruning =
                            neighbor_trace.layers[
                                std::min(
                                    round,
                                    neighbor_trace.layers.size() - 1)]
                                .red_pruning_ranks;
                        if (actual_pruning != expected_pruning) {
                            std::printf(
                                "RED PRUNING GRAMMAR MISMATCH "
                                "a=%zu round=%zu actual_ranks=%zu "
                                "expected_ranks=%zu\n",
                                a, round_number, actual_pruning.size(),
                                expected_pruning.size());
                        }
                        ok = ok && actual_pruning == expected_pruning;
                    }
                    const auto expected_difference =
                        class_ii_neighbor2_layer_extension(
                            static_cast<long long>(a), round_number);
                    const std::set<SNode<3>> actual_difference(
                        difference.begin(), difference.end());
                    if (!contains
                            || actual_difference != expected_difference) {
                        std::printf(
                            "CORONA CATALOGUE MISMATCH "
                            "a=%zu round=%zu actual=%zu expected=%zu\n",
                            a, round_number, actual_difference.size(),
                            expected_difference.size());
                    }
                    ok = ok
                        && contains
                        && actual_difference == expected_difference
                        && neighbor_layer
                            == class_ii_neighbor2_layer_candidate(
                                static_cast<long long>(a), round_number,
                                center_layer);
                    if (round_number >= 2) {
                        std::vector<ClassIIRedSuccessor> red_bridges;
                        if (round_number == 2) {
                            red_bridges =
                                class_ii_neighbor2_second_red_bridges();
                        } else if (round_number
                                < static_cast<std::size_t>(a - 1)) {
                            red_bridges =
                                class_ii_neighbor2_interior_red_bridges(
                                    round_number);
                        } else if (round_number
                                   == static_cast<std::size_t>(a - 1)) {
                            red_bridges =
                                class_ii_neighbor2_penultimate_red_bridges(
                                    static_cast<long long>(a));
                        } else {
                            red_bridges =
                                class_ii_neighbor2_terminal_red_bridges(
                                    static_cast<long long>(a));
                        }
                        for (const auto& bridge : red_bridges) {
                            const bool bridge_ok =
                                actual_difference.count(
                                    bridge.source) == 1
                                && (center_layer.count(bridge.target) == 1
                                    || actual_difference.count(
                                        bridge.target) == 1)
                                && class_ii_neighbor_transition_weight(
                                    2, static_cast<long long>(a),
                                    bridge.source, bridge.target) > 0;
                            if (!bridge_ok) {
                                std::printf(
                                    "RED BRIDGE MISMATCH "
                                    "a=%zu round=%zu source=%lld,"
                                    "(%lld,%lld,%lld),%lld target=%lld,"
                                    "(%lld,%lld,%lld),%lld "
                                    "source_in=%zu target_in=%zu weight=%lld\n",
                                    a, round_number,
                                    bridge.source.i,
                                    bridge.source.x[0],
                                    bridge.source.x[1],
                                    bridge.source.x[2],
                                    bridge.source.j,
                                    bridge.target.i,
                                    bridge.target.x[0],
                                    bridge.target.x[1],
                                    bridge.target.x[2],
                                    bridge.target.j,
                                    actual_difference.count(bridge.source),
                                    center_layer.count(bridge.target),
                                    class_ii_neighbor_transition_weight(
                                        2, static_cast<long long>(a),
                                        bridge.source, bridge.target));
                            }
                            ok = ok && bridge_ok;
                        }
                    }
                    if (compare_corona) {
                        std::printf(
                            "CORONA_OVERLAP,%zu,%zu,%zu,%zu,%zu,%d\n",
                            a, round + 1, center_layer.size(),
                            neighbor_layer.size(), difference.size(),
                            contains ? 1 : 0);
                    }
                    if (dump_corona_difference && a == 8) {
                        for (const auto& node : difference) {
                            std::printf(
                                "CORONA_DIFFERENCE,%zu,%zu,"
                                "%lld,%lld,%lld,%lld,%lld\n",
                                a, round + 1, node.i, node.x[0],
                                node.x[1], node.x[2], node.j);
                        }
                    }
                }
                if (dump_extension_witnesses && a == 8) {
                    const auto correction_sources =
                        class_ii_neighbor2_interior_extension_states(4);
                    const auto& sources = neighbor_trace.layers[3].nodes;
                    const auto& center_sources = center_trace.layers[3].nodes;
                    const auto targets =
                        class_ii_neighbor2_interior_extension_states(5);
                    for (const auto& target : targets) {
                        bool found = false;
                        for (const auto& source : sources) {
                            if (source.i != target.i) continue;
                            for (const auto& hop :
                                 neighbor_trace.signed_contact) {
                                if (source.j != hop.i
                                        || hop.j != target.j) continue;
                                bool coordinates_match = true;
                                for (std::size_t coordinate = 0;
                                     coordinate < 3; ++coordinate) {
                                    coordinates_match =
                                        coordinates_match
                                        && source.x[coordinate]
                                            + hop.x[coordinate]
                                            == target.x[coordinate];
                                }
                                if (!coordinates_match) continue;
                                std::printf(
                                    "EXTENSION_WITNESS,"
                                    "%lld,%lld,%lld,%lld,%lld,"
                                    "%lld,%lld,%lld,%lld,%lld,"
                                    "%lld,%lld,%lld,%lld,%lld,%d,%d\n",
                                    target.i, target.x[0], target.x[1],
                                    target.x[2], target.j,
                                    source.i, source.x[0], source.x[1],
                                    source.x[2], source.j,
                                    hop.i, hop.x[0], hop.x[1],
                                    hop.x[2], hop.j,
                                    correction_sources.count(source) ? 1 : 0,
                                    center_sources.count(source) ? 1 : 0);
                                found = true;
                                break;
                            }
                            if (found) break;
                        }
                        if (!found) {
                            std::printf(
                                "EXTENSION_WITNESS_MISSING,"
                                "%lld,%lld,%lld,%lld,%lld\n",
                                target.i, target.x[0], target.x[1],
                                target.x[2], target.j);
                        }
                    }
                }
            }
            if (dump_affine && a >= 3) {
                dump_core_nodes(a, index, neighbor_report, core);
            }
            if (dump_recurrent && a >= 3) {
                dump_recurrent_components(a, index, neighbor_report);
            }
            ok = ok && neighbor_report.converged;
            if (a >= 3 || index == 1) {
                const bool polynomial_match =
                    core.dominant_charpoly
                    == expected_neighbor_polynomial(
                        index, static_cast<long long>(a));
                if (!polynomial_match) {
                    std::printf(
                        "POLYNOMIAL FORMULA MISMATCH a=%zu swap=%zu expected=",
                        a, index);
                    print_coefficients(expected_neighbor_polynomial(
                        index, static_cast<long long>(a)));
                    std::printf("\n");
                }
                ok = ok && polynomial_match;
            }
            if (a >= 3) {
                const std::size_t expected_components =
                    index == 0 ? a : (index == 1 ? a - 1 : a + 1);
                const std::size_t expected_core =
                    index == 0 ? 15 : (index == 1 ? 17 : 39);
                if (index == 2) {
                    auto expected_boundary =
                        boundary_node_set(center_report);
                    const auto extension =
                        class_ii_neighbor2_center_extension(
                            static_cast<long long>(a));
                    expected_boundary.insert(
                        extension.begin(), extension.end());
                    ok = ok
                        && extension.size() == 30
                        && boundary_node_set(neighbor_report)
                            == expected_boundary;
                }
                ok = ok
                    && core.recurrent_components == expected_components
                    && core.dominant_core_size == expected_core
                    && core_node_set(neighbor_report, core)
                        == class_ii_neighbor_dominant_core_states(index);
                const auto component_polynomials =
                    recurrent_polynomials(neighbor_report);
                const auto component_node_sets =
                    recurrent_component_node_sets(neighbor_report);
                ok = ok
                    && component_polynomials.size() == expected_components
                    && component_node_sets.size() == expected_components;
                for (std::size_t rank = 0;
                     rank < component_polynomials.size(); ++rank) {
                    const auto expected_states =
                        class_ii_neighbor_recurrent_component_states(
                            index, static_cast<long long>(a), rank);
                    if (component_node_sets[rank] != expected_states) {
                        std::printf(
                            "RECURRENT CATALOGUE MISMATCH "
                            "a=%zu swap=%zu rank=%zu actual=%zu expected=%zu\n",
                            a, index, rank, component_node_sets[rank].size(),
                            expected_states.size());
                    }
                    const auto shell_catalog_matrix = catalog_adjacency(
                        neighbors[index].substitution, spectral.beta,
                        expected_states);
                    const auto shell_compressed_matrix =
                        class_ii_neighbor_compressed_matrix_for_states(
                            index, static_cast<long long>(a),
                            expected_states);
                    if (dump_recurrent_matrices) {
                        print_sparse_matrix(
                            a, index, rank, shell_compressed_matrix);
                    }
                    ok = ok
                        && component_polynomials[rank]
                            == class_ii_neighbor_expected_recurrent_polynomial(
                                index, static_cast<long long>(a), rank)
                        && component_node_sets[rank]
                            == expected_states
                        && shell_catalog_matrix == shell_compressed_matrix
                        && shell_compressed_matrix
                            == class_ii_neighbor_expected_recurrent_matrix(
                                index, static_cast<long long>(a), rank)
                        && charpoly_int(shell_compressed_matrix)
                            == component_polynomials[rank];
                }

                const auto catalog_matrix = catalog_adjacency(
                    neighbors[index].substitution, spectral.beta,
                    class_ii_neighbor_dominant_core_states(index));
                ok = ok
                    && catalog_matrix
                        == class_ii_neighbor_affine_matrix(
                            index, static_cast<long long>(a))
                    && catalog_matrix
                        == class_ii_neighbor_compressed_matrix(
                            index, static_cast<long long>(a))
                    && charpoly_int(catalog_matrix)
                        == expected_neighbor_polynomial(
                            index, static_cast<long long>(a));
                if (a == 3) {
                    adjacency_at_three[index] = catalog_matrix;
                } else if (a == 4) {
                    adjacency_difference[index] = matrix_difference(
                        catalog_matrix, adjacency_at_three[index]);
                } else {
                    ok = ok && catalog_matrix == affine_matrix_value(
                        adjacency_at_three[index],
                        adjacency_difference[index],
                        static_cast<long long>(a));
                }
            }
        }
    }
    if (dump_affine) {
        for (std::size_t neighbor = 0; neighbor < 3; ++neighbor) {
            for (std::size_t source = 0;
                 source < adjacency_at_three[neighbor].size(); ++source) {
                for (std::size_t target = 0;
                     target < adjacency_at_three[neighbor].size(); ++target) {
                    const long long slope =
                        adjacency_difference[neighbor][source][target];
                    const long long intercept =
                        adjacency_at_three[neighbor][source][target]
                        - 3 * slope;
                    if (intercept != 0 || slope != 0) {
                        std::printf(
                            "AFFINE_EDGE,%zu,%zu,%zu,%lld,%lld\n",
                            neighbor, source, target, intercept, slope);
                    }
                }
            }
        }
    }
    return ok ? 0 : 1;
}
