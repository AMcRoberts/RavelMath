// The complete general theory, exercised end to end with NO case
// splitting in the calling code: every entry below -- terminating,
// cyclotomic-padded, non-unit, and eventually-periodic alike -- goes
// through the exact same call. This is the "doesn't break" property
// demonstrated directly, not asserted.

#include <cassert>
#include <iostream>
#include <string>
#include "math/qbeta.hpp"
#include "math/sturm.hpp"
#include "ravel/proof/general_generator_theorem.hpp"

using namespace mathlib;

QBetaRing ring_from_low_first(const std::vector<long long>& low_first) {
    const std::size_t d = low_first.size();
    PolyZ p; p.ensure_size(d + 1);
    for (std::size_t i = 0; i < d; ++i) set_si(p.coeff(d - 1 - i), low_first[i]);
    set_si(p.coeff(d), 1);
    return QBetaRing(p);
}

void run(const std::string& name, std::vector<long long> low_first, std::size_t expected_primitive) {
    auto R = ring_from_low_first(low_first);
    auto beta_I = isolate_beta(R);
    auto c = ravel::proof::derive_general_generator_theorem(R, beta_I);
    std::cout << name << ": digits.size=" << c.digits.size()
              << " raw=" << c.raw_defect_classes << " primitive=" << c.primitive_generator_count
              << " proved=" << c.proved << "\n";
    assert(c.proved);
    assert(c.primitive_generator_count == expected_primitive);
}

int main() {
    // Terminating, no cyclotomic padding.
    run("golden ratio", {-1, -1}, 3);
    run("theta4 supergolden", {-1, 0, -1}, 3);
    // Terminating, WITH cyclotomic padding (alphabet > degree).
    run("theta1 plastic", {0, -1, -1}, 3);
    run("theta3", {-1, -1, 1, 0, -1}, 3);
    // Terminating, non-unit, no genuine extra generator.
    run("x^2-2x-2", {-2, -2}, 3);
    // Terminating, non-unit, GENUINE extra generators.
    run("x^3-2x^2-2", {-2, 0, -2}, 5);
    run("x^3-x^2-2x-2", {-1, -2, -2}, 5);
    // Eventually periodic (genuine preperiod), unit, GENUINE extra generators.
    run("x^3-2x^2-x+1", {-2, -1, 1}, 5);
    // Eventually periodic, no extra generators (max digit is 1 throughout).
    run("x^3-x^2-2x+1", {-1, -2, 1}, 3);

    std::cout << "\nALL PASS: one call, no case split, correct in every case -- terminating "
                 "or eventually periodic, padded or not, unit or not.\n";
}
