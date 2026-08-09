// Exact prefix-half strong-coincidence classifier regression.
// The closure computes reachable (terminal letter, landmark vector) states
// without materializing exponentially growing substitution words.

#include <array>
#include <cassert>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>

#include "adelic/coincidence_and_property_f.hpp"
#include "ravel/proof/coincidence_closure.hpp"

using namespace ravel::proof;

namespace {

template <std::size_t d>
std::array<std::array<long long, d>, d> incidence_matrix(
    const std::array<std::vector<long long>, d>& images) {
    std::array<std::array<long long, d>, d> matrix{};
    for (std::size_t column = 0; column < d; ++column)
        for (long long letter : images[column])
            ++matrix[static_cast<std::size_t>(letter)][column];
    return matrix;
}

}  // namespace

int main() {
    // The non-AR single-junction control used by the landmark-vector
    // reduction: sigma(0)=12, sigma(1)=2, sigma(2)=0.
    constexpr std::size_t d = 3;
    const std::array<std::vector<long long>, d> images = {
        std::vector<long long>{1, 2},
        std::vector<long long>{2},
        std::vector<long long>{0}};
    const auto matrix = incidence_matrix<d>(images);

    const auto direct = adelic::check_strong_coincidence<d>(images, 20, 5'000'000);
    assert(direct.holds && !direct.inconclusive);
    const auto prefix = check_prefix_coincidence_closure<d>(images, matrix, 20, 1'000'000);
    assert(prefix.holds && !prefix.inconclusive);
    assert(prefix.unresolved_pairs == 0);
    assert(prefix.pair_resolution_depths.size() == direct.pair_resolution_depths.size());
    for (std::size_t i = 0; i < prefix.pair_resolution_depths.size(); ++i) {
        assert(prefix.pair_resolution_depths[i] > 0);
        // The full checker may resolve earlier through a suffix witness, but
        // never later than the prefix-only closure if both profiles are valid.
        assert(prefix.pair_resolution_depths[i] >= direct.pair_resolution_depths[i]);
    }

    // The closure remains honest at a finite cutoff: a pair with disjoint
    // deterministic cycles is unresolved, not misclassified as a failure.
    const std::array<std::vector<long long>, 2> disjoint = {
        std::vector<long long>{0, 0}, std::vector<long long>{1, 1}};
    const std::array<std::array<long long, 2>, 2> disjoint_matrix = {{{{2, 0}}, {{0, 2}}}};
    const auto bounded = check_prefix_coincidence_closure<2>(
        disjoint, disjoint_matrix, 8, 1000);
    assert(!bounded.holds && bounded.inconclusive);
    assert(bounded.pair_resolution_depths == std::vector<long long>{-1});

    bool overflow_rejected = false;
    try {
        const std::array<std::array<long long, 2>, 2> large = {{{{2, 0}}, {{0, 1}}}};
        const ExactVec<2> huge = {std::numeric_limits<long long>::max(), 0};
        (void)exact_matvec<2>(large, huge);
    } catch (const std::overflow_error&) {
        overflow_rejected = true;
    }
    assert(overflow_rejected);

    std::cout << "coincidence_closure_prefix: exact closure agrees with word search; "
                 "finite cutoff remains inconclusive\n";
}
