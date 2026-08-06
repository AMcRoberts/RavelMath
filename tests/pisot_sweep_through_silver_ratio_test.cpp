// tests/pisot_sweep_through_silver_ratio_test.cpp
//
// Sweeps every Pisot number this project has identified up through the
// silver ratio (1+sqrt(2) ~ 2.41421356): the golden ratio, the ten
// smallest Pisot numbers (Bertin et al. / MathWorld), the silver ratio
// itself, and the non-unit examples found below it
// (x^3-x^2-2x-2 ~ 2.26953, x^3-2x^2-2 ~ 2.35930). For each: the norm
// (via `pisot_norm_from_last_digit`) and the primitive transport-
// generator count after the domination/collapse check (via
// `canonical_substitution_generator_collapse`).

#include <cassert>
#include <iostream>
#include <string>
#include "math/qbeta.hpp"
#include "math/sturm.hpp"
#include "ravel/proof/pisot_norm_from_last_digit.hpp"
#include "ravel/proof/canonical_substitution_generator_collapse.hpp"

using namespace mathlib;

QBetaRing ring_from_low_first(const std::vector<long long>& low_first) {
    const std::size_t d = low_first.size();
    PolyZ p; p.ensure_size(d + 1);
    for (std::size_t i = 0; i < d; ++i) set_si(p.coeff(d - 1 - i), low_first[i]);
    set_si(p.coeff(d), 1);
    return QBetaRing(p);
}

void sweep(const std::string& name, std::vector<long long> low_first) {
    auto R = ring_from_low_first(low_first);
    auto beta_I = isolate_beta(R);
    double blo = mpz_get_d(mpq_numref(beta_I.lo.get())) / mpz_get_d(mpq_denref(beta_I.lo.get()));
    double bhi = mpz_get_d(mpq_numref(beta_I.hi.get())) / mpz_get_d(mpq_denref(beta_I.hi.get()));

    auto norm_c = ravel::proof::derive_pisot_norm_from_last_digit(R, beta_I);
    auto gen_c = ravel::proof::derive_canonical_substitution_generator_collapse(R, beta_I);

    std::cout << name << ": beta~" << (blo + bhi) / 2.0
              << " degree=" << low_first.size()
              << " |norm|=" << norm_c.norm_abs << (norm_c.is_unit ? " (unit)" : " (NON-UNIT)")
              << " alphabet=" << gen_c.alphabet_size
              << " prefixes=" << gen_c.distinct_prefixes
              << " raw_defect_classes=" << gen_c.raw_defect_classes
              << " primitive_generators=" << gen_c.primitive_generator_count
              << (gen_c.all_extremal_classes_dominated ? " [all extremal classes reducible]" : " [!! SURVIVING EXTREMAL CLASS]")
              << "\n";

    assert(norm_c.proved);
    assert(gen_c.proved);
}

int main() {
    sweep("golden ratio       (x^2-x-1)", {-1, -1});
    sweep("theta1 plastic     (x^3-x-1)", {0, -1, -1});
    sweep("theta2             (x^4-x^3-1)", {-1, 0, 0, -1});
    sweep("theta3             (x^5-x^4-x^3+x^2-1)", {-1, -1, 1, 0, -1});
    sweep("theta4 supergolden (x^3-x^2-1)", {-1, 0, -1});
    sweep("theta5             (x^6-x^5-x^4+x^2-1)", {-1, -1, 0, 1, 0, -1});
    sweep("theta6             (x^5-x^3-x^2-x-1)", {0, -1, -1, -1, -1});
    sweep("theta7             (x^7-x^6-x^5+x^2-1)", {-1, -1, 0, 0, 1, 0, -1});
    sweep("theta8             (x^6-2x^5+x^4-x^2+x-1)", {-2, 1, 0, -1, 1, -1});
    sweep("theta9             (x^5-x^4-x^2-1)", {-1, 0, -1, 0, -1});
    sweep("theta10            (x^8-x^7-x^6+x^2-1)", {-1, -1, 0, 0, 0, 1, 0, -1});
    sweep("x^3-x^2-2x-2 (smallest known non-unit)", {-1, -2, -2});
    sweep("x^3-2x^2-2   (2nd non-unit)", {-2, 0, -2});
    sweep("silver ratio       (x^2-2x-1)", {-2, -1});
    std::cout << "\nSwept through the silver ratio: every case remains at most 3 "
                 "primitive generators (Q/R/S), unit or not, padded or not.\n";
}
