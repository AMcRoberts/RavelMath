// math/tests/test_common.hpp
//
// Common test helpers: counters (n_pass, n_fail) and EXPECT macros.
// Each tier test file includes this first; counters are shared across
// the tier tests when the mathlib_test driver #includes all tiers
// into a single translation unit.

#pragma once

#include <cstdio>
#include <cstdlib>
#include <string>

#include "math/bigint.hpp"
#include "math/qbeta.hpp"

namespace mathlib_test {

inline int n_pass = 0;
inline int n_fail = 0;

}  // namespace mathlib_test

// Common test helpers used across tier tests.
inline mathlib::BigInt B(long long x) { mathlib::BigInt r; mathlib::set_si(r, x); return r; }
inline mathlib::Rat Q_(long long n, long long d = 1) { mathlib::Rat r; mathlib::set_si(r, n, d); return r; }
inline mathlib::QElem qbeta_vec(std::initializer_list<std::pair<long long,long long>> rcs) {
    mathlib::QElem r; r.coeffs_.clear();
    for (auto [n, d] : rcs) { r.coeffs_.push_back(mathlib::Rat(n, d)); }
    return r;
}

#define EXPECT(cond, msg) do { \
    if (cond) { ++::mathlib_test::n_pass; } \
    else { ++::mathlib_test::n_fail; std::fprintf(stderr, "FAIL: %s (line %d)\n", msg, __LINE__); } \
} while (0)

#define EXPECT_EQ_BI_RAW(a, b) do { \
    if (cmp((a), (b)) == 0) { ++::mathlib_test::n_pass; } \
    else { ++::mathlib_test::n_fail; std::fprintf(stderr, "FAIL: %s == %s ?  line %d\n", \
        str(a).c_str(), str(b).c_str(), __LINE__); } \
} while (0)

#define EXPECT_EQ_PZ(a, b) do { \
    if ((a) == (b)) { ++::mathlib_test::n_pass; } \
    else { ++::mathlib_test::n_fail; std::fprintf(stderr, "FAIL: %s == %s ?  line %d\n", \
        str(a).c_str(), str(b).c_str(), __LINE__); } \
} while (0)

#define EXPECT_EQ_Q(a, b) do { \
    if ((a) == (b)) { ++::mathlib_test::n_pass; } \
    else { ++::mathlib_test::n_fail; std::fprintf(stderr, "FAIL: %s == %s ?  line %d\n", \
        str(a).c_str(), str(b).c_str(), __LINE__); } \
} while (0)

#define EXPECT_EQ_QELEM(a, b) do { \
    if ((a) == (b)) { ++::mathlib_test::n_pass; } \
    else { ++::mathlib_test::n_fail; std::fprintf(stderr, "FAIL: %s == %s ?  line %d\n", \
        str(a).c_str(), str(b).c_str(), __LINE__); } \
} while (0)

#define EXPECT_EQ_BI(a, b) do { \
    BigInt _b; set_si(_b, (b)); \
    if (cmp((a), _b) == 0) { ++::mathlib_test::n_pass; } \
    else { ++::mathlib_test::n_fail; std::fprintf(stderr, "FAIL: %s == %lld ?  line %d\n", \
        str(a).c_str(), (long long)(b), __LINE__); } \
} while (0)

#define EXPECT_EQ_RAT(a, b) do { \
    Rat _b; set_si(_b, (b), 1); \
    if (cmp((a), _b) == 0) { ++::mathlib_test::n_pass; } \
    else { ++::mathlib_test::n_fail; std::fprintf(stderr, "FAIL: %s == %lld ?  line %d\n", \
        str(a).c_str(), (long long)(b), __LINE__); } \
} while (0)

#define EXPECT_EQ_MAT(a, b) do { \
    if ((a) == (b)) { ++::mathlib_test::n_pass; } \
    else { ++::mathlib_test::n_fail; std::fprintf(stderr, "FAIL: %s != %s (line %d)\n", \
        str(a).c_str(), str(b).c_str(), __LINE__); } \
} while (0)
