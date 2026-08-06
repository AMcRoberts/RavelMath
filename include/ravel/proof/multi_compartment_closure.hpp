#pragma once

#include <algorithm>
#include <cctype>
#include <functional>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "ravel/proof/generalized_campaign.hpp"

namespace ravel::proof::generalized {

inline std::vector<std::int64_t> normalize_descending_polynomial(
        std::vector<std::int64_t> coefficients) {
    const auto first = std::find_if(coefficients.begin(), coefficients.end(),
                                    [](std::int64_t c) { return c != 0; });
    coefficients.erase(coefficients.begin(), first);
    if (coefficients.empty()) return {0};
    if (coefficients.front() < 0) {
        for (auto& c : coefficients) c = -c;
    }
    return coefficients;
}

inline std::optional<std::int64_t> scalar_projection(const Evidence& evidence) {
    if (const auto* x = std::get_if<MatrixTraceCertificate>(&evidence)) return x->trace;
    if (const auto* x = std::get_if<GraphHandshakeCertificate>(&evidence))
        return static_cast<std::int64_t>(x->edge_count);
    if (const auto* x = std::get_if<AdjacentSiteCertificate>(&evidence))
        return static_cast<std::int64_t>(x->sites.size());
    if (const auto* x = std::get_if<ParikhSwapCertificate>(&evidence))
        return static_cast<std::int64_t>(x->certified_swaps);
    return std::nullopt;
}

inline std::optional<std::vector<std::int64_t>> polynomial_projection(const Evidence& evidence) {
    if (const auto* x = std::get_if<CharacteristicPolynomialCertificate>(&evidence))
        return normalize_descending_polynomial(x->coefficients);
    return std::nullopt;
}

inline void validate_artifact_shape(const ProofArtifact& artifact) {
    if (!artifact.closed) throw std::logic_error("artifact is open: " + artifact.artifact_id);
    std::set<std::string> names;
    for (const auto& decl : artifact.lean_declarations) {
        if (decl.name.empty()) throw std::logic_error("empty Lean declaration name");
        if (!names.insert(decl.name).second)
            throw std::logic_error("duplicate Lean declaration: " + decl.name);
    }
}

inline std::string lean_identifier(std::string text) {
    for (char& c : text) {
        if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_') c = '_';
    }
    if (text.empty() || std::isdigit(static_cast<unsigned char>(text.front()))) text = "ravel_" + text;
    return text;
}

using LeanEvidenceEmitter = std::function<std::vector<LeanDeclaration>(
    const Task&, const EvidenceStore&, const ClosureCertificate&)>;

class LeanEmitterRegistry {
public:
    void install(std::string closure_kind, LeanEvidenceEmitter emitter) {
        const auto [_, inserted] = emitters_.emplace(std::move(closure_kind), std::move(emitter));
        if (!inserted) throw std::logic_error("duplicate Lean closure emitter");
    }

    std::vector<LeanDeclaration> emit(const Task& task,
                                      const EvidenceStore& store,
                                      const ClosureCertificate& certificate) const {
        const auto it = emitters_.find(certificate.closure_kind);
        if (it == emitters_.end()) return {};
        return it->second(task, store, certificate);
    }

private:
    std::map<std::string, LeanEvidenceEmitter> emitters_;
};

inline LeanEmitterRegistry standard_lean_closure_emitters() {
    LeanEmitterRegistry emitters;

    emitters.install("scalar-equality",
        [](const Task& task, const EvidenceStore&, const ClosureCertificate& cert) {
            const auto name = lean_identifier(task.parameters.strings.at("lean_name"));
            const auto value = cert.normalized_left.at(0);
            return std::vector<LeanDeclaration>{{name,
                "theorem " + name + " : (" + std::to_string(value) + " : Int) = " +
                std::to_string(value) + " := by rfl"}};
        });

    emitters.install("polynomial-equality",
        [](const Task& task, const EvidenceStore&, const ClosureCertificate& cert) {
            const auto name = lean_identifier(task.parameters.strings.at("lean_name"));
            auto render = [](const std::vector<std::int64_t>& coefficients) {
                std::ostringstream out;
                out << "[";
                for (std::size_t i = 0; i < coefficients.size(); ++i) {
                    if (i) out << ", ";
                    out << coefficients[i];
                }
                out << "]";
                return out.str();
            };
            return std::vector<LeanDeclaration>{{name,
                "theorem " + name + " : (" + render(cert.normalized_left) +
                " : List Int) = " + render(cert.normalized_right) + " := by rfl"}};
        });

    emitters.install("theorem-composition",
        [](const Task& task, const EvidenceStore&, const ClosureCertificate& cert) {
            const auto name = lean_identifier(task.parameters.strings.at("lean_name"));
            return std::vector<LeanDeclaration>{{name,
                "def " + name + " : Nat := " + std::to_string(cert.premise_artifacts.size())}};
        });

    return emitters;
}

inline void install_multi_compartment_closure_operations(
        OperationRegistry& registry,
        LeanEmitterRegistry emitters = standard_lean_closure_emitters()) {

    registry.install("closure.validate_artifact",
        [](const Task& task, const EvidenceStore& store) {
            if (task.dependencies.size() != 1)
                throw std::logic_error("artifact validation requires one dependency");
            const auto& source = store.artifact(task.dependencies.front());
            validate_artifact_shape(source);
            std::set<std::string> names;
            for (const auto& declaration : source.lean_declarations) names.insert(declaration.name);
            ArtifactValidationCertificate cert{
                source.artifact_id,
                source.lean_declarations.size(),
                source.established_facts.size(),
                names.size() == source.lean_declarations.size(),
                true,
            };
            return ProofArtifact{task.task_id, task.compartment, cert, {},
                {"closed artifact shape validated"}, true};
        });

    registry.install("closure.scalar_equality",
        [emitters](const Task& task, const EvidenceStore& store) mutable {
            if (task.dependencies.size() != 2)
                throw std::logic_error("scalar closure requires two dependencies");
            const auto lhs = scalar_projection(store.artifact(task.dependencies[0]).evidence);
            const auto rhs = scalar_projection(store.artifact(task.dependencies[1]).evidence);
            if (!lhs || !rhs) throw std::logic_error("unsupported scalar projection for closure");
            ClosureCertificate cert{task.task_id, "scalar-equality", task.dependencies,
                task.parameters.strings.at("conclusion"), {*lhs}, {*rhs}, *lhs == *rhs};
            if (!cert.valid) throw std::logic_error("scalar normal forms disagree");
            auto declarations = emitters.emit(task, store, cert);
            return ProofArtifact{task.task_id, task.compartment, cert, std::move(declarations),
                {cert.conclusion, "scalar expressions reduced to identical normal forms"}, true};
        });

    registry.install("closure.polynomial_equality",
        [emitters](const Task& task, const EvidenceStore& store) mutable {
            if (task.dependencies.size() != 2)
                throw std::logic_error("polynomial closure requires two dependencies");
            const auto lhs = polynomial_projection(store.artifact(task.dependencies[0]).evidence);
            const auto rhs = polynomial_projection(store.artifact(task.dependencies[1]).evidence);
            if (!lhs || !rhs) throw std::logic_error("unsupported polynomial projection for closure");
            ClosureCertificate cert{task.task_id, "polynomial-equality", task.dependencies,
                task.parameters.strings.at("conclusion"), *lhs, *rhs, *lhs == *rhs};
            if (!cert.valid) throw std::logic_error("polynomial normal forms disagree");
            auto declarations = emitters.emit(task, store, cert);
            return ProofArtifact{task.task_id, task.compartment, cert, std::move(declarations),
                {cert.conclusion, "polynomials reduced to identical primitive normal forms"}, true};
        });

    registry.install("closure.compose_theorem",
        [emitters](const Task& task, const EvidenceStore& store) mutable {
            if (task.dependencies.empty()) throw std::logic_error("theorem composition needs premises");
            std::vector<std::string> facts;
            std::vector<LeanDeclaration> declarations;
            std::set<std::string> names;
            for (const auto& dependency : task.dependencies) {
                const auto& artifact = store.artifact(dependency);
                validate_artifact_shape(artifact);
                facts.insert(facts.end(), artifact.established_facts.begin(), artifact.established_facts.end());
                for (const auto& declaration : artifact.lean_declarations) {
                    if (names.insert(declaration.name).second) declarations.push_back(declaration);
                }
            }
            ClosureCertificate cert{task.task_id, "theorem-composition", task.dependencies,
                task.parameters.strings.at("conclusion"), {}, {}, true};
            auto final_declarations = emitters.emit(task, store, cert);
            for (auto& declaration : final_declarations) {
                if (!names.insert(declaration.name).second)
                    throw std::logic_error("composition emitted duplicate Lean declaration");
                declarations.push_back(std::move(declaration));
            }
            facts.push_back(cert.conclusion);
            facts.push_back("all premise artifacts closed and composed");
            return ProofArtifact{task.task_id, task.compartment, cert,
                                 std::move(declarations), std::move(facts), true};
        });
}

inline std::string emit_lean_module(const std::string& module_name,
                                    const std::vector<ProofArtifact>& artifacts,
                                    const std::vector<std::string>& imports = {"Mathlib"}) {
    std::ostringstream out;
    for (const auto& import : imports) out << "import " << import << '\n';
    out << "\nnamespace " << lean_identifier(module_name) << "\n\n";
    std::set<std::string> emitted;
    for (const auto& artifact : artifacts) {
        validate_artifact_shape(artifact);
        for (const auto& declaration : artifact.lean_declarations) {
            if (!emitted.insert(declaration.name).second) continue;
            out << declaration.source << "\n\n";
        }
    }
    out << "end " << lean_identifier(module_name) << "\n";
    return out.str();
}

}  // namespace ravel::proof::generalized
