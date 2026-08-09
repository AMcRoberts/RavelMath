// Exact prefix-half strong-coincidence classifier regression.
// The closure computes reachable (terminal letter, landmark vector) states
// without materializing exponentially growing substitution words.

#include <array>
#include <cassert>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

#include "adelic/coincidence_and_property_f.hpp"
#include "ravel/proof/coincidence_closure.hpp"
#include "ravel/proof/reflective_lean_renderer.hpp"
#include "ravel/proof/strong_coincidence_certificate.hpp"

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
    const auto closure_full = check_strong_coincidence_closure<d>(
        images, matrix, 20, 1'000'000);
    assert(closure_full.holds && !closure_full.inconclusive);
    assert(closure_full.pair_resolution_depths == direct.pair_resolution_depths);

    mathlib::reflection::Trace trace("coincidence_closure_prefix_reflection");
    {
        mathlib::reflection::ScopedTrace scope(&trace);
        assert(stage_strong_coincidence_prefix_closure<d>(
                   images, 20, 1'000'000, "sigma_0_1 prefix closure") ==
               StrongCoincidencePrefixClosureStageResult::staged);
        assert(stage_strong_coincidence_closure<d>(
                   images, 20, 1'000'000, "sigma_0_1 full closure") ==
               StrongCoincidenceClosureStageResult::staged);
    }
    const auto staged = trace.find<
        mathlib::reflection::StrongCoincidencePrefixClosureCertificate>();
    assert(staged.size() == 1);
    assert(staged.front().second->pair_resolution_depths ==
           prefix.pair_resolution_depths);
    const auto full_staged = trace.find<
        mathlib::reflection::StrongCoincidenceClosureCertificate>();
    assert(full_staged.size() == 1);
    assert(full_staged.front().second->pair_resolution_depths ==
           direct.pair_resolution_depths);
    const std::string rendered = render_reflective_lean_module(trace);
    assert(rendered.find("strong_coincidence_prefix_closure_0_summary") !=
           std::string::npos);
    assert(rendered.find("strong_coincidence_closure_0_summary") !=
           std::string::npos);
    if (const char* path = std::getenv("RAVEL_PREFIX_CLOSURE_LEAN_OUT")) {
        std::ofstream out(path);
        out << rendered;
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
    const auto disjoint_full = check_strong_coincidence_closure<2>(
        disjoint, disjoint_matrix, 8, 1000);
    assert(!disjoint_full.holds && disjoint_full.inconclusive);
    const std::array<std::vector<long long>, 2> deterministic_cycle = {
        std::vector<long long>{1}, std::vector<long long>{0}};
    const std::array<std::array<long long, 2>, 2> cycle_matrix = {{{{0, 1}}, {{1, 0}}}};
    bool cycle_rejected = false;
    try {
        (void)check_prefix_coincidence_closure<2>(
            deterministic_cycle, cycle_matrix, 8, 1000);
    } catch (const std::invalid_argument&) {
        cycle_rejected = true;
    }
    assert(cycle_rejected);
    mathlib::reflection::Trace rejected_trace("coincidence_closure_rejection");
    {
        mathlib::reflection::ScopedTrace scope(&rejected_trace);
        assert(stage_strong_coincidence_closure<2>(
                   deterministic_cycle, 8, 1000, "deterministic cycle") ==
               StrongCoincidenceClosureStageResult::unsupported);
    }

    // Genuine multi-junction Pisot regression (sigma_{1,1}); letter 2 has
    // a forced one-step run into junction 0, while 0 and 1 branch.
    const std::array<std::vector<long long>, 3> sigma11 = {
        std::vector<long long>{0, 1, 2}, std::vector<long long>{0, 2},
        std::vector<long long>{0}};
    const auto sigma11_matrix = incidence_matrix<3>(sigma11);
    const auto sigma11_direct = adelic::check_strong_coincidence<3>(sigma11, 14, 1'000'000);
    const auto sigma11_closure = check_strong_coincidence_closure<3>(
        sigma11, sigma11_matrix, 14, 1'000'000);
    assert(sigma11_direct.holds && sigma11_closure.holds);
    assert(sigma11_closure.pair_resolution_depths ==
           sigma11_direct.pair_resolution_depths);

    // The non-unit rnd13 instance used by the finite Property-(F) classifier;
    // every pair resolves at depth 1, so the closure stays tiny despite the
    // larger alphabet and non-unimodular matrix.
    const std::array<std::vector<long long>, 4> rnd13 = {
        std::vector<long long>{0, 0, 1, 2, 3, 3},
        std::vector<long long>{0, 0, 2, 3, 3},
        std::vector<long long>{0, 0, 3, 3},
        std::vector<long long>{0, 0, 0, 2, 3, 3}};
    const auto rnd13_matrix = incidence_matrix<4>(rnd13);
    const auto rnd13_closure = check_strong_coincidence_closure<4>(
        rnd13, rnd13_matrix, 2, 1'000'000);
    assert(rnd13_closure.holds);
    assert(rnd13_closure.pair_resolution_depths == std::vector<long long>(6, 1));

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
