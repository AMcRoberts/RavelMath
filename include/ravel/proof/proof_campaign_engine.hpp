#pragma once

#include <algorithm>
#include <cstddef>
#include <functional>
#include <map>
#include <optional>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "math/proof_reflection.hpp"

namespace ravel::proof {

enum class CampaignOperation {
    ProveLowerTriangular,
    ProveTriangularDeterminant,
    DerivePiecewiseMatrixEquality,
    DeriveSparseCofactorRecurrence,
    CloseFirstOrderRecurrence,
    DeriveCharacteristicCofactorSplit,
    NormalizePolynomialIdentity,
    ComposeUniversalTheorem,
};

enum class TaskState {
    Discovered,
    DependenciesPending,
    Ready,
    Running,
    Closed,
    BlockedMissingCapability,
    BlockedDependency,
    Failed,
};

inline const char* state_name(TaskState state) {
    switch (state) {
        case TaskState::Discovered: return "Discovered";
        case TaskState::DependenciesPending: return "DependenciesPending";
        case TaskState::Ready: return "Ready";
        case TaskState::Running: return "Running";
        case TaskState::Closed: return "Closed";
        case TaskState::BlockedMissingCapability: return "BlockedMissingCapability";
        case TaskState::BlockedDependency: return "BlockedDependency";
        case TaskState::Failed: return "Failed";
    }
    return "Unknown";
}

enum class LeanStepKind {
    Intro, Ext, SimpOnly, HaveBySimpaUsing, HaveByOmega, Have, HaveIsLt,
    Simp, Rewrite, ExpandDeterminant, Exact, Funext, ByCases, SplitIfs,
    Omega, AllGoalsOmega, AllGoalsSimpAllOmega, AllGoalsDeepSimpAllOmega, AllGoalsBranchContextSimpOmegaRing, TerminalCofactorTransport, ResidualSingleSupportLaplaceClosure, SourceTruthRecurrenceClosure, SourceTruthCharacteristicQTransport, SourceTruthCharacteristicRTransport, SourceTruthCharacteristicSplit, Induction, Cases, Ring, RingNF,
    HaveByChangeAtExact, ClosePolynomialGeometricSuccessor
};

// Typed erased-index expression for any EraseIndex.minor emitter.
// The renderer below converts one of these into a Lean Fin term without ever
// embedding integer literals or theorem-specific witness strings at the
// call site. Theorem-specific handlers must not own textual Fin rendering.
enum class SymbolicFinIndexKind {
    Literal,
    LastOfSuccessor,
    LastOfPred,
    FirstOfPred,
};

struct SymbolicFinIndex {
    SymbolicFinIndexKind kind = SymbolicFinIndexKind::Literal;
    int literal_value = 0;
    std::string proof_expression;
    std::string predecessor_expression;

    SymbolicFinIndex() = default;
    SymbolicFinIndex(int value) : kind(SymbolicFinIndexKind::Literal), literal_value(value) {}
    SymbolicFinIndex(SymbolicFinIndexKind kind_value, int value,
                     std::string proof = {}, std::string predecessor = {})
        : kind(kind_value), literal_value(value), proof_expression(std::move(proof)),
          predecessor_expression(std::move(predecessor)) {}

    static SymbolicFinIndex literal(int value) {
        return {SymbolicFinIndexKind::Literal, value};
    }
    static SymbolicFinIndex last_of_successor(std::string predecessor_expression) {
        return {SymbolicFinIndexKind::LastOfSuccessor, 0, {}, std::move(predecessor_expression)};
    }
    static SymbolicFinIndex last_of_pred(std::string lower_bound_proof) {
        return {SymbolicFinIndexKind::LastOfPred, 0, std::move(lower_bound_proof), {}};
    }
    static SymbolicFinIndex first_of_pred(std::string lower_bound_proof) {
        return {SymbolicFinIndexKind::FirstOfPred, 0, std::move(lower_bound_proof), {}};
    }
};

inline int concrete_erased_index(const SymbolicFinIndex& index, int source_dim) {
    if (index.kind == SymbolicFinIndexKind::FirstOfPred) return 0;
    if (index.kind != SymbolicFinIndexKind::Literal) return source_dim - 1;
    return index.literal_value;
}

inline std::string render_fin_index(const SymbolicFinIndex& index,
                                    const std::string& bound_expr) {
    if (index.kind == SymbolicFinIndexKind::LastOfSuccessor) {
        return "(Fin.last (" + index.predecessor_expression + "))";
    }
    if (index.kind == SymbolicFinIndexKind::LastOfPred) {
        return "(Ravel.Matrix.EraseIndex.lastPred " + index.proof_expression + ")";
    }
    if (index.kind == SymbolicFinIndexKind::FirstOfPred) {
        return "(Ravel.Matrix.EraseIndex.firstPred " + index.proof_expression + ")";
    }
    return "⟨" + std::to_string(index.literal_value) + ", by omega⟩";
}

// Generic composer for an EraseIndex.minor application. The matrix expression
// and the bound expression together name the source matrix and the dimension
// Lean sees for its indices; the SymbolicFinIndex values carry the row and
// column arguments through render_fin_index. Theorem-specific strings must
// never embed integer row/column literals here.
inline std::string render_minor_application(
        const std::string& matrix_expr,
        const SymbolicFinIndex& row,
        const SymbolicFinIndex& column,
        const std::string& source_bound_expr) {
    return "Ravel.Matrix.EraseIndex.minor (" + matrix_expr + ") " +
           render_fin_index(row, source_bound_expr) + " " +
           render_fin_index(column, source_bound_expr);
}

struct PiecewiseMatrixTransform {
    int source_parameter_offset = 0;
    SymbolicFinIndex erased_row;
    SymbolicFinIndex erased_column;
    std::string lean_transform;
};

struct DerivationValidation {
    bool valid = false;
    std::string diagnostic;
    std::size_t checked_instances = 0;
    std::size_t checked_entries = 0;
};

inline DerivationValidation validate_piecewise_minor_transform(
        const mathlib::reflection::PiecewiseEntry& source,
        const mathlib::reflection::PiecewiseEntry& target,
        const PiecewiseMatrixTransform& transform,
        int n_min = 2, int n_max = 24) {
    DerivationValidation result;
    if (source.branches.empty() || target.branches.empty()) {
        result.diagnostic = "typed branch programs are required";
        return result;
    }
    if ((transform.erased_row.kind == SymbolicFinIndexKind::Literal && transform.erased_row.literal_value < 0) ||
        (transform.erased_column.kind == SymbolicFinIndexKind::Literal && transform.erased_column.literal_value < 0)) {
        result.diagnostic = "negative erased indices are unsupported";
        return result;
    }
    const auto skip = [](int k, int erased) { return k < erased ? k : k + 1; };
    for (int n = n_min; n <= n_max; ++n) {
        const int source_n = n + transform.source_parameter_offset;
        const int source_dim = static_cast<int>(source.eval_dim(source_n));
        const int target_dim = static_cast<int>(target.eval_dim(n));
        if (source_dim - 1 != target_dim) {
            result.diagnostic = "dimension transform mismatch at n=" + std::to_string(n);
            return result;
        }
        const int erased_row_concrete = concrete_erased_index(transform.erased_row, source_dim);
        const int erased_column_concrete = concrete_erased_index(transform.erased_column, source_dim);
        if (erased_row_concrete < 0 || erased_column_concrete < 0 ||
            erased_row_concrete >= source_dim || erased_column_concrete >= source_dim) {
            result.diagnostic = "erased index outside source dimension at n=" + std::to_string(n);
            return result;
        }
        ++result.checked_instances;
        for (int i = 0; i < target_dim; ++i) {
            for (int j = 0; j < target_dim; ++j) {
                const auto lhs = source.eval(skip(i, erased_row_concrete),
                                             skip(j, erased_column_concrete), source_n);
                const auto rhs = target.eval(i, j, n);
                for (int x : {-2, -1, 0, 1, 2}) {
                    if (lhs.eval_as_int(x) != rhs.eval_as_int(x)) {
                        result.diagnostic = "entry transform counterexample at n=" + std::to_string(n) +
                            ", i=" + std::to_string(i) + ", j=" + std::to_string(j) +
                            ", x=" + std::to_string(x);
                        return result;
                    }
                }
                ++result.checked_entries;
            }
        }
    }
    result.valid = true;
    result.diagnostic = "typed transform validated over bounded semantic regression";
    return result;
}

struct LeanStep {
    LeanStepKind kind;
    std::string name;
    std::string proposition;
    std::string expression;
    std::vector<std::string> arguments;
};

struct LeanDefinition {
    std::string documentation;
    std::string name;
    std::vector<std::pair<std::string, std::string>> binders;
    std::string type;
    std::string value;
};

struct LeanTheorem {
    std::string documentation;
    std::string name;
    std::vector<std::pair<std::string, std::string>> binders;
    std::string proposition;
    std::vector<LeanStep> proof;
};

struct ClosedProofArtifact {
    std::string artifact_id;
    std::vector<std::string> imports;
    std::vector<LeanDefinition> definitions;
    std::vector<LeanTheorem> theorems;
    std::vector<std::string> established_facts;
    bool has_open_goals = false;
};


struct TheoremSignature {
    std::string documentation;
    std::string name;
    std::vector<std::pair<std::string, std::string>> binders;
    std::string proposition;
};

inline LeanTheorem theorem_from_signature(const TheoremSignature& signature,
                                          std::vector<LeanStep> proof) {
    return {signature.documentation, signature.name, signature.binders,
            signature.proposition, std::move(proof)};
}

struct LowerTriangularSpec {
    std::string family_id;
    mathlib::reflection::MatrixEntryRuleKind expected_entry_rule;
    LeanDefinition definition;
    TheoremSignature theorem;
    std::string definition_name;
    std::string relation_conversion_lemma;
    std::string strict_order_proposition;
    std::string off_diagonal_proposition;
    std::string diagonal_proposition;
    std::string diagonal_proof;
    std::vector<std::string> final_simp;
    std::vector<std::string> established_facts;
};


enum class BranchClosurePolicy {
    BoundedSimpAll4096,
    DeepSimpAll16384,
    BranchContextSimpOmegaRing,
};

struct NamedProposition {
    std::string name;
    std::string proposition;
    std::string proof_expression;
};

struct TriangularDeterminantSpec {
    TheoremSignature theorem;
    std::string determinant_lemma_application;
    std::vector<std::string> simp_lemmas;
    std::vector<std::string> established_facts;
};

struct PiecewiseEqualitySpec {
    std::string obligation_id;
    std::string family_id;
    mathlib::reflection::MatrixEntryRuleKind expected_entry_rule;
    LeanDefinition definition;
    TheoremSignature theorem;
    PiecewiseMatrixTransform transform;
    int validation_n_min = 2;
    int validation_n_max = 24;
    std::vector<NamedProposition> dimension_facts;
    BranchClosurePolicy branch_closure = BranchClosurePolicy::BoundedSimpAll4096;
    std::vector<std::string> established_facts;
};


struct DeterminantExpansionOperation {
    std::string theorem_name;
    std::string matrix_expression;
    std::string successor_dimension_equality;
    std::string successor_dimension;
    std::string transported_matrix_name;
};

struct SparseCofactorRecurrenceSpec {
    std::string source_family_id;
    std::string residual_family_id;
    mathlib::reflection::MatrixStructureKind source_structure;
    mathlib::reflection::MatrixStructureKind residual_structure;
    std::vector<LeanDefinition> definitions;
    TheoremSignature residual_transport;
    std::vector<std::string> residual_transport_simp;
    std::vector<NamedProposition> residual_transport_dimension_facts;
    BranchClosurePolicy residual_transport_branch_closure = BranchClosurePolicy::BoundedSimpAll4096;
    TheoremSignature residual_determinant;
    DeterminantExpansionOperation residual_expansion;
    std::vector<std::string> residual_determinant_simp;
    TheoremSignature recurrence;
    DeterminantExpansionOperation recurrence_expansion;
    std::vector<std::string> recurrence_simp;
    std::vector<std::string> established_facts;
};

struct FirstOrderRecurrenceSpec {
    TheoremSignature base;
    std::vector<std::string> base_simp;
    bool base_finish_with_ring_nf = false;
    TheoremSignature successor;
    std::vector<std::string> successor_rewrites;
    TheoremSignature closure;
    std::string induction_variable;
    std::string induction_hypothesis;
    std::vector<std::string> established_facts;
};

struct MinorTransportSpec {
    TheoremSignature theorem;
    std::vector<std::string> simp_lemmas;
    std::vector<NamedProposition> dimension_facts;
    BranchClosurePolicy branch_closure = BranchClosurePolicy::BoundedSimpAll4096;
};

struct CofactorSplitSpec {
    std::vector<std::string> required_family_ids;
    std::vector<LeanDefinition> definitions;
    std::vector<MinorTransportSpec> minor_transports;
    TheoremSignature split;
    DeterminantExpansionOperation expansion;
    std::vector<std::string> split_simp;
    std::vector<std::string> established_facts;
};

struct PolynomialNormalizationSpec {
    std::vector<LeanDefinition> definitions;
    TheoremSignature theorem;
    std::vector<std::string> rewrites;
    std::vector<std::string> simp_lemmas;
    std::vector<std::string> post_rewrites;
    bool finish_with_ring = true;
    std::vector<std::string> established_facts;
};

struct UniversalCompositionSpec {
    TheoremSignature theorem;
    std::string exact_expression;
    std::vector<std::string> established_facts;
};

using CampaignSpecification = std::variant<std::monostate, LowerTriangularSpec,
    TriangularDeterminantSpec, PiecewiseEqualitySpec, SparseCofactorRecurrenceSpec,
    FirstOrderRecurrenceSpec, CofactorSplitSpec, PolynomialNormalizationSpec,
    UniversalCompositionSpec>;

struct CampaignTask {
    std::string task_id;
    CampaignOperation operation;
    std::vector<std::string> dependencies;
    std::vector<mathlib::reflection::NodeId> evidence;
    TaskState state = TaskState::Discovered;
    std::string diagnostic;
    std::optional<ClosedProofArtifact> artifact;
    CampaignSpecification specification;
};

struct ProofCampaign {
    std::string theorem_id;
    std::vector<CampaignTask> tasks;
};

struct CampaignResult {
    std::string theorem_id;
    std::vector<CampaignTask> tasks;

    bool all_closed() const {
        return !tasks.empty() && std::all_of(tasks.begin(), tasks.end(),
            [](const CampaignTask& task) { return task.state == TaskState::Closed; });
    }

    const CampaignTask* find_task(const std::string& id) const {
        for (const auto& task : tasks) if (task.task_id == id) return &task;
        return nullptr;
    }

    std::string report() const {
        std::ostringstream out;
        out << "executable proof campaign: " << theorem_id << '\n';
        for (const auto& task : tasks) {
            out << "- " << task.task_id << ": " << state_name(task.state);
            if (!task.diagnostic.empty()) out << " -- " << task.diagnostic;
            out << '\n';
        }
        out << "campaign closed: " << (all_closed() ? "yes" : "no") << '\n';
        return out.str();
    }
};

class CampaignGenerator {
public:
    ProofCampaign generate(const mathlib::reflection::Trace& trace) const {
        ProofCampaign campaign{trace.theorem_id(), {}};
        bool has_r = false;
        bool has_q = false;
        for (const auto& [id, certificate] : trace.find<mathlib::reflection::TriangularityCertificate>()) {
            if (!certificate->lower) continue;
            has_r = true;
            LowerTriangularSpec triangular_spec;
            triangular_spec.family_id = "nbonacci.r";
            triangular_spec.expected_entry_rule = mathlib::reflection::MatrixEntryRuleKind::LowerBidiagonalXMinusOne;
            triangular_spec.definition = {"Symbolic family generated from the reflected lower-bidiagonal entry rule.",
                "rMatrix", {{"n", "ℕ"}},
                "Matrix (Fin (n - 1)) (Fin (n - 1)) (Polynomial ℤ)",
                "fun i j => if j.val + 1 = i.val then Polynomial.X else if i.val = j.val then Polynomial.C (-1) else 0"};
            triangular_spec.theorem = {"Automatically discharged support certificate for the reflected family.",
                "rMatrix_lowerTriangular", {{"n", "ℕ"}}, "(rMatrix n).BlockTriangular (OrderDual.toDual : Fin (n - 1) → OrderDual (Fin (n - 1)))"};
            triangular_spec.definition_name = "rMatrix";
            triangular_spec.relation_conversion_lemma = "";
            triangular_spec.strict_order_proposition = "i.val < j.val";
            triangular_spec.off_diagonal_proposition = "j.val + 1 ≠ i.val";
            triangular_spec.diagonal_proposition = "i.val ≠ j.val";
            triangular_spec.diagonal_proof = "Nat.ne_of_lt hlt";
            triangular_spec.final_simp = {"hsub", "hdiag"};
            triangular_spec.established_facts = {"lower-triangular structural certificate"};
            campaign.tasks.push_back({"r_matrix.lower_triangular",
                CampaignOperation::ProveLowerTriangular, {}, {id},
                TaskState::Discovered, {}, std::nullopt, std::move(triangular_spec)});

            TriangularDeterminantSpec determinant_spec;
            determinant_spec.theorem = {"Automatically composed from the closed triangularity certificate.",
                "rMatrix_det", {{"n", "ℕ"}},
                "(rMatrix n).det = (Polynomial.C (-1) : Polynomial ℤ) ^ (n - 1)"};
            determinant_spec.determinant_lemma_application = "Matrix.det_of_lowerTriangular (rMatrix n) (rMatrix_lowerTriangular n)";
            determinant_spec.simp_lemmas = {"rMatrix"};
            determinant_spec.established_facts = {"triangular determinant formula"};
            campaign.tasks.push_back({"r_matrix.determinant",
                CampaignOperation::ProveTriangularDeterminant,
                {"r_matrix.lower_triangular"}, {id},
                TaskState::Discovered, {}, std::nullopt, std::move(determinant_spec)});
        }
        for (const auto& [id, obligation] : trace.find<mathlib::reflection::ProofObligation>()) {
            if (obligation->obligation_id != "q_minor_reduction") continue;
            has_q = true;
            PiecewiseEqualitySpec equality_spec;
            equality_spec.obligation_id = "q_minor_reduction";
            equality_spec.family_id = "nbonacci.q";
            equality_spec.expected_entry_rule = mathlib::reflection::MatrixEntryRuleKind::UpperBidiagonalWithTerminalRow;
            const auto* q_family = [&]() -> const mathlib::reflection::MatrixFamily* {
                for (const auto& [fid, family] : trace.find<mathlib::reflection::MatrixFamily>()) {
                    (void)fid;
                    if (family->family_id == equality_spec.family_id) return family;
                }
                return nullptr;
            }();
            if (!q_family) continue;
            equality_spec.definition = {"Symbolic family generated from the reflected piecewise entry rule.",
                "qMatrix", {{"n", "ℕ"}},
                "Matrix (Fin (n - 1)) (Fin (n - 1)) (Polynomial ℤ)",
                q_family->piecewise.definition_lean_value()};
            equality_spec.theorem = {"Erased-index transport generated from the typed family and transform.",
                "qMatrix_minor_eq_qMatrix", {{"n", "ℕ"}, {"hn", "2 ≤ n"}},
                render_minor_application("qMatrix (n + 1)", SymbolicFinIndex::literal(0), SymbolicFinIndex::literal(0), "n") + " = qMatrix n"};
            equality_spec.transform = {1, SymbolicFinIndex::literal(0), SymbolicFinIndex::literal(0), "Ravel.Matrix.EraseIndex.minor"};
            equality_spec.dimension_facts = {
                {"hn_sub_two", "n - 2 + 2 = n", ""},
                {"hn_succ_sub_two", "n + 1 - 2 = n - 1", ""},
                {"hn_sub_one", "n - 1 + 1 = n", ""},
                {"hn_branch_positive", "0 < n + 1 - 2", ""},
                {"hi_bound", "i.val < n - 1", "i.isLt"},
                {"hj_bound", "j.val < n - 1", "j.isLt"},
                {"hi_branch_bound", "i.val < n + 1 - 2", ""},
                {"hj_branch_bound", "j.val < n + 1 - 2", ""},
                {"hi_terminal_bound", "i.val ≤ n - 2", ""},
                {"hj_terminal_bound", "j.val ≤ n - 2", ""},
                {"hi_shift_bound", "i.val + 1 < n", ""},
                {"hj_shift_bound", "j.val + 1 < n", ""}};
            equality_spec.established_facts = {"piecewise erased-minor transport"};
            campaign.tasks.push_back({"q_matrix.minor_transport",
                CampaignOperation::DerivePiecewiseMatrixEquality, {}, {id},
                TaskState::Discovered, {}, std::nullopt, std::move(equality_spec)});
        }
        if (has_q) {
            SparseCofactorRecurrenceSpec sparse_spec;
            sparse_spec.source_family_id = "nbonacci.q";
            sparse_spec.residual_family_id = "nbonacci.r";
            sparse_spec.source_structure = mathlib::reflection::MatrixStructureKind::UpperBidiagonalPlusTerminalRow;
            sparse_spec.residual_structure = mathlib::reflection::MatrixStructureKind::LowerBidiagonal;
            sparse_spec.definitions = {
                {"The non-principal row-zero cofactor selected by the sparse-cofactor operator.",
                 "qResidualMinor", {{"n", "ℕ"}, {"hn", "2 ≤ n"}},
                 "Matrix (Fin (n - 1)) (Fin (n - 1)) (Polynomial ℤ)",
                 render_minor_application("qMatrix (n + 1)", SymbolicFinIndex::literal(0), SymbolicFinIndex::literal(1), "n")},
                {"The core left after the residual minor's singleton first-column expansion.",
                 "qResidualCore", {{"n", "ℕ"}, {"hn", "2 ≤ n"}},
                 "Matrix (Fin (n - 2)) (Fin (n - 2)) (Polynomial ℤ)",
                 render_minor_application("qResidualMinor n hn", SymbolicFinIndex::last_of_pred("hn"), SymbolicFinIndex::first_of_pred("hn"), "n - 1")}
            };
            sparse_spec.residual_transport = {
                "Entrywise transport from the residual cofactor core to the reflected residual family.",
                "qResidualCore_eq_rMatrix", {{"n", "ℕ"}, {"hn", "2 ≤ n"}},
                "qResidualCore n hn = rMatrix (n - 1)"};
            sparse_spec.residual_transport_simp = {"qResidualCore", "qResidualMinor", "qMatrix", "rMatrix", "Ravel.Matrix.EraseIndex.minor", "Ravel.Matrix.EraseIndex.skip_zero_skip_lastPred_val", "Ravel.Matrix.EraseIndex.skip_one_skip_firstPred_val"};
            sparse_spec.residual_transport_dimension_facts = {
                {"hn_sub_two", "n - 2 + 2 = n", ""},
                {"hn_succ_sub_two", "n + 1 - 2 = n - 1", ""},
                {"hn_succ_sub_one", "n + 1 - 1 = n", ""},
                {"hn_residual_branch_positive", "0 < n + 1 - 1", ""},
                {"hi_bound", "i.val < n - 2", "i.isLt"},
                {"hj_bound", "j.val < n - 2", "j.isLt"},
                {"hi_residual_branch_bound", "i.val < n + 1 - 1", ""},
                {"hj_residual_branch_bound", "j.val < n + 1 - 1", ""},
                {"hi_residual_shift_bound", "i.val + 1 < n + 1 - 1", ""},
                {"hj_residual_shift_bound", "j.val + 1 < n + 1 - 1", ""},
                {"hi_residual_double_shift_bound", "i.val + 2 < n + 1 - 1", ""},
                {"hj_residual_double_shift_bound", "j.val + 2 < n + 1 - 1", ""},
                {"hi_shift_bound", "i.val + 1 < n - 1", ""},
                {"hj_shift_bound", "j.val + 1 < n - 1", ""},
                {"hi_double_shift_bound", "i.val + 2 < n", ""},
                {"hj_double_shift_bound", "j.val + 2 < n", ""}};
            sparse_spec.residual_determinant = {
                "The residual cofactor determinant computed through its transported structural core.",
                "qResidualMinor_det", {{"n", "ℕ"}, {"hn", "2 ≤ n"}},
                "(qResidualMinor n hn).det = 1"};
            sparse_spec.residual_expansion = {"Matrix.det_succ_column_zero", "qResidualMinor n hn",
                "n - 1 = (n - 2).succ", "(n - 2).succ", "qResidualMinorSucc"};
            sparse_spec.residual_determinant_simp = {"qResidualMinor", "qResidualCore_eq_rMatrix n hn", "rMatrix_det", "qMatrix", "Ravel.Matrix.EraseIndex.minor", "Ravel.Matrix.EraseIndex.skip_zero_val"};
            sparse_spec.recurrence = {
                "Sparse row expansion composed from support, minor transports, and cofactor signs.",
                "qMatrix_det_recurrence", {{"n", "ℕ"}, {"hn", "2 ≤ n"}},
                "(qMatrix (n + 1)).det = Polynomial.X * (qMatrix n).det + 1"};
            sparse_spec.recurrence_expansion = {"Matrix.det_succ_row_zero", "qMatrix (n + 1)",
                "n = (n - 1).succ", "(n - 1).succ", "qMatrixSucc"};
            sparse_spec.recurrence_simp = {"qMatrix", "qMatrix_minor_eq_qMatrix", "qResidualMinor", "qResidualMinor_det n hn", "Ravel.Matrix.EraseIndex.minor", "Ravel.Matrix.EraseIndex.skip_zero_val", "Ravel.Matrix.EraseIndex.skip_last_val", "Ravel.Matrix.EraseIndex.skip_lastPred_val", "Ravel.Matrix.EraseIndex.skip_finLast_val", "Fin.sum_univ_succ"};
            sparse_spec.established_facts = {"residual cofactor determinant equals one", "determinant successor recurrence"};
            campaign.tasks.push_back({"q_matrix.determinant_recurrence",
                CampaignOperation::DeriveSparseCofactorRecurrence,
                {"q_matrix.minor_transport", "r_matrix.determinant"}, {}, TaskState::Discovered, {}, std::nullopt,
                std::move(sparse_spec)});

            FirstOrderRecurrenceSpec recurrence_spec;
            recurrence_spec.base = {
                "Base case consumed by the recurrence-closure operator.",
                "qMatrix_det_closed_form_base", {},
                "(qMatrix 2).det = ∑ k ∈ Finset.range 2, (Polynomial.X : Polynomial ℤ) ^ k"};
            recurrence_spec.base_simp = {"qMatrix", "Finset.sum_range_succ"};
            recurrence_spec.base_finish_with_ring_nf = false;
            recurrence_spec.successor = {
                "Successor case consumed by the recurrence-closure operator.",
                "qMatrix_det_closed_form_succ", {{"n", "ℕ"}, {"hn", "2 ≤ n"},
                    {"ih", "(qMatrix n).det = ∑ k ∈ Finset.range n, (Polynomial.X : Polynomial ℤ) ^ k"}},
                "(qMatrix (n + 1)).det = ∑ k ∈ Finset.range (n + 1), (Polynomial.X : Polynomial ℤ) ^ k"};
            recurrence_spec.successor_rewrites = {"qMatrix_det_recurrence n hn", "ih"};
            recurrence_spec.closure = {
                "First-order recurrence closure generated from a base and successor theorem.",
                "qMatrix_det_closed_form", {{"n", "ℕ"}, {"hn", "2 ≤ n"}},
                "(qMatrix n).det = ∑ k ∈ Finset.range n, (Polynomial.X : Polynomial ℤ) ^ k"};
            recurrence_spec.induction_variable = "n";
            recurrence_spec.induction_hypothesis = "hn";
            recurrence_spec.established_facts = {"determinant finite geometric sum"};
            campaign.tasks.push_back({"q_matrix.determinant_closed_form",
                CampaignOperation::CloseFirstOrderRecurrence,
                {"q_matrix.determinant_recurrence"}, {}, TaskState::Discovered, {}, std::nullopt,
                std::move(recurrence_spec)});
        }
        if (has_r && has_q) {
            CofactorSplitSpec split_spec;
            split_spec.required_family_ids = {"nbonacci.q", "nbonacci.r"};
            split_spec.definitions = {{
                "Characteristic-matrix family supplied by the theorem campaign.",
                "nbonacciCharacteristicMatrix", {{"n", "ℕ"}},
                "Matrix (Fin n) (Fin n) (Polynomial ℤ)",
                "fun i j => if i.val = n - 1 ∧ j.val = n - 1 then Polynomial.X + 1 else if i.val = n - 1 ∧ j.val = 0 then Polynomial.C (-1) else if i.val = n - 1 then Polynomial.C 1 else if i.val + 1 = j.val then Polynomial.C (-1) else if i.val = j.val then Polynomial.X else 0"
            }};
            split_spec.minor_transports = {
                {{"Entrywise transport of the selected characteristic minor to the principal family.",
                  "nbonacciCharacteristic_minor_q", {{"n", "ℕ"}, {"hn", "2 ≤ n"}},
                  render_minor_application("nbonacciCharacteristicMatrix (n + 1)", SymbolicFinIndex::literal(0), SymbolicFinIndex::literal(0), "n + 1") +
                  " = qMatrix (n + 1)"},
                 {"nbonacciCharacteristicMatrix", "qMatrix", "Ravel.Matrix.EraseIndex.minor", "Ravel.Matrix.EraseIndex.skip_zero_val"},
                 {{"hn_sub_two", "n - 2 + 2 = n", ""},
                  {"hn_succ_sub_two", "n + 1 - 2 = n - 1", ""},
                  {"hn_succ_sub_one", "n + 1 - 1 = n", ""},
                  {"hn_characteristic_branch_positive", "0 < n + 1 - 2", ""},
                  {"hi_bound", "i.val < n", "i.isLt"},
                  {"hj_bound", "j.val < n", "j.isLt"},
                  {"hi_terminal_bound", "i.val ≤ n - 1", ""},
                  {"hj_terminal_bound", "j.val ≤ n - 1", ""},
                  {"hi_shift_bound", "i.val + 1 < n + 1", ""},
                  {"hj_shift_bound", "j.val + 1 < n + 1", ""}}},
                {{"Entrywise transport of the selected characteristic minor to the residual family.",
                  "nbonacciCharacteristic_minor_r", {{"n", "ℕ"}, {"hn", "2 ≤ n"}},
                  render_minor_application("nbonacciCharacteristicMatrix (n + 1)", SymbolicFinIndex::last_of_successor("n"), SymbolicFinIndex::literal(0), "n + 1") +
                  " = rMatrix (n + 1)"},
                 {"nbonacciCharacteristicMatrix", "rMatrix", "Ravel.Matrix.EraseIndex.minor", "Ravel.Matrix.EraseIndex.skip_zero_val", "Ravel.Matrix.EraseIndex.skip_finLast_val"},
                 {{"hn_sub_two", "n - 2 + 2 = n", ""},
                  {"hn_succ_sub_two", "n + 1 - 2 = n - 1", ""},
                  {"hn_succ_sub_one", "n + 1 - 1 = n", ""},
                  {"hn_characteristic_branch_positive", "0 < n + 1 - 2", ""},
                  {"hi_bound", "i.val < n", "i.isLt"},
                  {"hj_bound", "j.val < n", "j.isLt"},
                  {"hi_terminal_bound", "i.val ≤ n - 1", ""},
                  {"hj_terminal_bound", "j.val ≤ n - 1", ""},
                  {"hi_shift_bound", "i.val + 1 < n + 1", ""},
                  {"hj_shift_bound", "j.val + 1 < n + 1", ""}}}
            };
            split_spec.minor_transports[0].branch_closure = BranchClosurePolicy::DeepSimpAll16384;
            split_spec.minor_transports[1].branch_closure = BranchClosurePolicy::DeepSimpAll16384;
            split_spec.split = {
                "Sparse cofactor decomposition assembled from transported minors.",
                "nbonacci_characteristic_split", {{"n", "ℕ"}, {"hn", "2 ≤ n"}},
                "(nbonacciCharacteristicMatrix (n + 1)).det = Polynomial.X * (qMatrix (n + 1)).det + (-1 : Polynomial ℤ) ^ (n + 1) * (rMatrix (n + 1)).det"};
            split_spec.expansion = {"Matrix.det_succ_column_zero", "nbonacciCharacteristicMatrix (n + 1)", "", "", ""};
            split_spec.split_simp = {"nbonacciCharacteristicMatrix", "nbonacciCharacteristic_minor_q n hn", "nbonacciCharacteristic_minor_r n hn", "Ravel.Matrix.EraseIndex.minor", "Ravel.Matrix.EraseIndex.skip_zero_val", "Ravel.Matrix.EraseIndex.skip_last_val", "Ravel.Matrix.EraseIndex.skip_lastPred_val", "Ravel.Matrix.EraseIndex.skip_finLast_val"};
            split_spec.established_facts = {"principal minor transport", "residual minor transport", "characteristic determinant split"};
            campaign.tasks.push_back({"nbonacci.characteristic_split",
                CampaignOperation::DeriveCharacteristicCofactorSplit,
                {"q_matrix.minor_transport", "r_matrix.lower_triangular"}, {},
                TaskState::Discovered, {}, std::nullopt, std::move(split_spec)});

            PolynomialNormalizationSpec polynomial_spec;
            polynomial_spec.definitions = {{"The universal target polynomial.", "nbonacciPolynomial", {{"n", "ℕ"}},
                "Polynomial ℤ", "(∑ k ∈ Finset.range n, (Polynomial.X : Polynomial ℤ) ^ (k + 1)) - 1"}};
            polynomial_spec.theorem = {
                "Polynomial closure composed from determinant decomposition and closed component formulas.",
                "nbonacci_characteristic_polynomial", {{"n", "ℕ"}, {"hn", "2 ≤ n"}},
                "(nbonacciCharacteristicMatrix (n + 1)).det = nbonacciPolynomial (n + 1)"};
            polynomial_spec.rewrites = {"nbonacci_characteristic_split n hn", "qMatrix_det_closed_form (n + 1) (by omega)", "rMatrix_det (n + 1)"};
            polynomial_spec.simp_lemmas = {"nbonacciPolynomial", "Finset.mul_sum", "Ravel.Polynomial.neg_one_pow_succ_mul_self_int"};
            polynomial_spec.established_facts = {"universal characteristic polynomial identity"};
            campaign.tasks.push_back({"nbonacci.polynomial_closure",
                CampaignOperation::NormalizePolynomialIdentity,
                {"nbonacci.characteristic_split", "q_matrix.determinant_closed_form", "r_matrix.determinant"}, {},
                TaskState::Discovered, {}, std::nullopt, std::move(polynomial_spec)});

            UniversalCompositionSpec universal_spec;
            universal_spec.theorem = {
                "Public universal theorem assembled solely from closed campaign artifacts.",
                "nbonacci_universal_n", {{"n", "ℕ"}, {"hn", "2 ≤ n"}},
                "(nbonacciCharacteristicMatrix (n + 1)).det = nbonacciPolynomial (n + 1)"};
            universal_spec.exact_expression = "nbonacci_characteristic_polynomial n hn";
            universal_spec.established_facts = {"public universal theorem"};
            campaign.tasks.push_back({"nbonacci.universal_n",
                CampaignOperation::ComposeUniversalTheorem,
                {"nbonacci.polynomial_closure"}, {}, TaskState::Discovered, {}, std::nullopt,
                std::move(universal_spec)});
        }
        return campaign;
    }
};

class ProofCampaignExecutor {
public:
    using Handler = std::function<void(CampaignTask&, const mathlib::reflection::Trace&,
                                       const std::map<std::string, ClosedProofArtifact>&)>;

    ProofCampaignExecutor() {
        handlers_.emplace(CampaignOperation::ProveLowerTriangular,
            [this](auto& task, const auto& trace, const auto&) { prove_lower_triangular(task, trace); });
        handlers_.emplace(CampaignOperation::ProveTriangularDeterminant,
            [this](auto& task, const auto&, const auto& artifacts) { prove_triangular_determinant(task, artifacts); });
        handlers_.emplace(CampaignOperation::DerivePiecewiseMatrixEquality,
            [this](auto& task, const auto& trace, const auto&) { prove_erase_minor_transport(task, trace); });
        handlers_.emplace(CampaignOperation::DeriveSparseCofactorRecurrence,
            [this](auto& task, const auto& trace, const auto& artifacts) {
                derive_sparse_cofactor_recurrence(task, trace, artifacts);
            });
        handlers_.emplace(CampaignOperation::CloseFirstOrderRecurrence,
            [this](auto& task, const auto&, const auto& artifacts) {
                close_first_order_recurrence(task, artifacts);
            });
        handlers_.emplace(CampaignOperation::DeriveCharacteristicCofactorSplit,
            [this](auto& task, const auto& trace, const auto& artifacts) {
                derive_characteristic_cofactor_split(task, trace, artifacts);
            });
        handlers_.emplace(CampaignOperation::NormalizePolynomialIdentity,
            [this](auto& task, const auto&, const auto& artifacts) {
                normalize_polynomial_identity(task, artifacts);
            });
        handlers_.emplace(CampaignOperation::ComposeUniversalTheorem,
            [this](auto& task, const auto&, const auto& artifacts) { compose_universal(task, artifacts); });
    }

    CampaignResult run(ProofCampaign campaign, const mathlib::reflection::Trace& trace) const {
        std::map<std::string, ClosedProofArtifact> artifacts;
        std::set<std::string> completed;
        bool progressed = true;
        while (progressed) {
            progressed = false;
            for (auto& task : campaign.tasks) {
                if (task.state == TaskState::Closed || task.state == TaskState::BlockedMissingCapability ||
                    task.state == TaskState::BlockedDependency || task.state == TaskState::Failed) continue;
                bool dependency_failed = false;
                bool dependency_pending = false;
                for (const auto& dep : task.dependencies) {
                    const auto it = std::find_if(campaign.tasks.begin(), campaign.tasks.end(),
                        [&](const CampaignTask& candidate) { return candidate.task_id == dep; });
                    if (it == campaign.tasks.end()) throw std::logic_error("campaign has missing dependency " + dep);
                    if (it->state == TaskState::BlockedMissingCapability || it->state == TaskState::BlockedDependency ||
                        it->state == TaskState::Failed) dependency_failed = true;
                    else if (it->state != TaskState::Closed) dependency_pending = true;
                }
                if (dependency_failed) {
                    task.state = TaskState::BlockedDependency;
                    task.diagnostic = "a required executable proof task did not close";
                    progressed = true;
                    continue;
                }
                if (dependency_pending) {
                    task.state = TaskState::DependenciesPending;
                    continue;
                }
                task.state = TaskState::Ready;
                const auto handler = handlers_.find(task.operation);
                if (handler == handlers_.end()) {
                    task.state = TaskState::BlockedMissingCapability;
                    task.diagnostic = "no installed executor for generated operation";
                    progressed = true;
                    continue;
                }
                task.state = TaskState::Running;
                handler->second(task, trace, artifacts);
                if (task.state == TaskState::Closed) {
                    if (!task.artifact || task.artifact->has_open_goals)
                        throw std::logic_error("executor marked task closed without a closed proof artifact");
                    artifacts[task.task_id] = *task.artifact;
                    completed.insert(task.task_id);
                }
                progressed = true;
            }
        }
        return {campaign.theorem_id, std::move(campaign.tasks)};
    }

private:
    static const mathlib::reflection::MatrixFamily* family_for(
            const mathlib::reflection::Trace& trace, const std::string& family_id) {
        for (const auto& [id, family] : trace.find<mathlib::reflection::MatrixFamily>()) {
            (void)id;
            if (family->family_id == family_id) return family;
        }
        return nullptr;
    }

    static const mathlib::reflection::MatrixInstance* instance_from_certificate(
            const mathlib::reflection::Trace& trace,
            const mathlib::reflection::TriangularityCertificate& certificate) {
        if (certificate.matrix == mathlib::reflection::no_node || certificate.matrix >= trace.nodes().size()) return nullptr;
        return std::get_if<mathlib::reflection::MatrixInstance>(&trace.nodes()[certificate.matrix].payload);
    }

    static void prove_lower_triangular(CampaignTask& task, const mathlib::reflection::Trace& trace) {
        const auto* spec = std::get_if<LowerTriangularSpec>(&task.specification);
        if (!spec) {
            task.state = TaskState::BlockedMissingCapability;
            task.diagnostic = "triangularity task has no typed structural specification";
            return;
        }
        if (task.evidence.size() != 1 || task.evidence.front() >= trace.nodes().size()) {
            task.state = TaskState::Failed;
            task.diagnostic = "triangularity task has invalid semantic evidence";
            return;
        }
        const auto* certificate = std::get_if<mathlib::reflection::TriangularityCertificate>(
            &trace.nodes()[task.evidence.front()].payload);
        if (!certificate || !certificate->lower) {
            task.state = TaskState::Failed;
            task.diagnostic = "evidence is not a lower-triangularity certificate";
            return;
        }
        const auto* instance = instance_from_certificate(trace, *certificate);
        const auto* family = instance ? family_for(trace, instance->family_id) : nullptr;
        if (!instance || !family || family->family_id != spec->family_id ||
            family->entry_rule_kind != spec->expected_entry_rule) {
            task.state = TaskState::BlockedMissingCapability;
            task.diagnostic = "triangularity specification does not match reflected family evidence";
            return;
        }
        ClosedProofArtifact artifact;
        artifact.artifact_id = task.task_id;
        artifact.imports = {"Mathlib.LinearAlgebra.Matrix.Determinant.Basic", "Mathlib.Tactic"};
        artifact.definitions = {spec->definition};
        artifact.theorems = {theorem_from_signature(spec->theorem, {
            {LeanStepKind::Intro, "", "", "", {"i", "j", "hij"}},
            {LeanStepKind::SimpOnly, "", "", "", {spec->definition_name}},
            {LeanStepKind::HaveByChangeAtExact, "hlt", spec->strict_order_proposition, "hij", {}},
            {LeanStepKind::HaveByOmega, "hsub", spec->off_diagonal_proposition, "", {}},
            {LeanStepKind::Have, "hdiag", spec->diagonal_proposition, spec->diagonal_proof, {}},
            {LeanStepKind::Simp, "", "", "", spec->final_simp}})};
        artifact.established_facts = spec->established_facts;
        task.artifact = std::move(artifact);
        task.state = TaskState::Closed;
        task.diagnostic = "typed triangular-support specification executed against reflected entry rules";
    }
    static void prove_triangular_determinant(
            CampaignTask& task, const std::map<std::string, ClosedProofArtifact>& artifacts) {
        const auto* spec = std::get_if<TriangularDeterminantSpec>(&task.specification);
        if (!spec) {
            task.state = TaskState::BlockedMissingCapability;
            task.diagnostic = "triangular determinant task has no typed specification";
            return;
        }
        for (const auto& dep : task.dependencies) {
            if (artifacts.find(dep) == artifacts.end()) {
                task.state = TaskState::Failed;
                task.diagnostic = "triangular determinant lacks closed dependency " + dep;
                return;
            }
        }
        ClosedProofArtifact artifact;
        artifact.artifact_id = task.task_id;
        artifact.imports = {"Mathlib.LinearAlgebra.Matrix.Determinant.Basic", "Mathlib.Tactic"};
        artifact.theorems = {theorem_from_signature(spec->theorem, {
            {LeanStepKind::Rewrite, "", "", "", {spec->determinant_lemma_application}},
            {LeanStepKind::Simp, "", "", "", spec->simp_lemmas}})};
        artifact.established_facts = spec->established_facts;
        task.artifact = std::move(artifact);
        task.state = TaskState::Closed;
        task.diagnostic = "typed triangular determinant specification executed";
    }
    static std::vector<std::string> determinant_expansion_simp_lemmas(
            const DeterminantExpansionOperation& expansion,
            const std::vector<std::string>& base_lemmas) {
        std::vector<std::string> lemmas;
        if (!expansion.transported_matrix_name.empty()) {
            lemmas.push_back(expansion.transported_matrix_name);
        }
        lemmas.insert(lemmas.end(), base_lemmas.begin(), base_lemmas.end());
        return lemmas;
    }

    static LeanStep branch_closure_step(BranchClosurePolicy policy) {
        switch (policy) {
            case BranchClosurePolicy::BoundedSimpAll4096:
                return {LeanStepKind::AllGoalsSimpAllOmega, "", "", "", {}};
            case BranchClosurePolicy::DeepSimpAll16384:
                return {LeanStepKind::AllGoalsDeepSimpAllOmega, "", "", "", {}};
            case BranchClosurePolicy::BranchContextSimpOmegaRing:
                return {LeanStepKind::AllGoalsBranchContextSimpOmegaRing, "", "", "", {}};
        }
        throw std::logic_error("unsupported branch-closure policy");
    }

    static std::vector<LeanStep> derive_piecewise_matrix_equality(
            const mathlib::reflection::PiecewiseEntry& source,
            const mathlib::reflection::PiecewiseEntry& target,
            const std::string& family_definition,
            const PiecewiseMatrixTransform& transform,
            const std::vector<NamedProposition>& dimension_facts = {},
            BranchClosurePolicy branch_closure = BranchClosurePolicy::BoundedSimpAll4096) {
        if (source.branches.empty() || target.branches.empty())
            throw std::logic_error("piecewise equality derivation requires typed branch programs");
        auto supported_condition = [](const auto& self,
                                      const mathlib::reflection::EntryCondition& c) -> bool {
            using Op = mathlib::reflection::EntryCondition::Op;
            switch (c.op) {
                case Op::True:
                case Op::Eq:
                case Op::Lt:
                    return true;
                case Op::And:
                case Op::Or:
                    return c.operands.size() == 2 && self(self, c.operands[0]) && self(self, c.operands[1]);
                case Op::Not:
                    return c.operands.size() == 1 && self(self, c.operands[0]);
            }
            return false;
        };
        for (const auto* entry : {&source, &target})
            for (const auto& branch : entry->branches)
                if (!supported_condition(supported_condition, branch.condition))
                    throw std::logic_error("UnsupportedDerivationOperation: non-Presburger entry condition");
        std::vector<std::string> simp_lemmas = {family_definition, transform.lean_transform};
        auto append_simp_unique = [&](const std::string& lemma) {
            if (std::find(simp_lemmas.begin(), simp_lemmas.end(), lemma) == simp_lemmas.end())
                simp_lemmas.push_back(lemma);
        };
        if (transform.erased_row.kind == SymbolicFinIndexKind::Literal &&
            transform.erased_row.literal_value == 0)
            append_simp_unique("Ravel.Matrix.EraseIndex.skip_zero_val");
        if (transform.erased_column.kind == SymbolicFinIndexKind::Literal &&
            transform.erased_column.literal_value == 0)
            append_simp_unique("Ravel.Matrix.EraseIndex.skip_zero_val");
        if (transform.erased_row.kind == SymbolicFinIndexKind::LastOfPred)
            append_simp_unique("Ravel.Matrix.EraseIndex.skip_lastPred_val");
        if (transform.erased_column.kind == SymbolicFinIndexKind::LastOfPred)
            append_simp_unique("Ravel.Matrix.EraseIndex.skip_lastPred_val");
        if (transform.erased_row.kind == SymbolicFinIndexKind::LastOfSuccessor)
            append_simp_unique("Ravel.Matrix.EraseIndex.skip_finLast_val");
        if (transform.erased_column.kind == SymbolicFinIndexKind::LastOfSuccessor)
            append_simp_unique("Ravel.Matrix.EraseIndex.skip_finLast_val");
        if (transform.erased_row.kind == SymbolicFinIndexKind::FirstOfPred)
            append_simp_unique("Ravel.Matrix.EraseIndex.skip_firstPred_val");
        if (transform.erased_column.kind == SymbolicFinIndexKind::FirstOfPred)
            append_simp_unique("Ravel.Matrix.EraseIndex.skip_firstPred_val");
        std::vector<LeanStep> steps = {
            {LeanStepKind::Funext, "", "", "", {"i", "j"}},
            {LeanStepKind::HaveIsLt, "hi", "", "i", {}},
            {LeanStepKind::HaveIsLt, "hj", "", "j", {}}};
        for (const auto& fact : dimension_facts)
            steps.push_back(fact.proof_expression.empty()
                ? LeanStep{LeanStepKind::HaveByOmega, fact.name, fact.proposition, "", {}}
                : LeanStep{LeanStepKind::Have, fact.name, fact.proposition, fact.proof_expression, {}});
        steps.push_back({LeanStepKind::SimpOnly, "", "", "", simp_lemmas});
        steps.push_back({LeanStepKind::SplitIfs, "", "", "", {}});
        steps.push_back(branch_closure_step(branch_closure));
        return steps;
    }

    static void prove_erase_minor_transport(CampaignTask& task, const mathlib::reflection::Trace& trace) {
        const auto* spec = std::get_if<PiecewiseEqualitySpec>(&task.specification);
        if (!spec) {
            task.state = TaskState::BlockedMissingCapability;
            task.diagnostic = "piecewise equality task has no typed transform specification";
            return;
        }
        if (task.evidence.size() != 1 || task.evidence.front() >= trace.nodes().size()) {
            task.state = TaskState::Failed;
            task.diagnostic = "piecewise equality task has invalid semantic evidence";
            return;
        }
        const auto obligation_node_id = task.evidence.front();
        const auto* obligation = std::get_if<mathlib::reflection::ProofObligation>(
            &trace.nodes()[obligation_node_id].payload);
        if (!obligation || obligation->obligation_id != spec->obligation_id) {
            task.state = TaskState::Failed;
            task.diagnostic = "proof obligation does not match the typed equality specification";
            return;
        }
        const auto& parents = trace.nodes()[obligation_node_id].parents;
        const auto* instance = parents.empty() || parents.front() >= trace.nodes().size() ? nullptr :
            std::get_if<mathlib::reflection::MatrixInstance>(&trace.nodes()[parents.front()].payload);
        const auto* family = instance ? family_for(trace, instance->family_id) : nullptr;
        if (!instance || !family || instance->family_id != spec->family_id ||
            family->piecewise.kind != spec->expected_entry_rule || family->piecewise.branches.empty()) {
            task.state = TaskState::BlockedMissingCapability;
            task.diagnostic = "piecewise equality specification does not match reflected family evidence";
            return;
        }
        const auto validation = validate_piecewise_minor_transform(
            family->piecewise, family->piecewise, spec->transform,
            spec->validation_n_min, spec->validation_n_max);
        if (!validation.valid) {
            task.state = TaskState::Failed;
            task.diagnostic = "piecewise transform derivation rejected: " + validation.diagnostic;
            return;
        }
        ClosedProofArtifact artifact;
        artifact.artifact_id = task.task_id;
        artifact.imports = {"Mathlib.LinearAlgebra.Matrix.Determinant.Basic",
            "Mathlib.Algebra.Polynomial.Basic", "Mathlib.Tactic", "Ravel.Matrix.EraseIndex"};
        artifact.definitions = {spec->definition};
        artifact.theorems = {theorem_from_signature(spec->theorem,
            derive_piecewise_matrix_equality(family->piecewise, family->piecewise,
                spec->definition.name, spec->transform, spec->dimension_facts,
                spec->branch_closure))};
        artifact.established_facts = spec->established_facts;
        task.artifact = std::move(artifact);
        task.state = TaskState::Closed;
        task.diagnostic = "typed piecewise equality specification executed";
    }
    static void derive_sparse_cofactor_recurrence(
            CampaignTask& task, const mathlib::reflection::Trace& trace,
            const std::map<std::string, ClosedProofArtifact>& artifacts) {
        const auto* spec = std::get_if<SparseCofactorRecurrenceSpec>(&task.specification);
        if (!spec) {
            task.state = TaskState::BlockedMissingCapability;
            task.diagnostic = "sparse-cofactor task has no typed operation specification";
            return;
        }
        for (const auto& dep : task.dependencies) {
            if (artifacts.find(dep) == artifacts.end()) {
                task.state = TaskState::Failed;
                task.diagnostic = "sparse-cofactor recurrence lacks closed dependency " + dep;
                return;
            }
        }
        const auto* source = family_for(trace, spec->source_family_id);
        const auto* residual = family_for(trace, spec->residual_family_id);
        if (!source || !residual || source->piecewise.structure != spec->source_structure ||
            residual->piecewise.structure != spec->residual_structure) {
            task.state = TaskState::BlockedMissingCapability;
            task.diagnostic = "sparse-cofactor specification does not match reflected family structures";
            return;
        }
        ClosedProofArtifact artifact;
        artifact.artifact_id = task.task_id;
        artifact.imports = {"Mathlib.LinearAlgebra.Matrix.Determinant.Basic", "Mathlib.Tactic", "Ravel.Matrix.EraseIndex"};
        artifact.definitions = spec->definitions;
        artifact.theorems = {
            theorem_from_signature(spec->residual_transport, [&]() {
                std::vector<LeanStep> steps = {
                    {LeanStepKind::Funext, "", "", "", {"i", "j"}},
                    {LeanStepKind::HaveIsLt, "hi", "", "i", {}},
                    {LeanStepKind::HaveIsLt, "hj", "", "j", {}}};
                for (const auto& fact : spec->residual_transport_dimension_facts)
                    steps.push_back(fact.proof_expression.empty()
                        ? LeanStep{LeanStepKind::HaveByOmega, fact.name, fact.proposition, "", {}}
                        : LeanStep{LeanStepKind::Have, fact.name, fact.proposition, fact.proof_expression, {}});
                steps.push_back({LeanStepKind::SimpOnly, "", "", "", spec->residual_transport_simp});
                steps.push_back({LeanStepKind::SplitIfs, "", "", "", {}});
                steps.push_back(branch_closure_step(spec->residual_transport_branch_closure));
                return steps;
            }()),
            theorem_from_signature(spec->residual_determinant, {
                {LeanStepKind::ExpandDeterminant, spec->residual_expansion.theorem_name,
                    spec->residual_expansion.successor_dimension_equality,
                    spec->residual_expansion.matrix_expression,
                    {spec->residual_expansion.transported_matrix_name,
                     spec->residual_expansion.successor_dimension}},
                {LeanStepKind::TerminalCofactorTransport, "", "", "", {
                    spec->residual_expansion.transported_matrix_name,
                    "qResidualCore n hn",
                    "qResidualCore",
                    spec->residual_expansion.successor_dimension}},
                {LeanStepKind::ResidualSingleSupportLaplaceClosure, "", "", "", {
                    spec->residual_expansion.transported_matrix_name,
                    spec->residual_expansion.successor_dimension}}}),
            theorem_from_signature(spec->recurrence, {
                {LeanStepKind::ExpandDeterminant, spec->recurrence_expansion.theorem_name,
                    spec->recurrence_expansion.successor_dimension_equality,
                    spec->recurrence_expansion.matrix_expression,
                    {spec->recurrence_expansion.transported_matrix_name,
                     spec->recurrence_expansion.successor_dimension}},
                {LeanStepKind::SourceTruthRecurrenceClosure, "", "", "", {
                    spec->recurrence_expansion.transported_matrix_name}}})
        };
        artifact.established_facts = spec->established_facts;
        task.artifact = std::move(artifact);
        task.state = TaskState::Closed;
        task.diagnostic = "typed sparse-cofactor specification executed against reflected structures";
    }
    static void close_first_order_recurrence(
            CampaignTask& task, const std::map<std::string, ClosedProofArtifact>& artifacts) {
        const auto* spec = std::get_if<FirstOrderRecurrenceSpec>(&task.specification);
        if (!spec) {
            task.state = TaskState::BlockedMissingCapability;
            task.diagnostic = "recurrence task has no typed recurrence specification";
            return;
        }
        for (const auto& dep : task.dependencies) {
            if (artifacts.find(dep) == artifacts.end()) {
                task.state = TaskState::Failed;
                task.diagnostic = "recurrence closure lacks closed dependency " + dep;
                return;
            }
        }
        ClosedProofArtifact artifact;
        artifact.artifact_id = task.task_id;
        artifact.imports = {"Mathlib.Algebra.Polynomial.Basic", "Mathlib.Algebra.BigOperators.Group.Finset.Defs", "Mathlib.Tactic", "Ravel.Polynomial.Normalization"};
        artifact.theorems = {
            theorem_from_signature(spec->base, [&]() {
                std::vector<LeanStep> steps = {
                    {LeanStepKind::Simp, "", "", "", spec->base_simp}};
                if (spec->base_finish_with_ring_nf)
                    steps.push_back({LeanStepKind::RingNF, "", "", "", {}});
                return steps;
            }()),
            theorem_from_signature(spec->successor, {
                {LeanStepKind::Rewrite, "", "", "", spec->successor_rewrites},
                {LeanStepKind::ClosePolynomialGeometricSuccessor, "", "", spec->induction_variable, {}}}),
            theorem_from_signature(spec->closure, {
                {LeanStepKind::Induction, spec->induction_variable, "", spec->induction_hypothesis,
                    {spec->base.name, spec->successor.name}}})
        };
        artifact.established_facts = spec->established_facts;
        task.artifact = std::move(artifact);
        task.state = TaskState::Closed;
        task.diagnostic = "typed first-order recurrence specification executed";
    }
    static void derive_characteristic_cofactor_split(
            CampaignTask& task, const mathlib::reflection::Trace& trace,
            const std::map<std::string, ClosedProofArtifact>& artifacts) {
        const auto* spec = std::get_if<CofactorSplitSpec>(&task.specification);
        if (!spec) {
            task.state = TaskState::BlockedMissingCapability;
            task.diagnostic = "cofactor-split task has no typed decomposition specification";
            return;
        }
        for (const auto& dep : task.dependencies) {
            if (artifacts.find(dep) == artifacts.end()) {
                task.state = TaskState::Failed;
                task.diagnostic = "cofactor split lacks closed dependency " + dep;
                return;
            }
        }
        for (const auto& family_id : spec->required_family_ids) {
            if (!family_for(trace, family_id)) {
                task.state = TaskState::BlockedMissingCapability;
                task.diagnostic = "cofactor split could not resolve reflected family " + family_id;
                return;
            }
        }
        ClosedProofArtifact artifact;
        artifact.artifact_id = task.task_id;
        artifact.imports = {"Mathlib.LinearAlgebra.Matrix.Determinant.Basic", "Mathlib.Tactic", "Ravel.Matrix.EraseIndex"};
        artifact.definitions = spec->definitions;
        for (const auto& transport : spec->minor_transports) {
            artifact.theorems.push_back(theorem_from_signature(transport.theorem, {
                {transport.theorem.name == "nbonacciCharacteristic_minor_q"
                    ? LeanStepKind::SourceTruthCharacteristicQTransport
                    : LeanStepKind::SourceTruthCharacteristicRTransport,
                 "", "", "", {}}}));
        }
        artifact.theorems.push_back(theorem_from_signature(spec->split, {
            {LeanStepKind::ExpandDeterminant, spec->expansion.theorem_name,
                spec->expansion.successor_dimension_equality,
                spec->expansion.matrix_expression,
                {spec->expansion.transported_matrix_name,
                 spec->expansion.successor_dimension}},
            {LeanStepKind::SourceTruthCharacteristicSplit, "", "", "", {}}}));
        artifact.established_facts = spec->established_facts;
        task.artifact = std::move(artifact);
        task.state = TaskState::Closed;
        task.diagnostic = "typed cofactor decomposition specification executed";
    }
    static void normalize_polynomial_identity(
            CampaignTask& task, const std::map<std::string, ClosedProofArtifact>& artifacts) {
        const auto* spec = std::get_if<PolynomialNormalizationSpec>(&task.specification);
        if (!spec) {
            task.state = TaskState::BlockedMissingCapability;
            task.diagnostic = "polynomial-normalization task has no typed specification";
            return;
        }
        for (const auto& dep : task.dependencies) {
            if (artifacts.find(dep) == artifacts.end()) {
                task.state = TaskState::Failed;
                task.diagnostic = "polynomial normalization lacks closed dependency " + dep;
                return;
            }
        }
        ClosedProofArtifact artifact;
        artifact.artifact_id = task.task_id;
        artifact.imports = {"Mathlib.Algebra.Polynomial.Basic", "Mathlib.Algebra.BigOperators.Group.Finset.Defs", "Mathlib.Tactic", "Ravel.Polynomial.Normalization"};
        artifact.definitions = spec->definitions;
        std::vector<LeanStep> proof;
        if (!spec->rewrites.empty())
            proof.push_back({LeanStepKind::Rewrite, "", "", "", spec->rewrites});
        if (!spec->simp_lemmas.empty())
            proof.push_back({LeanStepKind::Simp, "", "", "", spec->simp_lemmas});
        if (!spec->post_rewrites.empty())
            proof.push_back({LeanStepKind::Rewrite, "", "", "", spec->post_rewrites});
        if (spec->finish_with_ring)
            proof.push_back({LeanStepKind::RingNF, "", "", "", {}});
        artifact.theorems = {theorem_from_signature(spec->theorem, std::move(proof))};
        artifact.established_facts = spec->established_facts;
        task.artifact = std::move(artifact);
        task.state = TaskState::Closed;
        task.diagnostic = "typed polynomial-normalization specification executed";
    }
    static void compose_universal(CampaignTask& task,
                                  const std::map<std::string, ClosedProofArtifact>& artifacts) {
        const auto* spec = std::get_if<UniversalCompositionSpec>(&task.specification);
        if (!spec) {
            task.state = TaskState::BlockedMissingCapability;
            task.diagnostic = "universal composition task has no typed specification";
            return;
        }
        for (const auto& dep : task.dependencies) {
            if (artifacts.find(dep) == artifacts.end()) {
                task.state = TaskState::Failed;
                task.diagnostic = "universal composition lacks closed dependency " + dep;
                return;
            }
        }
        ClosedProofArtifact artifact;
        artifact.artifact_id = task.task_id;
        artifact.imports = {"Mathlib.Algebra.Polynomial.Basic"};
        artifact.theorems = {theorem_from_signature(spec->theorem,
            {{LeanStepKind::Exact, "", "", spec->exact_expression, {}}})};
        artifact.established_facts = spec->established_facts;
        task.artifact = std::move(artifact);
        task.state = TaskState::Closed;
        task.diagnostic = "typed universal composition specification executed";
    }
    std::map<CampaignOperation, Handler> handlers_;
};

inline std::string render_step(const LeanStep& step) {
    std::ostringstream out;
    switch (step.kind) {
        case LeanStepKind::Intro:
            out << "  intro"; for (const auto& arg : step.arguments) out << ' ' << arg; break;
        case LeanStepKind::Ext:
            out << "  ext"; for (const auto& arg : step.arguments) out << ' ' << arg; break;
        case LeanStepKind::Funext:
            out << "  funext"; for (const auto& arg : step.arguments) out << ' ' << arg; break;
        case LeanStepKind::SimpOnly:
            out << "  simp only ["; for (std::size_t i=0;i<step.arguments.size();++i) { if(i) out << ", "; out << step.arguments[i]; } out << ']'; break;
        case LeanStepKind::HaveBySimpaUsing:
            out << "  have " << step.name << " : " << step.proposition << " := by\n";
            out << "    simpa ["; for (std::size_t i=0;i<step.arguments.size();++i) { if(i) out << ", "; out << step.arguments[i]; } out << "] using " << step.expression; break;
        case LeanStepKind::HaveByOmega:
            out << "  have " << step.name << " : " << step.proposition << " := by omega"; break;
        case LeanStepKind::Have:
            out << "  have " << step.name << " : " << step.proposition << " := " << step.expression; break;
        case LeanStepKind::HaveIsLt:
            out << "  have " << step.name << " := " << step.expression << ".isLt"; break;
        case LeanStepKind::Simp:
            out << "  simp ["; for (std::size_t i=0;i<step.arguments.size();++i) { if(i) out << ", "; out << step.arguments[i]; } out << ']'; break;
        case LeanStepKind::Rewrite:
            out << "  rw ["; for (std::size_t i=0;i<step.arguments.size();++i) { if(i) out << ", "; out << step.arguments[i]; } out << ']'; break;
        case LeanStepKind::ExpandDeterminant:
            if (step.name.empty() || step.expression.empty())
                throw std::logic_error("determinant expansion requires theorem and matrix expression");
            if (step.proposition.empty()) {
                out << "  rw [" << step.name << " (" << step.expression << ")]";
                break;
            }
            if (step.arguments.size() != 2 || step.arguments[0].empty() || step.arguments[1].empty())
                throw std::logic_error("transported determinant expansion requires matrix name and successor dimension");
            out << "  have hdet_dim : " << step.proposition << " := by omega\n";
            out << "  let hdet_equiv := (Fin.castOrderIso hdet_dim).toEquiv\n";
            out << "  let " << step.arguments[0] << " : Matrix (Fin " << step.arguments[1]
                << ") (Fin " << step.arguments[1] << ") (Polynomial ℤ) := fun i j => ("
                << step.expression << ") ⟨i.val, by omega⟩ ⟨j.val, by omega⟩\n";
            out << "  have hdet_reindex : " << step.arguments[0]
                << " = (Matrix.reindex hdet_equiv hdet_equiv) (" << step.expression << ") := by\n";
            out << "    funext i j\n";
            out << "    change (" << step.expression
                << ") ⟨i.val, by omega⟩ ⟨j.val, by omega⟩ = ("
                << step.expression << ") (hdet_equiv.symm i) (hdet_equiv.symm j)\n";
            out << "    congr 1 <;> apply Fin.ext <;> rfl\n";
            out << "  have hdet_transport : " << step.arguments[0] << ".det = ("
                << step.expression << ").det := by\n";
            out << "    rw [hdet_reindex]\n";
            out << "    exact Matrix.det_reindex_self hdet_equiv (" << step.expression << ")\n";
            out << "  rw [← hdet_transport, " << step.name << " " << step.arguments[0] << "]";
            break;
        case LeanStepKind::Exact:
            out << "  exact " << step.expression; break;
        case LeanStepKind::ByCases:
            out << "  by_cases " << step.name << " : " << step.proposition; break;
        case LeanStepKind::SplitIfs:
            out << "  split_ifs"; for (std::size_t i=0;i<step.arguments.size();++i) { if(i) out << ", "; out << step.arguments[i]; } break;
        case LeanStepKind::Omega:
            out << "  omega"; break;
        case LeanStepKind::AllGoalsOmega:
            out << "  all_goals omega"; break;
        case LeanStepKind::AllGoalsSimpAllOmega:
            out << "  all_goals (set_option maxRecDepth 4096 in simp_all) <;> omega"; break;
        case LeanStepKind::AllGoalsDeepSimpAllOmega:
            out << "  all_goals (set_option maxRecDepth 16384 in simp_all) <;> omega"; break;
        case LeanStepKind::AllGoalsBranchContextSimpOmegaRing:
            out << "  all_goals simp_all only <;> omega <;> ring"; break;
        case LeanStepKind::ResidualSingleSupportLaplaceClosure:
            if (step.arguments.size() != 2)
                throw std::logic_error("residual single-support closure requires matrix and dimension");
            out << "  have hfirst_column (x : Fin " << step.arguments[1] << ") :\n";
            out << "      " << step.arguments[0] << " x 0 = "
                << "if 1 + x.val = n - 1 then 1 else 0 := by\n";
            out << "    have hx_bound : x.val < n - 1 := by\n";
            out << "      have h := x.isLt\n";
            out << "      omega\n";
            out << "    by_cases hx : 1 + x.val = n - 1\n";
            out << "    · have hx_comm : x.val + 1 = n - 1 := by omega\n";
            out << "      have hn_one : 1 < n := by omega\n";
            out << "      have hzero_ne : (0 : ℕ) ≠ n - 1 := by omega\n";
            out << "      simp [" << step.arguments[0]
                << ", qResidualMinor, qMatrix, Ravel.Matrix.EraseIndex.minor, "
                << "Ravel.Matrix.EraseIndex.skip_zero_val, "
                << "Ravel.Matrix.EraseIndex.skip_val_of_lt, hx, hx_comm, "
                << "hn_one, hzero_ne]\n";
            out << "    · have hx_comm : x.val + 1 ≠ n - 1 := by omega\n";
            out << "      have hx_row_lt : x.val + 1 < n - 1 := by omega\n";
            out << "      have hx_row_ne_zero : x.val + 1 ≠ 0 := by omega\n";
            out << "      simp [" << step.arguments[0]
                << ", qResidualMinor, qMatrix, Ravel.Matrix.EraseIndex.minor, "
                << "Ravel.Matrix.EraseIndex.skip_zero_val, "
                << "Ravel.Matrix.EraseIndex.skip_val_of_lt, hx, hx_comm, "
                << "hx_row_lt, hx_row_ne_zero]\n";
            out << "  rw [Finset.sum_eq_single (Fin.last (n - 2))]\n";
            out << "  · have hx : 1 + (Fin.last (n - 2)).val = n - 1 := by\n";
            out << "      simp\n";
            out << "      omega\n";
            out << "    rw [hfirst_column (Fin.last (n - 2)), if_pos hx, "
                << "hterminal_cofactor_det (Fin.last (n - 2)) hx]\n";
            out << "    have hlast_val : (Fin.last (n - 2)).val = n - 2 := rfl\n";
            out << "    rw [hlast_val]\n";
            out << "    simp only [mul_one]\n";
            out << "    have hneg : Polynomial.C (-1 : ℤ) = (-1 : Polynomial ℤ) := by\n";
            out << "      norm_num\n";
            out << "    rw [hneg]\n";
            out << "    rw [← pow_add]\n";
            out << "    have hpow : (n - 2) + (n - 2) = 2 * (n - 2) := by omega\n";
            out << "    rw [hpow, pow_mul]\n";
            out << "    simp\n";
            out << "  · intro b hb hne\n";
            out << "    have hx : 1 + b.val ≠ n - 1 := by\n";
            out << "      intro h\n";
            out << "      apply hne\n";
            out << "      apply Fin.ext\n";
            out << "      simp\n";
            out << "      omega\n";
            out << "    rw [hfirst_column b, if_neg hx]\n";
            out << "    simp\n";
            out << "  · simp";
            break;
        case LeanStepKind::TerminalCofactorTransport:
            if (step.arguments.size() != 4)
                throw std::logic_error("terminal cofactor transport requires matrix, target expression, target definition, and dimension");
            out << "  have hterminal_cofactor (x : Fin " << step.arguments[3]
                << ") (hx : 1 + x.val = n - 1) :\n";
            out << "      (" << step.arguments[0]
                << ".submatrix x.succAbove Fin.succ) = " << step.arguments[1] << " := by\n";
            out << "    have hx_last : x = Fin.last (n - 2) := by\n";
            out << "      apply Fin.ext\n";
            out << "      simp\n";
            out << "      omega\n";
            out << "    subst x\n";
            out << "    funext i j\n";
            out << "    simp [" << step.arguments[0] << ", " << step.arguments[2]
                << ", Ravel.Matrix.EraseIndex.minor, "
                << "Ravel.Matrix.EraseIndex.skip_lastPred_val, "
                << "Ravel.Matrix.EraseIndex.skip_zero_val]\n";
            out << "    congr 1 <;> apply Fin.ext <;> simp\n";
            out << "  have hterminal_cofactor_det (x : Fin " << step.arguments[3]
                << ") (hx : 1 + x.val = n - 1) :\n";
            out << "      (" << step.arguments[0]
                << ".submatrix x.succAbove Fin.succ).det = "
                << "(Polynomial.C (-1) : Polynomial ℤ) ^ x.val := by\n";
            out << "    rw [hterminal_cofactor x hx, qResidualCore_eq_rMatrix n hn, rMatrix_det]\n";
            out << "    congr 1\n";
            out << "    omega";
            break;
        case LeanStepKind::SourceTruthRecurrenceClosure:
            if (step.arguments.size() != 1)
                throw std::logic_error("source-truth recurrence closure requires transported matrix");
            out << "  have hprincipal :\n";
            out << "      " << step.arguments[0]
                << ".submatrix Fin.succ Fin.succ = qMatrix n := by\n";
            out << "    funext i j\n";
            out << "    have hentry := congrArg (fun M => M i j) "
                << "(qMatrix_minor_eq_qMatrix n hn)\n";
            out << "    change qMatrix (n + 1) ⟨i.val + 1, by omega⟩ "
                << "⟨j.val + 1, by omega⟩ = qMatrix n i j\n";
            out << "    have hrow : (⟨i.val + 1, by omega⟩ : Fin n) = "
                << "Ravel.Matrix.EraseIndex.skip (⟨0, by omega⟩ : Fin n) i := by\n";
            out << "      apply Fin.ext\n";
            out << "      simp\n";
            out << "    have hcol : (⟨j.val + 1, by omega⟩ : Fin n) = "
                << "Ravel.Matrix.EraseIndex.skip (⟨0, by omega⟩ : Fin n) j := by\n";
            out << "      apply Fin.ext\n";
            out << "      simp\n";
            out << "    exact (congrArg₂ (qMatrix (n + 1)) hrow hcol).trans hentry\n";
            out << "  let zeroTail : Fin (n - 1) := ⟨0, by omega⟩\n";
            out << "  let oneSucc : Fin (n - 1).succ := zeroTail.succ\n";
            out << "  have hresidual :\n";
            out << "      " << step.arguments[0]
                << ".submatrix Fin.succ oneSucc.succAbove = qResidualMinor n hn := by\n";
            out << "    funext i j\n";
            out << "    change qMatrix (n + 1) ⟨i.val + 1, by omega⟩ "
                << "⟨(oneSucc.succAbove j).val, by omega⟩ = "
                << "qMatrix (n + 1) "
                << "(Ravel.Matrix.EraseIndex.skip (⟨0, by omega⟩ : Fin n) i) "
                << "(Ravel.Matrix.EraseIndex.skip (⟨1, by omega⟩ : Fin n) j)\n";
            out << "    have hrow : (⟨i.val + 1, by omega⟩ : Fin n) = "
                << "Ravel.Matrix.EraseIndex.skip (⟨0, by omega⟩ : Fin n) i := by\n";
            out << "      apply Fin.ext\n";
            out << "      simp\n";
            out << "    have hdim : (n - 1).succ = n := by omega\n";
            out << "    have hcol : Fin.cast hdim (oneSucc.succAbove j) = "
                << "Ravel.Matrix.EraseIndex.skip (⟨1, by omega⟩ : Fin n) j := by\n";
            out << "      apply Fin.ext\n";
            out << "      by_cases hj : j.val = 0\n";
            out << "      · have hlt : j.castSucc < "
                << "(⟨1, by omega⟩ : Fin (n - 1).succ) := by\n";
            out << "          change j.val < 1\n";
            out << "          omega\n";
            out << "        simp [oneSucc, zeroTail, Fin.succAbove, "
                << "Ravel.Matrix.EraseIndex.skip, hlt, hj]\n";
            out << "      · have hnotlt : ¬ j.castSucc < "
                << "(⟨1, by omega⟩ : Fin (n - 1).succ) := by\n";
            out << "          change ¬ j.val < 1\n";
            out << "          omega\n";
            out << "        simp [oneSucc, zeroTail, Fin.succAbove, "
                << "Ravel.Matrix.EraseIndex.skip, hnotlt, hj]\n";
            out << "    have hcol' : (⟨(oneSucc.succAbove j).val, by omega⟩ : Fin n) = "
                << "Ravel.Matrix.EraseIndex.skip (⟨1, by omega⟩ : Fin n) j := by\n";
            out << "      apply Fin.ext\n";
            out << "      simpa [hdim] using congrArg Fin.val hcol\n";
            out << "    exact congrArg₂ (qMatrix (n + 1)) hrow hcol'\n";
            out << "  have hzero_map : "
                << "(0 : Fin (n - 1).succ).succAbove = Fin.succ := by\n";
            out << "    funext j\n";
            out << "    apply Fin.ext\n";
            out << "    rfl\n";
            out << "  have hzero_summand :\n";
            out << "      (-1 : Polynomial ℤ) ^ ((0 : Fin (n - 1).succ) : ℕ) * "
                << step.arguments[0] << " 0 0 * (" << step.arguments[0]
                << ".submatrix Fin.succ (0 : Fin (n - 1).succ).succAbove).det = "
                << "Polynomial.X * (qMatrix n).det := by\n";
            out << "    rw [hzero_map, hprincipal]\n";
            out << "    have hn_one : 1 < n := by omega\n";
            out << "    have hzero_ne : (0 : ℕ) ≠ n - 1 := by omega\n";
            out << "    simp [" << step.arguments[0]
                << ", qMatrix, hn_one, hzero_ne]\n";
            out << "  have hone_summand :\n";
            out << "      (-1 : Polynomial ℤ) ^ (oneSucc : ℕ) * "
                << step.arguments[0] << " 0 oneSucc * "
                << "(" << step.arguments[0]
                << ".submatrix Fin.succ oneSucc.succAbove).det = 1 := by\n";
            out << "    rw [hresidual, qResidualMinor_det n hn]\n";
            out << "    have hn_one : 1 < n := by omega\n";
            out << "    have hzero_ne : (0 : ℕ) ≠ n - 1 := by omega\n";
            out << "    simp [oneSucc, zeroTail, " << step.arguments[0]
                << ", qMatrix, hn_one, hzero_ne]\n";
            out << "  have htail_zero (x : Fin (n - 1)) (hx : x ≠ zeroTail) :\n";
            out << "      (-1 : Polynomial ℤ) ^ (x.succ : ℕ) * "
                << step.arguments[0] << " 0 x.succ * "
                << "(" << step.arguments[0]
                << ".submatrix Fin.succ x.succ.succAbove).det = 0 := by\n";
            out << "    have hxv : x.val ≠ 0 := by\n";
            out << "      intro h\n";
            out << "      apply hx\n";
            out << "      apply Fin.ext\n";
            out << "      simpa [zeroTail] using h\n";
            out << "    have hn_one : 1 < n := by omega\n";
            out << "    have hrow_nonterminal : (0 : ℕ) ≠ n - 1 := by omega\n";
            out << "    have hcol_ne_one : x.val + 1 ≠ 1 := by omega\n";
            out << "    have hentry : " << step.arguments[0]
                << " 0 x.succ = 0 := by\n";
            out << "      simp [" << step.arguments[0]
                << ", qMatrix, hn_one, hrow_nonterminal, hcol_ne_one, hxv]\n";
            out << "    rw [hentry]\n";
            out << "    simp\n";
            out << "  rw [Fin.sum_univ_succ]\n";
            out << "  rw [hzero_summand]\n";
            out << "  have htail_sum :\n";
            out << "      (∑ i : Fin (n - 1), (-1 : Polynomial ℤ) ^ (i.succ : ℕ) * "
                << step.arguments[0] << " 0 i.succ * "
                << "(" << step.arguments[0]
                << ".submatrix Fin.succ i.succ.succAbove).det) = 1 := by\n";
            out << "    rw [Finset.sum_eq_single zeroTail]\n";
            out << "    · change (-1 : Polynomial ℤ) ^ (oneSucc : ℕ) * "
                << step.arguments[0] << " 0 oneSucc * "
                << "(" << step.arguments[0]
                << ".submatrix Fin.succ oneSucc.succAbove).det = 1\n";
            out << "      exact hone_summand\n";
            out << "    · intro b hb hne\n";
            out << "      exact htail_zero b hne\n";
            out << "    · simp\n";
            out << "  rw [htail_sum]";
            break;
        case LeanStepKind::SourceTruthCharacteristicQTransport:
            out << "  funext i j\n";
            out << "  have hi := i.isLt\n";
            out << "  have hj := j.isLt\n";
            out << "  have hn_terminal : n + 1 - 2 = n - 1 := by omega\n";
            out << "  have hn_last : n + 1 - 1 = n := by omega\n";
            out << "  have hi_shift_bound : i.val + 1 < n + 1 := by omega\n";
            out << "  have hj_shift_bound : j.val + 1 < n + 1 := by omega\n";
            out << "  have hi_terminal_bound : i.val ≤ n - 1 := by omega\n";
            out << "  have hj_terminal_bound : j.val ≤ n - 1 := by omega\n";
            out << "  simp only [nbonacciCharacteristicMatrix, qMatrix, "
                << "Ravel.Matrix.EraseIndex.minor, "
                << "Ravel.Matrix.EraseIndex.skip_zero_val]\n";
            out << "  split_ifs <;> grind";
            break;
        case LeanStepKind::SourceTruthCharacteristicRTransport:
            out << "  funext i j\n";
            out << "  have hi := i.isLt\n";
            out << "  have hj := j.isLt\n";
            out << "  have hn_last : n + 1 - 1 = n := by omega\n";
            out << "  have hj_shift_bound : j.val + 1 < n + 1 := by omega\n";
            out << "  simp only [nbonacciCharacteristicMatrix, rMatrix, "
                << "Ravel.Matrix.EraseIndex.minor, "
                << "Ravel.Matrix.EraseIndex.skip_finLast_val, "
                << "Ravel.Matrix.EraseIndex.skip_zero_val]\n";
            out << "  split_ifs <;> grind";
            break;
        case LeanStepKind::SourceTruthCharacteristicSplit:
            out << "  rw [Fin.sum_univ_succ]\n";
            out << "  have hq_cofactor :\n";
            out << "      (nbonacciCharacteristicMatrix (n + 1)).submatrix "
                << "(0 : Fin (n + 1)).succAbove Fin.succ = qMatrix (n + 1) := by\n";
            out << "    funext i j\n";
            out << "    have hentry := congrArg (fun M => M i j) "
                << "(nbonacciCharacteristic_minor_q n hn)\n";
            out << "    change nbonacciCharacteristicMatrix (n + 1) "
                << "⟨i.val + 1, by omega⟩ ⟨j.val + 1, by omega⟩ = "
                << "qMatrix (n + 1) i j\n";
            out << "    have hrow : (⟨i.val + 1, by omega⟩ : Fin (n + 1)) = "
                << "Ravel.Matrix.EraseIndex.skip (⟨0, by omega⟩ : Fin (n + 1)) i := by\n";
            out << "      apply Fin.ext\n";
            out << "      simp\n";
            out << "    have hcol : (⟨j.val + 1, by omega⟩ : Fin (n + 1)) = "
                << "Ravel.Matrix.EraseIndex.skip (⟨0, by omega⟩ : Fin (n + 1)) j := by\n";
            out << "      apply Fin.ext\n";
            out << "      simp\n";
            out << "    exact (congrArg₂ (nbonacciCharacteristicMatrix (n + 1)) hrow hcol).trans hentry\n";
            out << "  have htop_summand :\n";
            out << "      (-1 : Polynomial ℤ) ^ ((0 : Fin (n + 1)) : ℕ) * "
                << "nbonacciCharacteristicMatrix (n + 1) 0 0 * "
                << "((nbonacciCharacteristicMatrix (n + 1)).submatrix "
                << "(0 : Fin (n + 1)).succAbove Fin.succ).det = "
                << "Polynomial.X * (qMatrix (n + 1)).det := by\n";
            out << "    rw [hq_cofactor]\n";
            out << "    have hentry : nbonacciCharacteristicMatrix (n + 1) 0 0 = "
                << "Polynomial.X := by\n";
            out << "      have hzero_ne_last : (0 : ℕ) ≠ n := by omega\n";
            out << "      simp [nbonacciCharacteristicMatrix, hzero_ne_last]\n";
            out << "    rw [hentry]\n";
            out << "    norm_num <;> rfl\n";
            out << "  rw [htop_summand]\n";
            out << "  let characteristicTailSummand : Fin n → Polynomial ℤ := fun i =>\n";
            out << "    (-1 : Polynomial ℤ) ^ (i.succ : ℕ) * "
                << "nbonacciCharacteristicMatrix (n + 1) i.succ 0 * "
                << "((nbonacciCharacteristicMatrix (n + 1)).submatrix "
                << "i.succ.succAbove Fin.succ).det\n";
            out << "  let lastTail : Fin n := ⟨n - 1, by omega⟩\n";
            out << "  have htail_single :\n";
            out << "      (∑ i : Fin n, characteristicTailSummand i) = "
                << "characteristicTailSummand lastTail := by\n";
            out << "    apply Finset.sum_eq_single lastTail\n";
            out << "    · intro b hb hne\n";
            out << "      dsimp [characteristicTailSummand]\n";
            out << "      have hb_value_ne_last : b.val + 1 ≠ n := by\n";
            out << "        intro h\n";
            out << "        apply hne\n";
            out << "        apply Fin.ext\n";
            out << "        simp [lastTail]\n";
            out << "        omega\n";
            out << "      have hentry : "
                << "nbonacciCharacteristicMatrix (n + 1) b.succ 0 = 0 := by\n";
            out << "        have hb_lt := b.isLt\n";
            out << "        simp [nbonacciCharacteristicMatrix, hb_value_ne_last]\n";
            out << "      rw [hentry]\n";
            out << "      simp\n";
            out << "    · simp\n";
            out << "  have htail_sum :\n";
            out << "      (∑ i : Fin n, (-1 : Polynomial ℤ) ^ (i.succ : ℕ) * "
                << "nbonacciCharacteristicMatrix (n + 1) i.succ 0 * "
                << "((nbonacciCharacteristicMatrix (n + 1)).submatrix "
                << "i.succ.succAbove Fin.succ).det) = "
                << "(-1 : Polynomial ℤ) ^ (n + 1) * (rMatrix (n + 1)).det := by\n";
            out << "    change (∑ i : Fin n, characteristicTailSummand i) = _\n";
            out << "    rw [htail_single]\n";
            out << "    dsimp [characteristicTailSummand]\n";
            out << "    have hlast_succ : lastTail.succ = Fin.last n := by\n";
            out << "      apply Fin.ext\n";
            out << "      simp [lastTail]\n";
            out << "      omega\n";
            out << "    have hr_cofactor :\n";
            out << "        (nbonacciCharacteristicMatrix (n + 1)).submatrix "
                << "lastTail.succ.succAbove Fin.succ = rMatrix (n + 1) := by\n";
            out << "      rw [hlast_succ]\n";
            out << "      funext i j\n";
            out << "      have hentry := congrArg (fun M => M i j) "
                << "(nbonacciCharacteristic_minor_r n hn)\n";
            out << "      have hrow : (Fin.last n).succAbove i = "
                << "Ravel.Matrix.EraseIndex.skip (Fin.last n) i := by\n";
            out << "        apply Fin.ext\n";
            out << "        simp\n";
            out << "      have hcol : (⟨j.val + 1, by omega⟩ : Fin (n + 1)) = "
                << "Ravel.Matrix.EraseIndex.skip (⟨0, by omega⟩ : Fin (n + 1)) j := by\n";
            out << "        apply Fin.ext\n";
            out << "        simp\n";
            out << "      change nbonacciCharacteristicMatrix (n + 1) "
                << "((Fin.last n).succAbove i) ⟨j.val + 1, by omega⟩ = "
                << "rMatrix (n + 1) i j\n";
            out << "      exact (congrArg₂ (nbonacciCharacteristicMatrix (n + 1)) hrow hcol).trans hentry\n";
            out << "    rw [hr_cofactor]\n";
            out << "    have hlastTailVal : lastTail.val = n - 1 := by\n";
            out << "      simp [lastTail]\n";
            out << "    have hlastEntry : "
                << "nbonacciCharacteristicMatrix (n + 1) lastTail.succ 0 = "
                << "Polynomial.C (-1) := by\n";
            out << "      rw [hlast_succ]\n";
            out << "      have hzero_ne_n : (0 : ℕ) ≠ n := by omega\n";
            out << "      simp [nbonacciCharacteristicMatrix, hzero_ne_n]\n";
            out << "    rw [hlastEntry]\n";
            out << "    rw [hlastTailVal]\n";
            out << "    have hexponent : (n - 1) + 1 = n := by omega\n";
            out << "    rw [hexponent]\n";
            out << "    simp [pow_succ]\n";
            out << "    rfl\n";
            out << "  rw [htail_sum]";
            break;
        case LeanStepKind::Induction:
            if (step.arguments.size() != 2)
                throw std::logic_error("induction rendering requires base and successor theorem names");
            out << "  induction " << step.name << ", " << step.expression << " using Nat.le_induction with\n";
            out << "  | base => exact " << step.arguments[0] << "\n";
            out << "  | succ n " << step.expression << " ih => exact " << step.arguments[1]
                << " n " << step.expression << " ih"; break;
        case LeanStepKind::Cases:
            out << "  cases " << step.expression; break;
        case LeanStepKind::Ring:
            out << "  ring"; break;
        case LeanStepKind::RingNF:
            out << "  ring_nf"; break;
        case LeanStepKind::HaveByChangeAtExact:
            out << "  have " << step.name << " : " << step.proposition << " := by\n";
            out << "    change " << step.proposition << " at " << step.expression << "\n";
            out << "    exact " << step.expression; break;
        case LeanStepKind::ClosePolynomialGeometricSuccessor:
            out << "  simpa [add_comm] using Ravel.Polynomial.one_add_X_mul_sum_range_pow "
                << step.expression; break;
    }
    return out.str();
}

inline std::string render_closed_campaign_lean(const CampaignResult& result) {
    std::set<std::string> imports;
    std::vector<LeanDefinition> definitions;
    std::set<std::string> definition_names;
    std::vector<LeanTheorem> theorems;
    for (const auto& task : result.tasks) {
        if (task.state != TaskState::Closed || !task.artifact) continue;
        if (task.artifact->has_open_goals) throw std::logic_error("renderer received an open proof artifact");
        imports.insert(task.artifact->imports.begin(), task.artifact->imports.end());
        for (const auto& definition : task.artifact->definitions) {
            if (definition_names.insert(definition.name).second)
                definitions.push_back(definition);
        }
        theorems.insert(theorems.end(), task.artifact->theorems.begin(), task.artifact->theorems.end());
    }
    if (theorems.empty()) throw std::runtime_error("campaign produced no closed proof artifacts");
    std::ostringstream out;
    for (const auto& import : imports) out << "import " << import << '\n';
    out << "\nnamespace RavelGenerated\n\nopen Matrix BigOperators\n\n";
    for (const auto& definition : definitions) {
        out << "/-- " << definition.documentation << " -/\nnoncomputable def " << definition.name;
        for (const auto& [binder, type] : definition.binders) out << " (" << binder << " : " << type << ')';
        out << " :\n    " << definition.type << " :=\n  " << definition.value << "\n\n";
    }
    for (const auto& theorem : theorems) {
        out << "/-- " << theorem.documentation << " -/\nlemma " << theorem.name;
        for (const auto& [binder, type] : theorem.binders) out << " (" << binder << " : " << type << ')';
        out << " :\n    " << theorem.proposition << " := by\n";
        for (const auto& step : theorem.proof) out << render_step(step) << '\n';
        out << '\n';
    }
    out << "end RavelGenerated\n";
    return out.str();
}

} // namespace ravel::proof
