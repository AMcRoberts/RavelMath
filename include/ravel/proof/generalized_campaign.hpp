#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <numeric>
#include <optional>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace ravel::proof::generalized {

enum class Compartment {
    Matrix,
    WordLanguage,
    Graph,
    AffineCatalogue,
    Spectral,
    CrossCompartment,
};

inline const char* compartment_name(Compartment c) {
    switch (c) {
        case Compartment::Matrix: return "matrix";
        case Compartment::WordLanguage: return "word-language";
        case Compartment::Graph: return "graph";
        case Compartment::AffineCatalogue: return "affine-catalogue";
        case Compartment::Spectral: return "spectral";
        case Compartment::CrossCompartment: return "cross-compartment";
    }
    return "unknown";
}

struct WordFamilyEvidence {
    std::string family_id;
    std::vector<std::vector<int>> images;
};

struct AdjacentSite {
    std::size_t image = 0;
    std::size_t offset = 0;
    int left = 0;
    int right = 0;

    friend bool operator==(const AdjacentSite&, const AdjacentSite&) = default;
    friend bool operator<(const AdjacentSite& a, const AdjacentSite& b) {
        return std::tie(a.image, a.offset, a.left, a.right) <
               std::tie(b.image, b.offset, b.left, b.right);
    }
};

struct AdjacentSiteCertificate {
    std::string family_id;
    std::vector<AdjacentSite> sites;
    bool exhaustive = false;
};

struct ParikhSwapCertificate {
    std::string family_id;
    std::size_t certified_swaps = 0;
    bool all_preserve_parikh = false;
};

struct AffineFamilyEvidence {
    std::string family_id;
    std::vector<std::int64_t> base;
    std::vector<std::int64_t> direction;
    std::int64_t base_parameter = 0;
};

struct AffineEvaluationCertificate {
    std::string family_id;
    std::int64_t parameter = 0;
    std::vector<std::int64_t> value;
};

struct AffineDifferenceCertificate {
    std::string family_id;
    std::int64_t left_parameter = 0;
    std::int64_t right_parameter = 0;
    std::vector<std::int64_t> difference;
    bool equals_parameter_delta_times_direction = false;
};

struct GraphEvidence {
    std::string graph_id;
    std::vector<std::vector<std::size_t>> adjacency;
    bool directed = true;
};

struct GraphHandshakeCertificate {
    std::string graph_id;
    std::size_t edge_count = 0;
    std::size_t out_degree_sum = 0;
    bool valid = false;
};

struct MatrixFamilyEvidence {
    std::string family_id;
    std::vector<std::vector<std::int64_t>> entries;
};

struct MatrixTraceCertificate {
    std::string family_id;
    std::int64_t trace = 0;
};

struct RecurrenceEvidence {
    std::string family_id;
    // x^k - c[0]x^(k-1) - ... - c[k-1]
    std::vector<std::int64_t> coefficients;
};

struct CharacteristicPolynomialCertificate {
    std::string family_id;
    // descending coefficients, monic first
    std::vector<std::int64_t> coefficients;
};

struct PolynomialTerm {
    std::int64_t coefficient = 0;
    std::size_t exponent = 0;
};

struct PolynomialExpressionEvidence {
    std::string expression_id;
    std::vector<PolynomialTerm> terms;
};

struct PolynomialNormalFormCertificate {
    std::string expression_id;
    // ascending coefficients: coefficient of X^i at index i
    std::vector<std::int64_t> coefficients;
    bool canonical = false;
};

struct FirstOrderPolynomialRecurrenceEvidence {
    std::string recurrence_id;
    // Supported shared schema: f 0 = 1 and f (n+1) = X * f n + 1.
    std::vector<std::int64_t> base_ascending;
    std::size_t multiplier_shift = 1;
    std::vector<std::int64_t> increment_ascending;
};

struct RecurrenceClosureCertificate {
    std::string recurrence_id;
    std::string schema;
    bool base_matches = false;
    bool step_matches = false;
    bool universally_closed = false;
};

struct LegacyCampaignArtifactEvidence {
    std::string source_campaign;
    std::string source_task;
    std::size_t definition_count = 0;
    std::size_t theorem_count = 0;
    std::vector<std::string> established_facts;
};


struct ClosureCertificate {
    std::string theorem_id;
    std::string closure_kind;
    std::vector<std::string> premise_artifacts;
    std::string conclusion;
    std::vector<std::int64_t> normalized_left;
    std::vector<std::int64_t> normalized_right;
    bool valid = false;
};

struct ArtifactValidationCertificate {
    std::string artifact_id;
    std::size_t declaration_count = 0;
    std::size_t fact_count = 0;
    bool declarations_unique = false;
    bool valid = false;
};
struct CrossCompartmentAgreementCertificate {
    std::string relation_id;
    bool agrees = false;
    std::string diagnostic;
};

using Evidence = std::variant<
    WordFamilyEvidence,
    AdjacentSiteCertificate,
    ParikhSwapCertificate,
    AffineFamilyEvidence,
    AffineEvaluationCertificate,
    AffineDifferenceCertificate,
    GraphEvidence,
    GraphHandshakeCertificate,
    MatrixFamilyEvidence,
    MatrixTraceCertificate,
    RecurrenceEvidence,
    CharacteristicPolynomialCertificate,
    PolynomialExpressionEvidence,
    PolynomialNormalFormCertificate,
    FirstOrderPolynomialRecurrenceEvidence,
    RecurrenceClosureCertificate,
    LegacyCampaignArtifactEvidence,
    ClosureCertificate,
    ArtifactValidationCertificate,
    CrossCompartmentAgreementCertificate>;

struct LeanDeclaration {
    std::string name;
    std::string source;
};

struct ProofArtifact {
    std::string artifact_id;
    Compartment compartment = Compartment::CrossCompartment;
    Evidence evidence;
    std::vector<LeanDeclaration> lean_declarations;
    std::vector<std::string> established_facts;
    bool closed = false;
};

struct ParameterMap {
    std::map<std::string, std::int64_t> integers;
    std::map<std::string, std::string> strings;

    std::int64_t integer(const std::string& key) const {
        const auto it = integers.find(key);
        if (it == integers.end()) throw std::logic_error("missing integer parameter: " + key);
        return it->second;
    }
};

struct Task {
    std::string task_id;
    std::string operation_id;
    Compartment compartment = Compartment::CrossCompartment;
    std::vector<std::string> dependencies;
    std::optional<Evidence> seed;
    ParameterMap parameters;
};

struct Plan {
    std::string plan_id;
    std::vector<Task> tasks;
};

struct TaskResult {
    std::string task_id;
    bool closed = false;
    std::string diagnostic;
    std::optional<ProofArtifact> artifact;
};

struct PlanResult {
    std::string plan_id;
    std::vector<TaskResult> tasks;

    bool all_closed() const {
        return !tasks.empty() && std::all_of(tasks.begin(), tasks.end(),
            [](const TaskResult& r) { return r.closed; });
    }

    const ProofArtifact& artifact(const std::string& task_id) const {
        for (const auto& r : tasks) {
            if (r.task_id == task_id && r.artifact) return *r.artifact;
        }
        throw std::logic_error("missing artifact: " + task_id);
    }

    std::string report() const {
        std::ostringstream out;
        out << "generalized campaign: " << plan_id << '\n';
        for (const auto& task : tasks) {
            out << "- " << task.task_id << ": " << (task.closed ? "closed" : "failed");
            if (!task.diagnostic.empty()) out << " -- " << task.diagnostic;
            out << '\n';
        }
        out << "campaign closed: " << (all_closed() ? "yes" : "no") << '\n';
        return out.str();
    }
};

class EvidenceStore {
public:
    void put(const std::string& id, ProofArtifact artifact) {
        if (!artifact.closed) throw std::logic_error("cannot store open artifact: " + id);
        artifacts_[id] = std::move(artifact);
    }

    const ProofArtifact& artifact(const std::string& id) const {
        const auto it = artifacts_.find(id);
        if (it == artifacts_.end()) throw std::logic_error("missing dependency artifact: " + id);
        return it->second;
    }

    template <class T>
    const T& evidence(const std::string& id) const {
        return std::get<T>(artifact(id).evidence);
    }

private:
    std::map<std::string, ProofArtifact> artifacts_;
};

using Operation = std::function<ProofArtifact(const Task&, const EvidenceStore&)>;

class OperationRegistry {
public:
    void install(std::string id, Operation operation) {
        const auto [_, inserted] = operations_.emplace(std::move(id), std::move(operation));
        if (!inserted) throw std::logic_error("duplicate generalized operation");
    }

    const Operation& operation(const std::string& id) const {
        const auto it = operations_.find(id);
        if (it == operations_.end()) throw std::logic_error("missing generalized operation: " + id);
        return it->second;
    }

    bool contains(const std::string& id) const { return operations_.contains(id); }

private:
    std::map<std::string, Operation> operations_;
};

class Executor {
public:
    explicit Executor(const OperationRegistry& registry) : registry_(registry) {}

    PlanResult run(const Plan& plan) const {
        EvidenceStore store;
        std::set<std::string> closed;
        std::vector<TaskResult> results;
        std::set<std::string> emitted;

        while (closed.size() < plan.tasks.size()) {
            bool progressed = false;
            for (const auto& task : plan.tasks) {
                if (closed.contains(task.task_id) || emitted.contains(task.task_id)) continue;
                const bool ready = std::all_of(task.dependencies.begin(), task.dependencies.end(),
                    [&](const std::string& d) { return closed.contains(d); });
                if (!ready) continue;
                progressed = true;
                TaskResult result{task.task_id, false, {}, std::nullopt};
                try {
                    if (!registry_.contains(task.operation_id))
                        throw std::logic_error("operation is not installed: " + task.operation_id);
                    auto artifact = registry_.operation(task.operation_id)(task, store);
                    if (!artifact.closed)
                        throw std::logic_error("operation returned an open artifact");
                    store.put(task.task_id, artifact);
                    result.closed = true;
                    result.artifact = std::move(artifact);
                    closed.insert(task.task_id);
                } catch (const std::exception& ex) {
                    result.diagnostic = ex.what();
                    emitted.insert(task.task_id);
                }
                results.push_back(std::move(result));
            }
            if (!progressed) break;
        }

        for (const auto& task : plan.tasks) {
            if (!closed.contains(task.task_id) && !emitted.contains(task.task_id)) {
                results.push_back({task.task_id, false, "dependencies did not close", std::nullopt});
            }
        }
        return {plan.plan_id, std::move(results)};
    }

private:
    const OperationRegistry& registry_;
};

inline std::vector<std::int64_t> parikh(const std::vector<int>& word) {
    int max_letter = -1;
    for (int x : word) max_letter = std::max(max_letter, x);
    std::vector<std::int64_t> result(static_cast<std::size_t>(max_letter + 1), 0);
    for (int x : word) {
        if (x < 0) throw std::logic_error("negative letters are unsupported");
        if (static_cast<std::size_t>(x) >= result.size()) result.resize(static_cast<std::size_t>(x + 1), 0);
        ++result[static_cast<std::size_t>(x)];
    }
    return result;
}

inline OperationRegistry standard_registry() {
    OperationRegistry registry;

    registry.install("word.enumerate_adjacent_unequal_sites",
        [](const Task& task, const EvidenceStore&) {
            const auto& family = std::get<WordFamilyEvidence>(*task.seed);
            AdjacentSiteCertificate cert{family.family_id, {}, true};
            for (std::size_t i = 0; i < family.images.size(); ++i) {
                const auto& word = family.images[i];
                for (std::size_t j = 0; j + 1 < word.size(); ++j) {
                    if (word[j] != word[j + 1]) cert.sites.push_back({i, j, word[j], word[j + 1]});
                }
            }
            return ProofArtifact{task.task_id, Compartment::WordLanguage, cert, {},
                {"all adjacent unequal sites enumerated by finite exhaustion"}, true};
        });

    registry.install("word.certify_swap_parikh_preservation",
        [](const Task& task, const EvidenceStore& store) {
            const auto& sites = store.evidence<AdjacentSiteCertificate>(task.dependencies.at(0));
            const auto& family = std::get<WordFamilyEvidence>(*task.seed);
            if (sites.family_id != family.family_id) throw std::logic_error("word family mismatch");
            std::size_t checked = 0;
            for (const auto& site : sites.sites) {
                auto swapped = family.images.at(site.image);
                const auto before = parikh(swapped);
                std::swap(swapped.at(site.offset), swapped.at(site.offset + 1));
                if (parikh(swapped) != before) throw std::logic_error("Parikh vector changed under swap");
                ++checked;
            }
            ParikhSwapCertificate cert{family.family_id, checked, true};
            return ProofArtifact{task.task_id, Compartment::WordLanguage, cert, {},
                {"every generated adjacent swap preserves the incidence column"}, true};
        });

    registry.install("affine.evaluate",
        [](const Task& task, const EvidenceStore&) {
            const auto& family = std::get<AffineFamilyEvidence>(*task.seed);
            if (family.base.size() != family.direction.size())
                throw std::logic_error("affine base/direction dimension mismatch");
            const auto parameter = task.parameters.integer("parameter");
            const auto delta = parameter - family.base_parameter;
            std::vector<std::int64_t> value(family.base.size());
            for (std::size_t i = 0; i < value.size(); ++i)
                value[i] = family.base[i] + delta * family.direction[i];
            AffineEvaluationCertificate cert{family.family_id, parameter, std::move(value)};
            return ProofArtifact{task.task_id, Compartment::AffineCatalogue, cert, {},
                {"affine family evaluated exactly"}, true};
        });

    registry.install("affine.certify_difference_law",
        [](const Task& task, const EvidenceStore& store) {
            const auto& left = store.evidence<AffineEvaluationCertificate>(task.dependencies.at(0));
            const auto& right = store.evidence<AffineEvaluationCertificate>(task.dependencies.at(1));
            const auto& family = std::get<AffineFamilyEvidence>(*task.seed);
            if (left.family_id != family.family_id || right.family_id != family.family_id)
                throw std::logic_error("affine family mismatch");
            std::vector<std::int64_t> diff(left.value.size());
            bool valid = diff.size() == right.value.size() && diff.size() == family.direction.size();
            for (std::size_t i = 0; valid && i < diff.size(); ++i) {
                diff[i] = right.value[i] - left.value[i];
                valid = diff[i] == (right.parameter - left.parameter) * family.direction[i];
            }
            AffineDifferenceCertificate cert{family.family_id, left.parameter, right.parameter,
                                             std::move(diff), valid};
            return ProofArtifact{task.task_id, Compartment::AffineCatalogue, cert, {},
                {"two-parameter affine transport law certified"}, valid};
        });

    registry.install("graph.certify_outdegree_handshake",
        [](const Task& task, const EvidenceStore&) {
            const auto& graph = std::get<GraphEvidence>(*task.seed);
            std::size_t sum = 0;
            for (const auto& row : graph.adjacency) {
                for (std::size_t v : row)
                    if (v >= graph.adjacency.size()) throw std::logic_error("edge endpoint outside graph");
                sum += row.size();
            }
            const std::size_t edges = graph.directed ? sum : sum / 2;
            const bool valid = graph.directed || sum % 2 == 0;
            GraphHandshakeCertificate cert{graph.graph_id, edges, sum, valid};
            return ProofArtifact{task.task_id, Compartment::Graph, cert, {},
                {"graph degree/edge accounting certified"}, valid};
        });

    registry.install("matrix.trace",
        [](const Task& task, const EvidenceStore&) {
            const auto& matrix = std::get<MatrixFamilyEvidence>(*task.seed);
            const std::size_t n = matrix.entries.size();
            std::int64_t trace = 0;
            for (std::size_t i = 0; i < n; ++i) {
                if (matrix.entries[i].size() != n) throw std::logic_error("matrix is not square");
                trace += matrix.entries[i][i];
            }
            MatrixTraceCertificate cert{matrix.family_id, trace};
            return ProofArtifact{task.task_id, Compartment::Matrix, cert, {},
                {"matrix trace computed exactly"}, true};
        });

    registry.install("spectral.recurrence_characteristic_polynomial",
        [](const Task& task, const EvidenceStore&) {
            const auto& recurrence = std::get<RecurrenceEvidence>(*task.seed);
            std::vector<std::int64_t> polynomial;
            polynomial.reserve(recurrence.coefficients.size() + 1);
            polynomial.push_back(1);
            for (auto c : recurrence.coefficients) polynomial.push_back(-c);
            CharacteristicPolynomialCertificate cert{recurrence.family_id, std::move(polynomial)};
            return ProofArtifact{task.task_id, Compartment::Spectral, cert, {},
                {"monic recurrence characteristic polynomial derived"}, true};
        });

    registry.install("cross.assert_integer_equal",
        [](const Task& task, const EvidenceStore& store) {
            if (task.dependencies.size() != 2) throw std::logic_error("cross equality needs two dependencies");
            const auto& a = store.artifact(task.dependencies[0]).evidence;
            const auto& b = store.artifact(task.dependencies[1]).evidence;
            std::optional<std::int64_t> lhs;
            std::optional<std::int64_t> rhs;
            if (const auto* x = std::get_if<MatrixTraceCertificate>(&a)) lhs = x->trace;
            if (const auto* x = std::get_if<GraphHandshakeCertificate>(&a)) lhs = static_cast<std::int64_t>(x->edge_count);
            if (const auto* x = std::get_if<MatrixTraceCertificate>(&b)) rhs = x->trace;
            if (const auto* x = std::get_if<GraphHandshakeCertificate>(&b)) rhs = static_cast<std::int64_t>(x->edge_count);
            if (!lhs || !rhs) throw std::logic_error("unsupported cross-compartment scalar projection");
            const bool agrees = *lhs == *rhs;
            CrossCompartmentAgreementCertificate cert{task.task_id, agrees,
                agrees ? "projected integer invariants agree" : "projected integer invariants disagree"};
            return ProofArtifact{task.task_id, Compartment::CrossCompartment, cert, {},
                {"cross-compartment scalar invariant comparison"}, agrees};
        });

    return registry;
}

}  // namespace ravel::proof::generalized
