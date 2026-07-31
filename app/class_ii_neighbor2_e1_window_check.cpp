// Window-validity + reverse-inclusion check for E_1 (Round 1). Uses
// the exact bit-exact in_H_sigma_exact predicate (same for center and
// neighbor since they share incidence matrix M, beta, and v) against
// the raw backward-branch categories harvested by
// class_ii_neighbor2_e1_harvest.cpp. Brute force over a wide x0 range
// per category, not a symbolic derivation -- a verification tool for
// one, before attempting the general proof.

#include <cstdio>
#include <cstdlib>
#include <set>
#include <vector>

#include "ravel/class_ii_neighbor_family.hpp"
#include "ravel/contact_boundary.hpp"
#include "ravel/core.hpp"
#include "ravel/substitution.hpp"
#include "ravel/substitution_neighborhood.hpp"

using namespace ravel;

namespace {

FiniteSubstitution class_ii(long long a) {
    FiniteSubstitution result(3);
    result[0].assign(static_cast<std::size_t>(a), 0);
    result[0].push_back(1);
    result[0].push_back(2);
    result[1].assign(static_cast<std::size_t>(a), 0);
    result[1].push_back(2);
    result[2] = {0};
    return result;
}

}  // namespace

int main(int argc, char** argv) {
    const long long a_min = argc > 1 ? std::atoll(argv[1]) : 3;
    const long long a_max = argc > 2 ? std::atoll(argv[2]) : 12;
    const long long x_bound = argc > 3 ? std::atoll(argv[3]) : 6;
    const long long x0_bound = argc > 4 ? std::atoll(argv[4]) : 40;

    const auto e1 = class_ii_neighbor2_initial_extension_states();

    for (long long a = a_min; a <= a_max; ++a) {
        const auto substitution = class_ii(a);
        const SubstitutionRule rule(substitution);
        // Perron root via a quick companion-matrix power iteration is
        // not exact; use the project's own exact classifier path by
        // constructing Substitution<3> with a double seed then relying
        // on in_H_sigma_exact's own bit-exact Sturm/Q(beta) machinery,
        // exactly as d_cont_check.hpp does elsewhere in this project.
        double beta_seed = static_cast<double>(a) + 1.0;
        for (int iter = 0; iter < 200; ++iter) {
            beta_seed = std::cbrt(
                static_cast<double>(a) * beta_seed * beta_seed
                + static_cast<double>(a + 1) * beta_seed + 1.0);
        }
        const Substitution<3> subst = make_substitution<3>(rule, beta_seed);

        const auto e1 = class_ii_neighbor2_initial_extension_states();
        std::set<SNode<3>> window_valid_raw;
        for (const auto& destination : e1) {
            (void)destination;
        }

        // Re-derive raw hits directly (cheap, exact) rather than
        // reparsing harvest logs, then filter by in_H_sigma_exact.
        long long destination_index = 0;
        for (const auto& destination : e1) {
            for (long long pi = 0; pi < 3; ++pi) {
                for (long long pj = 0; pj < 3; ++pj) {
                    for (long long x1 = -x_bound; x1 <= x_bound; ++x1) {
                        for (long long x2 = -x_bound;
                             x2 <= x_bound; ++x2) {
                            for (long long x0 = -x0_bound;
                                 x0 <= x0_bound; ++x0) {
                                const SNode<3> candidate{
                                    pi, {x0, x1, x2}, pj};
                                const long long weight =
                                    class_ii_neighbor_transition_weight(
                                        2, a, destination, candidate);
                                if (weight <= 0) continue;
                                IVec<3> xv{x0, x1, x2};
                                const bool valid =
                                    subst.in_H_sigma_exact(
                                        xv, static_cast<std::size_t>(pj));
                                if (valid) {
                                    window_valid_raw.insert(candidate);
                                }
                            }
                        }
                    }
                }
            }
            ++destination_index;
        }

        std::set<SNode<3>> extra, missing;
        std::set_difference(
            window_valid_raw.begin(), window_valid_raw.end(),
            e1.begin(), e1.end(), std::inserter(extra, extra.end()));
        std::set_difference(
            e1.begin(), e1.end(),
            window_valid_raw.begin(), window_valid_raw.end(),
            std::inserter(missing, missing.end()));

        std::printf(
            "WINDOW_CHECK,%lld,%s,%zu,%zu,extra=%zu,missing=%zu\n",
            a, (extra.empty() && missing.empty()) ? "MATCH" : "MISMATCH",
            window_valid_raw.size(), e1.size(), extra.size(), missing.size());
        for (const auto& node : extra) {
            std::printf(
                "  EXTRA,%lld,%lld,%lld,%lld,%lld,%lld\n",
                a, node.i, node.x[0], node.x[1], node.x[2], node.j);
        }
        for (const auto& node : missing) {
            std::printf(
                "  MISSING,%lld,%lld,%lld,%lld,%lld,%lld\n",
                a, node.i, node.x[0], node.x[1], node.x[2], node.j);
        }
    }
    return 0;
}
