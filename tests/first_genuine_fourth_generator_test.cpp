// tests/first_genuine_fourth_generator_test.cpp
//
// x^3-2x^2-2 (beta~2.35930, non-unit, digits (2,0,2)) is the project's
// first confirmed example of a genuine, irreducible fourth (and, by
// the symmetric -2 class, fifth) primitive positive transport
// generator: unlike every prior case (all Q/R/S generalized Pisot
// numbers up through the silver ratio, and the simplest non-unit
// substitution x^2-2x-2), the raw defect+2 and defect-2 classes here
// are NOT entrywise dominated by the square of the +-1 generator.
//
// Contrast case included directly: x^2-2x-2 (digits (2,2), no interior
// zero) has zero domination violations -- confirms the presence/absence
// of an interior zero digit between the two 2's is exactly what
// distinguishes the two outcomes, not just "non-unit" or "beta in
// [2,3)" in general (both are true of both examples).

#include <cassert>
#include <iostream>
#include "math/qbeta.hpp"
#include "math/sturm.hpp"
#include "ravel/proof/first_genuine_fourth_generator.hpp"

using namespace mathlib;

QBetaRing ring_from_low_first(const std::vector<long long>& low_first) {
    const std::size_t d = low_first.size();
    PolyZ p; p.ensure_size(d + 1);
    for (std::size_t i = 0; i < d; ++i) set_si(p.coeff(d - 1 - i), low_first[i]);
    set_si(p.coeff(d), 1);
    return QBetaRing(p);
}

int main() {
    // Contrast case: x^2-2x-2, digits (2,2) -- must still fully collapse.
    {
        auto R = ring_from_low_first({-2, -2});
        auto beta_I = isolate_beta(R);
        auto c = ravel::proof::derive_canonical_substitution_generator_collapse(R, beta_I);
        assert(c.proved);
        assert((c.digits == std::vector<long long>{2, 2}));
        assert(c.raw_defect_classes == 5);
        assert(c.primitive_generator_count == 3);
        assert(c.all_extremal_classes_dominated);
        std::cout << "x^2-2x-2 (contrast): raw=5 primitive=3, fully reducible, as before.\n";
    }

    // The genuine fourth/fifth generator witness.
    {
        auto R = ring_from_low_first({-2, 0, -2});  // x^3-2x^2-2
        auto beta_I = isolate_beta(R);
        auto c = ravel::proof::derive_first_genuine_fourth_generator_witness(R, beta_I);
        assert(c.proved);
        assert((c.digits == std::vector<long long>{2, 0, 2}));
        assert(c.alphabet_size == 3);
        assert(c.raw_defect_classes == 5);
        assert(!c.all_extremal_classes_dominated);
        assert(c.dominated.at(2) == false);
        assert(c.dominated.at(-2) == false);
        assert(c.primitive_generator_count == 5);
        std::cout << "x^3-2x^2-2: raw=5 primitive=5 -- GENUINE fourth and fifth "
                     "primitive generators confirmed, not reducible to Q/R/S.\n";
    }

    // Second witness with the same mechanism: two interior zeros give
    // two independent violations each of +2/-2.
    {
        auto R = ring_from_low_first({-1, -2, -2});  // x^3-x^2-2x-2
        auto beta_I = isolate_beta(R);
        auto c = ravel::proof::derive_first_genuine_fourth_generator_witness(R, beta_I);
        assert(c.proved);
        assert((c.digits == std::vector<long long>{2, 0, 1, 0, 2}));
        assert(!c.all_extremal_classes_dominated);
        assert(c.dominated.at(2) == false);
        assert(c.dominated.at(-2) == false);
        std::cout << "x^3-x^2-2x-2: also a genuine fourth/fifth generator witness "
                     "(smallest known non-unit Pisot number).\n";
    }

    std::cout << "PASS: the presence of an interior zero digit between two "
                 "nonzero digits is the structural signature that produces a "
                 "genuine fourth generator; its absence (x^2-2x-2) reduces to Q/R/S.\n";
}
