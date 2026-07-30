// Differential implementation of the literature's fixed-C corona.
//
// Loridant--Thuswaldner--Zhang Definition 3.9 composes every evolving
// layer with the fixed signed contact set ±C.  The historical project
// loop instead used the evolving layer itself as the connector set.
// This driver constructs C from first principles and compares both
// semantics at every corona layer before production results migrate.

#include <array>
#include <cstdint>
#include <cstdio>
#include <map>
#include <set>
#include <tuple>
#include <vector>

#include "math/charpoly.hpp"
#include "ravel/contact_boundary.hpp"
#include "ravel/class_ii_boundary_family.hpp"
#include "ravel/class_ii_neighbor_family.hpp"
#include "ravel/corona.hpp"
#include "ravel/d_cont_check.hpp"
#include "ravel/graph_divisor.hpp"
#include "ravel/spectral.hpp"
#include "ravel/substitution.hpp"
#include "ravel/survey.hpp"

using namespace ravel;

namespace {

using Matrix = std::vector<std::vector<long long>>;

std::set<SNode<3>> stable_base_a3;

bool anode_sets_equal(
        const std::set<ANode<3>>& lhs,
        const std::set<ANode<3>>& rhs) {
    return lhs.size() == rhs.size()
        && std::equal(
            lhs.begin(), lhs.end(), rhs.begin(),
            [](const auto& left, const auto& right) {
                return !(left < right) && !(right < left);
            });
}

bool anode_equal(const ANode<3>& lhs, const ANode<3>& rhs) {
    return !(lhs < rhs) && !(rhs < lhs);
}

std::vector<std::vector<std::int8_t>> sigma_a1(std::size_t a) {
    std::vector<std::vector<std::int8_t>> sigma(3);
    sigma[0].insert(sigma[0].end(), a, 0);
    sigma[0].push_back(1);
    sigma[0].push_back(2);
    sigma[1].insert(sigma[1].end(), a, 0);
    sigma[1].push_back(2);
    sigma[2] = {0};
    return sigma;
}

Matrix incidence(const std::vector<std::vector<std::int8_t>>& sigma) {
    Matrix M(3, std::vector<long long>(3, 0));
    for (std::size_t j = 0; j < 3; ++j) {
        for (auto letter : sigma[j]) ++M[static_cast<std::size_t>(letter)][j];
    }
    return M;
}

struct BuiltContact {
    Substitution<3> subst;
    std::set<SNode<3>> C;
    std::size_t seed_size = 0;
    std::size_t pre_contact_size = 0;
    bool seed_law = false;
    bool pre_contact_law = false;
    bool backward_layer_law = false;
    bool backward_witness_law = false;
    bool red_rank_law = false;
};

BuiltContact build_contact(
        const std::vector<std::vector<std::int8_t>>& sigma) {
    SubstitutionRule rule(sigma);
    const auto spectral = classify_matrix_spectral(incidence(sigma));
    auto subst = make_substitution<3>(rule, spectral.beta);
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
    std::set<ANode<3>> closure_visited(d_cont.begin(), d_cont.end());
    std::vector<ANode<3>> closure_frontier(d_cont.begin(), d_cont.end());
    std::vector<std::set<ANode<3>>> closure_layers;
    while (!closure_frontier.empty()) {
        std::vector<ANode<3>> next;
        for (const auto& node : closure_frontier)
            for (const auto& predecessor : backward_edges<3>(subst, node))
                if (closure_visited.insert(predecessor).second)
                    next.push_back(predecessor);
        if (!next.empty())
            closure_layers.emplace_back(next.begin(), next.end());
        closure_frontier = std::move(next);
    }
    const auto induced = induced_restricted_edges<3>(subst, pre_contact);
    const std::set<ANode<3>> pre_set(
        pre_contact.begin(), pre_contact.end());
    const auto reduced = red_anode<3>(pre_set, induced);
    std::set<SNode<3>> C;
    for (const auto& node : reduced.first) C.insert(to_simple<3>(node));
    const auto expected_pre_contact = class_ii_pre_contact_set();
    const std::set<ANode<3>> seed_set(d_cont.begin(), d_cont.end());
    const auto expected_exclusions = class_ii_contact_red_exclusions();
    bool red_rank_law = true;
    for (const auto& source : pre_set) {
        bool has_surviving_out = false;
        for (const auto& edge : induced) {
            if (!anode_equal(std::get<0>(edge), source)) continue;
            if (expected_exclusions.count(std::get<1>(edge)) == 0) {
                has_surviving_out = true;
                break;
            }
        }
        red_rank_law =
            red_rank_law
            && (has_surviving_out
                == (expected_exclusions.count(source) == 0));
    }
    const std::size_t a = sigma[0].size() - 2;
    const bool backward_layer_law =
        a < 2
        || (closure_layers.size() == 2
            && anode_sets_equal(
                closure_layers[0],
                class_ii_pre_contact_first_backward_layer())
            && anode_sets_equal(
                closure_layers[1],
                class_ii_pre_contact_second_backward_layer()));
    bool backward_witness_law = true;
    if (a >= 2) {
        for (const auto& expected :
             class_ii_pre_contact_backward_witnesses(a)) {
            bool found = false;
            for (const auto& actual :
                 backward_edge_witnesses<3>(
                     subst, expected.destination)) {
                found = found
                    || (anode_equal(
                            actual.predecessor, expected.predecessor)
                        && actual.type == expected.type
                        && actual.first_prefix_position
                            == expected.first_prefix_position
                        && actual.second_prefix_position
                            == expected.second_prefix_position);
            }
            backward_witness_law = backward_witness_law && found;
        }
    }
    return {
        std::move(subst), std::move(C), d_cont.size(),
        pre_contact.size(),
        anode_sets_equal(seed_set, class_ii_d_cont_set()),
        anode_sets_equal(pre_set, expected_pre_contact),
        backward_layer_law,
        backward_witness_law,
        red_rank_law};
}

Matrix adjacency(const Substitution<3>& subst,
                 const std::set<SNode<3>>& nodes) {
    const std::vector<SNode<3>> ordered(nodes.begin(), nodes.end());
    std::map<SNode<3>, std::size_t> index;
    for (std::size_t i = 0; i < ordered.size(); ++i) index[ordered[i]] = i;
    Matrix A(ordered.size(), std::vector<long long>(ordered.size(), 0));
    for (std::size_t source = 0; source < ordered.size(); ++source) {
        for (const auto& [destination, labels] :
             simple_forward_targets<3>(subst, ordered[source])) {
            (void)labels;
            const auto found = index.find(destination);
            if (found != index.end()) ++A[source][found->second];
        }
    }
    return A;
}

Matrix adjacency_exact(const Substitution<3>& subst,
                       const std::set<SNode<3>>& nodes) {
    const std::vector<SNode<3>> ordered(nodes.begin(), nodes.end());
    std::map<SNode<3>, std::size_t> index;
    for (std::size_t i = 0; i < ordered.size(); ++i) index[ordered[i]] = i;
    Matrix A(ordered.size(), std::vector<long long>(ordered.size(), 0));
    for (std::size_t source = 0; source < ordered.size(); ++source) {
        for (const auto& [destination, labels] :
             simple_forward_targets_exact<3>(subst, ordered[source])) {
            (void)labels;
            const auto found = index.find(destination);
            if (found != index.end()) ++A[source][found->second];
        }
    }
    return A;
}

mathlib::PolyZ dominant_core_charpoly(
        const Substitution<3>& subst,
        const std::set<SNode<3>>& nodes,
        std::size_t* core_size) {
    const auto graph = WeightedDigraph::from_dense(adjacency(subst, nodes));
    const auto core = extract_dominant_recurrent_core(graph).first;
    *core_size = core.n;
    Matrix dense(core.n, std::vector<long long>(core.n, 0));
    for (std::size_t u = 0; u < core.n; ++u) {
        for (const auto& [v, weight] : core.out_adj[u]) {
            dense[u][v] += weight;
        }
    }
    return mathlib::charpoly_faddeev_leverrier(dense);
}

mathlib::PolyZ component_charpoly(
        const WeightedDigraph& graph,
        const std::vector<std::size_t>& component) {
    std::map<std::size_t, std::size_t> local;
    for (std::size_t i = 0; i < component.size(); ++i) {
        local[component[i]] = i;
    }
    Matrix dense(
        component.size(), std::vector<long long>(component.size(), 0));
    for (std::size_t i = 0; i < component.size(); ++i) {
        for (const auto& [destination, weight] :
             graph.out_adj[component[i]]) {
            const auto found = local.find(destination);
            if (found != local.end()) dense[i][found->second] += weight;
        }
    }
    return mathlib::charpoly_faddeev_leverrier(dense);
}

void print_layers(const CoronaTrace<3>& trace) {
    for (const auto& layer : trace.layers) {
        if (layer.round == 1) {
            std::printf(" A1=%zu", layer.nodes.size());
        } else {
            std::printf(
                " A%d=%zu(pre=%zu)",
                layer.round, layer.nodes.size(),
                layer.pre_red_nodes.size());
        }
    }
}

void print_poly(const mathlib::PolyZ& polynomial) {
    std::printf("%s", mathlib::str(polynomial).c_str());
}

mathlib::PolyZ expected_dominant_polynomial(std::size_t a) {
    mathlib::PolyZ polynomial;
    polynomial.ensure_size(12);
    mathlib::set_si(polynomial.coeffs_[6],
                    -static_cast<long long>(a * (a + 1)));
    mathlib::set_si(polynomial.coeffs_[9],
                    -static_cast<long long>(a * a));
    mathlib::set_si(polynomial.coeffs_[11], 1);
    polynomial.trim();
    return polynomial;
}

std::vector<mathlib::PolyZ> expected_recurrent_polynomials(
        std::size_t a) {
    std::vector<mathlib::PolyZ> expected;
    if (a >= 2) {
        expected.push_back(mathlib::PolyZ{-1, 0, 1});
        for (std::size_t k = 2; k < a; ++k) {
            expected.push_back(mathlib::PolyZ{
                0, 0, -static_cast<long long>(k * k), 0, 1});
        }
    }
    expected.push_back(expected_dominant_polynomial(a));
    return expected;
}

bool run(std::size_t a) {
    auto built = build_contact(sigma_a1(a));
    const auto symbolic_backward =
        class_ii_contact_backward_envelope_certificate();
    std::size_t raw_backward_branches = 0;
    std::size_t valid_backward_branches = 0;
    std::set<ANode<3>> raw_backward_nodes;
    std::set<ANode<3>> valid_backward_nodes;
    for (const auto& destination : class_ii_pre_contact_set()) {
        const auto raw = backward_edge_witnesses<3>(
            built.subst, destination, false);
        raw_backward_branches += raw.size();
        for (const auto& witness : raw)
            raw_backward_nodes.insert(witness.predecessor);
        const auto valid =
            backward_edge_witnesses<3>(built.subst, destination);
        valid_backward_branches += valid.size();
        for (const auto& witness : valid)
            valid_backward_nodes.insert(witness.predecessor);
    }
    std::set<std::array<long long, 4>> raw_backward_categories;
    for (const auto& node : raw_backward_nodes)
        raw_backward_categories.insert(
            {node.i, node.x[1], node.x[2], node.j});
    const bool backward_category_law =
        raw_backward_categories
            == class_ii_pre_contact_backward_categories();
    std::map<ClassIIBackwardCategory, std::pair<long long, long long>>
        actual_ranges;
    for (const auto& node : raw_backward_nodes) {
        const ClassIIBackwardCategory key{
            node.i, node.x[1], node.x[2], node.j};
        auto [it, inserted] =
            actual_ranges.emplace(key, std::pair{node.x[0], node.x[0]});
        if (!inserted) {
            it->second.first = std::min(it->second.first, node.x[0]);
            it->second.second = std::max(it->second.second, node.x[0]);
        }
    }
    bool backward_range_law = true;
    std::set<ANode<3>> interval_envelope_nodes;
    const auto expected_ranges =
        class_ii_pre_contact_backward_category_ranges();
    backward_range_law =
        actual_ranges.size() == expected_ranges.size();
    for (const auto& [key, form] : expected_ranges) {
        const auto found = actual_ranges.find(key);
        backward_range_law =
            backward_range_law
            && found != actual_ranges.end()
            && found->second.first
                == form.minimum_intercept
                    + form.minimum_slope * static_cast<long long>(a)
            && found->second.second
                == form.maximum_intercept
                    + form.maximum_slope * static_cast<long long>(a);
        const long long minimum =
            form.minimum_intercept
                + form.minimum_slope * static_cast<long long>(a);
        const long long maximum =
            form.maximum_intercept
                + form.maximum_slope * static_cast<long long>(a);
        for (long long x0 = minimum; x0 <= maximum; ++x0)
            interval_envelope_nodes.insert(
                {key[0], {x0, key[1], key[2]}, key[3]});
    }
    std::set<ANode<3>> interval_holes;
    for (const auto& node : interval_envelope_nodes)
        if (raw_backward_nodes.count(node) == 0)
            interval_holes.insert(node);
    const bool interval_hole_law = interval_holes.size() == 1;
    if (a == 8)
        for (const auto& node : interval_holes)
            std::printf(
                "  interval hole: %lld:(%lld,%lld,%lld):%lld\n",
                node.i, node.x[0], node.x[1], node.x[2], node.j);
    const bool backward_count_law =
        raw_backward_branches == 18 * a * a + 72 * a + 68
        && raw_backward_nodes.size() == 38 * a + 54
        && valid_backward_branches == 6 * a + 21
        && valid_backward_nodes.size() == 14;
    auto expected_valid_backward_nodes = class_ii_pre_contact_set();
    for (const auto& excluded : class_ii_contact_red_exclusions())
        expected_valid_backward_nodes.erase(excluded);
    const bool backward_valid_node_law =
        anode_sets_equal(
            valid_backward_nodes, expected_valid_backward_nodes);
    bool face_candidate_law = true;
    if (a == 8) {
        const auto face_candidates = class_ii_d_cont_face_candidates();
        std::set<std::array<long long, 3>> face_vectors;
        for (const auto& node : face_candidates)
            face_vectors.insert(node.x);
        face_candidate_law =
            face_candidates.size() == 33 && face_vectors.size() == 13;
        std::printf(
            "  D_cont face-contact candidates before window: %zu"
            " (%zu vectors): %s\n",
            face_candidates.size(), face_vectors.size(),
            face_candidate_law ? "exact match" : "MISMATCH");
    }
    const bool contact_set_law = built.C == class_ii_contact_set();
    const auto fixed = algorithm2_trace<3>(
        built.subst, built.C,
        CoronaConnectorPolicy::fixed_signed_contact);
    const auto evolving = algorithm2_trace<3>(
        built.subst, built.C,
        CoronaConnectorPolicy::evolving_layer);

    std::size_t fixed_core = 0;
    std::size_t evolving_core = 0;
    const auto fixed_poly = dominant_core_charpoly(
        built.subst, fixed.final_nodes, &fixed_core);
    const auto evolving_poly = dominant_core_charpoly(
        built.subst, evolving.final_nodes, &evolving_core);
    const auto fixed_graph =
        WeightedDigraph::from_dense(adjacency(built.subst, fixed.final_nodes));
    std::size_t recurrent_components = 0;
    std::vector<std::vector<std::size_t>> recurrent;
    for (const auto& component : tarjan_scc(fixed_graph)) {
        if (is_recurrent_scc(fixed_graph, component)) {
            ++recurrent_components;
            recurrent.push_back(component);
        }
    }

    std::printf(
        "a=%zu |D_cont|=%zu |G_P|=%zu |C|=%zu\n  fixed:   ",
        a, built.seed_size, built.pre_contact_size, built.C.size());
    print_layers(fixed);
    std::printf(" final=%zu core=%zu poly=", fixed.final_nodes.size(),
                fixed_core);
    print_poly(fixed_poly);
    std::printf("\n  evolving:");
    print_layers(evolving);
    std::printf(" final=%zu core=%zu poly=", evolving.final_nodes.size(),
                evolving_core);
    print_poly(evolving_poly);
    std::printf("\n  semantic parity: nodes=%s polynomial=%s\n",
                fixed.final_nodes == evolving.final_nodes ? "yes" : "NO",
                fixed_poly == evolving_poly ? "yes" : "NO");
    std::printf("  recurrent SCCs=%zu (selected core=%zu)\n",
                recurrent_components, fixed_core);
    std::printf("  parameter-independent contact set: %s\n",
                contact_set_law ? "exact match" : "MISMATCH");
    std::printf(
        "  backward branch envelope: %zu branches, %zu raw nodes/"
        "%zu categories,"
        " %zu valid branches/%zu valid nodes\n",
        raw_backward_branches, raw_backward_nodes.size(),
        raw_backward_categories.size(),
        valid_backward_branches, valid_backward_nodes.size());
    std::printf("  fixed 51-category backward envelope: %s\n",
                backward_category_law ? "exact match" : "MISMATCH");
    std::printf("  affine x0 category ranges: %s\n",
                backward_range_law ? "exact match" : "MISMATCH");
    std::printf("  single interval-envelope hole: %s\n",
                interval_hole_law ? "exact match" : "MISMATCH");
    std::printf(
        "  symbolic backward envelope: categories=%d ranges=%d"
        " dominance=%d contact-sources=%d Red-absent=%d"
        " window=%zu/%zu restricted=%d"
        " branches=%lld+%llda+%llda^2"
        " valid=%lld+%llda unresolved-valid=%zu"
        " category-span=%lld+%llda exact=%d\n",
        symbolic_backward.categories_exact ? 1 : 0,
        symbolic_backward.affine_ranges_exact ? 1 : 0,
        symbolic_backward.range_dominance_exact ? 1 : 0,
        symbolic_backward.contact_sources_covered ? 1 : 0,
        symbolic_backward.red_exclusions_absent ? 1 : 0,
        symbolic_backward.bounded_window_cases
            - symbolic_backward.unresolved_window_cases,
        symbolic_backward.bounded_window_cases,
        symbolic_backward.restricted_nodes_exact ? 1 : 0,
        symbolic_backward.branch_count_coefficients[0],
        symbolic_backward.branch_count_coefficients[1],
        symbolic_backward.branch_count_coefficients[2],
        symbolic_backward.restricted_branch_count_coefficients[0],
        symbolic_backward.restricted_branch_count_coefficients[1],
        symbolic_backward.unresolved_branch_families,
        symbolic_backward.category_span_coefficients[0],
        symbolic_backward.category_span_coefficients[1],
        symbolic_backward.exact() ? 1 : 0);
    std::printf("  affine backward-envelope counts: %s\n",
                backward_count_law ? "exact match" : "MISMATCH");
    std::printf("  valid predecessors equal contact set: %s\n",
                backward_valid_node_law ? "exact match" : "MISMATCH");
    std::printf("  parameter-independent D_cont set: %s\n",
                built.seed_law ? "exact match" : "MISMATCH");
    std::printf("  parameter-independent pre-contact set: %s\n",
                built.pre_contact_law ? "exact match" : "MISMATCH");
    std::printf("  backward closure layers 9+6+1: %s\n",
                built.backward_layer_law ? "exact match" : "MISMATCH");
    std::printf("  seven affine backward witnesses: %s\n",
                built.backward_witness_law ? "exact match" : "MISMATCH");
    std::printf("  two-state contact Red rank: %s\n",
                built.red_rank_law ? "exact match" : "MISMATCH");
    bool recurrent_polynomial_law = true;
    bool recurrent_state_law = true;
    if (a >= 2) {
        auto unmatched = expected_recurrent_polynomials(a);
        for (const auto& component : recurrent) {
            const auto polynomial =
                component_charpoly(fixed_graph, component);
            auto found = unmatched.end();
            for (auto it = unmatched.begin(); it != unmatched.end(); ++it) {
                if (*it == polynomial) {
                    found = it;
                    break;
                }
            }
            if (found != unmatched.end()) unmatched.erase(found);
        }
        recurrent_polynomial_law = unmatched.empty()
            && recurrent.size()
                == expected_recurrent_polynomials(a).size();

        const std::vector<SNode<3>> ordered(
            fixed.final_nodes.begin(), fixed.final_nodes.end());
        for (std::size_t k = 1; k < a; ++k) {
            const auto expected =
                class_ii_recurrent_shell_component(a, k);
            bool found = false;
            for (const auto& component : recurrent) {
                std::set<SNode<3>> actual;
                for (std::size_t index : component) {
                    actual.insert(ordered[index]);
                }
                if (actual == expected) {
                    found = true;
                    break;
                }
            }
            recurrent_state_law = recurrent_state_law && found;
            recurrent_state_law =
                recurrent_state_law
                && adjacency(built.subst, expected)
                    == class_ii_recurrent_shell_matrix(k);
        }
    }
    std::printf("  recurrent polynomial family: %s\n",
                recurrent_polynomial_law ? "exact match" : "MISMATCH");
    std::printf("  recurrent affine state family: %s\n",
                recurrent_state_law ? "exact match" : "MISMATCH");
    bool layer_law = true;
    bool shell_law = true;
    bool full_affine_decomposition = true;
    if (a >= 2) {
        layer_law =
            built.C.size() == 14
            && fixed.final_nodes.size() == 20 * a + 8
            && fixed.layers.size() == (a == 2 ? 4 : a + 1)
            && fixed.layers[0].nodes.size() == 28
            && fixed.layers[1].nodes.size() == 47;
        for (std::size_t round = 3;
             layer_law && round <= a; ++round) {
            layer_law =
                fixed.layers[round - 1].nodes.size()
                == 20 * round + 8;
        }
        layer_law =
            layer_law
            && fixed.layers[fixed.layers.size() - 2].nodes.size()
                == 20 * a + 8
            && fixed.layers.back().nodes.size() == 20 * a + 8;

        for (std::size_t round = 4; round < a; ++round) {
            std::set<SNode<3>> added;
            const auto& previous = fixed.layers[round - 2].nodes;
            const auto& current = fixed.layers[round - 1].nodes;
            for (const auto& node : current) {
                if (previous.count(node) == 0) added.insert(node);
            }
            shell_law =
                shell_law
                && added == class_ii_interior_shell(round);
        }
        if (a >= 4) {
            std::set<SNode<3>> terminal_added;
            const auto& previous = fixed.layers[a - 2].nodes;
            const auto& terminal = fixed.layers[a - 1].nodes;
            for (const auto& node : terminal) {
                if (previous.count(node) == 0) terminal_added.insert(node);
            }
            shell_law =
                shell_law
                && terminal_added == class_ii_terminal_shell(a);

            if (a == 4) stable_base_a3 = fixed.layers[2].nodes;
            full_affine_decomposition =
                !stable_base_a3.empty()
                && fixed.layers[2].nodes == stable_base_a3;
            auto reconstructed = stable_base_a3;
            for (std::size_t round = 4; round < a; ++round) {
                const auto shell = class_ii_interior_shell(round);
                reconstructed.insert(shell.begin(), shell.end());
            }
            const auto terminal_shell = class_ii_terminal_shell(a);
            reconstructed.insert(
                terminal_shell.begin(), terminal_shell.end());
            full_affine_decomposition =
                full_affine_decomposition
                && reconstructed == fixed.final_nodes;
        }
    }
    std::printf("  fixed-contact Class-II layer law: %s\n",
                layer_law ? "exact match" : "MISMATCH");
    std::printf("  affine interior/terminal shells: %s\n",
                shell_law ? "exact match" : "MISMATCH");
    std::printf("  full 68-base-plus-shell decomposition: %s\n",
                full_affine_decomposition ? "exact match" : "MISMATCH");
    return fixed.converged
        && evolving.converged
        && fixed.final_nodes == evolving.final_nodes
        && backward_category_law
        && backward_range_law
        && interval_hole_law
        && symbolic_backward.exact()
        && backward_count_law
        && backward_valid_node_law
        && built.seed_law
        && face_candidate_law
        && built.pre_contact_law
        && built.backward_layer_law
        && built.backward_witness_law
        && built.red_rank_law
        && contact_set_law
        && fixed_poly == evolving_poly
        && recurrent_components == a
        && recurrent_polynomial_law
        && recurrent_state_law
        && layer_law
        && shell_law
        && full_affine_decomposition;
}

bool exact_shell_transition_sweep(std::size_t max_a) {
    for (std::size_t a = 2; a <= max_a; ++a) {
        const auto sigma = sigma_a1(a);
        SubstitutionRule rule(sigma);
        const auto spectral = classify_matrix_spectral(incidence(sigma));
        const auto subst = make_substitution<3>(rule, spectral.beta);
        for (std::size_t k = 1; k < a; ++k) {
            const auto states =
                class_ii_recurrent_shell_component(a, k);
            if (adjacency_exact(subst, states)
                    != class_ii_recurrent_shell_matrix(k)) {
                std::printf(
                    "exact shell transition mismatch at a=%zu k=%zu\n",
                    a, k);
                return false;
            }
        }
    }
    std::printf(
        "exact recurrent-shell transition sweep: "
        "2<=a<=%zu, all 1<=k<a match\n",
        max_a);
    return true;
}

bool shell_hop_witness_law(
        const Substitution<3>& subst, const std::set<SNode<3>>& C,
        std::size_t round) {
    const auto source = class_ii_interior_shell(round - 1);
    const auto target = class_ii_interior_shell(round);
    const auto hops = build_signed_contact_set<3>(C);
    const std::vector<SNode<3>> sources(source.begin(), source.end());
    const std::vector<SNode<3>> targets(target.begin(), target.end());
    for (std::size_t ti = 0; ti < targets.size(); ++ti) {
        bool found = false;
        for (std::size_t si = 0; si < sources.size() && !found; ++si) {
            if (sources[si].i != targets[ti].i) continue;
            for (const auto& hop : hops) {
                if (sources[si].j != hop.i || hop.j != targets[ti].j)
                    continue;
                bool equal = true;
                for (std::size_t coordinate = 0; coordinate < 3;
                     ++coordinate) {
                    equal = equal
                        && sources[si].x[coordinate] + hop.x[coordinate]
                            == targets[ti].x[coordinate];
                }
                if (equal) {
                    found =
                        same_letter_H<3>(
                            subst, sources[si].x,
                            static_cast<std::size_t>(sources[si].j))
                        && same_letter_H<3>(
                            subst, targets[ti].x,
                            static_cast<std::size_t>(targets[ti].j));
                    break;
                }
            }
        }
        if (!found) return false;
    }
    return true;
}

bool shell_survival_witness_law(
        const Substitution<3>& subst,
        const std::set<SNode<3>>& layer_nodes,
        std::size_t round) {
    const auto shell_set = class_ii_interior_shell(round);
    const std::vector<SNode<3>> shell(
        shell_set.begin(), shell_set.end());
    for (std::size_t source = 0; source < shell.size(); ++source) {
        bool found = false;
        for (const auto& [target, labels] :
             simple_forward_targets_exact<3>(subst, shell[source])) {
            (void)labels;
            if (layer_nodes.count(target) == 0) continue;
            found = true;
            break;
        }
        if (!found) return false;
    }
    return true;
}

}  // namespace

int main() {
    bool ok = true;
    for (std::size_t a = 1; a <= 8; ++a) ok &= run(a);
    auto witness_case = build_contact(sigma_a1(8));
    const auto witness_trace = algorithm2_trace<3>(
        witness_case.subst, witness_case.C,
        CoronaConnectorPolicy::fixed_signed_contact);
    bool hop_witness_law = true;
    for (std::size_t round = 5; round < 8; ++round) {
        hop_witness_law =
            hop_witness_law
            && shell_hop_witness_law(
                witness_case.subst, witness_case.C, round);
    }
    std::printf(
        "interior-shell fixed-contact hop witnesses: %s\n",
        hop_witness_law ? "exact match" : "MISMATCH");
    ok &= hop_witness_law;
    bool survival_witness_law = true;
    for (std::size_t round = 4; round < 8; ++round) {
        survival_witness_law =
            survival_witness_law
            && shell_survival_witness_law(
                witness_case.subst,
                witness_trace.layers[round - 1].nodes, round);
    }
    std::printf(
        "interior-shell exact Red-survival witnesses: %s\n",
        survival_witness_law ? "exact match" : "MISMATCH");
    ok &= survival_witness_law;
    // Exact geometric edge construction is intentionally bounded here:
    // unlike the closed matrix formula, its algebraic-coordinate solves are
    // comparatively expensive.  This remains a regression sweep, not the
    // proof of the parameterized statement.
    ok &= exact_shell_transition_sweep(16);
    return ok ? 0 : 1;
}
