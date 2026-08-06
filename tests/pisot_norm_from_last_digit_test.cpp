#include <cassert>
#include <iostream>
#include <string>
#include "math/qbeta.hpp"
#include "math/sturm.hpp"
#include "ravel/proof/pisot_norm_from_last_digit.hpp"

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
    auto c = ravel::proof::derive_pisot_norm_from_last_digit(R, beta_I);
    // Independent check: the algebraic norm of beta is +-(constant term of
    // its OWN minimal polynomial), not the last digit -- computed here
    // straight from R.charpoly(), with no reference to what the greedy
    // expansion did, so this is a genuine cross-check, not circular.
    long long minpoly_const = mpz_get_si(R.charpoly().coeff(0).get());
    long long true_norm_abs = minpoly_const < 0 ? -minpoly_const : minpoly_const;
    std::cout << name << ": digits.size=" << c.digits.size()
              << " last_digit=" << c.norm_abs << " true|norm|=" << true_norm_abs
              << " is_unit=" << c.is_unit << " proved=" << c.proved << "\n";
    assert(c.proved);
    assert(c.norm_abs == true_norm_abs);
}

int main() {
    check("golden ratio (x^2-x-1)", {-1, -1});
    check("theta1 plastic (x^3-x-1)", {0, -1, -1});
    check("theta2 (x^4-x^3-1)", {-1, 0, 0, -1});
    check("theta3 (x^5-x^4-x^3+x^2-1)", {-1, -1, 1, 0, -1});
    check("theta4 supergolden (x^3-x^2-1)", {-1, 0, -1});
    check("theta5 (x^6-x^5-x^4+x^2-1)", {-1, -1, 0, 1, 0, -1});
    check("theta6 (x^5-x^3-x^2-x-1)", {0, -1, -1, -1, -1});
    check("silver ratio (x^2-2x-1)", {-2, -1});
    check("x^2-2x-2", {-2, -2});
    check("x^3-2x^2-2", {-2, 0, -2});
    check("x^3-x^2-2x-2", {-1, -2, -2});
    std::cout << "ALL PASS: |norm(beta)| == last digit of the greedy expansion, "
                 "for every case (unit and non-unit, cyclotomic-padded and not).\n";
}
