// Research probe for the prefix-labelled contact/return-phase coupling.
// Reports how much of the bare boundary is reachable from each single
// contact fibre, then compares the recurrent lifted and balanced-pair
// characteristic polynomials.

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <map>
#include <tuple>
#include <vector>

#include "math/charpoly.hpp"
#include "math/poly_z.hpp"
#include "ravel/balanced_pair.hpp"
#include "ravel/contact_boundary.hpp"
#include "ravel/d_cont_check.hpp"
#include "ravel/graph_divisor.hpp"
#include "ravel/return_contact_lift.hpp"
#include "ravel/spectral.hpp"
#include "ravel/survey.hpp"

using namespace ravel;

namespace {

using Matrix = std::vector<std::vector<long long>>;

std::vector<std::vector<std::int8_t>> sigma_ab(int a, int b) {
    std::vector<std::vector<std::int8_t>> sigma(3);
    for (int k = 0; k < a; ++k) sigma[0].push_back(0);
    for (int k = 0; k < b; ++k) sigma[0].push_back(1);
    sigma[0].push_back(2);
    for (int k = 0; k < a; ++k) sigma[1].push_back(0);
    sigma[1].push_back(2);
    sigma[2] = {0};
    return sigma;
}

Matrix incidence(const std::vector<std::vector<std::int8_t>>& sigma) {
    Matrix result(sigma.size(), std::vector<long long>(sigma.size(), 0));
    for (std::size_t source = 0; source < sigma.size(); ++source) {
        for (auto letter : sigma[source]) {
            ++result[static_cast<std::size_t>(letter)][source];
        }
    }
    return result;
}

ContactBoundaryReport build_report(
        const std::vector<std::vector<std::int8_t>>& sigma) {
    SubstitutionRule rule(sigma);
    const auto spectral = classify_matrix_spectral(incidence(sigma));
    const auto substitution = make_substitution<3>(rule, spectral.beta);
    const auto candidates = search_D_cont<3>(substitution, 2);
    std::vector<std::tuple<long long, std::vector<long long>, long long>>
        d_cont;
    for (const auto& candidate : candidates) {
        d_cont.emplace_back(
            candidate.i,
            std::vector<long long>(
                candidate.x.begin(), candidate.x.end()),
            candidate.j);
    }
    return compute_contact_boundary_dispatch(
        rule, spectral.beta, spectral.b2, d_cont);
}

std::vector<SNode<3>> nodes_of(const ContactBoundaryReport& report) {
    std::vector<SNode<3>> nodes;
    for (const auto& [i, x, j] : report.boundary_nodes) {
        SNode<3> node{};
        node.i = i;
        node.j = j;
        std::copy_n(x.begin(), 3, node.x.begin());
        nodes.push_back(node);
    }
    return nodes;
}

struct CoreData {
    Matrix matrix;
    std::vector<std::size_t> original_indices;
};

CoreData recurrent_core_data(const Matrix& matrix) {
    const auto graph = WeightedDigraph::from_dense(matrix);
    const auto [core, original] = extract_dominant_recurrent_core(graph);
    Matrix result(core.n, std::vector<long long>(core.n, 0));
    for (std::size_t source = 0; source < core.n; ++source) {
        for (const auto& [destination, weight] : core.out_adj[source]) {
            result[source][destination] += weight;
        }
    }
    return {std::move(result), original};
}

Matrix recurrent_core(const Matrix& matrix) {
    return recurrent_core_data(matrix).matrix;
}

mathlib::PolyZ polynomial(const std::vector<long long>& coefficients) {
    mathlib::PolyZ result;
    result.ensure_size(coefficients.size());
    for (std::size_t i = 0; i < coefficients.size(); ++i) {
        mathlib::set_si(result.coeff(i), coefficients[i]);
    }
    return result;
}

void print_coefficients(
        const char* label, const std::vector<long long>& coefficients) {
    std::printf("  %s=[", label);
    for (std::size_t i = 0; i < coefficients.size(); ++i) {
        std::printf("%s%lld", i == 0 ? "" : ",", coefficients[i]);
    }
    std::printf("]\n");
}

void print_word(const std::vector<std::int8_t>& word) {
    for (auto letter : word) {
        std::printf("%d", static_cast<int>(letter));
    }
}

std::vector<long long> remove_zero_root_factor(
        std::vector<long long> coefficients) {
    while (coefficients.size() > 1 && coefficients.back() == 0) {
        coefficients.pop_back();
    }
    return coefficients;
}

bool run_case(
        const char* name,
        const std::vector<std::vector<std::int8_t>>& sigma) {
    const auto report = build_report(sigma);
    const auto nodes = nodes_of(report);
    const SubstitutionRule rule(sigma);
    const auto substitution = make_substitution<3>(rule, report.beta);
    const auto phases = build_return_phase_system(rule, 0);

    std::vector<ReturnContactState<3>> all_seeds;
    std::size_t best_projection = 0;
    std::size_t best_states = 0;
    std::size_t best_seed = 0;
    for (std::size_t seed = 0; seed < nodes.size(); ++seed) {
        auto fibre = compatible_return_contact_seeds(nodes[seed], phases);
        all_seeds.insert(all_seeds.end(), fibre.begin(), fibre.end());
        const auto component = build_reachable_return_contact_lift(
            substitution, nodes, phases, fibre);
        if (!component.complete()) return false;
        if (component.projected_node_count > best_projection) {
            best_projection = component.projected_node_count;
            best_states = component.states.size();
            best_seed = seed;
        }
    }

    const auto lift = build_reachable_return_contact_lift(
        substitution, nodes, phases, all_seeds);
    if (!lift.complete()) return false;
    Matrix lifted_matrix(
        lift.states.size(),
        std::vector<long long>(lift.states.size(), 0));
    for (const auto& edge : lift.edges) {
        ++lifted_matrix[edge.source][edge.destination];
    }
    const auto lifted_core = recurrent_core(lifted_matrix);
    const auto bare_core = recurrent_core(report.gb_matrix);
    const auto bp = balanced_pair_transition_graph(rule);
    const auto bp_data = recurrent_core_data(bp.matrix);
    const auto& bp_core = bp_data.matrix;
    const auto lifted_charpoly = charpoly_int(lifted_core);
    const auto bare_charpoly = charpoly_int(bare_core);
    const auto bp_charpoly = charpoly_int(bp_core);
    const auto common = mathlib::gcd(
        polynomial(lifted_charpoly), polynomial(bp_charpoly));
    const bool only_zero_modes_added =
        remove_zero_root_factor(lifted_charpoly)
            == remove_zero_root_factor(bare_charpoly);

    std::printf(
        "%s: bare=%zu phases=%zu full-lift=%zu/%zu core=%zu; "
        "best single fibre #%zu reaches %zu bare nodes in %zu states; "
        "BP core=%zu gcd-degree=%lld; lift adds only zero modes=%s\n",
        name, nodes.size(), phases.states.size(),
        lift.states.size(), lift.edges.size(), lifted_core.size(),
        best_seed, best_projection, best_states,
        bp_core.size(), common.degree(),
        only_zero_modes_added ? "yes" : "NO");
    print_coefficients("lifted-core charpoly high-first", lifted_charpoly);
    print_coefficients("bare-core charpoly high-first", bare_charpoly);
    print_coefficients("BP-core charpoly high-first", bp_charpoly);

    using EndpointSignature =
        std::tuple<std::int8_t, std::int8_t,
                   std::int8_t, std::int8_t, std::size_t>;
    std::map<EndpointSignature, std::size_t> signatures;
    std::map<std::tuple<std::int8_t, std::int8_t,
                        std::int8_t, std::int8_t>, std::size_t>
        endpoint_only;
    std::size_t max_word_length = 0;
    for (std::size_t original : bp_data.original_indices) {
        const auto& state = bp.states.at(original);
        max_word_length = std::max(max_word_length, state.left.size());
        ++signatures[{
            state.left.front(), state.right.front(),
            state.left.back(), state.right.back(), state.left.size()}];
        ++endpoint_only[{
            state.left.front(), state.right.front(),
            state.left.back(), state.right.back()}];
    }
    std::size_t colliding_states = 0;
    std::size_t collision_classes = 0;
    for (const auto& [signature, count] : signatures) {
        (void)signature;
        if (count > 1) {
            ++collision_classes;
            colliding_states += count;
        }
    }
    std::printf(
        "  BP word semantics: max length=%zu, endpoint+length "
        "signatures=%zu/%zu, collision classes=%zu (%zu states)\n",
        max_word_length, signatures.size(), bp_core.size(),
        collision_classes, colliding_states);
    std::size_t endpoint_collision_classes = 0;
    std::size_t endpoint_colliding_states = 0;
    for (const auto& [signature, count] : endpoint_only) {
        (void)signature;
        if (count > 1) {
            ++endpoint_collision_classes;
            endpoint_colliding_states += count;
        }
    }
    std::printf(
        "  without length: signatures=%zu/%zu, collision classes=%zu "
        "(%zu states)\n",
        endpoint_only.size(), bp_core.size(),
        endpoint_collision_classes, endpoint_colliding_states);
    std::printf("  BP recurrent words:");
    for (std::size_t original : bp_data.original_indices) {
        const auto& state = bp.states.at(original);
        std::printf(" (");
        print_word(state.left);
        std::printf("|");
        print_word(state.right);
        std::printf(")");
    }
    std::printf("\n");
    return report.converged
        && lift.projected_node_count == nodes.size()
        && lift.projected_edge_count > 0
        && only_zero_modes_added;
}

}  // namespace

int main() {
    std::printf("=== Sparse contact/return-phase lift ===\n");
    bool ok = true;
    ok &= run_case("Tribonacci control", {{0, 1}, {0, 2}, {0}});
    ok &= run_case("Class II sigma_{1,1}", sigma_ab(1, 1));
    ok &= run_case("Class II sigma_{2,1}", sigma_ab(2, 1));
    ok &= run_case("Class II sigma_{3,1}", sigma_ab(3, 1));
    ok &= run_case("Class II sigma_{4,1}", sigma_ab(4, 1));
    return ok ? 0 : 1;
}
