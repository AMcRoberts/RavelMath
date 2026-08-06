#pragma once

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace ravel::proof {

struct CyclicSignRun {
    int sign = 0;
    std::size_t length = 0;

    friend bool operator==(const CyclicSignRun&, const CyclicSignRun&) = default;
    friend bool operator<(const CyclicSignRun& lhs, const CyclicSignRun& rhs) {
        return std::pair{lhs.sign, lhs.length} < std::pair{rhs.sign, rhs.length};
    }
};

struct FaceRelativeSignGrammar {
    std::size_t dimension = 0;
    std::size_t active_face = 0;
    std::size_t target_face_offset = 0;
    std::size_t support = 0;
    std::vector<CyclicSignRun> runs;
    bool sign_canonical = false;
    bool replayed = false;

    friend bool operator==(const FaceRelativeSignGrammar&,
                           const FaceRelativeSignGrammar&) = default;
    friend bool operator<(const FaceRelativeSignGrammar& lhs,
                          const FaceRelativeSignGrammar& rhs) {
        return std::tie(lhs.dimension, lhs.active_face,
                        lhs.target_face_offset, lhs.support, lhs.runs,
                        lhs.sign_canonical) <
               std::tie(rhs.dimension, rhs.active_face,
                        rhs.target_face_offset, rhs.support, rhs.runs,
                        rhs.sign_canonical);
    }
};

inline int sign_of(long long value) {
    return value < 0 ? -1 : (value > 0 ? 1 : 0);
}

inline std::vector<int> rotate_signs_from_face(
    const std::vector<long long>& coordinates,
    std::size_t active_face) {
    if (coordinates.empty() || active_face >= coordinates.size())
        throw std::invalid_argument("face-relative grammar: malformed face");
    std::vector<int> result(coordinates.size());
    for (std::size_t k = 0; k < coordinates.size(); ++k)
        result[k] = sign_of(coordinates[(active_face + k) % coordinates.size()]);
    return result;
}

inline std::vector<CyclicSignRun> run_encode(const std::vector<int>& word) {
    if (word.empty()) return {};
    std::vector<CyclicSignRun> runs;
    runs.push_back({word.front(), 1});
    for (std::size_t k = 1; k < word.size(); ++k) {
        if (word[k] == runs.back().sign) ++runs.back().length;
        else runs.push_back({word[k], 1});
    }
    return runs;
}

inline std::vector<int> run_decode(const std::vector<CyclicSignRun>& runs) {
    std::vector<int> result;
    for (const auto& run : runs) {
        if (run.length == 0 || run.sign < -1 || run.sign > 1)
            throw std::invalid_argument("face-relative grammar: malformed run");
        result.insert(result.end(), run.length, run.sign);
    }
    return result;
}

inline std::vector<CyclicSignRun> negate_runs(
    const std::vector<CyclicSignRun>& runs) {
    auto result = runs;
    for (auto& run : result) run.sign = -run.sign;
    return result;
}

inline FaceRelativeSignGrammar derive_face_relative_sign_grammar(
    const std::vector<long long>& coordinates,
    std::size_t active_face,
    std::size_t target_face,
    bool canonicalize_sign = false) {
    if (coordinates.empty() || active_face >= coordinates.size() ||
        target_face >= coordinates.size())
        throw std::invalid_argument("face-relative grammar: malformed input");
    FaceRelativeSignGrammar out;
    out.dimension = coordinates.size();
    out.active_face = active_face;
    out.target_face_offset =
        (target_face + coordinates.size() - active_face) % coordinates.size();
    for (const auto value : coordinates) out.support += value != 0;
    const auto rotated = rotate_signs_from_face(coordinates, active_face);
    out.runs = run_encode(rotated);
    if (canonicalize_sign) {
        const auto negated = negate_runs(out.runs);
        if (negated < out.runs) out.runs = negated;
        out.sign_canonical = true;
    }
    const auto decoded = run_decode(out.runs);
    if (!canonicalize_sign) {
        out.replayed = decoded == rotated;
    } else {
        auto negated = rotated;
        for (auto& sign : negated) sign = -sign;
        out.replayed = decoded == rotated || decoded == negated;
    }
    return out;
}

inline std::string render_face_relative_sign_grammar_lean() {
    return R"LEAN(import Mathlib

namespace RavelPlayground

inductive Sign
  | neg | zero | pos
  deriving DecidableEq

structure SignRun where
  sign : Sign
  length : Nat
  positive : 0 < length

/-- A face-relative run grammar is dimension-parametric: it stores intervals
    of constant sign measured cyclically from the active face, rather than one
    proposition for every absolute coordinate. -/
structure FaceRelativeGrammar where
  dimension : Nat
  activeFace : Fin dimension
  targetOffset : Fin dimension
  runs : List SignRun
  totalLength : (runs.map SignRun.length).sum = dimension

/-- Decode the signed intervals represented by a grammar. -/
def decodeRuns : List SignRun → List Sign
  | [] => []
  | r :: rs => List.replicate r.length r.sign ++ decodeRuns rs

@[simp] theorem decodeRuns_nil : decodeRuns [] = [] := rfl

@[simp] theorem decodeRuns_cons (r : SignRun) (rs : List SignRun) :
    decodeRuns (r :: rs) =
      List.replicate r.length r.sign ++ decodeRuns rs := rfl

/-- Strict Bellman separation on face-relative interval roles lifts to every
    concrete competitor/core state pair. -/
theorem strict_concrete_of_interval_bellman
    {S T R : Type}
    (competitorRole : S → R) (coreRole : T → R)
    (competitorWalks : S → Nat) (coreIncomingWalks : T → Nat)
    (upper lower : R → Nat)
    (hupper : ∀ s, competitorWalks s ≤ upper (competitorRole s))
    (hlower : ∀ t, lower (coreRole t) ≤ coreIncomingWalks t)
    (hstrict : ∀ r q, upper r < lower q) :
    ∀ s t, competitorWalks s < coreIncomingWalks t := by
  intro s t
  exact lt_of_le_of_lt (hupper s)
    (lt_of_lt_of_le (hstrict (competitorRole s) (coreRole t)) (hlower t))

end RavelPlayground
)LEAN";
}

} // namespace ravel::proof