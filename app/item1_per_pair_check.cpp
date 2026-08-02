// item1_per_pair_check.cpp
//
// Tests the REDUCTION step of item 1's global-worst-margin claim
// (docs/DIRECTION_AND_OPEN_THREADS.md thread A item (1); see
// docs/RESEARCH_STATUS.md for how this was found). Groups every node of
// G_B's dominant recurrent core by its (i,j) face pair and finds each
// pair's actual max |t| (t = <x,v>), instead of just the single global
// worst node that item1_margin_trend_check.cpp reports.
//
// Two closed forms, found by inspecting the argmax x-vector per row:
//   - row i=0, column j in [1,n-2]: argmax x = -e_0 + e_j - e_{j+1},
//     giving margin(0,j) = (2-beta)*(1-beta^(j-n))/(beta-1), which is
//     an ALGEBRAIC IDENTITY (checked below): strictly decreasing in j.
//   - row i>=1 (any j>i): the SAME vector x = -e_i + e_{n-1} recurs
//     across the WHOLE row (not just at j=n-1), because
//     is_valid_simple_node's OR clause "[-x,i] in H_sigma" doesn't
//     depend on j at all -- once that disjunct holds for one j it
//     holds for all of them. margin(i,j) = v_j - (v_i - v_{n-1}),
//     also strictly decreasing in j.
// Chaining both: each row's own worst is at j=n-1, giving
// margin(i,n-1) = 2*v_{n-1} - v_i (row i>=1) or 2/beta-1 (row 0,
// i.e. the i=0 case of the same formula since v_0=1) -- and since
// v_i <= v_0 = 1 for every i, this is minimized at i=0. So IF these
// two closed forms really are each row's maximum (verified here,
// exactly, for n=3..7; not derived from the corona construction for
// general n), the global worst node is provably (i,j)=(0,n-1) /
// (n-1,0), matching item1_margin_trend_check.cpp's empirical finding.
//
// This driver is now an exact certificate, not merely a floating report.
// It reconstructs the LEFT Perron covector in Q(beta), verifies its
// sidedness, normalizes v_0=1, maximizes |<x,v>| by exact sign comparisons,
// requires exact equality with every row formula, and requires every residual
// margin to be strictly positive. It also independently reconstructs every
// dominant-core forward transition with exact rational linear algebra and
// exact window validity, requiring record-for-record agreement with the fast
// path. The doubles remain only as diagnostics.

#include <cmath>
#include <cstdio>
#include <map>
#include <set>
#include <vector>

#include "ravel/ambient_graph.hpp"
#include "ravel/balanced_pair.hpp"
#include "ravel/contact_boundary.hpp"
#include "ravel/core.hpp"
#include "ravel/corona.hpp"
#include "ravel/d_cont_check.hpp"
#include "ravel/faces.hpp"
#include "ravel/graph_divisor.hpp"
#include "ravel/substitution.hpp"
#include "math/charpoly.hpp"
#include "math/in_h_sigma.hpp"
#include "math/linalg_qbeta.hpp"
#include "math/sturm.hpp"

using namespace ravel;

namespace {

mathlib::QElem qpow(const mathlib::QBetaRing& R,
                    const mathlib::QElem& x,
                    std::size_t exponent) {
    mathlib::QElem result = R.one();
    for (std::size_t k = 0; k < exponent; ++k) result = R.mul(result, x);
    return result;
}

template <std::size_t D>
std::vector<long long> as_vector(const IVec<D>& x) {
    return std::vector<long long>(x.begin(), x.end());
}

bool equal_up_to_sign(const std::vector<long long>& x,
                      const std::vector<long long>& expected) {
    if (x.size() != expected.size()) return false;
    bool same = true;
    bool opposite = true;
    for (std::size_t k = 0; k < x.size(); ++k) {
        same = same && x[k] == expected[k];
        opposite = opposite && x[k] == -expected[k];
    }
    return same || opposite;
}

template <std::size_t D>
bool exact_simple_validity(const Substitution<D>& subst, const SNode<D>& node) {
    bool trivial = true;
    for (std::size_t k = 0; k < D; ++k)
        if (node.x[k] != 0) trivial = false;
    if (trivial && node.i == node.j) return false;
    const SNode<D> mirror = node.mirror();
    return subst.in_H_sigma_exact(node.x, static_cast<std::size_t>(node.j)) ||
           subst.in_H_sigma_exact(mirror.x,
                                  static_cast<std::size_t>(mirror.j));
}

template <std::size_t D>
using TransitionRecord =
    std::tuple<SNode<D>, std::vector<long long>, std::vector<long long>>;

template <std::size_t D>
std::multiset<TransitionRecord<D>> exact_forward_records(
    const Substitution<D>& subst, const SNode<D>& node) {
    std::multiset<TransitionRecord<D>> out;
    const auto parents_i = parent_decompositions<D>(subst.images, node.i);
    const auto parents_j = parent_decompositions<D>(subst.images, node.j);
    for (const auto& pd : parents_i) {
        const auto lp1 = abelianization<D>(pd.p);
        for (const auto& qd : parents_j) {
            const auto lq1 = abelianization<D>(qd.p);
            std::array<long long, D> rhs{};
            for (std::size_t k = 0; k < D; ++k)
                rhs[k] = node.x[k] + lq1[k] - lp1[k];
            const auto xprime = solve_Mx_eq_rhs_exact<D>(subst.M, rhs);
            if (!xprime.has_value()) continue;
            SNode<D> candidate{pd.parent_letter, *xprime, qd.parent_letter};
            if (!exact_simple_validity(subst, candidate)) continue;
            out.insert({candidate, pd.p, qd.p});
        }
    }
    return out;
}

template <std::size_t D>
std::multiset<TransitionRecord<D>> fast_forward_records(
    const Substitution<D>& subst, const SNode<D>& node) {
    std::multiset<TransitionRecord<D>> out;
    for (const auto& [destination, pq] : simple_forward_targets<D>(subst, node))
        out.insert({destination, pq.first, pq.second});
    return out;
}

std::vector<std::vector<std::int8_t>> n_bonacci_rule(std::size_t n) {
    std::vector<std::vector<std::int8_t>> sigma(n);
    for (std::size_t i = 0; i + 1 < n; ++i) sigma[i] = {0, static_cast<std::int8_t>(i + 1)};
    sigma[n - 1] = {0};
    return sigma;
}

template <std::size_t D>
bool check(std::size_t n, double beta) {
    auto sigma = n_bonacci_rule(n);
    SubstitutionRule rule(sigma);
    auto subst = make_substitution<D>(rule, beta);

    const auto M = rule.matrix();
    const mathlib::PolyZ cp = mathlib::charpoly_faddeev_leverrier(M);
    const mathlib::QBetaRing R(cp);
    const mathlib::RootInterval beta_interval = mathlib::isolate_beta(R);
    auto exact_eigen = mathlib::left_eigenvector_via_qbeta(M, R);
    if (!exact_eigen.ok ||
        !mathlib::verify_left_eigenvector(exact_eigen.v, M, R)) {
        std::printf("n=%zu: exact LEFT Perron covector construction FAILED\n", n);
        return false;
    }
    const auto v0_inverse = mathlib::invert_in_qbeta(exact_eigen.v[0], R);
    if (!v0_inverse.invertible) {
        std::printf("n=%zu: exact Perron normalization FAILED\n", n);
        return false;
    }
    mathlib::QBetaVec exact_v;
    exact_v.reserve(D);
    for (const auto& coordinate : exact_eigen.v)
        exact_v.push_back(R.mul(coordinate, v0_inverse.inverse));
    if (!(exact_v[0] == R.one()) ||
        !mathlib::verify_left_eigenvector(exact_v, M, R)) {
        std::printf("n=%zu: normalized LEFT Perron covector verification FAILED\n", n);
        return false;
    }
    const mathlib::QElem beta_inverse = R.beta_inverse();
    if (!(R.mul(R.beta_k(1), beta_inverse) == R.one())) {
        std::printf("n=%zu: exact beta inverse verification FAILED\n", n);
        return false;
    }

    auto d_cont_cands = search_D_cont<D>(subst, 2);
    std::vector<ANode<D>> d_cont_an;
    for (const auto& c : d_cont_cands) {
        ANode<D> nd; nd.i = c.i; nd.j = c.j; nd.x = c.x;
        d_cont_an.push_back(nd);
    }
    auto gp_nodes = backward_closure<D>(
        subst, d_cont_an, kContactBoundaryMaxNodesDefault);
    auto gp_edges = induced_restricted_edges<D>(subst, gp_nodes);
    std::set<ANode<D>> gp_set(gp_nodes.begin(), gp_nodes.end());
    auto red_gp = red_anode<D>(gp_set, gp_edges);
    std::set<SNode<D>> C;
    for (const auto& a : red_gp.first) {
        SNode<D> s; s.i = a.i; s.j = a.j; s.x = a.x;
        C.insert(s);
    }
    auto pmC = build_signed_contact_set<D>(C);
    auto A_prev = pmC;
    std::set<SNode<D>> gb;
    constexpr int MAX_ROUNDS = 8;
    constexpr std::size_t MAX_A_SIZE = 50000;
    for (int p = 2; p <= MAX_ROUNDS; ++p) {
        auto corona_nodes = c_corona<D>(subst, A_prev, A_prev);
        if (corona_nodes.size() > MAX_A_SIZE) break;
        std::vector<std::tuple<SNode<D>, SNode<D>, std::vector<long long>, std::vector<long long>>> edges;
        for (const auto& nn : corona_nodes) {
            auto fwd = simple_forward_targets<D>(subst, nn);
            for (const auto& [dest, pq] : fwd) {
                (void)pq;
                if (corona_nodes.count(dest) > 0) edges.push_back({nn, dest, {}, {}});
            }
        }
        auto red_result = red<D>(corona_nodes, edges);
        if (red_result.first == A_prev) { gb = red_result.first; break; }
        A_prev = red_result.first;
        gb = A_prev;
    }

    std::vector<SNode<D>> nodes(gb.begin(), gb.end());
    std::map<SNode<D>, std::size_t> idx;
    for (std::size_t i = 0; i < nodes.size(); ++i) idx[nodes[i]] = i;
    std::vector<std::vector<long long>> dense(nodes.size(), std::vector<long long>(nodes.size(), 0));
    for (std::size_t i = 0; i < nodes.size(); ++i) {
        auto fwd = simple_forward_targets<D>(subst, nodes[i]);
        for (const auto& [dest, pq] : fwd) {
            (void)pq;
            auto it = idx.find(dest);
            if (it != idx.end()) dense[i][it->second] += 1;
        }
    }
    auto gb_graph = WeightedDigraph::from_dense(dense);
    auto [dom_core, dom_idx] = extract_dominant_recurrent_core(gb_graph, 500);
    (void)dom_core;

    int exact_invalid_core_nodes = 0;
    int exact_transition_mismatches = 0;
    for (std::size_t u : dom_idx) {
        if (!exact_simple_validity(subst, nodes[u])) ++exact_invalid_core_nodes;
        if (fast_forward_records(subst, nodes[u]) !=
            exact_forward_records(subst, nodes[u]))
            ++exact_transition_mismatches;
    }

    std::map<std::pair<long long, long long>, std::vector<std::size_t>> groups;
    for (std::size_t u : dom_idx) groups[{nodes[u].i, nodes[u].j}].push_back(u);

    std::printf("=== n=%zu: %zu (i,j) pairs in dominant core (size %zu) ===\n",
                n, groups.size(), dom_idx.size());
    std::printf("  exact core audit: %d invalid nodes, %d transition-record "
                "mismatches\n",
                exact_invalid_core_nodes, exact_transition_mismatches);
    double worst_margin = 1e18;
    int mismatches = 0;
    int exact_mismatches = 0;
    int nonpositive_exact_margins = 0;
    int structural_witness_failures = 0;
    bool have_exact_worst = false;
    mathlib::QElem exact_worst = R.zero();
    for (auto& [ij, us] : groups) {
        auto [i, j] = ij;
        if (i == j) continue;  // diagonal identity-type nodes: not part of the claim
        double vi = subst.v[static_cast<std::size_t>(i)];
        double vj = subst.v[static_cast<std::size_t>(j)];
        double m = std::min(vi, vj);
        double max_abs_t = -1.0;
        for (std::size_t u : us) max_abs_t = std::max(max_abs_t, std::abs(subst.dot_v(nodes[u].x)));

        mathlib::QElem exact_max_abs_t = R.zero();
        for (std::size_t u : us) {
            mathlib::QElem t = mathlib::dot_qbeta(as_vector(nodes[u].x), exact_v, R);
            if (mathlib::qbeta_sign(t, R, beta_interval) < 0) t = R.neg(t);
            if (mathlib::qbeta_sign(R.sub(t, exact_max_abs_t), R, beta_interval) > 0)
                exact_max_abs_t = t;
        }

        // Closed-form prediction, using i<j WLOG (mirror pairs share margin).
        long long lo = std::min(i, j), hi = std::max(i, j);
        std::vector<long long> predicted_witness(D, 0);
        if (lo == 0 && hi < static_cast<long long>(n) - 1) {
            predicted_witness[0] = -1;
            predicted_witness[static_cast<std::size_t>(hi)] = 1;
            predicted_witness[static_cast<std::size_t>(hi + 1)] = -1;
        } else {
            predicted_witness[static_cast<std::size_t>(lo)] = -1;
            predicted_witness[D - 1] = 1;
        }
        bool structural_witness_found = false;
        for (std::size_t u : us) {
            mathlib::QElem t = mathlib::dot_qbeta(as_vector(nodes[u].x), exact_v, R);
            if (mathlib::qbeta_sign(t, R, beta_interval) < 0) t = R.neg(t);
            if (t == exact_max_abs_t &&
                equal_up_to_sign(as_vector(nodes[u].x), predicted_witness)) {
                structural_witness_found = true;
                break;
            }
        }
        if (!structural_witness_found) ++structural_witness_failures;
        double predicted;
        mathlib::QElem exact_predicted = R.zero();
        if (lo == 0 && hi == static_cast<long long>(n) - 1) {
            predicted = 1.0 - 1.0 / beta;  // shared boundary of both formulas
            exact_predicted = R.sub(R.one(), beta_inverse);
        } else if (lo == 0) {
            predicted = 1.0 - std::pow(beta, static_cast<double>(hi) - static_cast<double>(n));
            exact_predicted = R.sub(
                R.one(), qpow(R, beta_inverse, n - static_cast<std::size_t>(hi)));
        } else {
            double v_lo = subst.v[static_cast<std::size_t>(lo)];
            double v_last = subst.v[D - 1];
            predicted = v_lo - v_last;
            exact_predicted = R.sub(
                exact_v[static_cast<std::size_t>(lo)], exact_v[D - 1]);
        }
        bool match = std::fabs(max_abs_t - predicted) < 1e-6;
        if (!match) ++mismatches;
        const bool exact_match = exact_max_abs_t == exact_predicted;
        if (!exact_match) ++exact_mismatches;

        const auto& exact_vi = exact_v[static_cast<std::size_t>(i)];
        const auto& exact_vj = exact_v[static_cast<std::size_t>(j)];
        const mathlib::QElem exact_min =
            mathlib::qbeta_sign(R.sub(exact_vi, exact_vj), R, beta_interval) <= 0
                ? exact_vi : exact_vj;
        const mathlib::QElem exact_margin = R.sub(exact_min, exact_max_abs_t);
        const bool exact_positive =
            mathlib::qbeta_sign(exact_margin, R, beta_interval) > 0;
        if (!exact_positive) ++nonpositive_exact_margins;
        if (!have_exact_worst ||
            mathlib::qbeta_sign(R.sub(exact_margin, exact_worst), R,
                                beta_interval) < 0) {
            exact_worst = exact_margin;
            have_exact_worst = true;
        }
        double margin = m - max_abs_t;
        worst_margin = std::min(worst_margin, margin);
        std::printf("  (i=%2lld,j=%2lld): max|t|=%.6f  predicted=%.6f  %-3s  "
                    "exact=%s  witness=%s  margin=%.6f  exact-positive=%s\n",
                    i, j, max_abs_t, predicted, match ? "ok" : "MISMATCH",
                    exact_match ? "YES" : "NO",
                    structural_witness_found ? "YES" : "NO", margin,
                    exact_positive ? "YES" : "NO");
    }
    double closed_form_worst = 2.0 / beta - 1.0;
    const mathlib::QElem exact_closed_form_worst =
        R.sub(R.mul(R.from_int(2), beta_inverse), R.one());
    const bool exact_worst_match =
        have_exact_worst && exact_worst == exact_closed_form_worst;
    const bool exact_worst_positive =
        have_exact_worst &&
        mathlib::qbeta_sign(exact_worst, R, beta_interval) > 0;
    std::printf("  worst margin overall: %.6f  (closed form 2/beta-1 = %.6f)  "
                "%d floating mismatches\n",
                worst_margin, closed_form_worst, mismatches);
    std::printf("  exact certificate: %d formula mismatches, %d structural "
                "witness failures, %d nonpositive margins, exact global "
                "worst=%s, positive=%s\n\n",
                exact_mismatches, structural_witness_failures,
                nonpositive_exact_margins,
                exact_worst_match ? "YES" : "NO",
                exact_worst_positive ? "YES" : "NO");
    return exact_invalid_core_nodes == 0 && exact_transition_mismatches == 0 &&
           exact_mismatches == 0 && structural_witness_failures == 0 &&
           nonpositive_exact_margins == 0 && exact_worst_match &&
           exact_worst_positive;
}

}  // namespace

int main() {
    bool ok = true;
    ok = check<3>(3, 1.8392867552141612) && ok;
    ok = check<4>(4, 1.9275619754829254) && ok;
    ok = check<5>(5, 1.9659482366454853) && ok;
    ok = check<6>(6, 1.9835828434243288) && ok;
    ok = check<7>(7, 1.9919641966050352) && ok;
    std::printf("Exact per-pair Q(beta) certificate n=3..7: %s\n",
                ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
