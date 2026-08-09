#include <cassert>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "ravel/proof/generalized_campaign.hpp"
#include "ravel/proof/legacy_campaign_bridge.hpp"
#include "ravel/proof/multi_compartment_closure.hpp"

using namespace ravel::proof::generalized;

static ParameterMap closure_parameters(std::string conclusion, std::string lean_name) {
    ParameterMap p;
    p.strings["conclusion"] = std::move(conclusion);
    p.strings["lean_name"] = std::move(lean_name);
    return p;
}

int main() {
    auto registry = standard_registry();
    install_multi_compartment_closure_operations(registry);
    Executor executor(registry);

    const WordFamilyEvidence words{"classII", {{0,0,1,1,2}, {0,0,2}, {0}}};
    Plan word_plan{"word-closure", {
        {"sites", "word.enumerate_adjacent_unequal_sites", Compartment::WordLanguage, {}, words, {}},
        {"swaps", "word.certify_swap_parikh_preservation", Compartment::WordLanguage, {"sites"}, words, {}},
        {"site_swap_count", "closure.scalar_equality", Compartment::WordLanguage,
            {"sites", "swaps"}, std::nullopt,
            closure_parameters("literal swap sites and certified swaps have equal cardinality",
                               "classII_site_swap_count")},
        {"word_theorem", "closure.compose_theorem", Compartment::WordLanguage,
            {"site_swap_count", "swaps"}, std::nullopt,
            closure_parameters("the Class-II adjacent-swap certificate is closed",
                               "classII_adjacent_swap_closed")},
        {"word_validate", "closure.validate_artifact", Compartment::WordLanguage,
            {"word_theorem"}, std::nullopt, {}},
    }};
    const auto word = executor.run(word_plan);
    assert(word.all_closed());


    const AffineFamilyEvidence affine{"catalogue.A", {2, -1, 7}, {3, 4, -2}, 3};
    ParameterMap p5; p5.integers["parameter"] = 5;
    ParameterMap p11; p11.integers["parameter"] = 11;
    Plan affine_plan{"affine-closure", {
        {"A5", "affine.evaluate", Compartment::AffineCatalogue, {}, affine, p5},
        {"A11", "affine.evaluate", Compartment::AffineCatalogue, {}, affine, p11},
        {"transport", "affine.certify_difference_law", Compartment::AffineCatalogue,
            {"A5", "A11"}, affine, {}},
        {"affine_theorem", "closure.compose_theorem", Compartment::AffineCatalogue,
            {"A5", "A11", "transport"}, std::nullopt,
            closure_parameters("the affine catalogue transport certificate is closed",
                               "affine_catalogue_transport_closed")},
        {"affine_validate", "closure.validate_artifact", Compartment::AffineCatalogue,
            {"affine_theorem"}, std::nullopt, {}},
    }};
    const auto affine_result = executor.run(affine_plan);
    assert(affine_result.all_closed());

    const GraphEvidence graph{"G", {{1,2}, {2}, {0}}, true};
    const MatrixFamilyEvidence matrix{"M", {{2,0}, {0,2}}};
    Plan graph_matrix_plan{"graph-matrix-closure", {
        {"edges", "graph.certify_outdegree_handshake", Compartment::Graph, {}, graph, {}},
        {"trace", "matrix.trace", Compartment::Matrix, {}, matrix, {}},
        {"graph_matrix", "closure.scalar_equality", Compartment::CrossCompartment,
            {"edges", "trace"}, std::nullopt,
            closure_parameters("graph edge count agrees with matrix trace",
                               "graph_matrix_scalar_agreement")},
    }};
    const auto graph_matrix = executor.run(graph_matrix_plan);
    assert(graph_matrix.all_closed());

    const RecurrenceEvidence recurrence_a{"tribonacci.recurrence", {1,1,1}};
    const RecurrenceEvidence recurrence_b{"tribonacci.matrix", {1,1,1}};
    Plan spectral_plan{"spectral-closure", {
        {"from_recurrence", "spectral.recurrence_characteristic_polynomial", Compartment::Spectral,
            {}, recurrence_a, {}},
        {"from_matrix_model", "spectral.recurrence_characteristic_polynomial", Compartment::Matrix,
            {}, recurrence_b, {}},
        {"charpoly_agreement", "closure.polynomial_equality", Compartment::CrossCompartment,
            {"from_recurrence", "from_matrix_model"}, std::nullopt,
            closure_parameters("recurrence and matrix presentations have the same characteristic polynomial",
                               "tribonacci_charpoly_agreement")},
    }};
    const auto spectral = executor.run(spectral_plan);
    assert(spectral.all_closed());

    ravel::proof::ClosedProofArtifact legacy;
    legacy.artifact_id = "universal-n.final";
    legacy.established_facts = {"nbonacci universal-n determinant theorem kernel checked"};
    legacy.has_open_goals = false;
    auto imported = import_legacy_artifact("nbonacci_universal_n", "final", legacy);

    OperationRegistry bridge_registry;
    bridge_registry.install("seed.imported", [imported](const Task& task, const EvidenceStore&) {
        auto result = imported;
        result.artifact_id = task.task_id;
        return result;
    });
    install_multi_compartment_closure_operations(bridge_registry);
    Executor bridge_executor(bridge_registry);
    Plan bridge_plan{"legacy-closure-access", {
        {"legacy", "seed.imported", Compartment::Matrix, {}, std::nullopt, {}},
        {"legacy_validate", "closure.validate_artifact", Compartment::Matrix,
            {"legacy"}, std::nullopt, {}},
        {"legacy_compose", "closure.compose_theorem", Compartment::CrossCompartment,
            {"legacy", "legacy_validate"}, std::nullopt,
            closure_parameters("kernel-checked universal-n is available to generalized closure",
                               "universal_n_generalized_access")},
    }};
    const auto bridge = bridge_executor.run(bridge_plan);
    assert(bridge.all_closed());

    const std::string lean = emit_lean_module("RavelMultiCompartmentClosure", {
        word.artifact("word_theorem"),
        affine_result.artifact("affine_theorem"),
        graph_matrix.artifact("graph_matrix"),
        spectral.artifact("charpoly_agreement"),
        bridge.artifact("legacy_compose"),
    });
    assert(lean.find("classII_adjacent_swap_closed") != std::string::npos);
    assert(lean.find("affine_catalogue_transport_closed") != std::string::npos);
    assert(lean.find("graph_matrix_scalar_agreement") != std::string::npos);
    assert(lean.find("tribonacci_charpoly_agreement") != std::string::npos);
    assert(lean.find("universal_n_generalized_access") != std::string::npos);

    std::cout << word.report();
    std::cout << affine_result.report();
    std::cout << graph_matrix.report();
    std::cout << spectral.report();
    std::cout << bridge.report();
    std::cout << lean;

    std::ofstream out("lean/generated/multi_compartment_closure_validation.lean");
    out << lean;
    out.close();
}
