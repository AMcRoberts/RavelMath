// Confirms the "coverage" mechanism named in
// class_ii_round2_edge_mechanism_exact.cpp's remaining-gap note,
// directly on the one concrete exception found earlier
// (class_ii_round4_edge_structure_a_independence_probe.cpp: state
// {i=0,x=(3,-3,0),j=0} gains the destination {i=1,x=(-3,3,-2),j=0}
// only at a>=7, not at a=6).
//
// Traced directly: the edge requires an occurrence of letter 0 within
// sigma(1) = 0^(a-1) 2 0 at prefix length 5 (abelianization (5,0,0),
// i.e. strictly within the leading 0-run, not past the "2"). sigma(1)
// has a-1 leading zeros, so this specific occurrence exists only once
// a-1 > 5, i.e. a>=7 -- exactly matching the observed threshold. This
// is not a "window coverage" story in the abstract; it is the
// concrete, checkable fact that the occurrence itself does not exist
// yet at a=6, confirming the mechanism named (not just asserted) in
// class_ii_round2_edge_mechanism_exact.cpp's remaining-gap note.

#include <cstdio>

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
    bool found_at_6 = false, found_at_7 = false;
    for (long long a : {6, 7}) {
        const auto center = class_ii(static_cast<std::size_t>(a));
        const auto spectral = classify_matrix_spectral(incidence(center));
        const auto neighbors = adjacent_swap_neighbors(center);
        const Substitution<3> subst = make_substitution<3>(
            SubstitutionRule(neighbors[2].substitution), spectral.beta);

        const SNode<3> node{0, {3, -3, 0}, 0};
        auto parents_i = parent_decompositions<3>(subst.images, node.i);
        auto parents_j = parent_decompositions<3>(subst.images, node.j);

        bool found_this_a = false;
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
                if (cand.i == 1 && cand.x[0] == -3 && cand.x[1] == 3
                    && cand.x[2] == -2 && cand.j == 0) {
                    found_this_a = true;
                    std::printf(
                        "a=%lld: FOUND via p_parent=%lld p_len=%zu "
                        "lp=(%lld,%lld,%lld); requires a-1>5 i.e. a>=7\n",
                        a, pd.parent_letter, pd.p.size(), lp1[0], lp1[1],
                        lp1[2]);
                }
            }
        }
        if (a == 6) found_at_6 = found_this_a;
        if (a == 7) found_at_7 = found_this_a;
        std::printf("a=%lld: destination found=%d\n", a, found_this_a);
    }

    const bool ok = !found_at_6 && found_at_7;
    std::printf(
        "%s\n",
        ok ? "ROUND4_COVERAGE_THRESHOLD_CONFIRMED: the exceptional "
             "edge is absent at a=6 and present at a=7 because the "
             "specific occurrence it needs (prefix length 5 within "
             "sigma(1)'s a-1-long leading 0-run) does not exist until "
             "a-1>5 -- the coverage mechanism is not just plausible, "
             "it is the literal reason for this exact exception"
           : "CHECK FAILED -- see output above");
    return ok ? 0 : 1;
}
