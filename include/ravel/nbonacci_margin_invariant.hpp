// nbonacci_margin_invariant.hpp
//
// Dimension-parametric label and endpoint grammar for the dominant
// n-bonacci boundary-core margin certificate.
//
// This header deliberately contains no corona construction and no floating
// arithmetic.  It generates the candidate label pairs, their abstract
// parent transitions, and the sparse lattice vectors predicted to attain the
// signed height endpoints.  A core-backed executable can therefore compare
// discovered data against this independent grammar instead of rediscovering
// formulas in the same loop that claims to verify them.
//
// Scope: the grammar is a candidate arbitrary-n catalogue.  Its exact match
// to the computed dominant cores is certified for the enrolled finite range
// by app/item1_per_pair_check.cpp.  This header alone does not prove that the
// generated catalogue is the dominant recurrent core for every n.

#pragma once

#include <compare>
#include <cstddef>
#include <optional>
#include <set>
#include <stdexcept>
#include <tuple>
#include <vector>

namespace ravel::nbonacci_margin {

struct LabelPair {
    std::size_t i = 0;
    std::size_t j = 0;

    auto operator<=>(const LabelPair&) const = default;
};

struct LabelTransition {
    LabelPair source;
    LabelPair destination;
    long long delta = 0;

    auto operator<=>(const LabelTransition&) const = default;
};

struct EndpointWitnesses {
    std::vector<long long> lower;
    std::vector<long long> upper;
};

enum class DisplacementKind {
    Root,
    AlternatingTriple,
};

// Canonical sparse displacement coordinates.  `sign` is the coefficient at
// the first occupied index, so the represented vector is
//
//   sign * (e_a - e_b)                 (Root), or
//   sign * (e_a - e_b + e_c)           (AlternatingTriple),
//
// with a<b<c and sign in {+1,-1}.  This turns the former vector-level update
// experiment into a finite family of symbolic index recurrences.
struct DisplacementDescriptor {
    DisplacementKind kind = DisplacementKind::Root;
    long long sign = 1;
    std::size_t a = 0;
    std::size_t b = 1;
    std::size_t c = 0;  // ignored for roots

    auto operator<=>(const DisplacementDescriptor&) const = default;
};

struct CoreState {
    LabelPair pair;
    DisplacementDescriptor displacement;

    auto operator<=>(const CoreState&) const = default;
};

// Persistent evidence that a displacement was admitted by the canonical
// two-atom request grammar.  The witness is intentionally independent of
// corona/SCC machinery so it can be attached when a candidate is generated,
// carried through projection and Red, and replayed by later proof layers.
struct GradeTwoAtomWitness {
    bool derived = false;
    DisplacementDescriptor left;
    DisplacementDescriptor right;
    std::size_t overlap_size = 0;
    std::size_t cancellation_sites = 0;

    auto operator<=>(const GradeTwoAtomWitness&) const = default;
};

inline std::optional<DisplacementDescriptor> describe_displacement(
    const std::vector<long long>& x) {
    std::vector<std::size_t> support;
    for (std::size_t k = 0; k < x.size(); ++k)
        if (x[k] != 0) support.push_back(k);
    if (support.size() == 2) {
        const auto a = support[0], b = support[1];
        const long long sign = x[a];
        if ((sign == 1 || sign == -1) && x[b] == -sign)
            return DisplacementDescriptor{
                DisplacementKind::Root, sign, a, b, 0};
    }
    if (support.size() == 3) {
        const auto a = support[0], b = support[1], c = support[2];
        const long long sign = x[a];
        if ((sign == 1 || sign == -1) && x[b] == -sign &&
            x[c] == sign)
            return DisplacementDescriptor{
                DisplacementKind::AlternatingTriple, sign, a, b, c};
    }
    return std::nullopt;
}

inline std::vector<long long> displacement_from_descriptor(
    std::size_t n, DisplacementDescriptor descriptor) {
    if (n < 2 || (descriptor.sign != 1 && descriptor.sign != -1) ||
        descriptor.a >= descriptor.b || descriptor.b >= n ||
        (descriptor.kind == DisplacementKind::AlternatingTriple &&
         (descriptor.b >= descriptor.c || descriptor.c >= n)))
        throw std::invalid_argument(
            "displacement_from_descriptor: malformed descriptor");
    std::vector<long long> result(n, 0);
    result[descriptor.a] = descriptor.sign;
    result[descriptor.b] = -descriptor.sign;
    if (descriptor.kind == DisplacementKind::AlternatingTriple)
        result[descriptor.c] = descriptor.sign;
    return result;
}

// Exact symbolic form of the inverse-incidence update restricted to the
// sparse catalogue.  Returning nullopt means the concrete update leaves the
// catalogue.  These four cases are exhaustive:
//
//   root, a>0: delta=0 shifts; delta=sign appends the last coordinate;
//   root, a=0: delta=-sign folds the two survivors into a root;
//   triple, a>0: delta=sign shifts;
//   triple, a=0: delta=-sign folds into a root.
inline std::optional<DisplacementDescriptor> forward_descriptor(
    std::size_t n, DisplacementDescriptor descriptor, long long delta) {
    // Validate once through the canonical constructor.
    (void)displacement_from_descriptor(n, descriptor);
    if (delta < -1 || delta > 1)
        throw std::invalid_argument("forward_descriptor: delta outside {-1,0,1}");
    const std::size_t last = n - 1;
    if (descriptor.kind == DisplacementKind::Root) {
        if (descriptor.a > 0) {
            if (delta == 0)
                return DisplacementDescriptor{
                    DisplacementKind::Root, descriptor.sign,
                    descriptor.a - 1, descriptor.b - 1, 0};
            if (delta == descriptor.sign)
                return DisplacementDescriptor{
                    DisplacementKind::AlternatingTriple, descriptor.sign,
                    descriptor.a - 1, descriptor.b - 1, last};
            return std::nullopt;
        }
        if (delta == -descriptor.sign)
            return DisplacementDescriptor{
                DisplacementKind::Root, -descriptor.sign,
                descriptor.b - 1, last, 0};
        return std::nullopt;
    }
    if (descriptor.a > 0) {
        if (delta == descriptor.sign)
            return DisplacementDescriptor{
                DisplacementKind::AlternatingTriple, descriptor.sign,
                descriptor.a - 1, descriptor.b - 1, descriptor.c - 1};
        return std::nullopt;
    }
    if (delta == -descriptor.sign)
        return DisplacementDescriptor{
            DisplacementKind::Root, -descriptor.sign,
            descriptor.b - 1, descriptor.c - 1, 0};
    return std::nullopt;
}

inline bool supported_pair(std::size_t n, LabelPair pair) {
    if (n < 2 || pair.i >= n || pair.j >= n) return false;
    if (pair.i != pair.j) return true;
    return pair.i + 2 < n;  // diagonals 0,...,n-3 only
}

// Direct symbolic membership predicate for the candidate cyclic core.  It is
// the compressed form of the corona-backed node catalogues:
//
// * diagonal (i,i): both orientations of every root wholly above i;
// * i<j: a negative fan rooted at i reaching b>=j, and a positive fan
//   rooted at j reaching b>j;
// * i>j: the signed mirror of the preceding case;
// * alternating triples occur in those fans exactly when min(i,j)=0.
//
// The free involution (i,x,j) -> (j,-x,i) is manifest in the last two
// branches.  This predicate is purely integral and dimension-parametric; it
// contains no beta, corona, SCC, or floating-point computation.
inline bool predicted_core_member(
    std::size_t n, LabelPair pair, DisplacementDescriptor descriptor) {
    if (!supported_pair(n, pair)) return false;
    (void)displacement_from_descriptor(n, descriptor);
    if (pair.i == pair.j)
        return descriptor.kind == DisplacementKind::Root &&
               descriptor.a > pair.i;
    const bool triple_allowed = pair.i == 0 || pair.j == 0;
    if (descriptor.kind == DisplacementKind::AlternatingTriple &&
        !triple_allowed)
        return false;
    if (pair.i < pair.j) {
        return (descriptor.sign == -1 && descriptor.a == pair.i &&
                descriptor.b >= pair.j) ||
               (descriptor.sign == 1 && descriptor.a == pair.j &&
                descriptor.b > pair.j);
    }
    return (descriptor.sign == 1 && descriptor.a == pair.j &&
            descriptor.b >= pair.i) ||
           (descriptor.sign == -1 && descriptor.a == pair.i &&
            descriptor.b > pair.i);
}

inline bool predicted_core_member(std::size_t n, const CoreState& state) {
    return predicted_core_member(n, state.pair, state.displacement);
}

inline bool predicted_core_member(
    std::size_t n, LabelPair pair, const std::vector<long long>& x) {
    const auto descriptor = describe_displacement(x);
    return descriptor && predicted_core_member(n, pair, *descriptor);
}

inline bool lies_in_previous_alphabet_shadow(
    std::size_t n, const CoreState& state) {
    if (n < 4 || !predicted_core_member(n, state)) return false;
    const std::size_t last = n - 1;
    if (state.pair.i >= last || state.pair.j >= last ||
        state.displacement.a >= last || state.displacement.b >= last)
        return false;
    return state.displacement.kind == DisplacementKind::Root ||
           state.displacement.c < last;
}

// Exact crossing distances for the states whose indices all avoid n-1.  This
// set has the same states as G_(n-1), but NOT its induced edge relation: the
// inverse-incidence wrap still knows the ambient last index.  The distinction
// is deliberate, and guarded by the pure graph test.  `from_previous_shadow`
// and `to_previous_shadow` are verified against independent multi-source BFS.
inline std::size_t predicted_distance_from_previous_shadow(
    std::size_t n, const CoreState& state) {
    if (n < 4 || !predicted_core_member(n, state) ||
        lies_in_previous_alphabet_shadow(n, state))
        throw std::invalid_argument(
            "predicted_distance_from_previous_shadow: state is not new");
    const auto& d = state.displacement;
    if (d.a + 1 == n - 1) return 2;
    if (d.kind == DisplacementKind::AlternatingTriple && d.c == d.b + 1)
        return d.b == d.a + 1 ? 3 : 2;
    return 1;
}

inline std::size_t predicted_distance_to_previous_shadow(
    std::size_t n, const CoreState& state) {
    if (n < 4 || !predicted_core_member(n, state) ||
        lies_in_previous_alphabet_shadow(n, state))
        throw std::invalid_argument(
            "predicted_distance_to_previous_shadow: state is not new");
    if (state.displacement.kind == DisplacementKind::AlternatingTriple)
        return 1;
    const bool exactly_one_zero = (state.pair.i == 0) != (state.pair.j == 0);
    return exactly_one_zero ? 2 : 1;
}

inline std::set<LabelPair> label_pairs(std::size_t n) {
    if (n < 3) throw std::invalid_argument("label_pairs: n must be >= 3");
    std::set<LabelPair> result;
    for (std::size_t i = 0; i < n; ++i)
        for (std::size_t j = 0; j < n; ++j)
            if (supported_pair(n, {i, j})) result.insert({i, j});
    return result;
}

inline std::size_t predicted_pair_count(std::size_t n) {
    if (n < 3) throw std::invalid_argument(
        "predicted_pair_count: n must be >= 3");
    return n * n - 2;
}

// Node count in the cyclic-core stratum over a supported face pair.  The
// three cases were recovered from the independent corona-backed catalogues:
// diagonal triangular layers, the square 0-row/column layers, and odd-width
// interior layers.  Summing them over `label_pairs(n)` gives the core-size
// polynomial below exactly; occurrence/exhaustion is still the theorem seam.
inline std::size_t predicted_pair_node_count(std::size_t n, LabelPair pair) {
    if (!supported_pair(n, pair))
        throw std::invalid_argument(
            "predicted_pair_node_count: unsupported pair");
    if (pair.i == pair.j) {
        const std::size_t width = n - pair.i - 1;
        return width * (width - 1);
    }
    const std::size_t high = pair.i > pair.j ? pair.i : pair.j;
    if (pair.i == 0 || pair.j == 0) {
        const std::size_t width = n - high;
        return width * width;
    }
    return 2 * (n - high) - 1;
}

inline std::size_t predicted_core_size(std::size_t n) {
    if (n < 2) throw std::invalid_argument(
        "predicted_core_size: n must be >= 2");
    return (n - 1) * (5 * n * n - 10 * n + 6) / 3;
}

inline std::size_t predicted_core_edge_count(std::size_t n) {
    if (n < 3) throw std::invalid_argument(
        "predicted_core_edge_count: n must be >= 3");
    return (n - 1) * (11 * n * n - 34 * n + 30) / 3;
}

inline std::set<DisplacementDescriptor> displacement_descriptors(
    std::size_t n) {
    if (n < 2) throw std::invalid_argument(
        "displacement_descriptors: n must be >= 2");
    std::set<DisplacementDescriptor> result;
    for (std::size_t a = 0; a < n; ++a)
        for (std::size_t b = a + 1; b < n; ++b)
            for (long long sign : {-1LL, 1LL})
                result.insert({DisplacementKind::Root, sign, a, b, 0});
    for (std::size_t a = 0; a < n; ++a)
        for (std::size_t b = a + 1; b < n; ++b)
            for (std::size_t c = b + 1; c < n; ++c)
                for (long long sign : {-1LL, 1LL})
                    result.insert({DisplacementKind::AlternatingTriple,
                                   sign, a, b, c});
    return result;
}

// Derive a canonical unordered two-atom decomposition by forced complement.
// Once the left atom is selected, the right atom is exactly target-left and
// must itself be recognized by the canonical sparse descriptor grammar.
inline std::optional<GradeTwoAtomWitness> derive_grade_two_atom_witness(
    const std::vector<long long>& target) {
    const auto descriptors = displacement_descriptors(target.size());
    for (const auto& left : descriptors) {
        const auto lv = displacement_from_descriptor(target.size(), left);
        std::vector<long long> residual(target.size(), 0);
        for (std::size_t k = 0; k < target.size(); ++k)
            residual[k] = target[k] - lv[k];
        const auto right_opt = describe_displacement(residual);
        if (!right_opt || *right_opt < left) continue;
        const auto right = *right_opt;
        const auto rv = displacement_from_descriptor(target.size(), right);
        bool exact = true;
        std::size_t overlap = 0;
        std::size_t cancellations = 0;
        for (std::size_t k = 0; k < target.size(); ++k) {
            exact = exact && lv[k] + rv[k] == target[k];
            if (lv[k] != 0 && rv[k] != 0) {
                ++overlap;
                cancellations += lv[k] == -rv[k];
            }
        }
        if (!exact) continue;
        return GradeTwoAtomWitness{
            true, left, right, overlap, cancellations};
    }
    return std::nullopt;
}

// The displacement alphabet seen in every exact dominant core n=3,...,7:
// all oriented roots e_a-e_b and both alternating signings
// +/- (e_a-e_b+e_c), a<b<c.  This is the first state refinement beyond
// face labels; it retains the correlation that a label-only interval loses.
inline std::set<std::vector<long long>> displacement_catalogue(std::size_t n) {
    std::set<std::vector<long long>> result;
    for (const auto descriptor : displacement_descriptors(n))
        result.insert(displacement_from_descriptor(n, descriptor));
    return result;
}

inline std::size_t predicted_displacement_count(std::size_t n) {
    if (n < 2) throw std::invalid_argument(
        "predicted_displacement_count: n must be >= 2");
    return n * (n - 1) * (n + 1) / 3;
}

// Exact inverse-incidence update.  For n-bonacci, a source displacement x
// and prefix-height difference delta give M x' = x + delta e_0.  Since
// (Mx')_0=sum_k x'_k and (Mx')_r=x'_{r-1} for r>0, this formula is integral
// and dimension-parametric.
inline std::vector<long long> forward_displacement(
    const std::vector<long long>& x, long long delta) {
    if (x.size() < 2) throw std::invalid_argument(
        "forward_displacement: dimension must be >= 2");
    std::vector<long long> rhs = x;
    rhs[0] += delta;
    std::vector<long long> result(x.size(), 0);
    long long tail = rhs[0];
    for (std::size_t k = 0; k + 1 < x.size(); ++k) {
        result[k] = rhs[k + 1];
        tail -= rhs[k + 1];
    }
    result.back() = tail;
    return result;
}

inline std::vector<std::size_t> parents(std::size_t n, std::size_t letter) {
    if (letter >= n) throw std::out_of_range("parents: letter out of range");
    if (letter > 0) return {letter - 1};
    std::vector<std::size_t> result(n);
    for (std::size_t k = 0; k < n; ++k) result[k] = k;
    return result;
}

struct DescriptorPredecessor {
    DisplacementDescriptor displacement;
    long long delta = 0;
    auto operator<=>(const DescriptorPredecessor&) const = default;
};

// Finite inverse table for `forward_descriptor`.  Each result records both
// the predecessor descriptor and the delta its predecessor labels must
// realize.  There are at most two candidates, independent of n.
inline std::set<DescriptorPredecessor> predecessor_descriptors(
    std::size_t n, DisplacementDescriptor target) {
    (void)displacement_from_descriptor(n, target);
    const std::size_t last = n - 1;
    std::set<DescriptorPredecessor> result;
    if (target.kind == DisplacementKind::Root) {
        if (target.b < last) {
            result.insert({
                {DisplacementKind::Root, target.sign,
                 target.a + 1, target.b + 1, 0}, 0});
            result.insert({
                {DisplacementKind::AlternatingTriple, -target.sign,
                 0, target.a + 1, target.b + 1}, target.sign});
        } else {
            result.insert({
                {DisplacementKind::Root, -target.sign,
                 0, target.a + 1, 0}, target.sign});
        }
    } else if (target.c < last) {
        result.insert({
            {DisplacementKind::AlternatingTriple, target.sign,
             target.a + 1, target.b + 1, target.c + 1}, target.sign});
    } else {
        result.insert({
            {DisplacementKind::Root, target.sign,
             target.a + 1, target.b + 1, 0}, target.sign});
    }
    for (const auto& predecessor : result) {
        const auto image = forward_descriptor(
            n, predecessor.displacement, predecessor.delta);
        if (!image || *image != target)
            throw std::logic_error("predecessor descriptor table is inconsistent");
    }
    return result;
}

inline std::set<CoreState> predicted_core_successors(
    std::size_t n, const CoreState& source) {
    if (!predicted_core_member(n, source))
        throw std::invalid_argument(
            "predicted_core_successors: source outside core");
    const long long delta =
        (source.pair.j > 0 ? 1LL : 0LL) -
        (source.pair.i > 0 ? 1LL : 0LL);
    const auto next = forward_descriptor(n, source.displacement, delta);
    std::set<CoreState> result;
    if (!next) return result;
    for (const auto ip : parents(n, source.pair.i))
        for (const auto jp : parents(n, source.pair.j)) {
            const CoreState destination{{ip, jp}, *next};
            if (predicted_core_member(n, destination))
                result.insert(destination);
        }
    return result;
}

inline std::set<CoreState> predicted_core_predecessors(
    std::size_t n, const CoreState& target) {
    if (!predicted_core_member(n, target))
        throw std::invalid_argument(
            "predicted_core_predecessors: target outside core");
    std::set<std::size_t> source_i{0}, source_j{0};
    if (target.pair.i + 1 < n) source_i.insert(target.pair.i + 1);
    if (target.pair.j + 1 < n) source_j.insert(target.pair.j + 1);
    std::set<CoreState> result;
    for (const auto descriptor_predecessor :
         predecessor_descriptors(n, target.displacement)) {
        for (const auto i : source_i)
            for (const auto j : source_j) {
                const long long delta =
                    (j > 0 ? 1LL : 0LL) - (i > 0 ? 1LL : 0LL);
                if (delta != descriptor_predecessor.delta) continue;
                const CoreState source{
                    {i, j}, descriptor_predecessor.displacement};
                if (!predicted_core_member(n, source)) continue;
                if (predicted_core_successors(n, source).count(target))
                    result.insert(source);
            }
    }
    return result;
}

inline std::set<LabelTransition> label_transitions(std::size_t n) {
    const auto pairs = label_pairs(n);
    std::set<LabelTransition> result;
    for (const LabelPair source : pairs) {
        for (const std::size_t ip : parents(n, source.i)) {
            for (const std::size_t jp : parents(n, source.j)) {
                const LabelPair destination{ip, jp};
                if (!supported_pair(n, destination)) continue;
                // The (0,0) branch resets into the literal embedded
                // (n-1)-bonacci grammar, including its shorter diagonal
                // range—not merely into pairs whose indices avoid n-1.
                if (source.i == 0 && source.j == 0 &&
                    !supported_pair(n - 1, destination))
                    continue;
                const long long delta =
                    (source.j > 0 ? 1LL : 0LL) -
                    (source.i > 0 ? 1LL : 0LL);
                result.insert({source, destination, delta});
            }
        }
    }
    return result;
}

inline std::size_t predicted_transition_count(std::size_t n) {
    if (n < 3) throw std::invalid_argument(
        "predicted_transition_count: n must be >= 3");
    return 4 * n * n - 6 * n - 4;
}

inline std::vector<long long> unit_difference(
    std::size_t n, std::size_t positive, std::size_t negative) {
    std::vector<long long> result(n, 0);
    result[positive] += 1;
    result[negative] -= 1;
    return result;
}

// Sparse vectors predicted to attain the actual signed minimum and maximum
// in the dominant-core group with the requested ordered face pair.
inline EndpointWitnesses endpoint_witnesses(std::size_t n, LabelPair pair) {
    if (!supported_pair(n, pair))
        throw std::invalid_argument("endpoint_witnesses: unsupported pair");
    const std::size_t last = n - 1;

    // The terminal row/column groups are singletons, exchanged by mirror.
    if (pair.j == last && pair.i != last) {
        std::vector<long long> x(n, 0);
        if (pair.i == 0) {
            x[0] = -1;
            x[last] = 1;
        } else {
            x[pair.i] = -1;
            x[last] = 1;
        }
        return {x, x};
    }
    if (pair.i == last && pair.j != last) {
        EndpointWitnesses mirrored = endpoint_witnesses(n, {pair.j, pair.i});
        for (auto& x : mirrored.lower) x = -x;
        for (auto& x : mirrored.upper) x = -x;
        return {mirrored.upper, mirrored.lower};
    }

    if (pair.i == pair.j) {
        const auto upper = unit_difference(n, pair.i + 1, last);
        std::vector<long long> lower = upper;
        for (auto& x : lower) x = -x;
        return {lower, upper};
    }

    if (pair.i == 0) {
        std::vector<long long> lower(n, 0);
        lower[0] = -1;
        lower[pair.j] += 1;
        lower[pair.j + 1] -= 1;
        std::vector<long long> upper(n, 0);
        upper[pair.j] = 1;
        upper[pair.j + 1] = -1;
        if (pair.j + 2 < n) upper[pair.j + 2] = 1;
        return {lower, upper};
    }
    if (pair.j == 0) {
        EndpointWitnesses mirrored = endpoint_witnesses(n, {0, pair.i});
        for (auto& x : mirrored.lower) x = -x;
        for (auto& x : mirrored.upper) x = -x;
        return {mirrored.upper, mirrored.lower};
    }

    std::vector<long long> lower(n, 0);
    lower[pair.i] = -1;
    lower[last] = 1;
    std::vector<long long> upper(n, 0);
    upper[pair.j] = 1;
    upper[last] = -1;
    return {lower, upper};
}

}  // namespace ravel::nbonacci_margin
