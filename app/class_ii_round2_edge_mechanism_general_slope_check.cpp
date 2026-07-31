// Last refinement in this line of inquiry before stopping it for this
// session: class_ii_round2_edge_mechanism_by_type_check.cpp tested
// x2' = (q_len - p_len) + offset, i.e. slope EXACTLY 1 -- matching
// Round 1's six candidates, all of which had slope +/-1, but nothing
// established that every type must. This file fits whatever slope the
// data actually shows (from the first two edges with distinct
// q_len-p_len values) and checks it against every edge in the type,
// and separately checks whether x0'/x1' are themselves affine in
// p_len (rather than strictly constant).
//
// Result at a=8 (139 multi-edge types): a general (not forced-to-1)
// slope fits x2' for 103/139 types (up from 94/139 forced to slope 1)
// and a general affine-in-p_len fit works for x0'/x1' in 116/139
// (up from the 17-94-ish range in earlier, coarser tests). Real,
// incremental improvement, still not universal. Stopping this
// specific line of inquiry here for this session -- the remaining
// gap plausibly needs a genuinely two-parameter model (x' as an
// affine function of BOTH p_len and q_len independently, not reduced
// to one difference), which this file does not attempt.

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

}  // namespace

int main() {
    const long long a = 8;
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

    long long total_types = 0, clean_x2_general_slope = 0,
              x0x1_also_affine = 0;

    for (const auto& node : pre_red) {
        auto parents_i = parent_decompositions<3>(subst.images, node.i);
        auto parents_j = parent_decompositions<3>(subst.images, node.j);

        std::map<std::pair<long long, long long>,
                  std::vector<std::tuple<long long, long long, SNode<3>>>>
            by_type;
        for (const auto& pd : parents_i) {
            auto lp1 = abelianization<3>(pd.p);
            for (const auto& qd : parents_j) {
                auto lq1 = abelianization<3>(qd.p);
                std::array<long long, 3> rhs;
                for (int k = 0; k < 3; ++k)
                    rhs[k] = node.x[k] + lq1[k] - lp1[k];
                auto xprime_opt = solve_Mx_eq_rhs_exact<3>(subst.M, rhs);
                if (!xprime_opt.has_value()) continue;
                auto xprime = *xprime_opt;
                SNode<3> cand{pd.parent_letter, xprime, qd.parent_letter};
                if (!is_valid_simple_node<3>(subst, cand)) continue;
                if (!pre_red.count(cand)) continue;
                by_type[{pd.parent_letter, qd.parent_letter}].push_back(
                    {static_cast<long long>(pd.p.size()),
                     static_cast<long long>(qd.p.size()), cand});
            }
        }

        for (auto& [type, edges] : by_type) {
            if (edges.size() < 2) continue;
            ++total_types;

            auto& [p0, q0, d0] = edges[0];
            const long long diff0 = q0 - p0;
            bool found_slope = false;
            long long slope_num = 0, slope_den = 1;
            for (auto& [p1, q1, d1] : edges) {
                const long long diff1 = q1 - p1;
                if (diff1 != diff0) {
                    slope_num = d1.x[2] - d0.x[2];
                    slope_den = diff1 - diff0;
                    found_slope = true;
                    break;
                }
            }
            bool x2_ok = true;
            if (found_slope) {
                for (auto& [p, q, d] : edges) {
                    const long long diff = q - p;
                    if ((d.x[2] - d0.x[2]) * slope_den
                        != slope_num * (diff - diff0)) {
                        x2_ok = false;
                        break;
                    }
                }
            } else {
                for (auto& [p, q, d] : edges)
                    if (d.x[2] != d0.x[2]) {
                        x2_ok = false;
                        break;
                    }
            }
            if (x2_ok) ++clean_x2_general_slope;

            bool x0x1_affine = true;
            const auto& [pA, qA, dA] = edges[0];
            bool got_slope = false;
            long long dx0 = 0, dx1 = 0, dden = 1;
            for (auto& [pB, qB, dB] : edges) {
                if (pB != pA) {
                    dx0 = dB.x[0] - dA.x[0];
                    dx1 = dB.x[1] - dA.x[1];
                    dden = pB - pA;
                    got_slope = true;
                    break;
                }
            }
            if (got_slope) {
                for (auto& [p, q, d] : edges) {
                    const long long dp = p - pA;
                    if ((d.x[0] - dA.x[0]) * dden != dx0 * dp
                        || (d.x[1] - dA.x[1]) * dden != dx1 * dp) {
                        x0x1_affine = false;
                        break;
                    }
                }
            }
            if (x0x1_affine) ++x0x1_also_affine;
        }
    }

    std::printf(
        "a=%lld total_types=%lld x2_general_slope_ok=%lld/%lld "
        "x0x1_affine_in_p_ok=%lld/%lld\n",
        a, total_types, clean_x2_general_slope, total_types,
        x0x1_also_affine, total_types);
    return 0;
}
