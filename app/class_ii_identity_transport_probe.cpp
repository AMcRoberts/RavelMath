// Transport the Class-II identity across its fixed-incidence fiber, then
// cross a matrix wall (2026-08-02): does the same automation
// -- the universal closed form b^2*height(p,q,r) = [(p+q)*a+r]*b^2 +
// [p*(a+1)+q]*b + p, derived from b*(b-c)=c (the Class-II cubic
// restated) -- explain neighbor 0's own shell geometry too, or is it
// specific to neighbor 2?  After closing neighbors 0 and 1, the final
// section deliberately hops to Tribonacci and separates the part that
// breaks (the sparse quotient-ring identity) from the part that survives
// (exact reduction modulo the characteristic polynomial).
//
// AM: "we just proved it across one specific one-parameter family
// [a]. Can we hop over to a neighbor and prove it there too using the
// same automation bazooka?" sigma_a and its neighbor-0 variant share
// the same incidence matrix (hence the same b,c,1 Perron coordinates
// and the same cubic -- this was already confirmed for neighbor 2 by
// direct measurement of subst.v). If that sharing holds for neighbor
// 0 too, the SAME closed form should apply without modification --
// tested directly here, not assumed.
//
// Neighbor 0's regular shell states (include/ravel/class_ii_neighbor_
// family.hpp, class_ii_neighbor_regular_shell_states(0, a, parameter),
// m = a - parameter - 1) give ten (i, x, j) triples per shell. Their
// (p, q, r) are linear in m (the shell's own free parameter, playing
// the same role k played for neighbor 2) -- e.g. {-m, m, -1} is
// p=-m, q=m, r=-1; {m-1, -m, 1} is p=m-1, q=-m, r=1. Unlike neighbor
// 2's recurring k*(b-c) family, several of these do NOT have p=-q
// (the {-(m-1), m, r} and {m-1, -m, r} shapes have p+q = +-1, not 0)
// -- a genuinely different shape, not covered by the earlier
// trivariate proof's specific factorization. The closed form itself
// makes no p=-q assumption, so it is tested here regardless.

#include <array>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "math/poly_z.hpp"

using mathlib::divmod;
using mathlib::PolyZ;

namespace {

using BivPoly = std::vector<PolyZ>;
PolyZ zero_a() { return PolyZ(); }
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
BivPoly biv_shift(const BivPoly& x, int by) {
    BivPoly out(x.size() + static_cast<std::size_t>(by), zero_a());
    for (std::size_t i = 0; i < x.size(); ++i) out[i + static_cast<std::size_t>(by)] = x[i];
    return out;
}
long long biv_degree_b(const BivPoly& x) {
    for (long long i = static_cast<long long>(x.size()) - 1; i >= 0; --i)
        if (!x[static_cast<std::size_t>(i)].is_zero()) return i;
    return -1;
}
BivPoly cubic_bivariate() {
    BivPoly cubic(4, zero_a());
    { PolyZ p; mathlib::set_si(p.coeff(0), -1); cubic[0] = p; }
    { PolyZ p; mathlib::set_si(p.coeff(0), -1); mathlib::set_si(p.coeff(1), -1); cubic[1] = p; }
    { PolyZ p; mathlib::set_si(p.coeff(1), -1); cubic[2] = p; }
    { PolyZ p; mathlib::set_si(p.coeff(0), 1); cubic[3] = p; }
    return cubic;
}
BivPoly reduce_biv(BivPoly p, const BivPoly& cubic) {
    while (biv_degree_b(p) >= 3) {
        const long long d = biv_degree_b(p);
        const PolyZ lead = p[static_cast<std::size_t>(d)];
        BivPoly term = biv_scale(biv_shift(cubic, static_cast<int>(d - 3)), lead);
        p = biv_sub(p, term);
        p.resize(static_cast<std::size_t>(d));
    }
    return p;
}
PolyZ compose_shift(const PolyZ& p, long long c) {
    PolyZ shift;
    mathlib::set_si(shift.coeff(0), c);
    mathlib::set_si(shift.coeff(1), 1);
    PolyZ result;
    for (long long i = p.degree(); i >= 0; --i) {
        result = result * shift;
        PolyZ term;
        mathlib::set(term.coeff(0), p.coeff(static_cast<std::size_t>(i)));
        result = result + term;
    }
    return result;
}
bool all_coeffs_nonneg(const PolyZ& p) {
    for (long long i = 0; i <= p.degree(); ++i)
        if (mathlib::sgn(p.coeff(static_cast<std::size_t>(i))) < 0) return false;
    return true;
}
BivPoly biv_compose_shift(const BivPoly& p, long long c) {
    BivPoly out(p.size());
    for (std::size_t i = 0; i < p.size(); ++i) out[i] = compose_shift(p[i], c);
    return out;
}
bool biv_all_nonneg(const BivPoly& p) {
    for (const auto& cp : p)
        if (!all_coeffs_nonneg(cp)) return false;
    return true;
}
struct Cert { bool found = false; int steps = 0; };
Cert find_certificate(BivPoly p, const BivPoly& cubic, long long shift_c, int max_steps = 8) {
    for (int step = 0; step <= max_steps; ++step) {
        p = reduce_biv(p, cubic);
        if (biv_all_nonneg(biv_compose_shift(p, shift_c))) return {true, step};
        p = biv_shift(p, 1);
    }
    return {false, max_steps};
}

// Universal closed form (2026-08-02, verified for neighbor 2): given
// concrete p, q, r (m already substituted to a concrete integer),
// b^2*height = [(p+q)*a+r]*b^2 + [p*(a+1)+q]*b + p.
BivPoly closed_form(long long p, long long q, long long r) {
    BivPoly out(3, zero_a());
    { PolyZ x; mathlib::set_si(x.coeff(0), r); mathlib::set_si(x.coeff(1), p + q); out[2] = x; }
    { PolyZ x; mathlib::set_si(x.coeff(0), p + q); mathlib::set_si(x.coeff(1), p); out[1] = x; }
    { PolyZ x; mathlib::set_si(x.coeff(0), p); out[0] = x; }
    return out;
}

PolyZ affine_a(long long constant, long long slope = 0) {
    PolyZ p;
    mathlib::set_si(p.coeff(0), constant);
    mathlib::set_si(p.coeff(1), slope);
    return p;
}

// The same closed form with p and q affine in a.  This lets m remain
// symbolic at the upper endpoint m=a-offset instead of being swept.
BivPoly closed_form_affine(const PolyZ& p, const PolyZ& q, long long r) {
    const PolyZ a = affine_a(0, 1);
    const PolyZ one = affine_a(1);
    BivPoly out(3, zero_a());
    out[2] = (p + q) * a + affine_a(r);
    out[1] = p * (a + one) + q;
    out[0] = p;
    return out;
}
BivPoly b2_height_direct(long long p, long long q, long long r) {
    BivPoly h(4, zero_a());
    { PolyZ x; mathlib::set_si(x.coeff(0), q); h[1] = x; }
    { PolyZ x; mathlib::set_si(x.coeff(1), q); mathlib::set_si(x.coeff(0), r); h[2] = x; }
    { PolyZ x; mathlib::set_si(x.coeff(0), p); h[3] = x; }
    return h;
}
BivPoly b2_width(char w) {
    BivPoly out(4, zero_a());
    if (w == 'b') { PolyZ x; mathlib::set_si(x.coeff(0), 1); out[3] = x; }
    else if (w == 'c') { PolyZ a1; mathlib::set_si(a1.coeff(1), 1); out[2] = a1;
                          PolyZ one; mathlib::set_si(one.coeff(0), 1); out[1] = one; }
    else { PolyZ x; mathlib::set_si(x.coeff(0), 1); out[2] = x; }
    return out;
}
bool polyz_equal(const PolyZ& x, const PolyZ& y) { return (x - y).is_zero(); }
bool biv_equal(const BivPoly& x, const BivPoly& y) {
    std::size_t n = std::max(x.size(), y.size());
    for (std::size_t i = 0; i < n; ++i) {
        PolyZ xi = i < x.size() ? x[i] : zero_a();
        PolyZ yi = i < y.size() ? y[i] : zero_a();
        if (!polyz_equal(xi, yi)) return false;
    }
    return true;
}

PolyZ tribonacci_cubic() {
    PolyZ p;
    mathlib::set_si(p.coeff(0), -1);
    mathlib::set_si(p.coeff(1), -1);
    mathlib::set_si(p.coeff(2), -1);
    mathlib::set_si(p.coeff(3), 1);
    return p;
}

PolyZ reduce_univariate(PolyZ p, const PolyZ& modulus) {
    while (p.degree() >= modulus.degree()) p = divmod(p, modulus).r;
    return p;
}

PolyZ monomial(long long coefficient, std::size_t degree) {
    PolyZ p;
    mathlib::set_si(p.coeff(degree), coefficient);
    return p;
}

long long coefficient_si(const PolyZ& p, std::size_t degree) {
    if (static_cast<long long>(degree) > p.degree()) return 0;
    return mpz_get_si(p.coeff(degree).get());
}

struct BasisCoordinates {
    long long b = 0;
    long long c = 0;
    long long one = 0;
};

using Matrix3 = std::array<std::array<long long, 3>, 3>;

// Express a reduced degree-<3 polynomial in the supplied integral basis
// (b,c,1).  Both bases used below are unimodular, so the 2x2 determinant
// controlling the positive-degree coefficients is +/-1 and no denominator
// is introduced.
BasisCoordinates coordinates_in_perron_basis(
        const PolyZ& reduced, const PolyZ& b, const PolyZ& c) {
    const long long b1 = coefficient_si(b, 1);
    const long long b2 = coefficient_si(b, 2);
    const long long c1 = coefficient_si(c, 1);
    const long long c2 = coefficient_si(c, 2);
    const long long r1 = coefficient_si(reduced, 1);
    const long long r2 = coefficient_si(reduced, 2);
    const long long determinant = b1 * c2 - b2 * c1;
    if (determinant != 1 && determinant != -1)
        throw std::runtime_error("Perron coordinate basis is not unimodular");
    BasisCoordinates out;
    out.b = (r1 * c2 - r2 * c1) / determinant;
    out.c = (b1 * r2 - b2 * r1) / determinant;
    out.one = coefficient_si(reduced, 0)
            - out.b * coefficient_si(b, 0)
            - out.c * coefficient_si(c, 0);
    return out;
}

BasisCoordinates print_multiplication_tensor(
        const char* label, const PolyZ& modulus,
        const PolyZ& b, const PolyZ& c) {
    std::printf("  %s multiplication tensor in basis (b,c,1):\n", label);
    BasisCoordinates difference;
    for (const auto& product : std::vector<std::pair<const char*, PolyZ>>{
             {"b*b", b * b}, {"b*c", b * c}, {"c*c", c * c},
             {"b(b-c)", b * (b - c)}}) {
        const auto coords = coordinates_in_perron_basis(
            reduce_univariate(product.second, modulus), b, c);
        if (std::string(product.first) == "b(b-c)") difference = coords;
        std::printf("    %-6s = %lld*b %+lld*c %+lld\n", product.first,
                    coords.b, coords.c, coords.one);
    }
    return difference;
}

bool coordinates_equal(const BasisCoordinates& actual,
                       const BasisCoordinates& expected) {
    return actual.b == expected.b && actual.c == expected.c
        && actual.one == expected.one;
}

BasisCoordinates matrix_column(const Matrix3& matrix, std::size_t column) {
    return {matrix[0][column], matrix[1][column], matrix[2][column]};
}

bool beta_tensor_equals_incidence(const Matrix3& matrix,
                                  const PolyZ& modulus,
                                  const PolyZ& b, const PolyZ& c) {
    // In every basis supplied below b=beta. M^T*v=beta*v therefore says
    // the coordinates of beta*b, beta*c, beta*1 are columns 0,1,2 of M.
    return coordinates_equal(coordinates_in_perron_basis(
               reduce_univariate(b * b, modulus), b, c),
               matrix_column(matrix, 0))
        && coordinates_equal(coordinates_in_perron_basis(
               reduce_univariate(b * c, modulus), b, c),
               matrix_column(matrix, 1))
        && coordinates_equal(coordinates_in_perron_basis(b, b, c),
               matrix_column(matrix, 2));
}

PolyZ companion_cubic(long long quadratic, long long linear) {
    return monomial(1, 3) - monomial(quadratic, 2)
         - monomial(linear, 1) - monomial(1, 0);
}

// In the genuinely matrix-changing Tribonacci hop, use the same convention
// as in_H_sigma: the left Perron covector, scaled to the integral basis
// (b,c,1)=(beta,beta^2-beta,1).  Reduction modulo the Tribonacci cubic gives
//   b^2*(p*b+q*c+r) = (p+q+r)*beta^2 + (p+q)*beta + p.
PolyZ tribonacci_closed_form(long long p, long long q, long long r) {
    PolyZ out;
    mathlib::set_si(out.coeff(2), p + q + r);
    mathlib::set_si(out.coeff(1), p + q);
    mathlib::set_si(out.coeff(0), p);
    return out;
}

PolyZ tribonacci_direct(long long p, long long q, long long r) {
    // beta^2 * (p*beta + q*(beta^2-beta) + r).
    return monomial(q, 4) + monomial(p - q, 3) + monomial(r, 2);
}

}  // namespace

int main() {
    const BivPoly cubic = cubic_bivariate();

    // Neighbor 0's ten regular-shell (p(m), q(m), r, width) templates,
    // m the shell's own free parameter (domain 2<=m<=a-2, derived from
    // the code's own "parameter>=1, a>=parameter+3" with
    // m=a-parameter-1).
    struct Templ { long long pm, p0, qm, q0, r; char w; };  // p=pm*m+p0, q=qm*m+q0
    const std::vector<Templ> templates = {
        {-1, 0, 1, 0, -1, 'b'},   // {-m,m,-1} j=0
        {-1, 0, 1, 0, 0, 'b'},    // {-m,m,0} j=0
        {-1, 0, 1, 0, 0, 'c'},    // {-m,m,0} j=1
        {-1, 1, 1, 0, -1, 'b'},   // {-(m-1),m,-1} j=0
        {-1, 1, 1, 0, 0, 'b'},    // {-(m-1),m,0} j=0
        {1, -1, -1, 0, 0, 'b'},   // {m-1,-m,0} j=0
        {1, -1, -1, 0, 1, 'b'},   // {m-1,-m,1} j=0
        {1, 0, -1, 0, 0, 'b'},    // {m,-m,0} j=0
        {1, 0, -1, 0, 1, 'b'},    // {m,-m,1} j=0
    };

    // Neighbor 1 is the other adjacent-swap direction.  Its five-state
    // regular shells reduce to four distinct (x,j) strip tests.  This is a
    // useful second hop: it shares the incidence matrix and Perron direction,
    // but has a different recurrent-shell grammar and the slightly larger
    // domain m<=a-1 rather than neighbor 0's m<=a-2.
    const std::vector<Templ> neighbor1_templates = {
        {-1, 0, 1, 0, -1, 'c'},   // {-m,m,-1} j=1
        {-1, 0, 1, 0, 0, 'b'},    // {-m,m,0} j=0
        {1, 0, -1, 0, 0, 'b'},    // {m,-m,0} j=0
        {1, 0, -1, 0, 1, 'b'},    // {m,-m,1} j=0
    };

    auto run_neighbor = [&](const char* label,
                            const std::vector<Templ>& cases,
                            long long domain_offset) {
        long long total = 0, ok_count = 0;
        for (const auto& t : cases) {
          for (long long m = 2; m <= 40; ++m) {
            const long long p = t.pm * m + t.p0;
            const long long q = t.qm * m + t.q0;
            const long long shift_c = m + domain_offset;
            const BivPoly direct = reduce_biv(b2_height_direct(p, q, t.r), cubic);
            const BivPoly claimed = closed_form(p, q, t.r);
            const bool formula_ok = biv_equal(direct, claimed);

            const BivPoly b2w = b2_width(t.w);
            // same_letter_H accepts H_sigma UNION (-H_sigma): the actual
            // condition is -width < height < width, not the oriented
            // 0 < height < width condition used in the neighbor-2 proof's
            // already-positive representatives.  Certify both signed-strip
            // margins directly; this distinction is essential for neighbor
            // 0, whose literal recurrent catalogue contains both signs.
            const auto lower_cert = find_certificate(biv_sub(b2w, biv_scale(claimed, [] {
                PolyZ minus_one;
                mathlib::set_si(minus_one.coeff(0), -1);
                return minus_one;
            }())), cubic, shift_c);  // width + height > 0
            const auto upper_cert = find_certificate(biv_sub(b2w, claimed), cubic, shift_c);
            const bool positivity_ok = lower_cert.found && upper_cert.found;

            ++total;
            if (formula_ok && positivity_ok) ++ok_count;
            if (!formula_ok || !positivity_ok || m <= 3) {
                std::printf("%s p(m)=%lldm%+lld q(m)=%lldm%+lld r=%lld w=%c m=%lld: "
                            "formula_ok=%d domain_a>=%lld positivity_ok=%d "
                            "(lower_steps=%d upper_steps=%d)\n",
                            label, t.pm, t.p0, t.qm, t.q0, t.r, t.w, m,
                            static_cast<int>(formula_ok), shift_c,
                            static_cast<int>(positivity_ok),
                            lower_cert.steps, upper_cert.steps);
            }
          }
        }
        std::printf("\n%s: %lld / %lld (template, m) combinations: formula AND "
                    "signed-strip positivity both hold: %s\n\n", label,
                    ok_count, total,
                    ok_count == total ? "ALL HOLD"
                                      : "SOME FAIL -- see above");

        // Every margin width +/- height is affine in m.  Therefore, for
        // each fixed a, positivity on the full interval
        //   2 <= m <= a-domain_offset
        // follows from its two endpoints.  Keep a symbolic at both endpoints
        // and prove each resulting polynomial positive for every
        // a>=2+domain_offset by the same shift/nonnegative-coefficient
        // certificate.  This removes the finite m<=40 sweep from the theorem.
        bool all_m_symbolic = true;
        const long long domain_a = 2 + domain_offset;
        const PolyZ minus_one = affine_a(-1);
        for (const auto& t : cases) {
            const PolyZ p_at_2 = affine_a(2 * t.pm + t.p0);
            const PolyZ q_at_2 = affine_a(2 * t.qm + t.q0);
            const PolyZ p_at_upper =
                affine_a(t.p0 - t.pm * domain_offset, t.pm);
            const PolyZ q_at_upper =
                affine_a(t.q0 - t.qm * domain_offset, t.qm);
            for (const auto& endpoint : {
                     closed_form_affine(p_at_2, q_at_2, t.r),
                     closed_form_affine(p_at_upper, q_at_upper, t.r)}) {
                const BivPoly width = b2_width(t.w);
                const auto signed_lower = find_certificate(
                    biv_sub(width, biv_scale(endpoint, minus_one)),
                    cubic, domain_a);
                const auto signed_upper = find_certificate(
                    biv_sub(width, endpoint), cubic, domain_a);
                all_m_symbolic = signed_lower.found
                              && signed_upper.found
                              && all_m_symbolic;
            }
        }
        std::printf("%s: symbolic endpoint proof for every admissible "
                    "(a,m): %s\n\n", label,
                    all_m_symbolic ? "PROVEN"
                                   : "FAILED -- finite sweep is not a proof");
        return ok_count == total && all_m_symbolic;
    };

    const bool neighbor0_ok = run_neighbor("neighbor 0", templates, 2);
    const bool neighbor1_ok = run_neighbor("neighbor 1", neighbor1_templates, 1);
    std::printf("Two-direction hop result: %s\n",
                neighbor0_ok && neighbor1_ok
                    ? "THE SAME CLOSED FORM AND CERTIFICATE WORK FOR BOTH"
                    : "A NEIGHBOR BREAKS THE TRANSPORT -- INSPECT ABOVE");

    // Now leave the fixed-incidence fiber.  Tribonacci is still a
    // three-letter unimodular Pisot substitution, but its incidence matrix,
    // characteristic polynomial, and Perron direction differ.  The special
    // Class-II identity b*(b-c)=c should fail, while exact reduction modulo
    // the new characteristic polynomial should still produce a universal
    // linear closed form.  This locates the wall: not at word order, but at
    // the number-field/matrix stratum.
    const PolyZ trib_cubic = tribonacci_cubic();
    const PolyZ trib_b = monomial(1, 1);
    const PolyZ trib_c = monomial(1, 2) - monomial(1, 1);
    const PolyZ old_identity = trib_b * (trib_b - trib_c) - trib_c;
    const PolyZ old_identity_remainder =
        reduce_univariate(old_identity, trib_cubic);
    // The incidence-column difference predicts c-1, so add 1 to
    // old_identity=b*(b-c)-c.
    const PolyZ tribonacci_identity = old_identity + monomial(1, 0);
    const PolyZ tribonacci_identity_remainder =
        reduce_univariate(tribonacci_identity, trib_cubic);

    const std::vector<std::array<long long, 3>> probes = {
        {-3, 3, -1}, {-2, 2, 0}, {-1, 2, -1}, {0, 1, 0},
        {1, -1, 0}, {1, -1, 1}, {2, -2, -1}, {7, -3, 2},
    };
    bool tribonacci_formula_ok = true;
    for (const auto& pqr : probes) {
        const PolyZ direct = reduce_univariate(
            tribonacci_direct(pqr[0], pqr[1], pqr[2]), trib_cubic);
        const PolyZ claimed = tribonacci_closed_form(
            pqr[0], pqr[1], pqr[2]);
        tribonacci_formula_ok =
            polyz_equal(direct, claimed) && tribonacci_formula_ok;
    }
    std::printf("\nMatrix-changing hop (Tribonacci):\n");
    std::printf("  Class-II identity b*(b-c)=c transports: %s "
                "(exact remainder: %s)\n",
                old_identity_remainder.is_zero() ? "YES" : "NO",
                mathlib::str(old_identity_remainder).c_str());
    std::printf("  Tribonacci replacement b*(b-c)=c-1: %s "
                "(exact remainder: %s)\n",
                tribonacci_identity_remainder.is_zero() ? "YES" : "NO",
                mathlib::str(tribonacci_identity_remainder).c_str());
    std::printf("  characteristic-polynomial reduction transports: %s "
                "(%zu/%zu exact arbitrary triples)\n",
                tribonacci_formula_ok ? "YES" : "NO",
                tribonacci_formula_ok ? probes.size() : 0U, probes.size());
    std::printf("  boundary located: the clean identity is stratum-specific; "
                "the reduction bazooka is not.\n");

    // Derive the structure constants rather than recognizing identities by
    // eye.  Both bases are scaled LEFT Perron covectors, matching
    // in_H_sigma. M^T*v=beta*v says multiplication by beta=b has coordinate
    // matrix M itself, so the identity is literally col_0-col_1.
    const long long displayed_a = 7;
    const PolyZ class_cubic = monomial(1, 3)
                            - monomial(displayed_a, 2)
                            - monomial(displayed_a + 1, 1)
                            - monomial(1, 0);
    const PolyZ class_b = monomial(1, 1);
    const PolyZ class_c = monomial(1, 2)
                        - monomial(displayed_a, 1)
                        - monomial(1, 0);
    const auto class_difference = print_multiplication_tensor(
        "Class II (a=7)", class_cubic, class_b, class_c);
    const auto tribonacci_difference = print_multiplication_tensor(
        "Tribonacci", trib_cubic, trib_b, trib_c);
    // For companion cubics beta^3=A*beta^2+B*beta+1, the acceptance
    // covector basis is (b,c,1)=(beta,beta^2-A*beta,1). Incidence-column
    // subtraction gives b*(b-c)=(A-B)b+c-1.
    const auto companion32_difference = print_multiplication_tensor(
        "companion stratum (A=3,B=2)", companion_cubic(3, 2),
        monomial(1, 1), monomial(1, 2) - monomial(3, 1));
    const auto companion23_difference = print_multiplication_tensor(
        "companion stratum (A=2,B=3)", companion_cubic(2, 3),
        monomial(1, 1), monomial(1, 2) - monomial(2, 1));

    const Matrix3 class_matrix{{
        {{displayed_a, displayed_a, 1}}, {{1, 0, 0}}, {{1, 1, 0}}}};
    const Matrix3 tribonacci_matrix{{
        {{1, 1, 1}}, {{1, 0, 0}}, {{0, 1, 0}}}};
    const Matrix3 companion32_matrix{{
        {{3, 2, 1}}, {{1, 0, 0}}, {{0, 1, 0}}}};
    const Matrix3 companion23_matrix{{
        {{2, 3, 1}}, {{1, 0, 0}}, {{0, 1, 0}}}};

    const bool column_differences_ok =
        coordinates_equal(class_difference,
                          {class_matrix[0][0] - class_matrix[0][1],
                           class_matrix[1][0] - class_matrix[1][1],
                           class_matrix[2][0] - class_matrix[2][1]})
        && coordinates_equal(tribonacci_difference,
                          {tribonacci_matrix[0][0] - tribonacci_matrix[0][1],
                           tribonacci_matrix[1][0] - tribonacci_matrix[1][1],
                           tribonacci_matrix[2][0] - tribonacci_matrix[2][1]})
        && coordinates_equal(companion32_difference,
                          {companion32_matrix[0][0] - companion32_matrix[0][1],
                           companion32_matrix[1][0] - companion32_matrix[1][1],
                           companion32_matrix[2][0] - companion32_matrix[2][1]})
        && coordinates_equal(companion23_difference,
                          {companion23_matrix[0][0] - companion23_matrix[0][1],
                           companion23_matrix[1][0] - companion23_matrix[1][1],
                           companion23_matrix[2][0] - companion23_matrix[2][1]});
    const bool full_tensors_ok =
        beta_tensor_equals_incidence(class_matrix, class_cubic,
                                     class_b, class_c)
        && beta_tensor_equals_incidence(tribonacci_matrix, trib_cubic,
                                        trib_b, trib_c)
        && beta_tensor_equals_incidence(companion32_matrix,
                                        companion_cubic(3, 2),
                                        monomial(1, 1),
                                        monomial(1, 2) - monomial(3, 1))
        && beta_tensor_equals_incidence(companion23_matrix,
                                        companion_cubic(2, 3),
                                        monomial(1, 1),
                                        monomial(1, 2) - monomial(2, 1));
    const bool all_ok = neighbor0_ok && neighbor1_ok
        && !old_identity_remainder.is_zero()
        && tribonacci_identity_remainder.is_zero()
        && tribonacci_formula_ok && column_differences_ok && full_tensors_ok;
    std::printf("\nIncidence-column assertions: %s\n",
                column_differences_ok ? "ALL EXACT" : "FAIL");
    std::printf("Full multiplication-by-beta tensors equal M: %s\n",
                full_tensors_ok ? "ALL EXACT" : "FAIL");
    std::printf("Overall transport probe: %s\n", all_ok ? "PASS" : "FAIL");
    return all_ok ? 0 : 1;
}
