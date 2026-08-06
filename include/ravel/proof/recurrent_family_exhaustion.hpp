#pragma once

#include <cstddef>
#include <sstream>
#include <string>
#include <vector>

namespace ravel::proof {

enum class RecurrentFamilyKind {
    predicted_core,
    transported_competitor,
    lower_grade_descent,
    terminal_permutation,
    mixed_grade_weighted,
    unclassified
};

inline const char* recurrent_family_kind_name(RecurrentFamilyKind kind) {
    switch (kind) {
        case RecurrentFamilyKind::predicted_core: return "predicted_core";
        case RecurrentFamilyKind::transported_competitor: return "transported_competitor";
        case RecurrentFamilyKind::lower_grade_descent: return "lower_grade_descent";
        case RecurrentFamilyKind::terminal_permutation: return "terminal_permutation";
        case RecurrentFamilyKind::mixed_grade_weighted: return "mixed_grade_weighted";
        case RecurrentFamilyKind::unclassified: return "unclassified";
    }
    return "unclassified";
}

// One independently extracted recurrent SCC and the evidence explaining why it
// belongs to a certified structural family.  The classifier does not infer a
// family from spectral data: every flag must be produced by graph/arithmetic
// proof operations.
struct RecurrentFamilyWitness {
    std::size_t component = 0;
    std::size_t vertices = 0;
    std::size_t minimum_grade = 0;
    std::size_t maximum_grade = 0;
    RecurrentFamilyKind kind = RecurrentFamilyKind::unclassified;
    bool recurrent = false;
    bool family_predicate_replayed = false;
    bool transition_closed = false;
    bool no_return_after_exit = false;
    bool dominance_available = false;
    std::string explanation;
};

struct RecurrentFamilyExhaustionProof {
    std::size_t dimension = 0;
    std::size_t recurrent_components = 0;
    std::size_t classified_components = 0;
    std::size_t classified_vertices = 0;
    std::size_t transported_families = 0;
    bool extraction_complete = false;
    bool classes_disjoint = false;
    bool all_recurrent_classified = false;
    bool all_families_closed_or_no_return = false;
    bool all_competitors_dominated = false;
    bool proved = false;
    std::vector<RecurrentFamilyWitness> witnesses;
    std::string obstruction;
};

inline RecurrentFamilyExhaustionProof derive_recurrent_family_exhaustion(
    std::size_t dimension,
    std::size_t independently_extracted_recurrent_components,
    bool extraction_complete,
    bool classes_disjoint,
    std::vector<RecurrentFamilyWitness> witnesses) {
    RecurrentFamilyExhaustionProof out;
    out.dimension = dimension;
    out.recurrent_components = independently_extracted_recurrent_components;
    out.extraction_complete = extraction_complete;
    out.classes_disjoint = classes_disjoint;
    out.witnesses = std::move(witnesses);

    out.all_recurrent_classified = out.witnesses.size() == out.recurrent_components;
    out.all_families_closed_or_no_return = true;
    out.all_competitors_dominated = true;

    for (const auto& witness : out.witnesses) {
        if (!witness.recurrent ||
            witness.kind == RecurrentFamilyKind::unclassified ||
            !witness.family_predicate_replayed) {
            out.all_recurrent_classified = false;
            continue;
        }
        ++out.classified_components;
        out.classified_vertices += witness.vertices;
        if (witness.kind == RecurrentFamilyKind::transported_competitor)
            ++out.transported_families;
        if (!(witness.transition_closed || witness.no_return_after_exit))
            out.all_families_closed_or_no_return = false;
        if (witness.kind != RecurrentFamilyKind::predicted_core &&
            !witness.dominance_available)
            out.all_competitors_dominated = false;
    }

    if (!out.extraction_complete)
        out.obstruction = "independent recurrent-SCC extraction is incomplete";
    else if (!out.classes_disjoint)
        out.obstruction = "structural family predicates overlap";
    else if (!out.all_recurrent_classified)
        out.obstruction = "at least one recurrent SCC has no replayed family witness";
    else if (!out.all_families_closed_or_no_return)
        out.obstruction = "a classified family lacks closure or no-return evidence";
    else if (!out.all_competitors_dominated)
        out.obstruction = "a classified non-core family lacks a dominance theorem";

    out.proved = out.extraction_complete && out.classes_disjoint &&
        out.all_recurrent_classified && out.all_families_closed_or_no_return &&
        out.all_competitors_dominated;
    return out;
}

inline std::string render_recurrent_family_exhaustion_report(
    const RecurrentFamilyExhaustionProof& proof) {
    std::ostringstream out;
    out << "RECURRENT_FAMILY_EXHAUSTION\n";
    out << "dimension=" << proof.dimension << "\n";
    out << "recurrent_components=" << proof.recurrent_components << "\n";
    out << "classified_components=" << proof.classified_components << "\n";
    out << "classified_vertices=" << proof.classified_vertices << "\n";
    out << "transported_families=" << proof.transported_families << "\n";
    out << "extraction_complete=" << (proof.extraction_complete ? "true" : "false") << "\n";
    out << "classes_disjoint=" << (proof.classes_disjoint ? "true" : "false") << "\n";
    out << "all_recurrent_classified=" << (proof.all_recurrent_classified ? "true" : "false") << "\n";
    out << "all_families_closed_or_no_return="
        << (proof.all_families_closed_or_no_return ? "true" : "false") << "\n";
    out << "all_competitors_dominated="
        << (proof.all_competitors_dominated ? "true" : "false") << "\n";
    for (const auto& witness : proof.witnesses) {
        out << "family component=" << witness.component
            << " vertices=" << witness.vertices
            << " grade=" << witness.minimum_grade << ".." << witness.maximum_grade
            << " kind=" << recurrent_family_kind_name(witness.kind)
            << " closure=" << (witness.transition_closed ? "closed" :
                (witness.no_return_after_exit ? "no_return" : "missing"))
            << " dominance=" << (witness.dominance_available ? "available" : "missing");
        if (!witness.explanation.empty()) out << " reason=" << witness.explanation;
        out << "\n";
    }
    out << "proved=" << (proof.proved ? "true" : "false") << "\n";
    if (!proof.obstruction.empty()) out << "obstruction=" << proof.obstruction << "\n";
    return out.str();
}

inline std::string render_recurrent_family_exhaustion_lean() {
    return R"LEAN(import Mathlib

namespace RavelPlayground

/-- A finite recurrent-family classifier is exhaustive when every independently
    extracted recurrent component has exactly one certified family witness. -/
theorem recurrent_family_exhaustion
    {Component Family : Type}
    (recurrent : Component → Prop)
    (belongs : Component → Family → Prop)
    (classified : ∀ c, recurrent c → ∃! f, belongs c f) :
    ∀ c, recurrent c → ∃ f, belongs c f := by
  intro c hc
  exact (classified c hc).exists

/-- Once every non-core recurrent family carries strict dominance, exhaustion
    promotes the family-local result to all recurrent competitors. -/
theorem exhaustion_promotes_dominance
    {Component Family : Type}
    (recurrent core : Component → Prop)
    (belongs : Component → Family → Prop)
    (dominatedFamily : Family → Prop)
    (classified : ∀ c, recurrent c → ¬ core c → ∃ f, belongs c f)
    (familyDominance : ∀ c f, belongs c f → dominatedFamily f) :
    ∀ c, recurrent c → ¬ core c → ∃ f, belongs c f ∧ dominatedFamily f := by
  intro c hc hcore
  obtain ⟨f, hf⟩ := classified c hc hcore
  exact ⟨f, hf, familyDominance c f hf⟩

end RavelPlayground
)LEAN";
}

} // namespace ravel::proof

namespace ravel::proof {

// Inductive promotion boundary.  This is intentionally separate from the
// finite classifier: it records exactly what must be proved uniformly before
// bounded family exhaustion becomes an all-dimensions theorem.
struct InductiveFamilyExhaustionProof {
    std::size_t lower_dimension = 0;
    bool lower_exhaustion = false;
    bool transported_successors_total = false;
    bool new_terminal_families_classified = false;
    bool rejected_boundary_no_return = false;
    bool family_predicates_disjoint = false;
    bool proved = false;
    std::string obstruction;
};

inline InductiveFamilyExhaustionProof derive_inductive_family_exhaustion(
    std::size_t lower_dimension,
    bool lower_exhaustion,
    bool transported_successors_total,
    bool new_terminal_families_classified,
    bool rejected_boundary_no_return,
    bool family_predicates_disjoint) {
    InductiveFamilyExhaustionProof out;
    out.lower_dimension = lower_dimension;
    out.lower_exhaustion = lower_exhaustion;
    out.transported_successors_total = transported_successors_total;
    out.new_terminal_families_classified = new_terminal_families_classified;
    out.rejected_boundary_no_return = rejected_boundary_no_return;
    out.family_predicates_disjoint = family_predicates_disjoint;
    if (!lower_exhaustion)
        out.obstruction = "lower-dimensional recurrent families are not exhausted";
    else if (!transported_successors_total)
        out.obstruction = "a nonterminal lower family lacks a certified upper successor";
    else if (!new_terminal_families_classified)
        out.obstruction = "new upper terminal families are not uniformly classified";
    else if (!rejected_boundary_no_return)
        out.obstruction = "the rejected upper corona boundary lacks a no-return proof";
    else if (!family_predicates_disjoint)
        out.obstruction = "upper family predicates are not pairwise disjoint";
    out.proved = lower_exhaustion && transported_successors_total &&
        new_terminal_families_classified && rejected_boundary_no_return &&
        family_predicates_disjoint;
    return out;
}

inline std::string render_inductive_family_exhaustion_report(
    const InductiveFamilyExhaustionProof& proof) {
    std::ostringstream out;
    out << "INDUCTIVE_FAMILY_EXHAUSTION\n";
    out << "dimensions=" << proof.lower_dimension << "->"
        << proof.lower_dimension + 1 << "\n";
    out << "lower_exhaustion=" << (proof.lower_exhaustion ? "true" : "false") << "\n";
    out << "transported_successors_total="
        << (proof.transported_successors_total ? "true" : "false") << "\n";
    out << "new_terminal_families_classified="
        << (proof.new_terminal_families_classified ? "true" : "false") << "\n";
    out << "rejected_boundary_no_return="
        << (proof.rejected_boundary_no_return ? "true" : "false") << "\n";
    out << "family_predicates_disjoint="
        << (proof.family_predicates_disjoint ? "true" : "false") << "\n";
    out << "proved=" << (proof.proved ? "true" : "false") << "\n";
    if (!proof.obstruction.empty()) out << "obstruction=" << proof.obstruction << "\n";
    return out.str();
}

} // namespace ravel::proof
