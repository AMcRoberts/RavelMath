// An exact finite certificate (not yet a universal symbolic proof) for
// Round 2's Red exclusion: independently re-verifies, via
// simple_forward_targets_exact (exact rational arithmetic, not the
// trusted red_anode's own internal bookkeeping), that every one of the
// 123 states class_ii_round2_structure.cpp found Red prunes really
// does have every one of its valid forward edges landing either
// outside the 195-state raw set entirely, or on a state pruned at an
// earlier or equal rank -- never on a later rank or a survivor. That
// is precisely red_anode's own pruning criterion (a rank-k node's only
// edges go to nodes already gone by the time rank k is computed), and
// checking it independently (not just trusting red_anode's rank
// bookkeeping) is the same "verify, don't just trust the derivation"
// discipline this project applies throughout.
//
// What this establishes: at a=7 (checked), the 123-state, 3-rank Red
// exclusion is internally consistent under an independent
// forward-edge recomputation, not just under the trusted
// red_anode/red_trace machinery's own report. Checked for size/rank
// stability (not full state-by-state re-verification) at a=6,8,15,25
// too by class_ii_round2_structure.cpp and class_ii_round2_ccorona_stability.cpp.
//
// What this does NOT establish: a symbolic argument valid for literal
// every integer a. That would need, for each of the 123 states, the
// same kind of M-matrix/parent-decomposition derivation Round 1 used
// for its 2 pruned states -- tractable in principle (the machinery is
// the same), but a materially larger undertaking given some of these
// states have on the order of 200+ raw forward-target candidates
// before window filtering, not Round 1's handful.

#include <cstdio>
#include <set>
#include <vector>

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

Matrix incidence(const FiniteSubstitution& substitution) {
    Matrix matrix(3, std::vector<long long>(3, 0));
    for (std::size_t image = 0; image < 3; ++image) {
        for (const auto letter : substitution[image]) {
            ++matrix[static_cast<std::size_t>(letter)][image];
        }
    }
    return matrix;
}

CoronaTrace<3> corona_trace(const Substitution<3>& subst) {
    const auto seeds = search_D_cont<3>(subst, 2);
    std::vector<ANode<3>> d_cont;
    for (const auto& s : seeds) d_cont.push_back(ANode<3>{s.i, s.x, s.j});
    const auto pre_contact = backward_closure<3>(subst, d_cont);
    const auto induced = induced_restricted_edges<3>(subst, pre_contact);
    const std::set<ANode<3>> pre_set(pre_contact.begin(), pre_contact.end());
    const auto reduced = red_anode<3>(pre_set, induced);
    std::set<SNode<3>> contact;
    for (const auto& n : reduced.first) contact.insert(to_simple<3>(n));
    return algorithm2_trace<3>(
        subst, contact, CoronaConnectorPolicy::fixed_signed_contact,
        8, 0, CoronaEdgeArithmetic::exact_rational);
}

}  // namespace

int main() {
    bool all_ok = true;
    for (long long a : {6LL, 7LL, 8LL}) {
        const auto center = class_ii(static_cast<std::size_t>(a));
        const auto spectral = classify_matrix_spectral(incidence(center));
        const auto neighbors = adjacent_swap_neighbors(center);
        const Substitution<3> subst_n = make_substitution<3>(
            SubstitutionRule(neighbors[2].substitution), spectral.beta);

        const auto trace = corona_trace(subst_n);
        const auto& layer = trace.layers[1];
        const auto& pre_red = layer.pre_red_nodes;  // 195 raw states
        const auto& ranks = layer.red_pruning_ranks;
        const auto& survivors = layer.nodes;

        // rank_of[node] = 0,1,2,... for pruned nodes; -1 for survivors.
        std::map<SNode<3>, int> rank_of;
        for (const auto& n : survivors) rank_of[n] = -1;
        for (std::size_t r = 0; r < ranks.size(); ++r)
            for (const auto& n : ranks[r]) rank_of[n] = static_cast<int>(r);

        long long violations = 0;
        long long total_checked = 0;
        long long total_edges_within_195 = 0;

        for (std::size_t r = 0; r < ranks.size(); ++r) {
            for (const auto& n : ranks[r]) {
                ++total_checked;
                const auto targets =
                    simple_forward_targets_exact<3>(subst_n, n);
                for (const auto& [dest, pq] : targets) {
                    if (pre_red.count(dest) == 0)
                        continue;  // outside 195, fine
                    ++total_edges_within_195;
                    const auto it = rank_of.find(dest);
                    // dest must exist in rank_of (pre_red partitions
                    // fully into survivors + ranks); a missing entry
                    // would itself be a bug worth flagging.
                    if (it == rank_of.end()) {
                        ++violations;
                        std::printf(
                            "BUG: dest not classified: i=%lld "
                            "x=(%lld,%lld,%lld) j=%lld\n",
                            dest.i, dest.x[0], dest.x[1], dest.x[2],
                            dest.j);
                        continue;
                    }
                    const int dest_rank = it->second;
                    // Valid pruning edges go only to nodes already
                    // gone at or before rank r (rank <= r, and not a
                    // survivor, i.e. not -1).
                    const bool ok_edge =
                        dest_rank != -1
                        && dest_rank <= static_cast<int>(r);
                    if (!ok_edge) {
                        ++violations;
                        std::printf(
                            "VIOLATION: rank-%zu source i=%lld "
                            "x=(%lld,%lld,%lld) j=%lld -> dest_rank=%d "
                            "(i=%lld x=(%lld,%lld,%lld) j=%lld) -- this "
                            "contradicts red_anode's own ranking\n",
                            r, n.i, n.x[0], n.x[1], n.x[2], n.j,
                            dest_rank, dest.i, dest.x[0], dest.x[1],
                            dest.x[2], dest.j);
                    }
                }
            }
        }

        std::printf(
            "a=%lld pruned_states_checked=%lld edges_within_195=%lld "
            "violations=%lld\n",
            a, total_checked, total_edges_within_195, violations);

        all_ok = all_ok && violations == 0 && total_checked == 123;
    }

    std::printf(
        "%s\n",
        all_ok ? "ROUND2_RED_EXCLUSION_CERTIFICATE_OK: independent "
                  "recomputation via simple_forward_targets_exact "
                  "confirms every one of the 123 pruned states' edges "
                  "within the 195-state raw set land only on "
                  "earlier-or-equal ranks, never a survivor or a later "
                  "rank -- exact finite certificate at a=6,7,8, not "
                  "yet a universal symbolic proof"
                : "CERTIFICATE FAILED -- either red_anode's ranking is "
                  "inconsistent with an independent forward-edge "
                  "recomputation, or the pruned-state count changed");
    return all_ok ? 0 : 1;
}
