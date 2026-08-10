#include <cstdio>

#include "math/poly_z.hpp"
#include "ravel/marker_power_return_core.hpp"
#include "ravel/return_core_spectral_certificate.hpp"

using namespace ravel;

namespace {
int tests = 0;
int failures = 0;

void expect(bool value, const char* label) {
    ++tests;
    if (value) std::printf("  [ok]   %s\n", label);
    else { std::printf("  [FAIL] %s\n", label); ++failures; }
}
}

int main() {
    // sigma_{0,1}: 0->12, 1->2, 2->0; the marker-power probe supplies the
    // first marker-proper return substitution at sigma^3.
    const SubstitutionRule rule({{1, 2}, {2}, {0}});
    const auto power = marker_power_rule(rule, 3);
    const auto induced = build_return_substitution(power, 0);

    // If beta^3 = beta+1 and beta^3-beta-1=0, then y=beta^3 obeys
    // y^3 - 3 y^2 + 2 y - 1 = 0.  The remaining two modes are Phi_2^2.
    const mathlib::PolyZ cubic({-1, 2, -3, 1});
    const mathlib::PolyZ phi2_squared({1, 2, 1});
    const auto cert = certify_return_core_spectrum(induced, cubic, phi2_squared);

    expect(induced.words.size() == 5, "five return words are recovered");
    expect(cert.characteristic == mathlib::PolyZ({-1, 0, 0, -3, -1, 1}),
           "live incidence matrix has the expected exact characteristic polynomial");
    expect(cert.exact_factorization,
           "characteristic polynomial factors as cubic core times Phi_2^2");
    expect(cert.expanding_factor == cubic,
           "expanding factor is the minimal polynomial of beta^3");

    std::printf("%d tests run, %d failed.\n", tests, failures);
    return failures == 0 ? 0 : 1;
}
