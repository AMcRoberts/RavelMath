#pragma once

#include <algorithm>
#include <cstdint>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "ravel/proof/generalized_campaign.hpp"
#include "ravel/proof/multi_compartment_closure.hpp"

namespace ravel::proof::generalized {

inline std::vector<std::int64_t> normalize_ascending_terms(
        const std::vector<PolynomialTerm>& terms) {
    std::size_t degree = 0;
    for (const auto& term : terms) degree = std::max(degree, term.exponent);
    std::vector<std::int64_t> coefficients(terms.empty() ? 1 : degree + 1, 0);
    for (const auto& term : terms) coefficients.at(term.exponent) += term.coefficient;
    while (coefficients.size() > 1 && coefficients.back() == 0) coefficients.pop_back();
    return coefficients;
}

inline std::string render_int_list(const std::vector<std::int64_t>& values) {
    std::ostringstream out;
    out << "[";
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (i) out << ", ";
        out << values[i];
    }
    out << "]";
    return out.str();
}

inline LeanDeclaration geometric_recurrence_lean_declaration(const std::string& requested_name) {
    const auto name = lean_identifier(requested_name);
    const auto normal_form = name + "_normalForm";
    std::ostringstream out;
    out << "noncomputable def " << normal_form << " : ℕ → Polynomial ℤ\n"
        << "  | 0 => 1\n"
        << "  | n + 1 => Polynomial.X * " << normal_form << " n + 1\n\n"
        << "theorem " << name << "\n"
        << "    (f : ℕ → Polynomial ℤ)\n"
        << "    (h0 : f 0 = 1)\n"
        << "    (hstep : ∀ n : ℕ, f (n + 1) = Polynomial.X * f n + 1) :\n"
        << "    ∀ n : ℕ, f n = " << normal_form << " n := by\n"
        << "  intro n\n"
        << "  induction n with\n"
        << "  | zero =>\n"
        << "      simpa [" << normal_form << "] using h0\n"
        << "  | succ n ih =>\n"
        << "      rw [hstep n, ih]\n"
        << "      rfl\n";
    return {name, out.str()};
}

inline void install_shared_polynomial_closure_operations(OperationRegistry& registry) {
    registry.install("polynomial.normalize_expression",
        [](const Task& task, const EvidenceStore&) {
            if (!task.seed) throw std::logic_error("polynomial normalization requires seed evidence");
            const auto& expression = std::get<PolynomialExpressionEvidence>(*task.seed);
            auto coefficients = normalize_ascending_terms(expression.terms);
            PolynomialNormalFormCertificate cert{expression.expression_id,
                                                   std::move(coefficients), true};
            return ProofArtifact{task.task_id, task.compartment, cert, {},
                {"polynomial expression reduced by collecting equal exponents"}, true};
        });

    registry.install("closure.polynomial_normal_form_equality",
        [](const Task& task, const EvidenceStore& store) {
            if (task.dependencies.size() != 2)
                throw std::logic_error("normal-form equality requires two dependencies");
            const auto& lhs = store.evidence<PolynomialNormalFormCertificate>(task.dependencies[0]);
            const auto& rhs = store.evidence<PolynomialNormalFormCertificate>(task.dependencies[1]);
            if (!lhs.canonical || !rhs.canonical)
                throw std::logic_error("noncanonical polynomial evidence");
            const bool valid = lhs.coefficients == rhs.coefficients;
            if (!valid) throw std::logic_error("ascending polynomial normal forms disagree");
            const auto name = lean_identifier(task.parameters.strings.at("lean_name"));
            LeanDeclaration declaration{name,
                "theorem " + name + " : (" + render_int_list(lhs.coefficients) +
                " : List Int) = " + render_int_list(rhs.coefficients) + " := by rfl"};
            ClosureCertificate cert{task.task_id, "ascending-polynomial-equality",
                task.dependencies, task.parameters.strings.at("conclusion"),
                lhs.coefficients, rhs.coefficients, true};
            return ProofArtifact{task.task_id, task.compartment, cert, {std::move(declaration)},
                {cert.conclusion, "polynomial expressions share one canonical coefficient vector"}, true};
        });

    registry.install("recurrence.close_geometric_polynomial",
        [](const Task& task, const EvidenceStore&) {
            if (!task.seed) throw std::logic_error("recurrence closure requires seed evidence");
            const auto& recurrence = std::get<FirstOrderPolynomialRecurrenceEvidence>(*task.seed);
            const bool base_matches = recurrence.base_ascending == std::vector<std::int64_t>{1};
            const bool step_matches = recurrence.multiplier_shift == 1 &&
                                      recurrence.increment_ascending == std::vector<std::int64_t>{1};
            if (!base_matches || !step_matches)
                throw std::logic_error("recurrence does not match the installed geometric closure schema");
            RecurrenceClosureCertificate cert{recurrence.recurrence_id,
                "f(0)=1; f(n+1)=X*f(n)+1; canonical recursive normal form", base_matches, step_matches, true};
            const auto name = task.parameters.strings.at("lean_name");
            auto declaration = geometric_recurrence_lean_declaration(name);
            return ProofArtifact{task.task_id, task.compartment, cert, {std::move(declaration)},
                {task.parameters.strings.at("conclusion"),
                 "arbitrary polynomial sequence closed to canonical recursive normal form by shared induction backend"}, true};
        });
}

}  // namespace ravel::proof::generalized
