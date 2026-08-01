// quadratic_class_group_test.cpp
//
// Property/parity tests for include/adelic/quadratic_class_group.hpp:
// binary-quadratic-form class number computation for imaginary
// quadratic fields Q(sqrt(D)), D < 0.
//
// Checks, in order of independence from the enumeration algorithm
// itself (per docs/CPP_DESIGN_PHILOSOPHY.md: prefer checks against a
// closed form or an independent computation over checks against the
// same algorithm):
//
//   1. The nine Heegner numbers (D = -3,-4,-7,-8,-11,-19,-43,-67,-163
//      -- the COMPLETE, classically known list of negative fundamental
//      discriminants with class number exactly 1) each produce exactly
//      one reduced form, and that form is independently computed here
//      from a closed formula (the principal form: (1,0,-D/4) if D==0
//      mod 4, (1,1,(1-D)/4) if D==1 mod 4) rather than trusted from the
//      enumeration's own output -- so this checks the enumeration
//      against a second, independent construction, not against itself.
//   2. A handful of well-known h > 1 discriminants (-15, -20, -23, -24)
//      match their classically tabulated class numbers.
//   3. qform_reduce, a completely different code path (iterative
//      transformation) from enumerate_reduced_forms (direct search),
//      agrees with it: reducing a deliberately-unreduced form
//      equivalent to a known reduced form lands back on a form present
//      in the enumerated list.
//   4. Structural sanity: every enumerated form is primitive
//      (gcd(a,b,c)=1), has the claimed discriminant, and is actually
//      reduced by qform_is_reduced's own definition.

#include <cstdio>
#include <vector>

#include "adelic/quadratic_class_group.hpp"

using namespace std;
using namespace mathlib;
using namespace adelic;

namespace {
int n_pass = 0, n_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { ++n_pass; fprintf(stderr, "  [ok]   %s\n", (msg)); } \
    else { ++n_fail; fprintf(stderr, "  [FAIL] %s\n", (msg)); } \
} while (0)

// Independent construction of the principal form for a fundamental
// discriminant D < 0, from the closed formula (not from this file's
// own enumeration code).
QuadForm principal_form(long long D) {
    BigInt Dz(D);
    if (((D % 4) + 4) % 4 == 0) {
        BigInt c; neg(c, Dz); tdiv_q(c, c, BigInt(4));
        return QuadForm{BigInt(1), BigInt(0), c};
    } else {
        BigInt num; sub(num, BigInt(1), Dz);
        BigInt c; tdiv_q(c, num, BigInt(4));
        return QuadForm{BigInt(1), BigInt(1), c};
    }
}

bool forms_equal(const QuadForm& f, const QuadForm& g) {
    return cmp(f.a, g.a) == 0 && cmp(f.b, g.b) == 0 && cmp(f.c, g.c) == 0;
}

bool contains(const std::vector<QuadForm>& v, const QuadForm& f) {
    for (const auto& g : v) if (forms_equal(g, f)) return true;
    return false;
}

}  // namespace

int main() {
    fprintf(stderr, "=== the nine Heegner numbers: h(D) == 1, verified against an independent principal-form formula ===\n");
    std::vector<long long> heegner = {-3, -4, -7, -8, -11, -19, -43, -67, -163};
    for (long long D : heegner) {
        auto forms = enumerate_reduced_forms(BigInt(D));
        CHECK(forms.size() == 1, "Heegner discriminant produces exactly one reduced form");
        if (forms.size() == 1) {
            QuadForm expected = principal_form(D);
            CHECK(forms_equal(forms[0], expected),
                  "the single reduced form matches the independently-computed principal form");
        }
    }

    fprintf(stderr, "\n=== a handful of well-known h > 1 discriminants ===\n");
    struct { long long D; long long h; } known[] = {
        {-15, 2}, {-20, 2}, {-23, 3}, {-24, 2},
    };
    for (auto& kv : known) {
        long long h = quadratic_class_number(kv.D);
        CHECK(h == kv.h, "class number matches the classically tabulated value");
    }

    fprintf(stderr, "\n=== structural sanity: every enumerated form is primitive, has the right discriminant, and is reduced ===\n");
    {
        auto forms = enumerate_reduced_forms(BigInt(-23));
        bool all_good = true;
        for (const auto& f : forms) {
            BigInt D = qform_discriminant(f);
            if (cmp_si(D, -23) != 0) all_good = false;
            if (!qform_is_reduced(f)) all_good = false;
            BigInt g1, g2;
            gcd(g1, f.a, f.b);
            gcd(g2, g1, f.c);
            if (!is_one(g2)) all_good = false;
        }
        CHECK(all_good, "every D=-23 reduced form is primitive, has discriminant -23, and passes qform_is_reduced");
    }

    fprintf(stderr, "\n=== qform_reduce (a different code path) agrees with enumerate_reduced_forms ===\n");
    {
        // D = -23 has reduced forms (1,1,6), (2,1,3), (2,-1,3). Apply
        // the unimodular transformation (a,b,c) -> (a, b+2a, a+b+c)
        // (translation x -> x+y, preserves equivalence class and
        // discriminant) to deliberately unreduce a known form, then
        // check qform_reduce recovers a form present in the enumerated
        // set for that discriminant.
        auto forms = enumerate_reduced_forms(BigInt(-23));
        int agree_count = 0;
        for (const auto& f : forms) {
            QuadForm unreduced;
            unreduced.a = f.a;
            add(unreduced.b, f.b, unreduced.a);
            add(unreduced.b, unreduced.b, f.a);  // b + 2a
            BigInt tmp;
            add(tmp, f.a, f.b);
            add(tmp, tmp, f.c);
            unreduced.c = tmp;  // a+b+c

            BigInt D_before = qform_discriminant(unreduced);
            QuadForm reduced_back = qform_reduce(unreduced);
            BigInt D_after = qform_discriminant(reduced_back);
            if (cmp(D_before, D_after) != 0) continue;
            if (contains(forms, reduced_back)) ++agree_count;
        }
        CHECK(agree_count == 3,
              "qform_reduce, applied to a deliberately-unreduced transform of each D=-23 form, "
              "lands back on a form enumerate_reduced_forms also found -- for all 3 forms");
    }

    fprintf(stderr, "\n=== error handling ===\n");
    {
        bool threw = false;
        try { enumerate_reduced_forms(BigInt(23)); }  // positive D, invalid
        catch (const std::invalid_argument&) { threw = true; }
        CHECK(threw, "enumerate_reduced_forms rejects D >= 0");
    }
    {
        bool threw = false;
        try { enumerate_reduced_forms(BigInt(-6)); }  // -6 mod 4 == 2, invalid
        catch (const std::invalid_argument&) { threw = true; }
        CHECK(threw, "enumerate_reduced_forms rejects D not congruent to 0 or 1 mod 4");
    }

    fprintf(stderr, "\n%d/%d checks passed\n", n_pass, n_pass + n_fail);
    return n_fail == 0 ? 0 : 1;
}
