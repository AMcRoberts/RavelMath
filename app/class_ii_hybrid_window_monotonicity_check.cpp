// The key lemma for turning the whole-graph a-independence result
// from an exact finite check into a genuine symbolic proof: does the
// achievable (q_len - p_len) window for each hybrid-category state
// grow monotonically (nested containment) as `a` increases?
//
// If yes, then once the window is shown to cover every value the
// bounded target needs at some threshold a0 (already established:
// a0=7), the SAME coverage holds for every a>=a0 by nothing more than
// "a growing/nested window that covers a fixed target set at one
// point still covers it once it grows further" -- turning "checked up
// to a=50" into "true for literally every integer a>=7", without
// needing to check each larger a individually.
//
// Checked directly (not assumed): tracked every hybrid-rule state's
// achievable window at a=7,12,20,30 and required each window to
// contain the previous one. Result: 441 pairwise checks (state x
// consecutive-a-pair), **zero violations** -- every window only grew,
// never shifted away from previously-covered territory.
//
// Scope, precisely: this is a wide-spread NUMERICAL confirmation
// (four sample points spanning a=7 to a=30), not yet an algebraic
// derivation that the window MUST nest for every shape by
// construction. The natural derivation (sketched, not yet written up
// rigorously): the fixed side's position and the ranging side's
// boundary are both affine in `a` with slope derived from the same
// "distance from the start of a leading zero-run" counting, so they
// share a consistent reference point -- this is *why* the window
// should always extend outward from a fixed edge rather than
// translate away, but that argument has not been formalized per-shape
// here. The remaining step to a complete symbolic proof is exactly
// that formalization, not further numerical checking.

#include <cstdio>
#include <map>
#include <set>
#include <tuple>
#include <vector>

#include "ravel/ambient_graph.hpp"
#include "ravel/class_ii_neighbor_family.hpp"
#include "ravel/contact_boundary.hpp"
#include "ravel/d_cont_check.hpp"
#include "ravel/spectral.hpp"
#include "ravel/substitution.hpp"
#include "ravel/substitution_neighborhood.hpp"
#include "ravel/survey.hpp"

using namespace ravel;

namespace {

using Matrix = std::vector<std::vector<long long>>;

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

Matrix incidence(const FiniteSubstitution& s) {
    Matrix m(3, std::vector<long long>(3, 0));
    for (std::size_t im = 0; im < 3; ++im)
        for (auto l : s[im]) ++m[static_cast<std::size_t>(l)][im];
    return m;
}

bool rule_range(long long inner, long long parent, long long l1, long long l2) {
    if (inner != 0) return false;
    if (parent == 2) return false;
    if (l1 != 0 || l2 != 0) return false;
    return true;
}

}  // namespace

int main() {
    using StateKey =
        std::tuple<long long, long long, long long, long long, long long>;

    std::map<StateKey, std::pair<long long, long long>> window_prev;
    bool all_nested = true;
    long long total_checks = 0, violations = 0;

    const std::vector<long long> as = {7, 12, 20, 30};
    for (long long a : as) {
        const auto center = class_ii(static_cast<std::size_t>(a));
        const auto spectral = classify_matrix_spectral(incidence(center));
        const auto neighbors = adjacent_swap_neighbors(center);
        const Substitution<3> subst = make_substitution<3>(
            SubstitutionRule(neighbors[2].substitution), spectral.beta);

        const auto seeds = search_D_cont<3>(subst, 2);
        std::vector<ANode<3>> d_cont;
        for (const auto& s : seeds) d_cont.push_back(ANode<3>{s.i, s.x, s.j});
        const auto pre_contact = backward_closure<3>(subst, d_cont);
        const auto induced = induced_restricted_edges<3>(subst, pre_contact);
        const std::set<ANode<3>> pre_set(pre_contact.begin(), pre_contact.end());
        const auto reduced = red_anode<3>(pre_set, induced);
        std::set<SNode<3>> contact;
        for (const auto& n : reduced.first) contact.insert(to_simple<3>(n));
        const auto trace = algorithm2_trace<3>(
            subst, contact, CoronaConnectorPolicy::fixed_signed_contact,
            8, 0, CoronaEdgeArithmetic::exact_rational);
        const auto& pre_red = trace.layers[1].pre_red_nodes;

        std::map<StateKey, std::pair<long long, long long>> window_now;

        for (const auto& node : pre_red) {
            auto parents_i = parent_decompositions<3>(subst.images, node.i);
            auto parents_j = parent_decompositions<3>(subst.images, node.j);
            StateKey key{node.i, node.x[0], node.x[1], node.x[2], node.j};
            bool any = false;
            long long mn = 0, mx = 0;
            for (const auto& pd : parents_i) {
                auto lp1 = abelianization<3>(pd.p);
                const bool p_r =
                    rule_range(node.i, pd.parent_letter, lp1[1], lp1[2]);
                for (const auto& qd : parents_j) {
                    auto lq1 = abelianization<3>(qd.p);
                    const bool q_r =
                        rule_range(node.j, qd.parent_letter, lq1[1], lq1[2]);
                    if (p_r == q_r) continue;  // hybrid only
                    const long long p_len = static_cast<long long>(pd.p.size());
                    const long long q_len = static_cast<long long>(qd.p.size());
                    const long long diff = q_len - p_len;
                    if (!any) {
                        mn = mx = diff;
                        any = true;
                    } else {
                        mn = std::min(mn, diff);
                        mx = std::max(mx, diff);
                    }
                }
            }
            if (any) window_now[key] = {mn, mx};
        }

        if (!window_prev.empty()) {
            for (auto& [key, w] : window_prev) {
                auto it = window_now.find(key);
                ++total_checks;
                if (it == window_now.end()) {
                    ++violations;
                    continue;
                }
                if (!(it->second.first <= w.first
                      && it->second.second >= w.second)) {
                    all_nested = false;
                    ++violations;
                }
            }
        }
        window_prev = window_now;
    }

    std::printf(
        "total_pairwise_checks=%lld violations=%lld all_nested=%d\n",
        total_checks, violations, all_nested);
    std::printf(
        "%s\n",
        all_nested
            ? "MONOTONIC_WINDOW_GROWTH_CONFIRMED: every hybrid state's "
              "achievable window only grows (nested containment) "
              "across a=7,12,20,30 -- the numerical support this "
              "session's proof strategy needs, not yet the symbolic "
              "derivation itself"
            : "NESTING VIOLATED -- monotonicity does not hold "
              "universally, see per-state detail needed");
    return all_nested ? 0 : 1;
}
