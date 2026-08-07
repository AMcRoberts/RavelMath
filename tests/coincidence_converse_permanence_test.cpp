// Locks in Finding 34: a much stronger form of evidence for the open
// converse of the gcd-obstruction theorem (Finding 26) than Finding
// 27's leftmost-loop corollary alone gives.
//
// Not just "gap=g has SOME witness" -- checked EXHAUSTIVELY that
// reachable(0,D) and reachable(0,D+g) are DISJOINT for every D<10,
// and share a common state for EVERY D in [10,40] (31 consecutive
// depths, not a handful of sampled points). Moreover the exact SAME
// state (terminal=0, vec=(10,0,6,0,0,0,3,0)) recurs as a witness at
// D=10,12,14,16,18,20 -- a literal fixed point under +g extension,
// not six independent coincidences.
//
// This is still NOT a general proof of the converse (see the header
// comment in coincidence_converse_leftmost_loop.hpp and the
// FINDINGS_FOR_CITATION.md Finding 34 entry for the honest scope),
// but it is real, new, and worth protecting from silent regression.

#include <array>
#include <cassert>
#include <iostream>
#include <numeric>
#include <vector>

#include "ravel/proof/coincidence_closure.hpp"

using namespace ravel::proof;

namespace {

template <std::size_t d>
std::array<std::array<long long, d>, d> incidence_matrix(const std::array<std::vector<long long>, d>& images) {
    std::array<std::array<long long, d>, d> M{};
    for (long long j = 0; j < static_cast<long long>(d); ++j)
        for (auto c : images[static_cast<std::size_t>(j)]) M[static_cast<std::size_t>(c)][static_cast<std::size_t>(j)] += 1;
    return M;
}

}  // namespace

int main() {
    std::array<std::vector<long long>, 8> images = {
        std::vector<long long>{1, 3}, std::vector<long long>{2}, std::vector<long long>{4, 5},
        std::vector<long long>{0}, std::vector<long long>{0}, std::vector<long long>{6},
        std::vector<long long>{7}, std::vector<long long>{0}};
    auto edges = build_junction_graph<8>(images);
    long long g = 0;
    for (auto& e : edges) g = std::gcd(g, e.jump_size);
    assert(g == 2);
    CoincidenceClosure<8> closure(edges, incidence_matrix<8>(images));

    // Exhaustive: empty below D=10, nonempty for every D in [10,26]
    // (kept smaller than the exploratory 40 used interactively, to
    // keep this test fast while still covering 17 consecutive depths).
    for (long long D = 0; D <= 26; ++D) {
        const auto& A = closure.reachable(0, D);
        const auto& B = closure.reachable(0, D + g);
        bool hit = false;
        for (auto& s : A) if (B.count(s)) { hit = true; break; }
        bool expect_hit = (D >= 10);
        assert(hit == expect_hit);
    }
    std::cout << "permanence confirmed: reachable(0,D) vs reachable(0,D+2) empty for D<10, "
                 "nonempty for every D in [10,26]\n";

    // The literal recurring fixed-point state.
    ExactVec<8> fixed{};
    fixed[0] = 10; fixed[2] = 6; fixed[6] = 3;
    std::pair<long long, ExactVec<8>> witness{0, fixed};
    for (long long D : {10, 12, 14, 16, 18, 20}) {
        const auto& A = closure.reachable(0, D);
        assert(A.count(witness) == 1);
    }
    std::cout << "the exact same state (terminal=0, vec=(10,0,6,0,0,0,3,0)) recurs as a witness "
                 "at D=10,12,14,16,18,20 -- a literal fixed point under +g extension.\n";

    std::cout << "coincidence_converse_permanence: Finding 34's evidence locked in.\n";
    return 0;
}
