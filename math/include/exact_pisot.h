// exact_pisot.h - public C interface
#pragma once

#include "mini-gmp/mini-gmp.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Maximum polynomial degree this library's exact Sturm-chain/PRS
 * machinery supports. poly_t's internal coefficient array (private
 * to exact_pisot.c) is sized POLY_COEFFS_CAP = PISOT_MAX_POLY_DEGREE+1
 * slots to hold exactly this. This is the single source of truth for
 * that capacity: every public entry point that accepts a degree
 * enforces this bound against it, and exact_pisot.c's internal
 * POLY_COEFFS_CAP is defined in terms of it. Changing the supported
 * degree range means changing only this constant. */
#define PISOT_MAX_POLY_DEGREE 15

typedef struct {
    int is_pisot;
    mpz_ptr beta_lo_num, beta_lo_den;
    mpz_ptr beta_hi_num, beta_hi_den;
    int n_real_inside;
    int has_complex_pair;
    int is_complex_modulus_lt_1;
    mpz_ptr cm_lo_num, cm_lo_den;
    mpz_ptr cm_hi_num, cm_hi_den;
    mpz_ptr det_abs;
} pisot_info_t;

void pisot_info_clear(pisot_info_t* info);

/* Classify the characteristic polynomial of a 3x3 integer matrix as
 * Pisot.  On success returns nonzero and fills `out` (which the
 * caller must clear with pisot_info_clear). */
int pisot_classify_3x3(const long long M[3][3], pisot_info_t* out);

/* Classify the characteristic polynomial of a 4x4 integer matrix as
 * Pisot.  Same conventions as pisot_classify_3x3. */
int pisot_classify_4x4(const long long M[4][4], pisot_info_t* out);

/* Classify an arbitrary-degree (1..PISOT_MAX_POLY_DEGREE) monic
 * integer polynomial directly from coefficients (low-to-high,
 * coeffs[0..degree-1]; the leading coeffs[degree]=1 is implicit and
 * not included). Enforces the degree bound itself and returns 0
 * (failure) if it's violated -- callers do not need to check first.
 * Rigorous for any degree in range, but returns 0 (failure, not a
 * guess) whenever the polynomial has more than one complex-conjugate
 * pair -- see the comment at pisot_classify_degree_n's definition in
 * exact_pisot.c. */
int pisot_classify_degree_n(const long long* coeffs, int degree, pisot_info_t* out);

/* Render a rational num/den as a decimal string (10 digits past the
 * decimal point).  `buf` must be at least 64 bytes. */
void mpq_class_str(char* buf, int buf_size,
                   const mpz_t num, const mpz_t den);

/* =================================================================
 * Generic Sturm-isolated root (arbitrary integer polynomial).
 * =================================================================
 *
 * Given a polynomial f(x) of degree `degree` with integer
 * coefficients `coeffs[0..degree]` (low-to-high ordering), and a
 * known bracket (lo_num/lo_den, hi_num/hi_den) that contains
 * EXACTLY ONE real root of f, isolate that root to a bracket
 * (lo_out_*, hi_out_*) of width < 2^-tol_bits.  Returns 1 on
 * success, 0 on failure (no root in the bracket, multiple roots,
 * non-squarefree, degree out of range, etc.).
 *
 * This is a thin public entry point over the same Sturm-chain
 * machinery that pisot_classify_* uses internally — exposed for
 * callers that already know a root is bracketed and just want
 * it isolated to high precision (e.g. transfer-operator
 * eigenvalue certification, exact matrix-eigenvalue verification).
 *
 * Memory: on success, the four `mpz_ptr *out` parameters are set
 * to heap-allocated mpz_t values the caller must free
 * (mpz_clear + free).  The tolerance is in bits; tol_bits=300
 * is ~90 decimal digits.
 */
int isolate_real_root_generic_mpz(mpz_srcptr* coeffs, int degree,
                                  long long lo_num, long long lo_den,
                                  long long hi_num, long long hi_den,
                                  int tol_bits,
                                  mpz_ptr* lo_out_num, mpz_ptr* lo_out_den,
                                  mpz_ptr* hi_out_num, mpz_ptr* hi_out_den);

#ifdef __cplusplus
}
#endif