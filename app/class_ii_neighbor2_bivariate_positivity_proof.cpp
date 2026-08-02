// Genuine bivariate proof (2026-08-02) that the interior regime's
// recurring same_letter_H family -- height = k*(b-c), width in
// {b,c} -- is positive for EVERY real a in its actual domain a>=k+2
// (equivalently k<=a-2, the interior-regime constraint itself), not
// just the finitely many concrete a already checked by app/
// class_ii_neighbor2_interior_regime_structure_check.cpp.
//
// A first draft of this file swept k while checking positivity for
// "every a>=0" -- too strong a claim, and it caught itself: the check
// genuinely failed for k>=3 with no domain restriction (verified, not
// assumed -- the sweep prints the concrete failures). The actual claim
// needs the same a>=k+2 coupling the hand derivation used explicitly;
// fixed by substituting a=m+(k+2) (m>=0) before the coefficient check,
// so "every coefficient in m is non-negative" proves positivity for
// exactly the domain that matters, not a stronger and partly false one.
//
// The certificate search in that file and in
// class_ii_neighbor2_same_letter_h_symbolic_proof.cpp substitutes a
// concrete integer a before reducing -- strong, exact evidence at
// every tested (a,k) pair, but still evidence at points, not a
// universal-in-a proof. This file removes that gap for the `a`
// direction: it represents polynomials in `b` whose COEFFICIENTS are
// themselves exact integer polynomials IN a (mathlib::PolyZ, reused
// as the coefficient ring instead of BigInt), reduces modulo the
// Class-II cubic using the identical long-division algorithm (valid
// because the cubic is monic in b, with coefficients that are
// themselves simple polynomials in a -- -a, -(a+1), -1), and checks
// whether the final bivariate polynomial (in the monomial basis
// a^i * b^j) has every coefficient non-negative. If so, the claim
// holds for literally every real a >= 0 -- not a sampled range -- for
// that specific tested integer k. Sweeping k then gives a genuine
// closed-form proof for every (a, k) with a >= 0 and k in the swept
// range, not per-point evidence.
//
// This is the concrete next step AM asked for after "that's strong
// evidence, what's necessary to prove the certified closure?": remove
// the per-a substitution, not just add more sample points.

#include <cstdio>
#include <vector>

#include "math/poly_z.hpp"

using mathlib::BigInt;
using mathlib::divmod;
using mathlib::PolyZ;

namespace {

// A polynomial in b whose coefficients are polynomials in a:
// BivPoly[i] = coefficient of b^i, itself a PolyZ in a.
using BivPoly = std::vector<PolyZ>;

PolyZ zero_a() { return PolyZ(); }

BivPoly biv_add(const BivPoly& x, const BivPoly& y) {
    BivPoly out(std::max(x.size(), y.size()), zero_a());
    for (std::size_t i = 0; i < x.size(); ++i) out[i] = out[i] + x[i];
    for (std::size_t i = 0; i < y.size(); ++i) out[i] = out[i] + y[i];
    return out;
}
BivPoly biv_sub(const BivPoly& x, const BivPoly& y) {
    BivPoly out(std::max(x.size(), y.size()), zero_a());
    for (std::size_t i = 0; i < x.size(); ++i) out[i] = out[i] + x[i];
    for (std::size_t i = 0; i < y.size(); ++i) out[i] = out[i] - y[i];
    return out;
}
BivPoly biv_scale(const BivPoly& x, const PolyZ& s) {
    BivPoly out(x.size());
    for (std::size_t i = 0; i < x.size(); ++i) out[i] = x[i] * s;
    return out;
}
BivPoly biv_shift(const BivPoly& x, int by) {  // multiply by b^by
    BivPoly out(x.size() + static_cast<std::size_t>(by), zero_a());
    for (std::size_t i = 0; i < x.size(); ++i) out[i + static_cast<std::size_t>(by)] = x[i];
    return out;
}
long long biv_degree_b(const BivPoly& x) {
    for (long long i = static_cast<long long>(x.size()) - 1; i >= 0; --i)
        if (!x[static_cast<std::size_t>(i)].is_zero()) return i;
    return -1;
}

// The Class-II cubic, b^3 - a*b^2 - (a+1)*b - 1, as a BivPoly: degree
// 3 in b, coefficients that are simple polynomials in a.
BivPoly cubic_bivariate() {
    BivPoly cubic(4, zero_a());
    // b^0 coeff: -1
    { PolyZ p; mathlib::set_si(p.coeff(0), -1); cubic[0] = p; }
    // b^1 coeff: -(a+1) = -a - 1
    { PolyZ p; mathlib::set_si(p.coeff(0), -1); mathlib::set_si(p.coeff(1), -1); cubic[1] = p; }
    // b^2 coeff: -a
    { PolyZ p; mathlib::set_si(p.coeff(1), -1); cubic[2] = p; }
    // b^3 coeff: 1
    { PolyZ p; mathlib::set_si(p.coeff(0), 1); cubic[3] = p; }
    return cubic;
}

// Reduce a BivPoly modulo the (monic-in-b) cubic via ordinary long
// division in b, with PolyZ-in-a coefficient arithmetic at each step
// (exact, no floating point, no per-a substitution).
BivPoly reduce_biv(BivPoly p, const BivPoly& cubic) {
    while (biv_degree_b(p) >= 3) {
        const long long d = biv_degree_b(p);
        const PolyZ lead = p[static_cast<std::size_t>(d)];  // leading coeff (cubic is monic, so divide by 1)
        BivPoly term = biv_shift(cubic, static_cast<int>(d - 3));
        term = biv_scale(term, lead);
        p = biv_sub(p, term);
        p.resize(static_cast<std::size_t>(d));  // drop the now-zero top term explicitly
    }
    return p;
}

// height = k*b - k*c = k*(b-c). b^2*height = k*b^3 - k*a*b^2 - k*b.
BivPoly b2_height_k(long long k) {
    BivPoly h(4, zero_a());
    { PolyZ p; mathlib::set_si(p.coeff(0), -k); h[1] = p; }               // -k * b
    { PolyZ p; mathlib::set_si(p.coeff(1), -k); h[2] = p; }               // -k*a * b^2
    { PolyZ p; mathlib::set_si(p.coeff(0), k); h[3] = p; }                // k * b^3
    return h;
}

// General height = p*b + q*c + r (any integer p,q,r), matching the 13
// families in app/class_ii_neighbor2_same_letter_h_symbolic_proof.cpp
// exactly, but bivariate (a stays a free variable, not substituted).
// b^2*height = p*b^3 + q*a*b^2 + r*b^2 + q*b.
BivPoly b2_height_general(long long p, long long q, long long r) {
    BivPoly h(4, zero_a());
    { PolyZ c; mathlib::set_si(c.coeff(0), q); h[1] = c; }                // q*b
    { PolyZ c; mathlib::set_si(c.coeff(1), q); mathlib::set_si(c.coeff(0), r); h[2] = c; }  // (q*a+r)*b^2
    { PolyZ c; mathlib::set_si(c.coeff(0), p); h[3] = c; }                // p*b^3
    return h;
}
BivPoly b2_width(char w) {
    BivPoly out(4, zero_a());
    if (w == 'b') { PolyZ p; mathlib::set_si(p.coeff(0), 1); out[3] = p; }
    else if (w == 'c') { PolyZ a1; mathlib::set_si(a1.coeff(1), 1); out[2] = a1;
                          PolyZ one; mathlib::set_si(one.coeff(0), 1); out[1] = one; }
    else { PolyZ p; mathlib::set_si(p.coeff(0), 1); out[2] = p; }
    return out;
}

bool biv_all_nonneg(const BivPoly& p) {
    for (const auto& coeff_poly : p)
        for (long long i = 0; i <= coeff_poly.degree(); ++i)
            if (mathlib::sgn(coeff_poly.coeff(static_cast<std::size_t>(i))) < 0) return false;
    return true;
}

BivPoly biv_mul_b(const BivPoly& x) { return biv_shift(x, 1); }

// Compose a PolyZ p(a) with the linear substitution a = m + c,
// returning the resulting PolyZ in m (Horner's scheme, exact BigInt
// arithmetic throughout).
PolyZ compose_shift(const PolyZ& p, long long c) {
    PolyZ shift;  // A(m) = m + c
    mathlib::set_si(shift.coeff(0), c);
    mathlib::set_si(shift.coeff(1), 1);
    PolyZ result;  // starts at 0
    for (long long i = p.degree(); i >= 0; --i) {
        result = result * shift;
        PolyZ term;
        mathlib::set(term.coeff(0), p.coeff(static_cast<std::size_t>(i)));
        result = result + term;
    }
    return result;
}

BivPoly biv_compose_shift(const BivPoly& p, long long c) {
    BivPoly out(p.size());
    for (std::size_t i = 0; i < p.size(); ++i) out[i] = compose_shift(p[i], c);
    return out;
}

struct BivCert { bool found = false; int steps = 0; };
// shift_c: substitute a = m + shift_c before the nonneg check, i.e.
// prove positivity for a >= shift_c (the actual domain, e.g. k+2 for
// the k<=a-2 constraint), not for every a>=0 -- the latter is a
// stronger (and, for k>2, FALSE) claim this project's own earlier
// sweep caught by testing rather than assuming.
BivCert find_biv_certificate(BivPoly p, const BivPoly& cubic,
                             long long shift_c, int max_steps = 6) {
    for (int step = 0; step <= max_steps; ++step) {
        p = reduce_biv(p, cubic);
        BivPoly shifted = biv_compose_shift(p, shift_c);
        if (biv_all_nonneg(shifted)) return {true, step};
        p = biv_mul_b(p);
    }
    return {false, max_steps};
}

std::string str_biv(const BivPoly& p) {
    std::string out;
    for (std::size_t i = 0; i < p.size(); ++i) {
        if (p[i].is_zero()) continue;
        if (!out.empty()) out += " + ";
        out += "(" + mathlib::str(p[i]) + ")*b^" + std::to_string(i);
    }
    return out.empty() ? "0" : out;
}

}  // namespace

int main() {
    const BivPoly cubic = cubic_bivariate();
    std::printf("Bivariate (in a AND b simultaneously, no per-a substitution) "
                "positivity proof\nfor height=k*(b-c), width in {b,c}, "
                "swept over k.\n\n");

    long long all_ok = 1;
    for (long long k = 1; k <= 60; ++k) {
        const long long shift_c = k + 2;  // the actual domain: a >= k+2 (i.e. k <= a-2)
        const BivPoly b2h = b2_height_k(k);
        for (char w : {'b', 'c'}) {
            const BivPoly b2w = b2_width(w);
            const auto lower_cert = find_biv_certificate(b2h, cubic, shift_c);
            const auto upper_cert =
                find_biv_certificate(biv_sub(b2w, b2h), cubic, shift_c);
            const bool ok = lower_cert.found && upper_cert.found;
            if (!ok) all_ok = 0;
            if (k <= 3 || !ok) {
                std::printf("k=%lld width=%c (domain a>=%lld): lower_found=%d(steps=%d) "
                            "upper_found=%d(steps=%d) %s\n",
                            k, w, shift_c, lower_cert.found, lower_cert.steps,
                            upper_cert.found, upper_cert.steps,
                            ok ? "PROVEN FOR EVERY a IN DOMAIN" : "NOT PROVEN");
            }
        }
    }
    std::printf("\nSwept k=1..60 (both widths), each against its own actual "
                "domain a>=k+2: all proven for every real a in that domain "
                "simultaneously (no per-a substitution): %s\n",
                all_ok ? "YES" : "NO -- see failures above");

    // Show the actual reduced closed forms for k=1 (the original
    // worked example) as a human-checkable certificate.
    std::printf("\n--- k=1 closed forms (for cross-checking against the "
                "hand derivation) ---\n");
    {
        BivPoly p = reduce_biv(b2_height_k(1), cubic);
        std::printf("lower (k=1), 0 extra steps: %s\n", str_biv(p).c_str());
        BivPoly u = reduce_biv(biv_sub(b2_width('b'), b2_height_k(1)), cubic);
        u = reduce_biv(biv_mul_b(u), cubic);
        std::printf("upper width=b (k=1), 1 extra step: %s\n", str_biv(u).c_str());
    }

    // ---- The symbolic REASON, verified exactly (not just observed
    // as "0 extra steps") ----
    // AM: "look for a symbolic reason why the reduction happens" --
    // then "if you can automate it...". Hypothesis, checked first in
    // sympy (job scratch, exact rational cancellation, ratio=1 not
    // just proportional): b*(b-c) - c, cleared of its 1/b denominator
    // by multiplying through by b, is EXACTLY the Class-II cubic --
    // not merely a consequence of it, algebraically identical to it.
    // Verified here with this project's own exact BigInt bivariate
    // arithmetic: b^2*(b-c) - b*c should reduce to the EXACT ZERO
    // polynomial (every coefficient zero, not just non-negative) once
    // the cubic itself is subtracted out -- i.e. b^2*(b-c) - b*c IS
    // the cubic, coefficient for coefficient, not merely proportional
    // to it.
    std::printf("\n--- The symbolic reason (verified exactly) ---\n");
    {
        BivPoly b2_height1 = b2_height_k(1);  // b^2*(b-c)
        BivPoly bc(2, zero_a());              // b*c = a*b + 1
        { PolyZ p; mathlib::set_si(p.coeff(1), 1); bc[1] = p; }
        { PolyZ p; mathlib::set_si(p.coeff(0), 1); bc[0] = p; }
        BivPoly diff = biv_sub(b2_height1, bc);  // should equal -cubic (or cubic, up to sign)
        bool matches_cubic = true, matches_neg_cubic = true;
        BivPoly neg_cubic(cubic.size());
        for (std::size_t i = 0; i < cubic.size(); ++i) neg_cubic[i] = zero_a() - cubic[i];
        for (std::size_t i = 0; i < std::max(diff.size(), cubic.size()); ++i) {
            PolyZ d = i < diff.size() ? diff[i] : zero_a();
            PolyZ cposs = i < cubic.size() ? cubic[i] : zero_a();
            PolyZ cneg = i < neg_cubic.size() ? neg_cubic[i] : zero_a();
            if (d != cposs) matches_cubic = false;
            if (d != cneg) matches_neg_cubic = false;
        }
        std::printf("b^2*(b-c) - b*c  ==  cubic exactly (coefficient for "
                    "coefficient, not just proportional): %s\n",
                    matches_cubic ? "TRUE" : (matches_neg_cubic ? "TRUE (up to sign)" : "FALSE"));
        std::printf("b^2*(b-c) - b*c = %s\n", str_biv(diff).c_str());
        std::printf("cubic (as bivariate)      = %s\n", str_biv(cubic).c_str());
        std::printf("\nInterpretation: b*(b-c) = c is EXACTLY equivalent to the "
                    "Class-II cubic relation -- not a consequence of it discovered\n"
                    "by algebra, algebraically identical to it. Equivalently: "
                    "b^2 = c*(b+1), or b-c = c/b. This is why k*(b-c) reduces\n"
                    "almost immediately for every k: it is not an arbitrary "
                    "quantity that happens to behave well under the cubic --\n"
                    "it IS (up to the factor k) the cubic's own defining "
                    "relation, restated in terms of b and c instead of b alone.\n"
                    "b^2*k*(b-c) = k*b*c = k*(a*b+1) is therefore not a "
                    "coincidence of the reduction algorithm; it follows from\n"
                    "one substitution (c=a+1/b) applied to one identity (b(b-c)=c) "
                    "that IS the cubic.\n");
    }

    // ---- Automated: extend the bivariate (universal-in-a) proof to
    // ALL 13 round-2 families, not just the k*(b-c) subfamily. This
    // upgrades Finding 15 (docs/FINDINGS_FOR_CITATION.md) from
    // "exact-certified at 13 concrete a-values" to "proven for every
    // real a in the stated domain" -- the same strengthening just
    // done for k*(b-c), applied to the whole table automatically. ----
    std::printf("\n--- All 13 round-2 families, bivariate, domain a>=7 ---\n");
    struct Family { long long p, q, r; char w; };
    const std::vector<Family> all_families = {
        {-1, 1, 1, '1'}, {-1, 1, 1, 'b'}, {-1, 1, 1, 'c'},
        {0, 1, 0, 'b'},
        {1, -1, 0, '1'}, {1, -1, 0, 'b'}, {1, -1, 0, 'c'},
        {1, -1, 1, 'b'},
        {2, -2, -1, 'b'}, {2, -2, -1, 'c'},
        {2, -2, 0, 'b'}, {2, -2, 0, 'c'},
        {2, -1, -1, 'b'},
    };
    const long long domain_a = 7;  // this project's standard domain for this family
    long long families_ok = 0;
    for (const auto& fam : all_families) {
        const BivPoly b2h = b2_height_general(fam.p, fam.q, fam.r);
        const BivPoly b2w = b2_width(fam.w);
        const auto lower_cert = find_biv_certificate(b2h, cubic, domain_a);
        const auto upper_cert = find_biv_certificate(biv_sub(b2w, b2h), cubic, domain_a);
        const bool ok = lower_cert.found && upper_cert.found;
        if (ok) ++families_ok;
        std::printf("height=%+lldb%+lldc%+lld width=%c (domain a>=%lld): "
                    "lower_steps=%d upper_steps=%d %s\n",
                    fam.p, fam.q, fam.r, fam.w, domain_a,
                    lower_cert.steps, upper_cert.steps,
                    ok ? "PROVEN FOR EVERY a IN DOMAIN" : "NOT PROVEN");
    }
    std::printf("\n%lld / %zu round-2 families proven bivariate (every real "
                "a>=%lld at once, not sampled): %s\n",
                families_ok, all_families.size(), domain_a,
                families_ok == static_cast<long long>(all_families.size())
                    ? "ALL CLOSED -- Finding 15 upgradeable to universal-in-a"
                    : "INCOMPLETE -- see failures above");
    return 0;
}
