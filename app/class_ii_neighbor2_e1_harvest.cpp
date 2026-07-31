// Data-harvesting driver for the Round 1 symbolic derivation
// (docs/GLOBAL_CATALOGUE_OCCURRENCE_EXHAUSTION.md). Uses the already
// generic, already-trusted class_ii_neighbor_transition_weight (exact
// O(1) prefix-interval arithmetic, no word expansion) to brute-force
// find every raw backward-predecessor branch into each of E_1's 22
// states, across a range of a. Not a proof; a data source for fitting
// and then verifying a symbolic formula.

#include <cstdio>
#include <cstdlib>

#include "ravel/class_ii_neighbor_family.hpp"

using namespace ravel;

int main(int argc, char** argv) {
    const long long a_min = argc > 1 ? std::atoll(argv[1]) : 3;
    const long long a_max = argc > 2 ? std::atoll(argv[2]) : 20;
    const long long x_bound = argc > 3 ? std::atoll(argv[3]) : 6;
    const long long x0_bound = argc > 4 ? std::atoll(argv[4]) : 80;

    const auto e1 = class_ii_neighbor2_initial_extension_states();
    long long destination_index = 0;
    for (const auto& destination : e1) {
        for (long long a = a_min; a <= a_max; ++a) {
            for (long long pi = 0; pi < 3; ++pi) {
                for (long long pj = 0; pj < 3; ++pj) {
                    for (long long x1 = -x_bound; x1 <= x_bound; ++x1) {
                        for (long long x2 = -x_bound; x2 <= x_bound; ++x2) {
                            long long min_hit = 0;
                            long long max_hit = 0;
                            bool any_hit = false;
                            for (long long x0 = -x0_bound;
                                 x0 <= x0_bound; ++x0) {
                                const SNode<3> candidate{
                                    pi, {x0, x1, x2}, pj};
                                // transition_weight(a, source, target)
                                // substitutes TARGET and searches for
                                // SOURCE inside its image -- i.e. target
                                // is the parent, source the child. For
                                // backward closure we want "does
                                // substituting CANDIDATE produce
                                // DESTINATION as a child", so candidate
                                // is target and destination is source.
                                const long long weight =
                                    class_ii_neighbor_transition_weight(
                                        2, a, destination, candidate);
                                if (weight > 0) {
                                    if (!any_hit) min_hit = x0;
                                    max_hit = x0;
                                    any_hit = true;
                                }
                            }
                            if (any_hit) {
                                std::printf(
                                    "HIT,%lld,%lld,%lld,%lld,%lld,%lld,%lld,"
                                    "%lld,%lld,%lld,%lld\n",
                                    destination_index, a, pi, x1, x2, pj,
                                    min_hit, max_hit,
                                    destination.i, destination.x[0],
                                    destination.j);
                                if (min_hit == -x0_bound
                                        || max_hit == x0_bound) {
                                    std::printf(
                                        "  WARNING_BOUND_TOUCHED,%lld,%lld\n",
                                        destination_index, a);
                                }
                            }
                        }
                    }
                }
            }
        }
        ++destination_index;
    }
    return 0;
}
