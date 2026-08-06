#include <cassert>
#include <iostream>
#include <string>
#include "math/qbeta.hpp"
#include "math/sturm.hpp"
#include "ravel/proof/beta_in_one_two_forces_qrs.hpp"

using namespace mathlib;

mathlib::QBetaRing ring_from_low_first(const std::vector<long long>& low_first) {
    const std::size_t d = low_first.size();
    PolyZ p; p.ensure_size(d + 1);
    for (std::size_t i = 0; i < d; ++i) set_si(p.coeff(d - 1 - i), low_first[i]);
    set_si(p.coeff(d), 1);
    return QBetaRing(p);
}

void check(const std::string& name, std::vector<long long> low_first) {
    auto R = ring_from_low_first(low_first);
    auto beta_I = isolate_beta(R);
    auto c = ravel::proof::derive_beta_in_one_two_forces_qrs(R, beta_I);
    std::cout << name << ": alphabet=" << c.alphabet_size << " digits.size=" << c.digits.size()
              << " prefixes=" << c.prefixes_found.size() << " defect_classes=" << c.defect_class_count
              << " proved=" << c.proved << (c.proved ? "" : (" obstruction=" + c.obstruction)) << "\n";
    assert(c.proved);
}

int main() {
    check("golden ratio (x^2-x-1)", {-1, -1});
    check("theta1 plastic (x^3-x-1)", {0, -1, -1});
    check("theta2 (x^4-x^3-1)", {-1, 0, 0, -1});
    check("theta3 (x^5-x^4-x^3+x^2-1)", {-1, -1, 1, 0, -1});
    check("theta4 supergolden (x^3-x^2-1)", {-1, 0, -1});
    check("theta5 (x^6-x^5-x^4+x^2-1)", {-1, -1, 0, 1, 0, -1});
    check("theta6 (x^5-x^3-x^2-x-1)", {0, -1, -1, -1, -1});
    std::cout << "ALL PASS: every Pisot number in (1,2) forces the Q/R/S "
                 "three-generator shape via the canonical beta-substitution.\n";
}
