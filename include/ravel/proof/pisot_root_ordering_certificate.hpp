// ravel/proof/pisot_root_ordering_certificate.hpp
//
// Finding 29's constructive-accumulation claim: for the a-bonacci
// family (charpoly x^n - a(x^{n-1}+...+x+1)), the dominant root at
// n=3 is strictly less than the dominant root at n=4, which is
// strictly less than a+1. Previously checked only via floating-point
// midpoints (app/probe_pisot_accumulation_structure.cpp). This stages
// the SAME exact rational brackets `pisot_classify_3x3`/`_4x4`
// already certify (Sturm-chain isolation, no floating point) and
// checks the two gap inequalities exactly, so the reflection pipeline
// can hand Lean the concrete rational bounds rather than a decimal
// approximation.

#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>

#include "exact_pisot.h"
#include "math/proof_reflection.hpp"

namespace ravel::proof {

struct PisotBracket {
    long long lo_num = 0, lo_den = 1;
    long long hi_num = 0, hi_den = 1;
    bool ok = false;
};

// Extracts an mpz value as a long long, requiring it to fit exactly
// (the classifier's brackets are tol_bits=40, well within int64, but
// this is checked rather than assumed).
inline long long mpz_require_si(mpz_srcptr z) {
    if (!mpz_fits_slong_p(z)) throw std::overflow_error("pisot bracket value does not fit in long long");
    return mpz_get_si(z);
}

inline PisotBracket abonacci_bracket_n3(long long a) {
    long long M[3][3] = {{a, a, a}, {1, 0, 0}, {0, 1, 0}};
    pisot_info_t info;
    PisotBracket result;
    if (pisot_classify_3x3(M, &info) && info.is_pisot) {
        result.lo_num = mpz_require_si(info.beta_lo_num);
        result.lo_den = mpz_require_si(info.beta_lo_den);
        result.hi_num = mpz_require_si(info.beta_hi_num);
        result.hi_den = mpz_require_si(info.beta_hi_den);
        result.ok = true;
    }
    pisot_info_clear(&info);
    return result;
}

inline PisotBracket abonacci_bracket_n4(long long a) {
    long long M[4][4] = {{a, a, a, a}, {1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}};
    pisot_info_t info;
    PisotBracket result;
    if (pisot_classify_4x4(M, &info) && info.is_pisot) {
        result.lo_num = mpz_require_si(info.beta_lo_num);
        result.lo_den = mpz_require_si(info.beta_lo_den);
        result.hi_num = mpz_require_si(info.beta_hi_num);
        result.hi_den = mpz_require_si(info.beta_hi_den);
        result.ok = true;
    }
    pisot_info_clear(&info);
    return result;
}

// Stages the check hi3/den3 < lo4/den4 (via cross-multiplication in
// 128-bit arithmetic -- exact, no rounding, no overflow risk even
// though brackets can have denominators up to ~2^40) and
// hi4/den4 < (a+1), for the a-bonacci family's n=3 and n=4 dominant-
// root brackets. Records nothing unless both exact inequalities hold.
inline void stage_pisot_root_ordering(long long a) {
    const auto b3 = abonacci_bracket_n3(a);
    const auto b4 = abonacci_bracket_n4(a);
    if (!b3.ok || !b4.ok) return;
    // hi3/den3 < lo4/den4  <=>  hi3*den4 < lo4*den3  (all denominators positive).
    const bool gap_ok =
        static_cast<__int128>(b3.hi_num) * b4.lo_den
        < static_cast<__int128>(b4.lo_num) * b3.hi_den;
    // hi4/den4 < a+1  <=>  hi4 < (a+1)*den4.
    const bool bound_ok =
        static_cast<__int128>(b4.hi_num)
        < static_cast<__int128>(a + 1) * b4.hi_den;
    if (!gap_ok || !bound_ok) return;
    if (!mathlib::reflection::enabled()) return;
    mathlib::reflection::PisotRootOrderingCertificate node;
    node.a = a;
    node.hi3_num = b3.hi_num; node.hi3_den = b3.hi_den;
    node.lo4_num = b4.lo_num; node.lo4_den = b4.lo_den;
    node.hi4_num = b4.hi_num; node.hi4_den = b4.hi_den;
    mathlib::reflection::record(mathlib::reflection::NodeKind::LemmaApplication, node);
}

}  // namespace ravel::proof
