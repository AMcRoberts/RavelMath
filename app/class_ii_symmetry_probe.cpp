// Exact automorphism probe for Thread A3 (Class-II Pisot substitutions).
//
// The n-bonacci proof uses a free Z/2 automorphism of the dominant
// recurrent contact-boundary core.  For sigma_{1,1}, the familiar mirror
// map is only partial.  This driver asks the more basic, formula-free
// question: does the weighted directed core have any nontrivial
// automorphism at all?
//
// Automorphisms are enumerated exactly.  A directed, weighted
// Weisfeiler-Leman refinement first partitions vertices; a backtracking
// search then tries only color-compatible images and verifies every
// adjacency entry.  Tribonacci is included as a positive control.

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <functional>
#include <map>
#include <numeric>
#include <set>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "math/charpoly.hpp"
#include "math/poly_z.hpp"
#include "ravel/balanced_pair.hpp"
#include "ravel/contact_boundary.hpp"
#include "ravel/d_cont_check.hpp"
#include "ravel/graph_divisor.hpp"
#include "ravel/return_substitution.hpp"
#include "ravel/spectral.hpp"
#include "ravel/substitution.hpp"
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

std::vector<std::vector<long long>> incidence(
        const std::vector<std::vector<std::int8_t>>& sigma) {
    Matrix M(sigma.size(), std::vector<long long>(sigma.size(), 0));
    for (std::size_t j = 0; j < sigma.size(); ++j) {
        for (std::int8_t letter : sigma[j]) {
            ++M[static_cast<std::size_t>(letter)][j];
        }
    }
    return M;
}

ContactBoundaryReport build_report(
        const std::vector<std::vector<std::int8_t>>& sigma) {
    SubstitutionRule rule(sigma);
    auto spectral = classify_matrix_spectral(incidence(sigma));
    auto subst = make_substitution<3>(rule, spectral.beta);
    auto candidates = search_D_cont<3>(subst, 2);
    std::vector<std::tuple<long long, std::vector<long long>, long long>>
        d_cont;
    for (const auto& c : candidates) {
        d_cont.emplace_back(
            c.i, std::vector<long long>(c.x.begin(), c.x.end()), c.j);
    }
    return compute_contact_boundary_dispatch(
        rule, spectral.beta, spectral.b2, d_cont);
}

Matrix dominant_core(const Matrix& full) {
    auto graph = WeightedDigraph::from_dense(full);
    auto [core, original_indices] =
        extract_dominant_recurrent_core(graph);
    (void)original_indices;
    Matrix out(core.n, std::vector<long long>(core.n, 0));
    for (std::size_t u = 0; u < core.n; ++u) {
        for (const auto& [v, weight] : core.out_adj[u]) {
            out[u][v] += weight;
        }
    }
    return out;
}

Matrix expected_class_ii_core_matrix(std::size_t a) {
    Matrix matrix(11, std::vector<long long>(11, 0));
    matrix[0][1] = 1;
    matrix[1][10] = 1;
    matrix[2][10] = 1;
    matrix[3][2] = 1;
    matrix[4][5] = 1;
    matrix[4][7] = 1;
    matrix[5][4] = 1;
    matrix[5][6] = 1;
    matrix[6][3] = static_cast<long long>(a);
    matrix[6][5] = static_cast<long long>(a - 1);
    matrix[6][7] = static_cast<long long>(a - 1);
    matrix[7][1] = static_cast<long long>(a);
    matrix[7][4] = static_cast<long long>(a - 1);
    matrix[7][6] = static_cast<long long>(a - 1);
    matrix[8][0] = 1;
    matrix[8][3] = static_cast<long long>(a - 1);
    matrix[8][5] = static_cast<long long>(a);
    matrix[8][7] = static_cast<long long>(a);
    matrix[9][8] = 1;
    matrix[10][9] = 1;
    return matrix;
}

Matrix direct_transition_matrix(
        const ContactBoundaryReport& report,
        const std::vector<std::vector<std::int8_t>>& sigma) {
    SubstitutionRule rule(sigma);
    auto subst = make_substitution<3>(rule, report.beta);
    std::vector<SNode<3>> nodes(report.boundary_nodes.size());
    std::map<SNode<3>, std::size_t> index;
    for (std::size_t u = 0; u < report.boundary_nodes.size(); ++u) {
        nodes[u].i = std::get<0>(report.boundary_nodes[u]);
        nodes[u].j = std::get<2>(report.boundary_nodes[u]);
        const auto& x = std::get<1>(report.boundary_nodes[u]);
        for (std::size_t k = 0; k < 3 && k < x.size(); ++k) {
            nodes[u].x[k] = x[k];
        }
        index[nodes[u]] = u;
    }
    Matrix direct(nodes.size(), std::vector<long long>(nodes.size(), 0));
    for (std::size_t u = 0; u < nodes.size(); ++u) {
        for (const auto& [destination, labels] :
             simple_forward_targets<3>(subst, nodes[u])) {
            (void)labels;
            auto it = index.find(destination);
            if (it != index.end()) ++direct[u][it->second];
        }
    }
    return direct;
}

std::vector<SNode<3>> report_nodes(const ContactBoundaryReport& report) {
    std::vector<SNode<3>> nodes(report.boundary_nodes.size());
    for (std::size_t u = 0; u < report.boundary_nodes.size(); ++u) {
        nodes[u].i = std::get<0>(report.boundary_nodes[u]);
        nodes[u].j = std::get<2>(report.boundary_nodes[u]);
        const auto& x = std::get<1>(report.boundary_nodes[u]);
        for (std::size_t k = 0; k < 3 && k < x.size(); ++k) {
            nodes[u].x[k] = x[k];
        }
    }
    return nodes;
}

std::vector<int> refine_colors(const Matrix& A) {
    const std::size_t n = A.size();
    std::vector<int> colors(n, 0);
    for (;;) {
        std::vector<std::string> signatures(n);
        for (std::size_t u = 0; u < n; ++u) {
            std::map<std::pair<int, long long>, int> outgoing;
            std::map<std::pair<int, long long>, int> incoming;
            for (std::size_t v = 0; v < n; ++v) {
                if (A[u][v] != 0) ++outgoing[{colors[v], A[u][v]}];
                if (A[v][u] != 0) ++incoming[{colors[v], A[v][u]}];
            }
            std::string s = std::to_string(colors[u]) + "|o";
            for (const auto& [key, count] : outgoing) {
                s += ":" + std::to_string(key.first) + ","
                   + std::to_string(key.second) + ","
                   + std::to_string(count);
            }
            s += "|i";
            for (const auto& [key, count] : incoming) {
                s += ":" + std::to_string(key.first) + ","
                   + std::to_string(key.second) + ","
                   + std::to_string(count);
            }
            signatures[u] = std::move(s);
        }
        std::map<std::string, int> ids;
        std::vector<int> next(n);
        for (std::size_t u = 0; u < n; ++u) {
            auto [it, inserted] =
                ids.emplace(signatures[u], static_cast<int>(ids.size()));
            (void)inserted;
            next[u] = it->second;
        }
        if (next == colors) return colors;
        colors = std::move(next);
    }
}

struct AutomorphismSummary {
    std::size_t count = 0;
    std::size_t nonidentity = 0;
    std::map<std::string, std::size_t> cycle_types;
};

std::string cycle_type(const std::vector<int>& permutation) {
    std::vector<bool> seen(permutation.size(), false);
    std::vector<int> lengths;
    for (std::size_t start = 0; start < permutation.size(); ++start) {
        if (seen[start]) continue;
        int length = 0;
        std::size_t u = start;
        do {
            seen[u] = true;
            u = static_cast<std::size_t>(permutation[u]);
            ++length;
        } while (!seen[u]);
        lengths.push_back(length);
    }
    std::sort(lengths.begin(), lengths.end());
    std::string result;
    for (int length : lengths) {
        if (!result.empty()) result += "+";
        result += std::to_string(length);
    }
    return result;
}

AutomorphismSummary enumerate_automorphisms(const Matrix& A) {
    const std::size_t n = A.size();
    const auto colors = refine_colors(A);
    std::vector<std::vector<int>> candidates(n);
    for (std::size_t u = 0; u < n; ++u) {
        for (std::size_t v = 0; v < n; ++v) {
            if (colors[u] == colors[v]) {
                candidates[u].push_back(static_cast<int>(v));
            }
        }
    }

    std::vector<int> order(n);
    std::iota(order.begin(), order.end(), 0);
    std::stable_sort(order.begin(), order.end(), [&](int a, int b) {
        return candidates[a].size() < candidates[b].size();
    });

    std::vector<int> image(n, -1);
    std::vector<bool> used(n, false);
    AutomorphismSummary summary;
    std::function<void(std::size_t)> search = [&](std::size_t depth) {
        if (depth == n) {
            ++summary.count;
            bool identity = true;
            for (std::size_t u = 0; u < n; ++u) {
                if (image[u] != static_cast<int>(u)) {
                    identity = false;
                    break;
                }
            }
            if (!identity) ++summary.nonidentity;
            ++summary.cycle_types[cycle_type(image)];
            return;
        }
        const int u = order[depth];
        for (int v : candidates[static_cast<std::size_t>(u)]) {
            if (used[static_cast<std::size_t>(v)]) continue;
            bool compatible = A[static_cast<std::size_t>(u)]
                               [static_cast<std::size_t>(u)]
                           == A[static_cast<std::size_t>(v)]
                               [static_cast<std::size_t>(v)];
            for (std::size_t w = 0; compatible && w < n; ++w) {
                if (image[w] < 0) continue;
                compatible =
                    A[static_cast<std::size_t>(u)][w]
                        == A[static_cast<std::size_t>(v)]
                            [static_cast<std::size_t>(image[w])]
                    && A[w][static_cast<std::size_t>(u)]
                        == A[static_cast<std::size_t>(image[w])]
                            [static_cast<std::size_t>(v)];
            }
            if (!compatible) continue;
            image[static_cast<std::size_t>(u)] = v;
            used[static_cast<std::size_t>(v)] = true;
            search(depth + 1);
            used[static_cast<std::size_t>(v)] = false;
            image[static_cast<std::size_t>(u)] = -1;
        }
    };
    search(0);
    return summary;
}

std::vector<int> outgoing_equitable_colors(const Matrix& A) {
    const std::size_t n = A.size();
    std::vector<int> colors(n, 0);
    for (;;) {
        std::vector<std::string> signatures(n);
        for (std::size_t u = 0; u < n; ++u) {
            std::map<int, long long> weights;
            for (std::size_t v = 0; v < n; ++v) {
                weights[colors[v]] += A[u][v];
            }
            std::string signature = std::to_string(colors[u]);
            for (const auto& [color, weight] : weights) {
                signature += "|" + std::to_string(color) + ":"
                           + std::to_string(weight);
            }
            signatures[u] = std::move(signature);
        }
        std::map<std::string, int> ids;
        std::vector<int> next(n);
        for (std::size_t u = 0; u < n; ++u) {
            auto [it, inserted] =
                ids.emplace(signatures[u], static_cast<int>(ids.size()));
            (void)inserted;
            next[u] = it->second;
        }
        if (next == colors) return colors;
        colors = std::move(next);
    }
}

Matrix outgoing_quotient(const Matrix& A, const std::vector<int>& colors) {
    int class_count = colors.empty()
        ? 0 : *std::max_element(colors.begin(), colors.end()) + 1;
    Matrix quotient(
        static_cast<std::size_t>(class_count),
        std::vector<long long>(static_cast<std::size_t>(class_count), 0));
    std::vector<int> representative(
        static_cast<std::size_t>(class_count), -1);
    for (std::size_t u = 0; u < colors.size(); ++u) {
        if (representative[static_cast<std::size_t>(colors[u])] < 0) {
            representative[static_cast<std::size_t>(colors[u])] =
                static_cast<int>(u);
        }
    }
    for (int color = 0; color < class_count; ++color) {
        std::size_t u = static_cast<std::size_t>(
            representative[static_cast<std::size_t>(color)]);
        for (std::size_t v = 0; v < A.size(); ++v) {
            quotient[static_cast<std::size_t>(color)]
                    [static_cast<std::size_t>(colors[v])] += A[u][v];
        }
    }
    return quotient;
}

mathlib::PolyZ polynomial(const std::vector<long long>& low_first) {
    mathlib::PolyZ result;
    result.ensure_size(low_first.size());
    for (std::size_t i = 0; i < low_first.size(); ++i) {
        mathlib::set_si(result.coeff(i), low_first[i]);
    }
    return result;
}

Matrix balanced_pair_core(
        const std::vector<std::vector<std::int8_t>>& sigma) {
    SubstitutionRule rule(sigma);
    return dominant_core(balanced_pair_transition_graph(rule).matrix);
}

long long report_equitable_overlap(
        const Matrix& gb_core,
        const std::vector<std::vector<std::int8_t>>& sigma) {
    auto bp_core = balanced_pair_core(sigma);
    auto gb_colors = outgoing_equitable_colors(gb_core);
    auto bp_colors = outgoing_equitable_colors(bp_core);
    auto gb_quotient = outgoing_quotient(gb_core, gb_colors);
    auto bp_quotient = outgoing_quotient(bp_core, bp_colors);
    const auto gb_coefficients = charpoly_int(gb_quotient);
    const auto bp_coefficients = charpoly_int(bp_quotient);
    auto gb_polynomial = polynomial(gb_coefficients);
    auto bp_polynomial = polynomial(bp_coefficients);
    auto common = mathlib::gcd(gb_polynomial, bp_polynomial);
    auto phase = build_return_phase_system(
        SubstitutionRule(sigma), /*marker=*/0);
    auto phase_polynomial =
        polynomial(charpoly_int(phase.incidence_matrix()));
    auto phase_gb_common = mathlib::gcd(
        phase_polynomial, gb_polynomial);
    auto phase_bp_common = mathlib::gcd(
        phase_polynomial, bp_polynomial);
    std::printf(
        "    outgoing equitable quotients: G_B %zu->%zu, BP %zu->%zu; "
        "charpoly gcd degree=%lld\n",
        gb_core.size(), gb_quotient.size(),
        bp_core.size(), bp_quotient.size(), common.degree());
    std::printf(
        "    return phase tower: %zu states; gcd degrees with "
        "G_B quotient=%lld, BP quotient=%lld\n",
        phase.states.size(),
        phase_gb_common.degree(), phase_bp_common.degree());
    std::printf("    G_B quotient charpoly high-first: [");
    for (std::size_t i = 0; i < gb_coefficients.size(); ++i) {
        std::printf("%s%lld", i ? "," : "", gb_coefficients[i]);
    }
    std::printf("]\n    BP quotient charpoly high-first: [");
    for (std::size_t i = 0; i < bp_coefficients.size(); ++i) {
        std::printf("%s%lld", i ? "," : "", bp_coefficients[i]);
    }
    std::printf("]\n");
    return common.degree();
}

bool run_case(const char* name,
              const std::vector<std::vector<std::int8_t>>& sigma,
              std::size_t expected_min_automorphisms,
              long long expected_gcd_degree) {
    auto report = build_report(sigma);
    std::printf("%s: |G_B|=%zu\n", name, report.boundary_size);
    std::size_t minimum_count = static_cast<std::size_t>(-1);
    long long gcd_degree = -1;
    const std::vector<std::pair<const char*, Matrix>> constructions = {
        {"report adjacency", dominant_core(report.gb_matrix)},
        {"direct transitions",
         dominant_core(direct_transition_matrix(report, sigma))},
    };
    for (const auto& [construction, core] : constructions) {
        auto colors = refine_colors(core);
        std::set<int> color_classes(colors.begin(), colors.end());
        auto summary = enumerate_automorphisms(core);
        minimum_count = std::min(minimum_count, summary.count);
        std::printf(
            "  %s: core=%zu, refined colors=%zu, "
            "automorphisms=%zu, nonidentity=%zu\n",
            construction, core.size(), color_classes.size(),
            summary.count, summary.nonidentity);
        std::printf("    cycle types:");
        for (const auto& [type, count] : summary.cycle_types) {
            std::printf(" %s(x%zu)", type.c_str(), count);
        }
        std::printf("\n");
        if (std::string(construction) == "direct transitions") {
            gcd_degree = report_equitable_overlap(core, sigma);
        }
    }
    const auto direct = direct_transition_matrix(report, sigma);
    const auto graph = WeightedDigraph::from_dense(direct);
    const auto [direct_core, original] =
        extract_dominant_recurrent_core(graph);
    bool family_matrix_match = true;
    if (direct_core.n == 11) {
        const auto nodes = report_nodes(report);
        std::printf("  direct core triples:");
        for (std::size_t index : original) {
            const auto& node = nodes[index];
            std::printf(
                " [%lld,(%lld,%lld,%lld),%lld]",
                node.i, node.x[0], node.x[1], node.x[2], node.j);
        }
        std::printf("\n");
        std::size_t a = 0;
        while (a < sigma[0].size() && sigma[0][a] == 0) ++a;
        Matrix direct_core_matrix(
            direct_core.n, std::vector<long long>(direct_core.n, 0));
        for (std::size_t source = 0; source < direct_core.n; ++source) {
            for (const auto& [destination, weight] :
                 direct_core.out_adj[source]) {
                direct_core_matrix[source][destination] += weight;
            }
        }
        family_matrix_match =
            a >= 2
            && direct_core_matrix == expected_class_ii_core_matrix(a);
        std::printf(
            "  explicit 11-state family matrix: %s\n",
            family_matrix_match ? "exact match" : "MISMATCH");
        if (a == 2 || a == 3) {
            std::printf("  direct core matrix a=%zu:\n", a);
            for (std::size_t source = 0; source < direct_core.n;
                 ++source) {
                std::vector<long long> row(direct_core.n, 0);
                for (const auto& [destination, weight] :
                     direct_core.out_adj[source]) {
                    row[destination] += weight;
                }
                std::printf("    ");
                for (long long value : row) {
                    std::printf("%lld ", value);
                }
                std::printf("\n");
            }
        }
    }
    return report.converged
        && minimum_count >= expected_min_automorphisms
        && gcd_degree == expected_gcd_degree
        && family_matrix_match;
}

}  // namespace

int main() {
    std::printf("=== Thread A3: exact dominant-core automorphisms ===\n");
    bool ok = true;
    ok &= run_case(
        "Tribonacci control",
        {{0, 1}, {0, 2}, {0}},
        /*expected_min_automorphisms=*/2,
        /*expected_gcd_degree=*/4);
    ok &= run_case(
        "Class II sigma_{1,1}",
        sigma_ab(1, 1),
        /*expected_min_automorphisms=*/1,
        /*expected_gcd_degree=*/0);
    ok &= run_case(
        "Class II sigma_{2,1}",
        sigma_ab(2, 1),
        /*expected_min_automorphisms=*/1,
        /*expected_gcd_degree=*/0);
    ok &= run_case(
        "Class II sigma_{3,1}",
        sigma_ab(3, 1),
        /*expected_min_automorphisms=*/1,
        /*expected_gcd_degree=*/0);
    ok &= run_case(
        "Class II sigma_{4,1}",
        sigma_ab(4, 1),
        /*expected_min_automorphisms=*/1,
        /*expected_gcd_degree=*/0);
    ok &= run_case(
        "Class II sigma_{5,1}",
        sigma_ab(5, 1),
        /*expected_min_automorphisms=*/1,
        /*expected_gcd_degree=*/0);
    ok &= run_case(
        "Class II sigma_{6,1}",
        sigma_ab(6, 1),
        /*expected_min_automorphisms=*/1,
        /*expected_gcd_degree=*/0);
    return ok ? 0 : 1;
}
