// nbonacci_margin_invariant_test.cpp
//
// Pure, corona-free checks for the parametric n-bonacci label and sparse
// endpoint grammar. Finite-core agreement is checked separately by the exact
// research target; this protects the generated formulas themselves.

#include "ravel/nbonacci_margin_invariant.hpp"

#include <cstdio>

using namespace ravel::nbonacci_margin;

namespace {

int checks = 0;
int failures = 0;

void require(bool condition, const char* label, std::size_t n) {
    ++checks;
    if (condition) return;
    ++failures;
    std::printf("FAIL n=%zu: %s\n", n, label);
}

std::vector<long long> negated(std::vector<long long> x) {
    for (auto& coefficient : x) coefficient = -coefficient;
    return x;
}

}  // namespace

int main() {
    for (std::size_t n = 3; n <= 64; ++n) {
        const auto pairs = label_pairs(n);
        const auto transitions = label_transitions(n);
        const auto displacements = displacement_catalogue(n);
        require(pairs.size() == predicted_pair_count(n),
                "pair-count polynomial", n);
        require(transitions.size() == predicted_transition_count(n),
                "transition-count polynomial", n);
        require(displacements.size() == predicted_displacement_count(n),
                "displacement-count polynomial", n);
        std::size_t stratified_core_size = 0;
        for (const LabelPair pair : pairs)
            stratified_core_size += predicted_pair_node_count(n, pair);
        require(stratified_core_size == predicted_core_size(n),
                "pair strata sum to core-size polynomial", n);
        for (const auto& x : displacements) {
            const auto descriptor = describe_displacement(x);
            require(descriptor.has_value(),
                    "displacement has canonical descriptor", n);
            require(descriptor.has_value() &&
                        displacement_from_descriptor(n, *descriptor) == x,
                    "displacement descriptor round trip", n);
            std::size_t support = 0;
            long long norm_one = 0;
            for (const long long coordinate : x) {
                support += coordinate != 0;
                norm_one += coordinate < 0 ? -coordinate : coordinate;
            }
            require(x.size() == n && (support == 2 || support == 3) &&
                        norm_one == static_cast<long long>(support),
                    "displacement sparse signed-unit shape", n);
            require(displacements.count(negated(x)) == 1,
                    "displacement mirror closure", n);
            for (long long delta = -1; delta <= 1; ++delta) {
                const auto xprime = forward_displacement(x, delta);
                const auto predicted = descriptor.has_value()
                    ? forward_descriptor(n, *descriptor, delta)
                    : std::optional<DisplacementDescriptor>{};
                const bool concrete_stays = displacements.count(xprime) == 1;
                require(predicted.has_value() == concrete_stays,
                        "symbolic update exactly predicts catalogue closure", n);
                require(!predicted.has_value() ||
                            displacement_from_descriptor(n, *predicted) == xprime,
                        "symbolic update equals concrete affine update", n);
                std::vector<long long> reconstructed(n, 0);
                for (std::size_t k = 0; k < n; ++k)
                    reconstructed[0] += xprime[k];
                for (std::size_t k = 1; k < n; ++k)
                    reconstructed[k] = xprime[k - 1];
                std::vector<long long> rhs = x;
                rhs[0] += delta;
                require(reconstructed == rhs,
                        "inverse-incidence affine update", n);
                require(forward_displacement(negated(x), -delta) ==
                            negated(xprime),
                        "affine update mirror law", n);
            }
        }

        // Exhaust the direct core-membership grammar on a substantial
        // all-parameter grid.  The upper bound keeps the default regression
        // light while still checking millions of pair/descriptor relations,
        // well beyond the corona-backed discovery range n=3,...,7.
        if (n <= 20) {
            for (const LabelPair pair : pairs) {
                std::size_t member_count = 0;
                for (const auto& x : displacements) {
                    const auto descriptor = describe_displacement(x);
                    require(descriptor.has_value(),
                            "membership input has descriptor", n);
                    if (!descriptor) continue;
                    const bool member =
                        predicted_core_member(n, pair, *descriptor);
                    require(member == predicted_core_member(n, pair, x),
                            "descriptor/vector membership overloads agree", n);
                    member_count += member;
                    auto mirrored = *descriptor;
                    mirrored.sign = -mirrored.sign;
                    require(member == predicted_core_member(
                                          n, {pair.j, pair.i}, mirrored),
                            "core-membership mirror law", n);
                }
                require(member_count == predicted_pair_node_count(n, pair),
                        "membership predicate realizes pair stratum count", n);
            }
        }

        for (const LabelPair pair : pairs) {
            require(supported_pair(n, pair), "generated pair supported", n);
            const EndpointWitnesses endpoints = endpoint_witnesses(n, pair);
            require(endpoints.lower.size() == n && endpoints.upper.size() == n,
                    "endpoint dimensions", n);

            const LabelPair mirror{pair.j, pair.i};
            require(supported_pair(n, mirror), "mirror pair supported", n);
            const EndpointWitnesses mirror_endpoints =
                endpoint_witnesses(n, mirror);
            require(mirror_endpoints.lower == negated(endpoints.upper) &&
                    mirror_endpoints.upper == negated(endpoints.lower),
                    "signed endpoint mirror law", n);

            if (pair.j + 1 == n && pair.i != pair.j)
                require(endpoints.lower == endpoints.upper,
                        "terminal-column group is a singleton", n);
        }

        for (const LabelTransition& transition : transitions) {
            require(pairs.count(transition.source) == 1,
                    "transition source supported", n);
            require(pairs.count(transition.destination) == 1,
                    "transition destination supported", n);
            const long long expected_delta =
                (transition.source.j > 0 ? 1LL : 0LL) -
                (transition.source.i > 0 ? 1LL : 0LL);
            require(transition.delta == expected_delta,
                    "transition delta formula", n);
            if (transition.source.i == 0 && transition.source.j == 0)
                require(supported_pair(n - 1, transition.destination),
                        "(0,0) reset enters embedded grammar", n);
        }
    }

    std::printf("n-bonacci margin grammar: %d checks, %d failures\n",
                checks, failures);
    return failures == 0 ? 0 : 1;
}
