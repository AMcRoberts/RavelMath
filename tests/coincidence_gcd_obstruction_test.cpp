// Locks in the gcd-obstruction finding: for a single-junction
// substitution, pairs whose run-in gap is not divisible by the gcd of
// the junction's jump sizes never coincide (checked exhaustively up to
// a real depth bound); pairs whose gap IS divisible by it generically
// do, at manageable depth. Two structurally different substitutions,
// one with gcd=2 (obstruction present) and one with gcd=1 (no
// obstruction), both checked directly against the real closure tool.

#include <array>
#include <cassert>
#include <iostream>
#include <string>

#include "ravel/proof/coincidence_closure.hpp"

using namespace ravel::proof;

namespace {

template <std::size_t N>
long long first_coincidence_gap(CoincidenceClosure<N>& closure, long long gap, long long max_depth) {
    for (long long K = gap; K <= max_depth; ++K) {
        const auto& hi = closure.reachable(0, K);
        const auto& lo = closure.reachable(0, K - gap);
        for (auto& s : hi)
            if (lo.count(s)) return K;
    }
    return -1;
}

}  // namespace

int main() {
    // gcd=2 case: junction 0, side-A chain length 3 (jump size 4), side-B
    // chain length 1 (jump size 2). gcd(4,2)=2.
    {
        constexpr std::size_t N = 5;
        std::array<std::vector<long long>, N> images;
        images[0] = {1, 4};
        images[1] = {2}; images[2] = {3}; images[3] = {0}; images[4] = {0};
        std::array<std::array<long long, N>, N> M{};
        for (std::size_t j = 0; j < N; ++j) for (auto c : images[j]) M[c][j] += 1;
        auto edges = build_junction_graph<N>(images);
        CoincidenceClosure<N> closure(edges, M);

        for (long long gap = 1; gap <= 7; ++gap) {
            long long found = first_coincidence_gap<N>(closure, gap, 40);
            std::cout << "gcd=2 case, gap=" << gap << ": " << (found < 0 ? "NOT found by 40" : std::to_string(found)) << "\n";
            if (gap % 2 == 0) {
                assert(found > 0);  // divisible by gcd -- must resolve
            } else {
                assert(found < 0);  // not divisible by gcd -- must NOT resolve within the tested bound
            }
        }
    }

    // gcd=1 case: the original sigma_{0,1}. No obstruction expected --
    // every gap should resolve, including gap=1 matching the known
    // ground truth depth 12 exactly.
    {
        constexpr std::size_t N = 3;
        std::array<std::vector<long long>, N> images = {std::vector<long long>{1, 2}, std::vector<long long>{2}, std::vector<long long>{0}};
        std::array<std::array<long long, N>, N> M{};
        for (std::size_t j = 0; j < N; ++j) for (auto c : images[j]) M[c][j] += 1;
        auto edges = build_junction_graph<N>(images);
        CoincidenceClosure<N> closure(edges, M);

        for (long long gap = 1; gap <= 6; ++gap) {
            long long found = first_coincidence_gap<N>(closure, gap, 40);
            std::cout << "gcd=1 case, gap=" << gap << ": " << (found < 0 ? "NOT found by 40" : std::to_string(found)) << "\n";
            assert(found > 0);  // no obstruction possible -- every gap must resolve
        }
        // gap=1 specifically must match the already-established ground
        // truth (brute-force materialization found depth 12 for this
        // exact pair earlier tonight).
        assert(first_coincidence_gap<N>(closure, 1, 40) == 12);
    }

    std::cout << "coincidence_gcd_obstruction: gcd=2 case blocks every odd gap and resolves every "
                 "even gap; gcd=1 case has no obstruction at all and reproduces the known depth-12 "
                 "ground truth exactly.\n";
    return 0;
}
