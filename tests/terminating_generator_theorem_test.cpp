#include <cassert>
#include <iostream>
#include <set>
#include <string>
#include "math/qbeta.hpp"
#include "math/sturm.hpp"
#include "ravel/proof/terminating_generator_theorem.hpp"
#include "ravel/proof/canonical_substitution_generator_collapse.hpp"

using namespace mathlib;

QBetaRing ring_from_low_first(const std::vector<long long>& low_first) {
    const std::size_t d = low_first.size();
    PolyZ p; p.ensure_size(d + 1);
    for (std::size_t i = 0; i < d; ++i) set_si(p.coeff(d - 1 - i), low_first[i]);
    set_si(p.coeff(d), 1);
    return QBetaRing(p);
}

void check(const std::string& name, std::vector<long long> low_first) {
    auto R = ring_from_low_first(low_first);
    auto beta_I = isolate_beta(R);
    auto ge = ravel::exact_greedy_beta_expansion_of_one(R, beta_I, 60);
    if (!ge.terminated) { std::cout << name << ": not terminating, skipped (out of this theorem's scope)\n"; return; }

    auto thm = ravel::proof::derive_terminating_generator_theorem(ge.digits);
    auto ground = ravel::proof::derive_canonical_substitution_generator_collapse(R, beta_I);

    std::cout << name << ": premise(every nonzero letter unique parent)=" << thm.every_nonzero_letter_has_unique_parent
              << " letter0_occurrences=" << thm.letter_zero_occurrence_count
              << " predicted_count=" << thm.predicted_generators.size()
              << " ground_truth_primitive=" << ground.primitive_generator_count << "\n";

    assert(thm.proved);
    assert(thm.every_nonzero_letter_has_unique_parent);  // the load-bearing structural fact, checked directly
    assert(thm.predicted_generators.size() == ground.primitive_generator_count);
}

int main() {
    check("golden ratio", {-1, -1});
    check("theta1 plastic", {0, -1, -1});
    check("theta2", {-1, 0, 0, -1});
    check("theta3", {-1, -1, 1, 0, -1});
    check("theta4 supergolden", {-1, 0, -1});
    check("theta9", {-1, 0, -1, 0, -1});
    check("x^2-2x-2", {-2, -2});
    check("x^3-2x^2-2", {-2, 0, -2});
    check("x^3-x^2-2x-2", {-1, -2, -2});
    std::cout << "\nALL PASS: the load-bearing structural premise (every nonzero letter has "
                 "exactly one parent decomposition) holds in every terminating case checked, "
                 "and the predicted generator set's SIZE matches ground truth exactly -- this "
                 "is the proof's own premise verified directly, not just its numeric conclusion.\n";
}
