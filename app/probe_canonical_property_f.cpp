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
              long long node_budget = 300000,
              bool retain_boundary_sinks = true) {
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
        result = adelic::check_property_f<d>(automaton, node_budget,
                                             nullptr, nullptr, nullptr, nullptr,
                                             nullptr, retain_boundary_sinks);
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

template <std::size_t d>
void run_reduced_field_case(const char* name,
                            const std::vector<std::vector<long long>>& source,
                            const mathlib::PolyZ& minimal_polynomial,
                            long long node_budget = 300000,
                            bool retain_boundary_sinks = true) {
    std::array<std::vector<long long>, d> images{};
    std::vector<std::vector<long long>> matrix(d, std::vector<long long>(d, 0));
    for (std::size_t j = 0; j < d; ++j) {
        images[j] = source[j];
        for (long long letter : source[j]) ++matrix[static_cast<std::size_t>(letter)][j];
    }
    mathlib::QBetaRing ring(minimal_polynomial);
    const auto eig = mathlib::left_eigenvector_via_qbeta_reduced_factor(matrix, ring);
    if (!eig.ok) throw std::runtime_error("reduced-field eigenvector: no nonsingular minor");
    const auto automaton = adelic::build_prefix_automaton<d>(images, eig.v, ring);
    const auto result = adelic::check_property_f<d>(automaton, node_budget,
                                                    nullptr, nullptr, nullptr, nullptr,
                                                    nullptr, retain_boundary_sinks);
    const auto coin = adelic::check_strong_coincidence<d>(images);
    std::printf("%s: reduced-field prefixes=%zu coincidence=%s depth=%lld propertyF=%s nodes=%lld\n",
                name, automaton.distinct_prefixes.size(),
                coin.holds ? "HOLDS" : (coin.inconclusive ? "INCONCLUSIVE" : "FAILS"),
                coin.depth_reached,
                result.holds ? "HOLDS" : (result.inconclusive ? "INCONCLUSIVE" : "FAILS"),
                result.nodes_explored);
}

mathlib::QBetaRing ring_from_low_first(const std::vector<long long>& low_first) {
    mathlib::PolyZ p;
    p.ensure_size(low_first.size() + 1);
    for (std::size_t i = 0; i < low_first.size(); ++i)
        mathlib::set_si(p.coeff(low_first.size() - 1 - i), low_first[i]);
    mathlib::set_si(p.coeff(low_first.size()), 1);
    return mathlib::QBetaRing(p);
}

template <std::size_t d>
void run_derived_terminating_case(const char* name,
                                  const std::vector<long long>& low_first,
                                  long long node_budget = 300000,
                                  bool retain_boundary_sinks = true) {
    const auto ring = ring_from_low_first(low_first);
    const auto interval = mathlib::isolate_beta(ring);
    const auto expansion = ravel::exact_greedy_beta_expansion_of_one(ring, interval);
    if (!expansion.terminated)
        throw std::runtime_error("derived probe expected a terminating expansion");
    const auto source = ravel::canonical_beta_substitution_from_digits(expansion.digits);
    run_reduced_field_case<d>(name, source, ring.charpoly_, node_budget,
                              retain_boundary_sinks);
}

void run_derived_terminating_case_dynamic(const char* name,
                                          const std::vector<long long>& low_first,
                                          long long node_budget = 300000,
                                          bool retain_boundary_sinks = true) {
    const auto ring = ring_from_low_first(low_first);
    const auto interval = mathlib::isolate_beta(ring);
    const auto expansion = ravel::exact_greedy_beta_expansion_of_one(ring, interval);
    if (!expansion.terminated)
        throw std::runtime_error("derived probe expected a terminating expansion");
    const auto source = ravel::canonical_beta_substitution_from_digits(expansion.digits);
    switch (source.size()) {
        case 2: run_reduced_field_case<2>(name, source, ring.charpoly_, node_budget, retain_boundary_sinks); break;
        case 3: run_reduced_field_case<3>(name, source, ring.charpoly_, node_budget, retain_boundary_sinks); break;
        case 4: run_reduced_field_case<4>(name, source, ring.charpoly_, node_budget, retain_boundary_sinks); break;
        case 5: run_reduced_field_case<5>(name, source, ring.charpoly_, node_budget, retain_boundary_sinks); break;
        case 6: run_reduced_field_case<6>(name, source, ring.charpoly_, node_budget, retain_boundary_sinks); break;
        case 7: run_reduced_field_case<7>(name, source, ring.charpoly_, node_budget, retain_boundary_sinks); break;
        case 8: run_reduced_field_case<8>(name, source, ring.charpoly_, node_budget, retain_boundary_sinks); break;
        default: throw std::runtime_error("derived probe: unsupported canonical alphabet size");
    }
}

}  // namespace

int main() {
    run_case<3>("supergolden x^3-x^2-1", {{0, 1}, {2}, {0}});
    run_case<3>("plastic x^3-x-1", {{1}, {2}, {0, 1}});
    const mathlib::PolyZ third_minpoly = {-1, 0, 1, -1, -1, 1};
    run_reduced_field_case<7>("third-smallest Pisot (minimal field)",
                              {{0, 1}, {2}, {3}, {0, 4}, {5}, {6}, {0}},
                              third_minpoly, 1000000);
    run_case<3>("eventually-periodic x^3-2x^2-x+1", {{0, 0, 1}, {2}, {0, 1}}, 300000);
    run_derived_terminating_case<4>("theta2 canonical x^4-x^3-1", {-1, 0, 0, -1}, 1000000);
    run_derived_terminating_case_dynamic("theta5 canonical x^6-x^5-x^4+x^2-1",
                                         {-1, -1, 0, 1, 0, -1}, 3000000, false);
}
