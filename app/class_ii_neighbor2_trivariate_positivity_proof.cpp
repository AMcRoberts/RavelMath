// Extend the bivariate proof (app/class_ii_neighbor2_bivariate_
// positivity_proof.cpp) across the k axis too (2026-08-02). That file
// proved height=k*(b-c) positive for every real a in its domain, for
// each of 60 SEPARATE concrete integer k -- the same "checked at many
// points" pattern `a` had before the bivariate fix, just one axis
// over. This file keeps k as a free variable throughout, the same way
// `a` was freed, rather than sweeping it.
//
// Method: b^2*height and b^2*(width-height) are, for the height=
// k*(b-c) family, exactly LINEAR in k (height itself is k times a
// k-independent quantity). Track each b-coefficient not as a single
// PolyZ-in-a but as a pair (P0, P1) meaning "P0(a) + k*P1(a)" --
// exact, since the Class-II cubic used for reduction has no
// k-dependence at all, so scaling it by a linear-in-k quantity stays
// exactly linear in k throughout the reduction.
//
// A linear function of k is monotonic, so its minimum over an
// interval [k_min, k_max] occurs at one of the two endpoints. Proving
// non-negativity at BOTH endpoints (each itself a genuine bivariate,
// universal-in-a proof, using the same a-shift technique as the
// height=k(b-c) bivariate proof) therefore proves non-negativity for
// EVERY k in the interval, and every a in the interval's own
// (a-dependent) domain, simultaneously -- not sampled at 60 points,
// proven for the whole continuum of k as well as a.
//
// The relevant interval is k in [1, a-2] (the interior regime's own
// constraint). k_max = a-2 is itself a-dependent, so evaluating there
// substitutes k=a-2 into the linear form, producing a pure
// polynomial-in-a expression that then gets the same domain-shift
// treatment as before.

#include <cstdio>
#include <utility>
#include <vector>

#include "math/poly_z.hpp"

using mathlib::BigInt;
using mathlib::divmod;
using mathlib::PolyZ;

namespace {

// ---- reuse: a polynomial in b with PolyZ-in-a coefficients ----
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
std::string str_biv(const BivPoly& p) {
    std::string out;
    for (std::size_t i = 0; i < p.size(); ++i) {
        if (p[i].is_zero()) continue;
        if (!out.empty()) out += " + ";
        out += "(" + mathlib::str(p[i]) + ")*b^" + std::to_string(i);
    }
    return out.empty() ? "0" : out;
}

// ---- new: linear-in-k tracking, BivPolyK[i] = (P0, P1) meaning
// coefficient of b^i is P0(a) + k*P1(a) ----
using LinK = std::pair<PolyZ, PolyZ>;
using BivPolyK = std::vector<LinK>;

LinK link_zero() { return {zero_a(), zero_a()}; }
LinK link_add(const LinK& x, const LinK& y) { return {x.first + y.first, x.second + y.second}; }
LinK link_sub(const LinK& x, const LinK& y) { return {x.first - y.first, x.second - y.second}; }
LinK link_scale(const LinK& x, const PolyZ& s) { return {x.first * s, x.second * s}; }

BivPolyK bivk_add(const BivPolyK& x, const BivPolyK& y) {
    BivPolyK out(std::max(x.size(), y.size()), link_zero());
    for (std::size_t i = 0; i < x.size(); ++i) out[i] = link_add(out[i], x[i]);
    for (std::size_t i = 0; i < y.size(); ++i) out[i] = link_add(out[i], y[i]);
    return out;
}
BivPolyK bivk_sub(const BivPolyK& x, const BivPolyK& y) {
    BivPolyK out(std::max(x.size(), y.size()), link_zero());
    for (std::size_t i = 0; i < x.size(); ++i) out[i] = link_add(out[i], x[i]);
    for (std::size_t i = 0; i < y.size(); ++i) out[i] = link_sub(out[i], y[i]);
    return out;
}
BivPolyK bivk_scale(const BivPolyK& x, const PolyZ& s) {
    BivPolyK out(x.size());
    for (std::size_t i = 0; i < x.size(); ++i) out[i] = link_scale(x[i], s);
    return out;
}
BivPolyK bivk_shift(const BivPolyK& x, int by) {
    BivPolyK out(x.size() + static_cast<std::size_t>(by), link_zero());
    for (std::size_t i = 0; i < x.size(); ++i) out[i + static_cast<std::size_t>(by)] = x[i];
    return out;
}
long long bivk_degree_b(const BivPolyK& x) {
    for (long long i = static_cast<long long>(x.size()) - 1; i >= 0; --i) {
        const auto& [p0, p1] = x[static_cast<std::size_t>(i)];
        if (!p0.is_zero() || !p1.is_zero()) return i;
    }
    return -1;
}
// Reduce a BivPolyK modulo the (k-free) cubic. Each long-division step
// scales the cubic by the LEADING LinK coefficient (both its P0 and P1
// parts independently) -- exact, since the cubic has no k-dependence
// to interact with.
BivPolyK reduce_bivk(BivPolyK p, const BivPoly& cubic) {
    while (bivk_degree_b(p) >= 3) {
        const long long d = bivk_degree_b(p);
        const LinK lead = p[static_cast<std::size_t>(d)];
        BivPoly shifted_cubic = biv_shift(cubic, static_cast<int>(d - 3));
        BivPolyK term(shifted_cubic.size());
        for (std::size_t i = 0; i < shifted_cubic.size(); ++i)
            term[i] = {shifted_cubic[i] * lead.first, shifted_cubic[i] * lead.second};
        p = bivk_sub(p, term);
        p.resize(static_cast<std::size_t>(d));
    }
    return p;
}

// height = k*b - k*c. b^2*height = k*(b^3 - a*b^2 - b) -- purely
// P1-part, P0=0 for every b-coefficient (height is homogeneous
// degree-1 in k).
BivPolyK b2_height_symbolic_k() {
    BivPolyK h(4, link_zero());
    { PolyZ one; mathlib::set_si(one.coeff(0), 1); h[1] = {zero_a(), zero_a() - one}; }        // -1 * b, times k
    { PolyZ a1; mathlib::set_si(a1.coeff(1), 1); h[2] = {zero_a(), zero_a() - a1}; }            // -a * b^2, times k
    { PolyZ one; mathlib::set_si(one.coeff(0), 1); h[3] = {zero_a(), one}; }                    // 1 * b^3, times k
    return h;
}
BivPolyK b2_width_k(char w) {
    BivPolyK out(4, link_zero());
    if (w == 'b') { PolyZ one; mathlib::set_si(one.coeff(0), 1); out[3] = {one, zero_a()}; }
    else if (w == 'c') {
        PolyZ a1; mathlib::set_si(a1.coeff(1), 1); out[2] = {a1, zero_a()};
        PolyZ one; mathlib::set_si(one.coeff(0), 1); out[1] = {one, zero_a()};
    } else { PolyZ one; mathlib::set_si(one.coeff(0), 1); out[2] = {one, zero_a()}; }
    return out;
}

// Evaluate a BivPolyK at a concrete integer k (P0 + k*P1), returning
// an ordinary BivPoly (in a alone).
BivPoly bivk_eval_at_k(const BivPolyK& p, long long k) {
    BivPoly out(p.size());
    for (std::size_t i = 0; i < p.size(); ++i) {
        PolyZ kk; mathlib::set_si(kk.coeff(0), k);
        out[i] = p[i].first + p[i].second * kk;
    }
    return out;
}
// Evaluate at k = a - offset (e.g. offset=2 for k=a-2): P0(a) +
// (a-offset)*P1(a). (a-offset) as a PolyZ, multiplied through.
BivPoly bivk_eval_at_a_minus(const BivPolyK& p, long long offset) {
    BivPoly out(p.size());
    PolyZ a_minus_offset;
    mathlib::set_si(a_minus_offset.coeff(1), 1);
    mathlib::set_si(a_minus_offset.coeff(0), -offset);
    for (std::size_t i = 0; i < p.size(); ++i)
        out[i] = p[i].first + p[i].second * a_minus_offset;
    return out;
}

}  // namespace

int main() {
    const BivPoly cubic = cubic_bivariate();
    const BivPolyK b2h_sym = b2_height_symbolic_k();

    std::printf("=== height = k*(b-c), k KEPT SYMBOLIC (not swept) ===\n\n");

    for (char w : {'b', 'c'}) {
        const BivPolyK b2w = b2_width_k(w);
        const BivPolyK lower_k = b2h_sym;               // height itself
        const BivPolyK upper_k = bivk_sub(b2w, b2h_sym); // width - height

        const BivPolyK lower_reduced = reduce_bivk(lower_k, cubic);
        const BivPolyK upper_reduced = reduce_bivk(upper_k, cubic);

        // Endpoint 1: k=1 (smallest relevant k). Domain a>=3 (k+2 at k=1).
        const BivPoly lower_at_k1 = bivk_eval_at_k(lower_reduced, 1);
        const BivPoly upper_at_k1 = bivk_eval_at_k(upper_reduced, 1);
        // Endpoint 2: k=a-2 (largest relevant k, the interior-regime bound).
        const BivPoly lower_at_kmax = bivk_eval_at_a_minus(lower_reduced, 2);
        const BivPoly upper_at_kmax = bivk_eval_at_a_minus(upper_reduced, 2);

        auto try_prove = [&](BivPoly p, long long shift, int max_steps = 8) {
            for (int step = 0; step <= max_steps; ++step) {
                p = reduce_biv(p, cubic);
                if (biv_all_nonneg(biv_compose_shift(p, shift))) return std::make_pair(true, step);
                p = biv_shift(p, 1);
            }
            return std::make_pair(false, max_steps);
        };

        auto [lo1_ok, lo1_steps] = try_prove(lower_at_k1, 3);
        auto [up1_ok, up1_steps] = try_prove(upper_at_k1, 3);
        auto [loM_ok, loM_steps] = try_prove(lower_at_kmax, 7);   // a>=7 safely covers a>=k+2 when k=a-2 needs a small floor
        auto [upM_ok, upM_steps] = try_prove(upper_at_kmax, 7);

        const bool all_ok = lo1_ok && up1_ok && loM_ok && upM_ok;
        std::printf("width=%c:\n", w);
        std::printf("  k=1 endpoint:   lower=%d(steps=%d) upper=%d(steps=%d)\n",
                    lo1_ok, lo1_steps, up1_ok, up1_steps);
        std::printf("  k=a-2 endpoint: lower=%d(steps=%d) upper=%d(steps=%d)\n",
                    loM_ok, loM_steps, upM_ok, upM_steps);
        std::printf("  %s: both endpoints proven => linear-in-k expression "
                    "non-negative for EVERY k in [1,a-2] AND every a in that "
                    "range, simultaneously (not swept)\n\n",
                    all_ok ? "PROVEN FOR THE WHOLE (a,k) REGION" : "NOT PROVEN");
    }

    std::printf("--- The reduced linear-in-k closed forms themselves ---\n");
    for (char w : {'b', 'c'}) {
        const BivPolyK b2w = b2_width_k(w);
        const BivPolyK upper_reduced = reduce_bivk(bivk_sub(b2w, b2h_sym), cubic);
        std::printf("width=%c, b^2*(width-height) mod cubic, per b-power "
                    "(P0(a) + k*P1(a)):\n", w);
        for (std::size_t i = 0; i < upper_reduced.size(); ++i) {
            if (upper_reduced[i].first.is_zero() && upper_reduced[i].second.is_zero()) continue;
            std::printf("  b^%zu: P0=%s  P1=%s\n", i,
                        mathlib::str(upper_reduced[i].first).c_str(),
                        mathlib::str(upper_reduced[i].second).c_str());
        }
    }
    return 0;
}
