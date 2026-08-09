// Probe property (F) on canonical beta-substitutions already derived by the
// exact digit-expansion machinery.  This is a Stage-2 data point: it keeps
// the generator/Q-R-S campaign honest by checking the independent geometric
// automaton on the same substitutions.

#include <array>
#include <cstdio>
#include <stdexcept>
#include <vector>

#include "adelic/coincidence_and_property_f.hpp"
#include "math/charpoly.hpp"
#include "math/linalg_qbeta.hpp"
#include "ravel/canonical_beta_substitution.hpp"

namespace {

template <std::size_t d>
void run_case(const char* name, const std::vector<std::vector<long long>>& source,
              long long node_budget = 300000) {
    std::array<std::vector<long long>, d> images{};
    std::vector<std::vector<long long>> matrix(d, std::vector<long long>(d, 0));
    for (std::size_t j = 0; j < d; ++j) {
        images[j] = source[j];
        for (long long letter : source[j]) ++matrix[static_cast<std::size_t>(letter)][j];
    }
    const auto cp = mathlib::charpoly_faddeev_leverrier(matrix);
    mathlib::QBetaRing ring(cp);
    std::vector<std::vector<long long>> transpose(d, std::vector<long long>(d, 0));
    for (std::size_t i = 0; i < d; ++i)
        for (std::size_t j = 0; j < d; ++j) transpose[j][i] = matrix[i][j];
    mathlib::EigenvectorResult eig;
    try {
        eig = mathlib::right_eigenvector_via_qbeta(transpose, ring);
    } catch (const std::exception& ex) {
        std::printf("%s: charpoly=%s qbeta dimension exception: %s\n",
                    name, mathlib::str(cp).c_str(), ex.what());
        return;
    }
    if (!eig.ok) {
        std::printf("%s: charpoly=%s left Perron eigenvector failed\n",
                    name, mathlib::str(cp).c_str());
        return;
    }
    const auto automaton = adelic::build_prefix_automaton<d>(images, eig.v, ring);
    adelic::PropertyFResult result;
    try {
        result = adelic::check_property_f<d>(automaton, node_budget);
    } catch (const std::exception& ex) {
        std::printf("%s: charpoly=%s prefixes=%zu propertyF exception: %s\n",
                    name, mathlib::str(cp).c_str(), automaton.distinct_prefixes.size(), ex.what());
        return;
    }
    const auto coin = adelic::check_strong_coincidence<d>(images);
    std::printf("%s: charpoly=%s prefixes=%zu coincidence=%s depth=%lld propertyF=%s nodes=%lld\n",
                name, mathlib::str(cp).c_str(), automaton.distinct_prefixes.size(),
                coin.holds ? "HOLDS" : (coin.inconclusive ? "INCONCLUSIVE" : "FAILS"),
                coin.depth_reached,
                result.holds ? "HOLDS" : (result.inconclusive ? "INCONCLUSIVE" : "FAILS"),
                result.nodes_explored);
}

}  // namespace

int main() {
    run_case<3>("supergolden x^3-x^2-1", {{0, 1}, {2}, {0}});
    run_case<3>("plastic x^3-x-1", {{1}, {2}, {0, 1}});
    run_case<7>("third-smallest Pisot (cyclotomic lift)",
                {{0, 1}, {2}, {3}, {0, 4}, {5}, {6}, {0}}, 1000000);
    run_case<3>("eventually-periodic x^3-2x^2-x+1", {{0, 0, 1}, {2}, {0, 1}}, 300000);
}
