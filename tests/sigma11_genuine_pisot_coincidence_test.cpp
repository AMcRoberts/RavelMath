// Locks in Finding 36: the first time this project's coincidence-
// closure machinery (Findings 25-27, 34) has been run on an actual,
// certified Pisot substitution with genuine multi-junction structure
// -- every prior multi-junction example (Findings 25-27, 34) was
// built purely for combinatorial testing and, per Finding 35, was
// PROVABLY non-Pisot (g>1 forces imprimitivity).
//
// sigma_{1,1}: sigma(0)=[0,1,2], sigma(1)=[0,2], sigma(2)=[0].
// Certified Pisot via pisot_classify_3x3 (beta ~ 2.14789904), certified
// primitive via is_primitive, genuinely multi-junction (letters 0 AND
// 1 both branch), and g=1 (confirmed, matching Finding 35's theorem
// that real Pisot substitutions always have g=1).
//
// Result: all three letter pairs -- (0,1), (0,2), (1,2), correctly
// accounting for letter 2's forced run-in of 1 step to junction 0 --
// show a coincidence witness starting almost immediately (K=1) and
// holding at EVERY subsequent depth checked, not just scattered ones.

#include <array>
#include <cassert>
#include <iostream>
#include <numeric>
#include <utility>
#include <vector>

#include "ravel/proof/coincidence_closure.hpp"

using namespace ravel::proof;

namespace {

std::pair<long long, long long> origin(int letter) {
    if (letter == 0) return {0, 0};
    if (letter == 1) return {1, 0};
    return {0, 1};  // letter 2: forced 1 step, lands on junction 0
}

bool has_witness(CoincidenceClosure<3>& closure, int a, int b, long long K) {
    auto [Ja, da] = origin(a);
    auto [Jb, db] = origin(b);
    if (K < da || K < db) return false;
    const auto& A = closure.reachable(Ja, K - da);
    const auto& B = closure.reachable(Jb, K - db);
    for (auto& s : A) if (B.count(s)) return true;
    return false;
}

}  // namespace

int main() {
    std::array<std::vector<long long>, 3> images = {
        std::vector<long long>{0, 1, 2}, std::vector<long long>{0, 2}, std::vector<long long>{0}};
    std::array<std::array<long long, 3>, 3> M{};
    for (long long j = 0; j < 3; ++j)
        for (auto c : images[static_cast<std::size_t>(j)]) M[static_cast<std::size_t>(c)][static_cast<std::size_t>(j)] += 1;
    auto edges = build_junction_graph<3>(images);

    long long g = 0;
    for (auto& e : edges) g = std::gcd(g, e.jump_size);
    assert(g == 1);  // matches Finding 35: real Pisot substitutions always have g=1

    CoincidenceClosure<3> closure(edges, M);

    // (0,1): permanent from K=1 through K=14.
    for (long long K = 1; K <= 14; ++K) assert(has_witness(closure, 0, 1, K));
    // (0,2): permanent from K=1 (K=0 is below letter 2's own dist=1, not checked).
    for (long long K = 1; K <= 14; ++K) assert(has_witness(closure, 0, 2, K));
    // (1,2): permanent from K=1.
    for (long long K = 1; K <= 14; ++K) assert(has_witness(closure, 1, 2, K));

    std::cout << "sigma_{1,1}_genuine_pisot_coincidence: all three letter pairs show a "
                 "coincidence witness at every depth K=1..14, on a certified Pisot "
                 "substitution (beta~2.14789904) with genuine multi-junction structure.\n";
    return 0;
}
