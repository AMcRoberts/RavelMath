#include <cassert>
#include <iostream>

#include "ravel/proof/generalized_campaign.hpp"

using namespace ravel::proof::generalized;

int main() {
    const auto registry = standard_registry();
    const Executor executor(registry);

    const WordFamilyEvidence class_ii{"classII(3,2)", {{0,0,0,1,1,2}, {0,0,0,2}, {0}}};
    Plan word_plan{"word-family-pipeline", {
        {"sites", "word.enumerate_adjacent_unequal_sites", Compartment::WordLanguage, {}, class_ii, {}},
        {"parikh", "word.certify_swap_parikh_preservation", Compartment::WordLanguage, {"sites"}, class_ii, {}},
    }};
    const auto word_result = executor.run(word_plan);
    assert(word_result.all_closed());
    const auto& sites = std::get<AdjacentSiteCertificate>(word_result.artifact("sites").evidence);
    assert(sites.exhaustive && sites.sites.size() == 3);
    const auto& parikh = std::get<ParikhSwapCertificate>(word_result.artifact("parikh").evidence);
    assert(parikh.all_preserve_parikh && parikh.certified_swaps == 3);

    const AffineFamilyEvidence affine{"catalogue.A", {2, -1, 7}, {3, 4, -2}, 3};
    ParameterMap p5; p5.integers["parameter"] = 5;
    ParameterMap p11; p11.integers["parameter"] = 11;
    Plan affine_plan{"affine-family-pipeline", {
        {"A5", "affine.evaluate", Compartment::AffineCatalogue, {}, affine, p5},
        {"A11", "affine.evaluate", Compartment::AffineCatalogue, {}, affine, p11},
        {"difference", "affine.certify_difference_law", Compartment::AffineCatalogue,
            {"A5", "A11"}, affine, {}},
    }};
    const auto affine_result = executor.run(affine_plan);
    assert(affine_result.all_closed());
    const auto& diff = std::get<AffineDifferenceCertificate>(affine_result.artifact("difference").evidence);
    assert(diff.equals_parameter_delta_times_direction);
    assert((diff.difference == std::vector<std::int64_t>{18, 24, -12}));

    const GraphEvidence graph{"G", {{1,2}, {2}, {0}}, true};
    const MatrixFamilyEvidence matrix{"M", {{2,0}, {0,2}}};
    Plan mixed_plan{"cross-compartment-pipeline", {
        {"edges", "graph.certify_outdegree_handshake", Compartment::Graph, {}, graph, {}},
        {"trace", "matrix.trace", Compartment::Matrix, {}, matrix, {}},
        {"agreement", "cross.assert_integer_equal", Compartment::CrossCompartment,
            {"edges", "trace"}, std::nullopt, {}},
    }};
    const auto mixed_result = executor.run(mixed_plan);
    assert(mixed_result.all_closed());

    const RecurrenceEvidence recurrence{"tribonacci", {1,1,1}};
    Plan spectral_plan{"spectral-pipeline", {
        {"charpoly", "spectral.recurrence_characteristic_polynomial", Compartment::Spectral,
            {}, recurrence, {}},
    }};
    const auto spectral_result = executor.run(spectral_plan);
    assert(spectral_result.all_closed());
    const auto& polynomial = std::get<CharacteristicPolynomialCertificate>(
        spectral_result.artifact("charpoly").evidence);
    assert((polynomial.coefficients == std::vector<std::int64_t>{1,-1,-1,-1}));

    std::cout << word_result.report();
    std::cout << affine_result.report();
    std::cout << mixed_result.report();
    std::cout << spectral_result.report();
}
