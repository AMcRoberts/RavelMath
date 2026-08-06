// Verifies the leftmost-loop corollary (Finding 27): for a junction on
// its own eventual leftmost (always-child-0) cycle of length L, every
// gap = m*L coincides at EXACTLY K = gap, via the trivial zero-vector
// witness -- not a search result, a direct construction. Cross-checked
// against the same two substitutions used throughout Findings 25-26.

#include <array>
#include <cassert>
#include <iostream>
#include <numeric>

#include "ravel/proof/coincidence_closure.hpp"
#include "ravel/proof/coincidence_converse_leftmost_loop.hpp"

using namespace ravel::proof;

template <std::size_t d>
std::array<std::array<long long, d>, d> incidence_matrix(const std::array<std::vector<long long>, d>& images) {
    std::array<std::array<long long, d>, d> M{};
    for (long long j = 0; j < static_cast<long long>(d); ++j)
        for (auto c : images[static_cast<std::size_t>(j)]) M[static_cast<std::size_t>(c)][static_cast<std::size_t>(j)] += 1;
    return M;
}

int main() {
    // gcd=1 single-junction case: sigma(0)=[0,0,1] means child_index=0
    // AT junction 0 is letter 0 itself -- a length-1 leftmost self-loop.
    {
        std::array<std::vector<long long>, 3> images = {
            std::vector<long long>{0, 0, 1}, std::vector<long long>{2}, std::vector<long long>{0, 1}};
        auto edges = build_junction_graph<3>(images);
        auto L = leftmost_loop_length<3>(edges, 0);
        assert(L.has_value() && *L == 1);
        CoincidenceClosure<3> closure(edges, incidence_matrix<3>(images));
        for (long long m = 0; m <= 6; ++m) {
            long long gap = m * (*L);
            const auto& A = closure.reachable(0, gap);
            assert(A.count({0, ExactVec<3>{}}) == 1);
        }
        std::cout << "gcd=1 case: leftmost loop length L=" << *L << ", zero-vector witness confirmed for gap=0..6\n";
    }

    // gcd=2 multi-junction case: leftmost cycle is 0 -> 2 -> 0, length 4
    // (a PROPER multiple of g=2 -- this is the whole point: L can exceed
    // g, and gaps that are multiples of g but not of L, like gap=2,
    // genuinely need the full search, not this corollary).
    {
        std::array<std::vector<long long>, 8> images = {
            std::vector<long long>{1, 3}, std::vector<long long>{2}, std::vector<long long>{4, 5},
            std::vector<long long>{0}, std::vector<long long>{0}, std::vector<long long>{6},
            std::vector<long long>{7}, std::vector<long long>{0}};
        auto edges = build_junction_graph<8>(images);
        auto L = leftmost_loop_length<8>(edges, 0);
        assert(L.has_value() && *L == 4);
        CoincidenceClosure<8> closure(edges, incidence_matrix<8>(images));
        for (long long m = 0; m <= 3; ++m) {
            long long gap = m * (*L);
            const auto& A = closure.reachable(0, gap);
            assert(A.count({0, ExactVec<8>{}}) == 1);
        }
        // And confirm the corollary does NOT (trivially) cover gap=2,
        // which is 0 mod g=2 but not mod L=4 -- reachable(0,2) must NOT
        // already contain the zero-vector witness, or L would be wrong.
        {
            const auto& A2 = closure.reachable(0, 2);
            assert(A2.count({0, ExactVec<8>{}}) == 0);
        }
        std::cout << "gcd=2 case: leftmost loop length L=" << *L
                  << " (a proper multiple of g=2), zero-vector witness confirmed for gap=0,4,8,12; "
                     "gap=2 correctly NOT covered by this corollary alone.\n";
    }

    std::cout << "coincidence_converse_leftmost_loop: corollary holds exactly as constructed, "
                 "on both known examples.\n";
    return 0;
}
