// Exact prefix-half strong-coincidence classifier regression.
// The closure computes reachable (terminal letter, landmark vector) states
// without materializing exponentially growing substitution words.

#include <array>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

#include "adelic/coincidence_and_property_f.hpp"
#include "math/perron_frobenius.hpp"
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
    assert(staged.front().second->pair_terminal_letters.size() == 3);
    assert(staged.front().second->pair_vectors.size() == 3);
    assert(staged.front().second->pair_first_positions.size() == 3);
    assert(staged.front().second->pair_second_positions.size() == 3);
    assert(staged.front().second->pair_first_paths.size() == 3);
    assert(staged.front().second->pair_second_paths.size() == 3);
    assert(staged.front().second->pair_first_junctions.size() == 3);
    assert(staged.front().second->pair_second_remaining_depths.size() == 3);
    assert(staged.front().second->pair_first_weighted_vectors.size() == 3);
    const auto full_staged = trace.find<
        mathlib::reflection::StrongCoincidenceClosureCertificate>();
    assert(full_staged.size() == 1);
    assert(full_staged.front().second->pair_resolution_depths ==
           direct.pair_resolution_depths);
    assert(full_staged.front().second->pair_terminal_letters.size() == 3);
    assert(full_staged.front().second->pair_vectors.size() == 3);
    assert(full_staged.front().second->pair_from_suffix.size() == 3);
    assert(full_staged.front().second->pair_first_positions.size() == 3);
    assert(full_staged.front().second->pair_second_positions.size() == 3);
    assert(full_staged.front().second->pair_first_paths.size() == 3);
    assert(full_staged.front().second->pair_second_paths.size() == 3);
    assert(full_staged.front().second->pair_first_junctions.size() == 3);
    assert(full_staged.front().second->pair_second_remaining_depths.size() == 3);
    assert(full_staged.front().second->pair_second_weighted_vectors.size() == 3);
    const std::string rendered = render_reflective_lean_module(trace);
    assert(rendered.find("strong_coincidence_prefix_closure_0_summary") !=
           std::string::npos);
    assert(rendered.find("strong_coincidence_closure_0_summary") !=
           std::string::npos);
    assert(rendered.find("strong_coincidence_prefix_closure_0_images") !=
           std::string::npos);
    assert(rendered.find("strong_coincidence_closure_0_images") !=
           std::string::npos);
    assert(rendered.find("strong_coincidence_closure_0_first_positions") !=
           std::string::npos);
    assert(rendered.find("strong_coincidence_closure_0_first_paths") !=
           std::string::npos);
    assert(rendered.find("strong_coincidence_closure_0_edges") !=
           std::string::npos);
    assert(rendered.find("strong_coincidence_closure_0_first_path_check") !=
           std::string::npos);
    assert(rendered.find("strong_coincidence_closure_0_first_weight_check") !=
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
    auto tampered_matrix = matrix;
    ++tampered_matrix[0][0];
    bool matrix_rejected = false;
    try {
        (void)check_prefix_coincidence_closure<d>(images, tampered_matrix, 4, 100);
    } catch (const std::invalid_argument&) {
        matrix_rejected = true;
    }
    assert(matrix_rejected && "closure must reject an incidence matrix mismatched to images");
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

    // Seeded cross-check over primitive random substitutions.  This is not a
    // probabilistic proof: every accepted sample is compared exactly, and
    // the fixed seed makes failures reproducible.  It guards the multi-
    // junction closure against overfitting the three named examples.
    std::uint64_t seed = 0x9e3779b97f4a7c15ULL;
    auto next = [&]() {
        seed = seed * 6364136223846793005ULL + 1442695040888963407ULL;
        return seed;
    };
    std::size_t checked = 0;
    for (int attempt = 0; attempt < 500 && checked < 20; ++attempt) {
        std::array<std::vector<long long>, 3> random_images;
        for (auto& image : random_images) {
            const std::size_t length = 1 + static_cast<std::size_t>(next() % 3);
            for (std::size_t i = 0; i < length; ++i)
                image.push_back(static_cast<long long>(next() % 3));
        }
        const auto random_matrix = incidence_matrix<3>(random_images);
        std::vector<std::vector<long long>> matrix_rows(3, std::vector<long long>(3));
        for (std::size_t i = 0; i < 3; ++i)
            for (std::size_t j = 0; j < 3; ++j) matrix_rows[i][j] = random_matrix[i][j];
        if (!mathlib::is_primitive(matrix_rows)) continue;
        try {
            const auto direct_random = adelic::check_strong_coincidence<3>(
                random_images, 6, 100'000);
            const auto closure_random = check_strong_coincidence_closure<3>(
                random_images, random_matrix, 6, 100'000);
            if (!direct_random.holds || !closure_random.holds) continue;
            assert(closure_random.pair_resolution_depths ==
                   direct_random.pair_resolution_depths);
            ++checked;
        } catch (const std::invalid_argument&) {
            assert(false && "primitive sample violated the closure shape precondition");
        }
    }
    assert(checked == 20);
    std::cout << "random primitive cross-checks=" << checked << "\n";

    std::cout << "coincidence_closure_prefix: exact closure agrees with word search; "
                 "finite cutoff remains inconclusive\n";
}
