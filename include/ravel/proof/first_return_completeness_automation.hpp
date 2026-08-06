#pragma once

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <map>
#include <optional>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "ravel/proof/symbolic_residual_induction.hpp"

namespace ravel::proof {

/*
 * A theorem-search layer, not a regression harness.
 *
 * Concrete residual algebras are observations used to discover a symbolic
 * quotient.  The accepted endpoint is a dimension-parametric collection of
 * predicates with proved predecessor equations.  Until that endpoint is
 * reached the machine returns a replayable mathematical obstruction, never a
 * successful finite certificate masquerading as the theorem.
 */

enum class ResidualAtomKind : std::uint8_t {
    coordinate_eq,
    center_eq,
    positive_count_eq,
    negative_count_eq,
    zero_count_eq,
    boundary_count_eq
};

struct ResidualAtom {
    ResidualAtomKind kind{};
    std::int64_t a = 0;
    std::int64_t b = 0;

    friend bool operator<(const ResidualAtom& x, const ResidualAtom& y) {
        return std::tie(x.kind, x.a, x.b) < std::tie(y.kind, y.a, y.b);
    }
    friend bool operator==(const ResidualAtom&, const ResidualAtom&) = default;
};

inline bool evaluate_atom(const ResidualAtom& atom,
                          const SymbolicControllerState& state) {
    switch (atom.kind) {
    case ResidualAtomKind::coordinate_eq:
        return atom.a >= 0 && static_cast<std::size_t>(atom.a) < state.size() &&
               state[static_cast<std::size_t>(atom.a)] == atom.b;
    case ResidualAtomKind::center_eq:
        return controller_center(state) == atom.a;
    case ResidualAtomKind::positive_count_eq:
        return std::count(state.begin(), state.end(), 1) == atom.a;
    case ResidualAtomKind::negative_count_eq:
        return std::count(state.begin(), state.end(), -1) == atom.a;
    case ResidualAtomKind::zero_count_eq:
        return std::count(state.begin(), state.end(), 0) == atom.a;
    case ResidualAtomKind::boundary_count_eq:
        return std::count_if(state.begin(), state.end(), [](auto x) {
                   return x == -1 || x == 1;
               }) == atom.a;
    }
    return false;
}

struct ResidualFormula {
    // Exact DNF over the available atom vocabulary.  Each cube stores signed
    // atom indices: +(i+1) means atom i, -(i+1) means its negation.
    std::vector<std::vector<std::int32_t>> cubes;
    bool exact = false;
};

struct FormulaCounterexample {
    std::size_t dimension = 0;
    std::size_t residual = 0;
    std::size_t state = 0;
    bool expected = false;
    bool observed = false;
    SymbolicControllerState concrete_state;
    std::string reason;
};

struct TransitionCounterexample {
    std::size_t dimension = 0;
    std::size_t left_residual = 0;
    std::size_t right_residual = 0;
    std::int64_t digit = 0;
    std::size_t left_target = 0;
    std::size_t right_target = 0;
    std::string reason;
};

struct DimensionModel {
    ResidualInductionProof exact;
    std::vector<ResidualFormula> formulas;
};

struct FirstReturnCompletenessAutomationResult {
    std::size_t min_dimension = 0;
    std::size_t max_dimension = 0;
    std::size_t refinement_rounds = 0;
    std::vector<ResidualAtom> atoms;
    std::vector<DimensionModel> models;
    std::optional<FormulaCounterexample> formula_counterexample;
    std::optional<TransitionCounterexample> transition_counterexample;
    bool fixed_dimension_formulas_exact = false;
    bool predecessor_closed = false;
    bool dimension_parametric = false;
    bool universal_theorem_emitted = false;
    std::string obstruction;
};

inline std::vector<ResidualAtom> base_atom_vocabulary(std::size_t max_n) {
    std::vector<ResidualAtom> atoms;
    for (std::size_t i = 0; i < max_n; ++i)
        for (const auto v : {-1LL, 0LL, 1LL})
            atoms.push_back({ResidualAtomKind::coordinate_eq,
                             static_cast<std::int64_t>(i), v});
    for (std::int64_t v = -static_cast<std::int64_t>(max_n);
         v <= static_cast<std::int64_t>(max_n); ++v)
        atoms.push_back({ResidualAtomKind::center_eq, v, 0});
    for (std::int64_t k = 0; k <= static_cast<std::int64_t>(max_n); ++k) {
        atoms.push_back({ResidualAtomKind::positive_count_eq, k, 0});
        atoms.push_back({ResidualAtomKind::negative_count_eq, k, 0});
        atoms.push_back({ResidualAtomKind::zero_count_eq, k, 0});
        atoms.push_back({ResidualAtomKind::boundary_count_eq, k, 0});
    }
    return atoms;
}

inline std::vector<bool> atom_signature(
    const std::vector<ResidualAtom>& atoms,
    const SymbolicControllerState& state) {
    std::vector<bool> result;
    result.reserve(atoms.size());
    for (const auto& atom : atoms) result.push_back(evaluate_atom(atom, state));
    return result;
}

/* Learn an exact finite DNF by grouping states by their full atom signature.
 * A mixed signature is an exact counterexample showing the current symbolic
 * vocabulary cannot express the residual. */
inline std::pair<ResidualFormula, std::optional<FormulaCounterexample>>
learn_exact_formula(const ResidualInductionProof& proof,
                    std::size_t residual,
                    const std::vector<ResidualAtom>& atoms) {
    const auto representative = proof.quotient_members.at(residual).front();
    const auto& bits = proof.concrete_family.members.at(representative);
    std::map<std::vector<bool>, std::pair<bool, std::size_t>> classes;
    for (std::size_t s = 0; s < proof.states.size(); ++s) {
        const auto sig = atom_signature(atoms, proof.states[s]);
        auto [it, inserted] = classes.emplace(sig, std::pair{bits[s], s});
        if (!inserted && it->second.first != bits[s]) {
            FormulaCounterexample ce;
            ce.dimension = proof.dimension;
            ce.residual = residual;
            ce.state = s;
            ce.expected = bits[s];
            ce.observed = it->second.first;
            ce.concrete_state = proof.states[s];
            ce.reason = "two states have the same symbolic signature but different residual membership";
            return {{}, ce};
        }
    }

    ResidualFormula f;
    for (const auto& [sig, label_and_state] : classes) {
        if (!label_and_state.first) continue;
        std::vector<std::int32_t> cube;
        cube.reserve(sig.size());
        for (std::size_t i = 0; i < sig.size(); ++i)
            cube.push_back(sig[i] ? static_cast<std::int32_t>(i + 1)
                                  : -static_cast<std::int32_t>(i + 1));
        f.cubes.push_back(std::move(cube));
    }
    f.exact = true;
    return {std::move(f), std::nullopt};
}

inline std::string state_string(const SymbolicControllerState& state) {
    std::ostringstream o;
    o << '[';
    for (std::size_t i = 0; i < state.size(); ++i) {
        if (i) o << ',';
        o << state[i];
    }
    return o.str() + ']';
}

inline FirstReturnCompletenessAutomationResult
run_first_return_completeness_automation(std::size_t min_n,
                                         std::size_t max_n,
                                         std::size_t max_rounds = 32) {
    if (min_n < 1 || max_n < min_n)
        throw std::invalid_argument("invalid dimension interval");

    FirstReturnCompletenessAutomationResult result;
    result.min_dimension = min_n;
    result.max_dimension = max_n;
    result.atoms = base_atom_vocabulary(max_n);

    for (std::size_t n = min_n; n <= max_n; ++n) {
        DimensionModel model;
        model.exact = derive_residual_induction_proof(n);
        if (!model.exact.valid) {
            result.obstruction = "exact residual algebra failed at n=" +
                                 std::to_string(n) + ": " + model.exact.failure;
            return result;
        }
        result.models.push_back(std::move(model));
    }

    // Refinement is deliberately monotone.  The current atom generator begins
    // with every coordinate/value atom, so a finite model is always separable;
    // later exact-algebra predicates can be inserted here from minimized
    // counterexamples without changing the theorem-search contract.
    for (std::size_t round = 0; round < max_rounds; ++round) {
        result.refinement_rounds = round + 1;
        bool all_exact = true;
        for (auto& model : result.models) {
            model.formulas.clear();
            model.formulas.reserve(model.exact.quotient_members.size());
            for (std::size_t q = 0; q < model.exact.quotient_members.size(); ++q) {
                auto [formula, ce] =
                    learn_exact_formula(model.exact, q, result.atoms);
                if (ce) {
                    result.formula_counterexample = *ce;
                    all_exact = false;
                    break;
                }
                model.formulas.push_back(std::move(formula));
            }
            if (!all_exact) break;
        }
        if (all_exact) {
            result.fixed_dimension_formulas_exact = true;
            break;
        }

        // A future algebraic separator synthesizer is called here.  Refuse to
        // invent a separator when the current exact vocabulary cannot do so.
        result.obstruction = "symbolic vocabulary collision requires a new exact separator";
        return result;
    }

    if (!result.fixed_dimension_formulas_exact) {
        result.obstruction = "refinement budget exhausted";
        return result;
    }

    // Each model is already an exact predecessor algebra.  Formula exactness
    // transports that closure to the learned finite formulas.
    result.predecessor_closed = std::all_of(
        result.models.begin(), result.models.end(), [](const DimensionModel& m) {
            return m.exact.predecessor_congruent;
        });

    // Critical soundness boundary: formulas containing concrete coordinate
    // numbers or a number of cubes that changes with n are not a parametric
    // theorem.  The machine reports this instead of emitting a fake universal
    // result.  The next refinement layer must replace coordinates by roles and
    // derive a finite schema stable under n -> n+1.
    result.dimension_parametric = false;
    result.obstruction =
        "exact fixed-dimensional formulas were derived, but no proved role-based "
        "n->n+1 generalization has yet replaced concrete coordinate atoms";
    return result;
}

inline std::string render_automation_report(
    const FirstReturnCompletenessAutomationResult& r) {
    std::ostringstream o;
    o << "FIRST_RETURN_COMPLETENESS_AUTOMATION\n";
    o << "dimensions=" << r.min_dimension << ".." << r.max_dimension << "\n";
    o << "refinement_rounds=" << r.refinement_rounds << "\n";
    o << "atoms=" << r.atoms.size() << "\n";
    for (const auto& m : r.models) {
        o << "n=" << m.exact.dimension
          << " states=" << m.exact.states.size()
          << " residuals=" << m.exact.quotient_members.size()
          << " formulas=" << m.formulas.size() << "\n";
    }
    o << "fixed_dimension_formulas_exact="
      << (r.fixed_dimension_formulas_exact ? "true" : "false") << "\n";
    o << "predecessor_closed=" << (r.predecessor_closed ? "true" : "false") << "\n";
    o << "dimension_parametric=" << (r.dimension_parametric ? "true" : "false") << "\n";
    o << "universal_theorem_emitted="
      << (r.universal_theorem_emitted ? "true" : "false") << "\n";
    if (r.formula_counterexample) {
        const auto& c = *r.formula_counterexample;
        o << "formula_counterexample=n" << c.dimension << ":q" << c.residual
          << ":state" << c.state << ':' << state_string(c.concrete_state)
          << ':' << c.reason << "\n";
    }
    if (r.transition_counterexample) {
        const auto& c = *r.transition_counterexample;
        o << "transition_counterexample=n" << c.dimension << ":q"
          << c.left_residual << ",q" << c.right_residual << ":digit"
          << c.digit << ":" << c.reason << "\n";
    }
    o << "obstruction=" << r.obstruction << "\n";
    return o.str();
}

inline void write_automation_checkpoint(
    const FirstReturnCompletenessAutomationResult& r,
    const std::filesystem::path& path) {
    std::filesystem::create_directories(path.parent_path());
    std::ofstream out(path);
    if (!out) throw std::runtime_error("cannot write automation checkpoint");
    out << render_automation_report(r);
}

} // namespace ravel::proof
