#include <cassert>
#include <iostream>
#include <string>

#include "ravel/proof/generalized_campaign.hpp"
#include "ravel/proof/multi_compartment_closure.hpp"
#include "ravel/proof/shared_polynomial_closure.hpp"

using namespace ravel::proof::generalized;

static ParameterMap parameters(std::string conclusion, std::string lean_name) {
    ParameterMap p;
    p.strings["conclusion"] = std::move(conclusion);
    p.strings["lean_name"] = std::move(lean_name);
    return p;
}

int main() {
    auto registry = standard_registry();
    install_multi_compartment_closure_operations(registry);
    install_shared_polynomial_closure_operations(registry);
    Executor executor(registry);

    const PolynomialExpressionEvidence matrix_expression{
        "matrix.cofactor.polynomial",
        {{1, 0}, {1, 1}, {1, 2}, {1, 3}, {-1, 0}, {1, 0}}
    };
    const PolynomialExpressionEvidence spectral_expression{
        "spectral.closed.polynomial",
        {{1, 3}, {1, 2}, {1, 1}, {1, 0}}
    };
    const FirstOrderPolynomialRecurrenceEvidence matrix_recurrence{
        "matrix.qdet", {1}, 1, {1}
    };
    const FirstOrderPolynomialRecurrenceEvidence spectral_recurrence{
        "spectral.geometric-prefix", {1}, 1, {1}
    };

    Plan plan{"shared-polynomial-and-recurrence-closure", {
        {"matrix_nf", "polynomial.normalize_expression", Compartment::Matrix,
            {}, matrix_expression, {}},
        {"spectral_nf", "polynomial.normalize_expression", Compartment::Spectral,
            {}, spectral_expression, {}},
        {"nf_agreement", "closure.polynomial_normal_form_equality", Compartment::CrossCompartment,
            {"matrix_nf", "spectral_nf"}, std::nullopt,
            parameters("matrix and spectral polynomial expressions normalize identically",
                       "matrix_spectral_polynomial_normal_form")},
        {"matrix_recurrence_closed", "recurrence.close_geometric_polynomial", Compartment::Matrix,
            {}, matrix_recurrence, parameters(
                "the matrix determinant recurrence has the geometric-sum closed form",
                "close_matrix_geometric_recurrence")},
        {"spectral_recurrence_closed", "recurrence.close_geometric_polynomial", Compartment::Spectral,
            {}, spectral_recurrence, parameters(
                "the spectral prefix recurrence has the geometric-sum closed form",
                "close_spectral_geometric_recurrence")},
        {"recurrence_composed", "closure.compose_theorem", Compartment::CrossCompartment,
            {"matrix_recurrence_closed", "spectral_recurrence_closed", "nf_agreement"},
            std::nullopt, parameters(
                "matrix and spectral compartments share recurrence and normalization closure",
                "shared_matrix_spectral_closure_manifest")},
    }};
    const auto result = executor.run(plan);
    if (!result.all_closed()) {
        std::cerr << result.report();
        return 1;
    }

    const std::string lean = emit_lean_module("RavelSharedPolynomialClosure", {
        result.artifact("nf_agreement"),
        result.artifact("matrix_recurrence_closed"),
        result.artifact("spectral_recurrence_closed"),
        result.artifact("recurrence_composed"),
    });
    assert(lean.find("close_matrix_geometric_recurrence") != std::string::npos);
    assert(lean.find("close_spectral_geometric_recurrence") != std::string::npos);
    assert(lean.find("matrix_spectral_polynomial_normal_form") != std::string::npos);
    assert(lean.find("∀ n : ℕ") != std::string::npos);
    assert(lean.find("_normalForm") != std::string::npos);
    assert(lean.find("Finset.range") == std::string::npos);

    std::cout << result.report();
    std::cout << lean;
}
