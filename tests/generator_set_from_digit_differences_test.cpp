// Verifies the cheap, closed-form digit-difference prediction against
// the expensive ground-truth check (full role catalogue + domination)
// across every Pisot number this project has examined.

#include <cassert>
#include <iostream>
#include <set>
#include <string>
#include "math/qbeta.hpp"
#include "math/sturm.hpp"
#include "ravel/proof/canonical_substitution_generator_collapse.hpp"
#include "ravel/proof/generator_set_from_digit_differences.hpp"

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

    auto ground_truth = ravel::proof::derive_canonical_substitution_generator_collapse(R, beta_I);
    auto predicted = ravel::proof::predict_generator_set_from_digits(ground_truth.digits);

    std::cout << name << ": predicted={";
    for (auto g : predicted) std::cout << g << ",";
    std::cout << "} (count=" << predicted.size() << ")  ground-truth primitive_generator_count="
              << ground_truth.primitive_generator_count
              << " (raw_defect_classes=" << ground_truth.raw_defect_classes << ")\n";

    assert(ground_truth.proved);
    if (predicted.size() != ground_truth.primitive_generator_count) {
        std::cout << "  MISMATCH digits=";
        for (auto d : ground_truth.digits) std::cout << d << ",";
        std::cout << "\n";
    }
    assert(predicted.size() == ground_truth.primitive_generator_count);
}

int main() {
    check("golden ratio (x^2-x-1)", {-1, -1});
    check("theta1 plastic (x^3-x-1)", {0, -1, -1});
    check("theta2 (x^4-x^3-1)", {-1, 0, 0, -1});
    check("theta3 (x^5-x^4-x^3+x^2-1)", {-1, -1, 1, 0, -1});
    check("theta4 supergolden (x^3-x^2-1)", {-1, 0, -1});
    check("theta5 (x^6-x^5-x^4+x^2-1)", {-1, -1, 0, 1, 0, -1});
    check("theta6 (x^5-x^3-x^2-x-1)", {0, -1, -1, -1, -1});
    check("theta7 (x^7-x^6-x^5+x^2-1)", {-1, -1, 0, 0, 1, 0, -1});
    check("theta8 (x^6-2x^5+x^4-x^2+x-1)", {-2, 1, 0, -1, 1, -1});
    check("theta9 (x^5-x^4-x^2-1)", {-1, 0, -1, 0, -1});
    check("theta10 (x^8-x^7-x^6+x^2-1)", {-1, -1, 0, 0, 0, 1, 0, -1});
    check("silver ratio (x^2-2x-1)", {-2, -1});
    check("x^2-2x-2", {-2, -2});
    check("x^3-2x^2-2", {-2, 0, -2});
    check("x^3-x^2-2x-2", {-1, -2, -2});
    std::cout << "\nALL PASS: the digit-pairwise-difference prediction exactly matches "
                 "the expensive role-catalogue ground truth for every case, including "
                 "the two genuine-fourth-generator witnesses.\n";
}
