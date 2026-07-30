// app/rnd13_factor_probe.cpp
//
// Adelic tiling plan (docs/ADELIC_TILING_PLAN.md) §4, steps 1-2,
// applied to the 4-letter rnd13 substitution.
//
// charpoly(rnd13) = x^4 - 4x^3 - 8x^2 - 6x - 2 (incidence matrix
// M[i][j] = count of letter i in sigma(j); cross-checked with SymPy
// independently of this codebase: det = -2, matches |det M| = 2 and
// beta ~ 5.6236 from FINDINGS_FOR_CITATION.md Finding 2).
//
// Step 1: irreducibility over Q -- SymPy's factor() on this
// polynomial returns it unfactored, so it is irreducible over Q;
// this driver does not re-derive that, it assumes it (recorded here
// so the assumption is written down, per CPP_DESIGN_PHILOSOPHY.md
// section 4's "name the paper/computation a piece mirrors").
//
// Step 2: factor (2) in O_K via Dedekind's criterion, with the
// p-maximality check from dedekind_factorization.hpp.

#include <cstdio>
#include "math/bigint.hpp"
#include "math/poly_z.hpp"
#include "adelic/dedekind_factorization.hpp"

int main() {
    using mathlib::PolyZ;
    using mathlib::set_si;
    using mathlib::str;

    PolyZ f;
    f.ensure_size(5);
    set_si(f.coeff(0), -2);
    set_si(f.coeff(1), -6);
    set_si(f.coeff(2), -8);
    set_si(f.coeff(3), -4);
    set_si(f.coeff(4), 1);

    printf("rnd13 charpoly: %s\n", str(f).c_str());
    printf("(irreducibility over Q verified externally via SymPy factor(); "
           "not re-derived in this driver)\n\n");

    auto fac = adelic::factor_prime_in_qbeta(f, 2);
    printf("Factoring (2) in Z[beta]:\n");
    printf("  p-maximal at 2: %s\n", fac.maximal ? "YES" : "NO");
    printf("  %zu prime ideal(s) found in Z[beta]:\n", fac.prime_ideals.size());
    for (const auto& pi : fac.prime_ideals) {
        printf("    p=%lld  e=%lld  f=%lld  ef=%lld  g(x)=%s\n",
               pi.p, pi.e, pi.f, pi.e * pi.f, str(pi.g).c_str());
    }
    if (!fac.maximal) {
        printf("\n  NOTE: Z[beta] is NOT 2-maximal -- the factorization "
               "above is only valid in the suborder Z[beta], not "
               "necessarily in the true ring of integers O_K. The "
               "Round 2/4 method (not implemented) would be needed "
               "before trusting this for the adelic tiling classifier.\n");
    } else {
        printf("\n  Z[beta] IS 2-maximal -- this factorization is the "
               "correct factorization of (2) in O_K.\n");
    }
    return 0;
}
