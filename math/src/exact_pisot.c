/* exact_pisot.c
 *
 * Exact Pisot polynomial classification using Sturm sequencing.
 * Pure C; uses mini-gmp's mpz_t for the output interface and
 * mini-mpq's mpq_t internally for the polynomial ring.
 *
 * API:
 *   int pisot_classify_3x3(const long long M[3][3],
 *                          pisot_info_t *out);
 *
 *   Computes the characteristic polynomial det(λI - M) of a 3x3
 *   integer matrix, then classifies it as Pisot via Sturm-based
 *   root isolation.  On success, fills `out` with:
 *     - is_pisot (1 if Pisot)
 *     - beta_lo_num/den, beta_hi_num/den: isolating interval for β
 *     - n_real_inside: number of real roots strictly in (-1, 1)
 *     - has_complex_pair: 1 if there's at least one complex pair
 *     - is_complex_modulus_lt_1: 1 if the complex |.|² < 1
 *     - det_abs (integer)
 *
 *   Returns 0 on failure.
 *
 * Why mpq_t internally: Sturm sequencing requires Euclidean polynomial
 * division by the leading coefficient of the divisor.  In Z, the
 * divisor's leading coefficient may not divide the dividend's leading
 * coefficient evenly (e.g. f' = 3x² + ... for a cubic Pisot char poly),
 * so division over Z doesn't terminate (the leading coefficient of r
 * never reaches 0).  Switching to Q arithmetic makes the division
 * always exact.  The signs of the Sturm chain at evaluation points
 * are preserved by the transition from Z to Q (any positive scalar
 * multiplies preserve sign), so the Sturm sign-change count V(x) is
 * the same in either ring.
 *
 * The output interface keeps mpz_ptr (num/den pairs) for compatibility
 * with the existing test driver.
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "exact_pisot.h"
#include "mini-gmp/mini-gmp.h"
#include "mini-gmp/mini-mpq.h"

/* =================================================================
 * Heap-allocated mpz/mpq helpers
 * =================================================================
 *
 * mini-gmp uses stack mpz_t[1] / mpq_t[1] by convention; we heap-
 * allocate so polynomials are plain value types that can be passed
 * by pointer and copied.
 */

static mpz_ptr mpz_new(void) {
    mpz_ptr p = (mpz_ptr)malloc(sizeof(__mpz_struct));
    mpz_init(p);
    return p;
}
static mpz_ptr mpz_new_si(long long v) {
    mpz_ptr p = (mpz_ptr)malloc(sizeof(__mpz_struct));
    mpz_init_set_si(p, v);
    return p;
}
static mpz_ptr mpz_new_copy(const mpz_t v) {
    mpz_ptr p = (mpz_ptr)malloc(sizeof(__mpz_struct));
    mpz_init_set(p, v);
    return p;
}
static void mpz_drop(mpz_ptr* p) {
    if (*p) { mpz_clear(*p); free(*p); *p = NULL; }
}

static mpq_ptr mpq_new(void) {
    mpq_ptr q = (mpq_ptr)malloc(sizeof(__mpq_struct));
    mpq_init(q);
    return q;
}
static mpq_ptr mpq_new_si(long num, unsigned long den) {
    mpq_ptr q = (mpq_ptr)malloc(sizeof(__mpq_struct));
    mpq_init(q);
    mpq_set_si(q, num, den);
    mpq_canonicalize(q);
    return q;
}
static void mpq_drop(mpq_ptr* p) {
    if (*p) { mpq_clear(*p); free(*p); *p = NULL; }
}

/* =================================================================
 * Polynomial ring over Q
 * ================================================================= */

typedef struct {
    mpq_ptr coeffs[16];  /* up to degree 15 */
    int degree;
} poly_t;

static void poly_init(poly_t* p) {
    memset(p, 0, sizeof(*p));
    p->degree = -1;
}
static void poly_clear(poly_t* p) {
    for (int i = 0; i <= p->degree; ++i) mpq_drop(&p->coeffs[i]);
}
static void poly_set_coeff(poly_t* p, int i, const mpq_t v) {
    while (p->degree < i) {
        int j = ++p->degree;
        p->coeffs[j] = mpq_new();
    }
    mpq_set(p->coeffs[i], v);
    mpq_canonicalize(p->coeffs[i]);
}
static void poly_set_si(poly_t* p, int i, long num, unsigned long den) {
    while (p->degree < i) {
        int j = ++p->degree;
        p->coeffs[j] = mpq_new();
    }
    mpq_set_si(p->coeffs[i], num, den);
    mpq_canonicalize(p->coeffs[i]);
}
static int poly_is_zero(const poly_t* p) {
    if (p->degree < 0) return 1;
    for (int i = 0; i <= p->degree; ++i)
        if (mpq_sgn(p->coeffs[i]) != 0) return 0;
    return 1;
}
static int poly_degree(const poly_t* p) {
    int d = p->degree;
    while (d >= 0 && mpq_sgn(p->coeffs[d]) == 0) --d;
    return d;
}
static void poly_normalize(poly_t* p) {
    while (p->degree >= 0 && mpq_sgn(p->coeffs[p->degree]) == 0) {
        mpq_drop(&p->coeffs[p->degree]);
        p->degree--;
    }
}

/* Polynomial division over Q.  Properly divides by b[db] at each
 * step, so it terminates for any non-zero divisor. */
static void poly_divmod(const poly_t* a, const poly_t* b,
                        poly_t* q, poly_t* r) {
    poly_init(r);
    for (int i = 0; i <= a->degree; ++i) poly_set_coeff(r, i, a->coeffs[i]);
    poly_init(q);
    int db = poly_degree(b);
    if (db < 0) return;
    if (r->degree < db) return;

    mpq_ptr lc_inv = mpq_new();
    mpq_inv(lc_inv, b->coeffs[db]);  /* 1 / b[db] */
    mpq_ptr t1 = mpq_new();

    while (!poly_is_zero(r) && r->degree >= db) {
        int dr = poly_degree(r);
        int shift = dr - db;
        if (q->degree < shift) {
            while (q->degree < shift) {
                int j = ++q->degree;
                q->coeffs[j] = mpq_new();
            }
        }
        /* q[shift] += r[dr] / b[db] */
        mpq_mul(t1, r->coeffs[dr], lc_inv);
        mpq_add(q->coeffs[shift], q->coeffs[shift], t1);
        /* r -= (r[dr] / b[db]) * b * x^shift
         * Equivalently: r -= q[shift] * b * x^shift
         *               r[shift + j] -= q[shift] * b[j] for j = 0..db */
        for (int j = 0; j <= db; ++j) {
            mpq_mul(t1, q->coeffs[shift], b->coeffs[j]);
            mpq_sub(r->coeffs[shift + j], r->coeffs[shift + j], t1);
        }
        poly_normalize(r);
    }
    mpq_drop(&lc_inv); mpq_drop(&t1);
}

static int poly_is_squarefree(const poly_t* f) {
    int n = f->degree;
    if (n < 1) return 1;
    /* Formal derivative over Q: c'_i = (i+1) * c_{i+1} */
    poly_t fp;
    poly_init(&fp);
    for (int i = 0; i < n; ++i) {
        mpq_ptr t = mpq_new();
        mpq_ptr k = mpq_new_si(i + 1, 1);
        mpq_mul(t, k, f->coeffs[i+1]);
        poly_set_coeff(&fp, i, t);
        mpq_drop(&t); mpq_drop(&k);
    }
    /* g = gcd(f, fp); check degree of g */
    poly_t a = *f, b = fp;
    while (!poly_is_zero(&b)) {
        poly_t q, r;
        poly_divmod(&a, &b, &q, &r);
        a = b;
        b = r;
        poly_clear(&q);
    }
    int g_degree = poly_degree(&a);
    poly_clear(&a); poly_clear(&fp);
    return g_degree == 0;
}

/* Evaluate f at rational x = x_num / x_den using Horner.
 * Returns result as an mpq_t. */
static void poly_eval_mpq(mpq_t result, const poly_t* p,
                          const mpz_t x_num, const mpz_t x_den) {
    mpq_ptr x = mpq_new();
    mpq_set_num(x, x_num);
    mpq_set_den(x, x_den);
    mpq_canonicalize(x);

    mpq_ptr acc = mpq_new();
    mpq_set_ui(acc, 0, 1);
    for (int k = p->degree; k >= 0; --k) {
        mpq_mul(acc, acc, x);
        mpq_add(acc, acc, p->coeffs[k]);
    }
    mpq_set(result, acc);
    mpq_drop(&x); mpq_drop(&acc);
}

static int mpq_sgn_at(const poly_t* p, const mpz_t x_num, const mpz_t x_den) {
    mpq_ptr v = mpq_new();
    poly_eval_mpq(v, p, x_num, x_den);
    int s = mpq_sgn(v);
    mpq_drop(&v);
    return s;
}

/* =================================================================
 * Sturm chain + sign-change counting
 * ================================================================= */

/* Build Sturm chain of f: seq[0] = f, seq[1] = f',
 * seq[i+1] = -rem(seq[i-1], seq[i]).  Stop when remainder is
 * constant or zero. */
static int sturm_chain_build(poly_t* seq, const poly_t* f) {
    if (f->degree <= 0) return 0;
    poly_init(&seq[0]);
    for (int i = 0; i <= f->degree; ++i)
        poly_set_coeff(&seq[0], i, f->coeffs[i]);

    /* Derivative over Q. */
    poly_init(&seq[1]);
    for (int i = 0; i < f->degree; ++i) {
        mpq_ptr t = mpq_new();
        mpq_ptr k = mpq_new_si(i + 1, 1);
        mpq_mul(t, k, f->coeffs[i+1]);
        poly_set_coeff(&seq[1], i, t);
        mpq_drop(&t); mpq_drop(&k);
    }
    poly_normalize(&seq[1]);

    int len = 2;
    while (len < 16) {
        poly_t q, r;
        poly_divmod(&seq[len-2], &seq[len-1], &q, &r);
        if (poly_is_zero(&r)) {
            poly_clear(&q); poly_clear(&r);
            break;
        }
        /* Negate r for the Sturm sign convention. */
        for (int i = 0; i <= r.degree; ++i) mpq_neg(r.coeffs[i], r.coeffs[i]);
        poly_init(&seq[len]);
        for (int i = 0; i <= r.degree; ++i)
            poly_set_coeff(&seq[len], i, r.coeffs[i]);
        poly_normalize(&seq[len]);
        len++;
        poly_clear(&q); poly_clear(&r);
        if (seq[len-1].degree == 0) break;
    }
    return len;
}

/* Sturm sign-change count at rational x.  Zeros in the chain are
 * skipped (do not contribute to changes), but the previous non-zero
 * sign is preserved across a zero entry — so a sign change across a
 * zero is still counted. */
static int sturm_V(poly_t* seq, int len,
                   const mpz_t x_num, const mpz_t x_den) {
    int changes = 0;
    int prev_sign = 0;
    for (int i = 0; i < len; ++i) {
        int sg = mpq_sgn_at(&seq[i], x_num, x_den);
        if (sg != 0) {
            if (prev_sign != 0 && sg != prev_sign) ++changes;
            prev_sign = sg;
        }
        /* if sg == 0, keep prev_sign unchanged */
    }
    return changes;
}

static int sturm_count(poly_t* seq, int len,
                       const mpz_t a_num, const mpz_t a_den,
                       const mpz_t b_num, const mpz_t b_den) {
    return sturm_V(seq, len, a_num, a_den) - sturm_V(seq, len, b_num, b_den);
}

/* =================================================================
 * Isolating intervals and root isolation via bisection
 * ================================================================= */

/* Rational endpoints stored as mpz_t pairs for compatibility
 * with the output struct (which uses mpz_ptr). */
typedef struct {
    mpz_ptr lo_num, lo_den, hi_num, hi_den;
} iv_t;

static int width_lt_tol(const iv_t* iv, int tol_bits) {
    /* width = (hi_n/hi_d) - (lo_n/lo_d); check width < 2^-tol. */
    mpz_ptr diff_n = mpz_new();
    mpz_ptr diff_d = mpz_new();
    mpz_ptr t = mpz_new();
    mpz_mul(diff_n, iv->hi_num, iv->lo_den);
    mpz_mul(t, iv->lo_num, iv->hi_den);
    mpz_sub(diff_n, diff_n, t);
    mpz_mul(diff_d, iv->hi_den, iv->lo_den);
    /* width * 2^tol < 1 <=> diff_n * 2^tol < diff_d */
    mpz_mul_2exp(t, diff_n, tol_bits);
    int lt = mpz_cmp(t, diff_d) < 0;
    mpz_drop(&diff_n); mpz_drop(&diff_d); mpz_drop(&t);
    return lt;
}

static iv_t* isolate_roots(poly_t* seq, int seq_len,
                            const mpz_t lo_num, const mpz_t lo_den,
                            const mpz_t hi_num, const mpz_t hi_den,
                            int tol_bits, int* out_count) {
    int cap = 16;
    int len = 0;
    iv_t* result = (iv_t*)malloc(cap * sizeof(iv_t));
    /* Stack of intervals. */
    iv_t* stack = (iv_t*)malloc(64 * sizeof(iv_t));
    int scap = 64, stack_len = 0;
    iv_t init;
    init.lo_num = mpz_new_copy(lo_num);
    init.lo_den = mpz_new_copy(lo_den);
    init.hi_num = mpz_new_copy(hi_num);
    init.hi_den = mpz_new_copy(hi_den);
    stack[stack_len++] = init;

    mpz_ptr mid_n = mpz_new_si(0);
    mpz_ptr mid_d = mpz_new_si(1);
    mpz_ptr t1 = mpz_new_si(0);
    mpz_ptr t2 = mpz_new_si(0);

    while (stack_len > 0) {
        iv_t cur = stack[--stack_len];
        int n = sturm_count(seq, seq_len,
                            cur.lo_num, cur.lo_den,
                            cur.hi_num, cur.hi_den);
        if (n == 0) {
            mpz_drop(&cur.lo_num); mpz_drop(&cur.lo_den);
            mpz_drop(&cur.hi_num); mpz_drop(&cur.hi_den);
            continue;
        }
        if (n == 1) {
            iv_t cur2 = cur;
            while (!width_lt_tol(&cur2, tol_bits)) {
                /* mid = (lo + hi) / 2 */
                mpz_mul(t1, cur2.lo_num, cur2.hi_den);
                mpz_mul(t2, cur2.hi_num, cur2.lo_den);
                mpz_add(mid_n, t1, t2);
                mpz_mul(mid_d, cur2.lo_den, cur2.hi_den);
                mpz_mul_2exp(mid_d, mid_d, 1);
                /* Simplify mid by gcd so digit growth stays bounded. */
                mpz_gcd(t1, mid_n, mid_d);
                if (mpz_sgn(t1) != 0) {
                    mpz_divexact(mid_n, mid_n, t1);
                    mpz_divexact(mid_d, mid_d, t1);
                }
                int nl = sturm_count(seq, seq_len,
                                     cur2.lo_num, cur2.lo_den,
                                     mid_n, mid_d);
                if (nl > 0) {
                    mpz_set(cur2.hi_num, mid_n);
                    mpz_set(cur2.hi_den, mid_d);
                } else {
                    mpz_set(cur2.lo_num, mid_n);
                    mpz_set(cur2.lo_den, mid_d);
                }
            }
            if (len >= cap) { cap *= 2; result = (iv_t*)realloc(result, cap * sizeof(iv_t)); }
            result[len++] = cur2;
        } else {
            /* Bisect */
            mpz_mul(t1, cur.lo_num, cur.hi_den);
            mpz_mul(t2, cur.hi_num, cur.lo_den);
            mpz_add(mid_n, t1, t2);
            mpz_mul(mid_d, cur.lo_den, cur.hi_den);
            mpz_mul_2exp(mid_d, mid_d, 1);
            iv_t left, right;
            left.lo_num = mpz_new_copy(cur.lo_num);
            left.lo_den = mpz_new_copy(cur.lo_den);
            left.hi_num = mpz_new_copy(mid_n);
            left.hi_den = mpz_new_copy(mid_d);
            right.lo_num = mpz_new_copy(mid_n);
            right.lo_den = mpz_new_copy(mid_d);
            right.hi_num = mpz_new_copy(cur.hi_num);
            right.hi_den = mpz_new_copy(cur.hi_den);
            if (stack_len + 2 >= scap) { scap *= 2; stack = (iv_t*)realloc(stack, scap * sizeof(iv_t)); }
            stack[stack_len++] = right;
            stack[stack_len++] = left;
            mpz_drop(&cur.lo_num); mpz_drop(&cur.lo_den);
            mpz_drop(&cur.hi_num); mpz_drop(&cur.hi_den);
        }
    }
    free(stack);
    mpz_drop(&mid_n); mpz_drop(&mid_d);
    mpz_drop(&t1); mpz_drop(&t2);
    /* Sort by lower endpoint. */
    for (int i = 1; i < len; ++i) {
        iv_t cur = result[i];
        int j = i;
        while (j > 0) {
            mpz_ptr lhs = mpz_new_si(0);
            mpz_ptr rhs = mpz_new_si(0);
            mpz_mul(lhs, cur.lo_num, result[j-1].lo_den);
            mpz_mul(rhs, result[j-1].lo_num, cur.lo_den);
            int cmp = mpz_cmp(lhs, rhs);
            mpz_drop(&lhs); mpz_drop(&rhs);
            if (cmp >= 0) break;
            result[j] = result[j-1];
            --j;
        }
        result[j] = cur;
    }
    *out_count = len;
    return result;
}

/* =================================================================
 * Output struct management
 * ================================================================= */

void pisot_info_clear(pisot_info_t* info) {
    mpz_drop(&info->beta_lo_num); mpz_drop(&info->beta_lo_den);
    mpz_drop(&info->beta_hi_num); mpz_drop(&info->beta_hi_den);
    mpz_drop(&info->cm_lo_num); mpz_drop(&info->cm_lo_den);
    mpz_drop(&info->cm_hi_num); mpz_drop(&info->cm_hi_den);
    mpz_drop(&info->det_abs);
}

void mpq_class_str(char* buf, int buf_size,
                   const mpz_t num, const mpz_t den) {
    double d_num = mpz_get_d(num);
    double d_den = mpz_get_d(den);
    double q = d_den != 0.0 ? d_num / d_den : 0.0;
    snprintf(buf, buf_size, "%.10g", q);
}

/* =================================================================
 * Characteristic polynomials for n×n integer matrices
 * =================================================================
 *
 * For an n×n integer matrix M, the characteristic polynomial
 * det(λI - M) is monic of degree n with integer coefficients:
 *
 *   λ^n - e_1 λ^(n-1) + e_2 λ^(n-2) - ... + (-1)^n e_n
 *
 * where e_k is the k-th elementary symmetric polynomial of the
 * eigenvalues, equivalently the sum of all k×k principal minors
 * of M.  Currently we implement the n=3 and n=4 cases (the two
 * sizes relevant to Pisot substitution classification in this
 * project).
 */

/* 3x3 determinant via Leibniz formula. */
static long long det_3x3(const long long* M) {
#define M3(row, col) M[(row) * 3 + (col)]
    long long det =
          M3(0, 0)*(M3(1, 1)*M3(2, 2) - M3(1, 2)*M3(2, 1))
        - M3(0, 1)*(M3(1, 0)*M3(2, 2) - M3(1, 2)*M3(2, 0))
        + M3(0, 2)*(M3(1, 0)*M3(2, 1) - M3(1, 1)*M3(2, 0));
#undef M3
    return det;
}

/* 4x4 determinant via cofactor expansion on the first row,
 * using det_3x3 for the 3x3 minors. */
static long long det_4x4(const long long* M) {
#define M4(row, col) M[(row) * 4 + (col)]
    long long det = 0;
    for (int j = 0; j < 4; ++j) {
        long long sub[3][3];
        int sr = 0;
        for (int r = 1; r < 4; ++r) {
            int sc = 0;
            for (int c = 0; c < 4; ++c) {
                if (c == j) continue;
                sub[sr][sc] = M4(r, c);
                ++sc;
            }
            ++sr;
        }
        long long sgn = (j % 2 == 0) ? 1 : -1;
        det += sgn * M4(0, j) * det_3x3(&sub[0][0]);
    }
#undef M4
    return det;
}

/* Compute characteristic polynomial coefficients for a 3x3 matrix.
 *   coeffs[0] = constant = -det
 *   coeffs[1] = sum of 2x2 principal minors
 *   coeffs[2] = -tr
 *   coeffs[3] = 1 (leading) */
static void char_poly_3x3(const long long M[3][3], long long coeffs[4]) {
    long long tr = M[0][0] + M[1][1] + M[2][2];
    long long e2 = M[0][0]*M[1][1] - M[0][1]*M[1][0]
                 + M[0][0]*M[2][2] - M[0][2]*M[2][0]
                 + M[1][1]*M[2][2] - M[1][2]*M[2][1];
    long long det = det_3x3(&M[0][0]);
    coeffs[0] = -det;
    coeffs[1] = e2;
    coeffs[2] = -tr;
    coeffs[3] = 1;
}

/* Compute characteristic polynomial coefficients for a 4x4 matrix.
 *   coeffs[0] = det (since (-1)^4 = +1)
 *   coeffs[1] = -e_3 (sum of 4 principal 3x3 minors)
 *   coeffs[2] = +e_2 (sum of 6 principal 2x2 minors)
 *   coeffs[3] = -e_1 = -tr
 *   coeffs[4] = 1 (leading) */
static void char_poly_4x4(const long long M[4][4], long long coeffs[5]) {
    /* e_1 = trace */
    long long e1 = M[0][0] + M[1][1] + M[2][2] + M[3][3];
    /* e_2 = sum of 6 principal 2x2 minors M[i][i]·M[j][j] - M[i][j]·M[j][i]
     * over all (i, j) with 0 ≤ i < j ≤ 3. */
    long long e2 = 0;
    for (int i = 0; i < 4; ++i) {
        for (int j = i + 1; j < 4; ++j) {
            e2 += M[i][i]*M[j][j] - M[i][j]*M[j][i];
        }
    }
    /* e_3 = sum of 4 principal 3x3 minors, one per "missing" index k:
     * minor_k = det of the 3x3 submatrix obtained by deleting row k
     * and column k. */
    long long e3 = 0;
    for (int k = 0; k < 4; ++k) {
        long long sub[3][3];
        int sr = 0;
        for (int r = 0; r < 4; ++r) {
            if (r == k) continue;
            int sc = 0;
            for (int c = 0; c < 4; ++c) {
                if (c == k) continue;
                sub[sr][sc] = M[r][c];
                ++sc;
            }
            ++sr;
        }
        e3 += det_3x3(&sub[0][0]);
    }
    /* e_4 = determinant */
    long long det = det_4x4(&M[0][0]);
    coeffs[0] = det;       /* (-1)^4 = +1 */
    coeffs[1] = -e3;
    coeffs[2] = e2;
    coeffs[3] = -e1;
    coeffs[4] = 1;
}

/* =================================================================
 * Generic Pisot classification (degree-generic driver)
 * ================================================================= */

/* Run the Pisot classifier on a characteristic polynomial whose
 * coefficients are given by coeffs[0..degree], where coeffs[degree]
 * is the leading coefficient (must be nonzero — the function
 * assumes monic).  Returns 0 on failure, 1 on success. */
static int pisot_classify_poly(const long long *coeffs, int degree,
                               pisot_info_t *out) {
    poly_t p;
    poly_init(&p);
    for (int i = 0; i < degree; ++i) {
        poly_set_si(&p, i, coeffs[i], 1);
    }
    poly_set_si(&p, degree, 1, 1);  /* leading */
    poly_normalize(&p);

    /* |det M| from coefficient parity: for odd degree, coeffs[0] = -det;
     * for even degree, coeffs[0] = +det.  But we want |det| so just
     * take absolute value of coeffs[0]. */
    long long abs_det_signed = coeffs[0] < 0 ? -coeffs[0] : coeffs[0];

    out->is_pisot = 0;
    out->beta_lo_num = mpz_new_si(0); out->beta_lo_den = mpz_new_si(1);
    out->beta_hi_num = mpz_new_si(0); out->beta_hi_den = mpz_new_si(1);
    out->n_real_inside = 0;
    out->has_complex_pair = 0;
    out->is_complex_modulus_lt_1 = 0;
    out->cm_lo_num = mpz_new_si(0); out->cm_lo_den = mpz_new_si(1);
    out->cm_hi_num = mpz_new_si(0); out->cm_hi_den = mpz_new_si(1);
    out->det_abs = mpz_new();
    mpz_set_si(out->det_abs, abs_det_signed);

    if (!poly_is_squarefree(&p)) {
        poly_clear(&p);
        return 0;
    }
    if (p.degree < 1) {
        poly_clear(&p);
        return 0;
    }

    /* Build Sturm chain. */
    poly_t seq[16];
    int slen = sturm_chain_build(seq, &p);
    if (slen < 2) {
        for (int i = 0; i < slen; ++i) poly_clear(&seq[i]);
        poly_clear(&p);
        return 0;
    }

    /* Cauchy bound: |root| < 1 + max_i |coeff_i|.  Char poly is
     * over Z so we use the integer numerators. */
    mpz_ptr max_abs = mpz_new_si(0);
    mpz_ptr tmp_abs = mpz_new();
    for (int i = 0; i < p.degree; ++i) {
        mpz_ptr num = mpq_numref(p.coeffs[i]);
        mpz_abs(tmp_abs, num);
        if (mpz_cmp(tmp_abs, max_abs) > 0) mpz_set(max_abs, tmp_abs);
    }
    mpz_ptr bound = mpz_new();
    mpz_add_ui(bound, max_abs, 1);
    mpz_ptr negbound = mpz_new();
    mpz_neg(negbound, bound);

    /* Count real roots in (-bound, bound]. */
    mpz_ptr one = mpz_new_si(1);
    int total_real = sturm_count(seq, slen, negbound, one, bound, one);
    int n_complex = (p.degree - total_real) / 2;
    out->has_complex_pair = (n_complex > 0);

    /* Isolate real roots in (-1, 1). */
    mpz_ptr neg_one = mpz_new_si(-1);
    int inside_count = 0;
    iv_t* inside = isolate_roots(seq, slen,
                                  neg_one, one, one, one,
                                  40, &inside_count);

    /* Verify each inside interval is strictly inside (-1, 1). */
    int ok = 1;
    for (int i = 0; i < inside_count; ++i) {
        /* lo > -1 iff lo_p + lo_q > 0 (since lo_q > 0). */
        mpz_ptr t1 = mpz_new();
        mpz_add(t1, inside[i].lo_num, inside[i].lo_den);
        int gt_neg1 = mpz_sgn(t1) > 0;
        mpz_drop(&t1);
        /* hi < 1 iff hi_p - hi_q < 0. */
        mpz_ptr t2 = mpz_new();
        mpz_sub(t2, inside[i].hi_num, inside[i].hi_den);
        int lt_pos1 = mpz_sgn(t2) < 0;
        mpz_drop(&t2);
        if (!gt_neg1 || !lt_pos1) { ok = 0; break; }
    }
    out->n_real_inside = inside_count;

    /* Isolate real roots in (1, bound]. */
    int outside_count = 0;
    iv_t* outside = NULL;
    if (ok) {
        outside = isolate_roots(seq, slen,
                                 one, one, bound, one,
                                 40, &outside_count);
        if (outside_count != 1) ok = 0;
    }

    if (ok) {
        mpz_set(out->beta_lo_num, outside[0].lo_num);
        mpz_set(out->beta_lo_den, outside[0].lo_den);
        mpz_set(out->beta_hi_num, outside[0].hi_num);
        mpz_set(out->beta_hi_den, outside[0].hi_den);
        out->is_pisot = 1;

        if (n_complex > 0) {
            /* For Pisot, |complex|² < 1.  Check this by verifying
             * |det M| < β_lo · ∏|real sec|_max (loose bound).
             * Equivalent: denom_lo = β_lo · product_real > |det M|
             * implies |complex|² < 1. */
            mpz_ptr denom_lo = mpz_new();
            mpz_mul(denom_lo, out->beta_lo_num, out->beta_lo_den);
            for (int i = 0; i < inside_count; ++i) {
                mpz_ptr abs_lo = mpz_new();
                mpz_abs(abs_lo, inside[i].lo_num);
                mpz_ptr abs_hi = mpz_new();
                mpz_abs(abs_hi, inside[i].hi_num);
                mpz_ptr num_max = mpz_new();
                mpz_ptr den_max = mpz_new();
                mpz_mul(num_max, abs_lo, inside[i].hi_den);
                mpz_ptr t = mpz_new();
                mpz_mul(t, abs_hi, inside[i].lo_den);
                if (mpz_cmp(num_max, t) > 0) {
                    mpz_set(den_max, inside[i].hi_den);
                } else {
                    mpz_set(num_max, t);
                    mpz_set(den_max, inside[i].lo_den);
                }
                mpz_mul(denom_lo, denom_lo, num_max);
                mpz_divexact(denom_lo, denom_lo, den_max);
                mpz_drop(&abs_lo); mpz_drop(&abs_hi);
                mpz_drop(&num_max); mpz_drop(&den_max); mpz_drop(&t);
            }
            out->is_complex_modulus_lt_1 = (mpz_cmp(denom_lo, out->det_abs) > 0);
            mpz_drop(&denom_lo);
        }
    }

    /* Cleanup. */
    for (int i = 0; i < inside_count; ++i) {
        mpz_drop(&inside[i].lo_num); mpz_drop(&inside[i].lo_den);
        mpz_drop(&inside[i].hi_num); mpz_drop(&inside[i].hi_den);
    }
    free(inside);
    if (outside) {
        for (int i = 0; i < outside_count; ++i) {
            mpz_drop(&outside[i].lo_num); mpz_drop(&outside[i].lo_den);
            mpz_drop(&outside[i].hi_num); mpz_drop(&outside[i].hi_den);
        }
        free(outside);
    }
    for (int i = 0; i < slen; ++i) poly_clear(&seq[i]);
    poly_clear(&p);
    mpz_drop(&max_abs); mpz_drop(&tmp_abs);
    mpz_drop(&bound); mpz_drop(&negbound);
    mpz_drop(&one); mpz_drop(&neg_one);
    return ok;
}

/* =================================================================
 * Public API: 3x3 and 4x4 Pisot classifier wrappers
 * ================================================================= */

/* Classify the characteristic polynomial of a 3x3 integer matrix
 * as Pisot.  The matrix is in row-major order.  On success returns
 * nonzero and fills `out` (caller must call pisot_info_clear). */
int pisot_classify_3x3(const long long M[3][3], pisot_info_t* out) {
    long long coeffs[4];
    char_poly_3x3(M, coeffs);
    return pisot_classify_poly(coeffs, 3, out);
}

/* Classify the characteristic polynomial of a 4x4 integer matrix
 * as Pisot.  The matrix is in row-major order.  On success returns
 * nonzero and fills `out` (caller must call pisot_info_clear). */
int pisot_classify_4x4(const long long M[4][4], pisot_info_t* out) {
    long long coeffs[5];
    char_poly_4x4(M, coeffs);
    return pisot_classify_poly(coeffs, 4, out);
}

/* =================================================================
 * Generic root isolation (arbitrary integer polynomial, not
 * necessarily monic) -- reuses the same Sturm-chain/bisection
 * internals as the Pisot classifier above, exposed for callers that
 * already know a root is bracketed and just want it isolated to
 * high precision (e.g. transfer-operator eigenvalue certification
 * via gkw_sturm_certify.cpp, which uses this to certify the top
 * two eigenvalues of the M-node Chebyshev collocation matrix).
 *
 * Coefficients are in low-to-high order (coeffs[i] is the
 * coefficient of x^i).  The degree may be any value in [1, 15]
 * (poly_t's internal capacity); the polynomial is treated as
 * monic in degree, but the leading coefficient is allowed to be
 * arbitrary as long as the Sturm chain builds.
 * ================================================================= */
int isolate_real_root_generic_mpz(mpz_srcptr* coeffs, int degree,
                                  long long lo_num, long long lo_den,
                                  long long hi_num, long long hi_den,
                                  int tol_bits,
                                  mpz_ptr* lo_out_num, mpz_ptr* lo_out_den,
                                  mpz_ptr* hi_out_num, mpz_ptr* hi_out_den) {
    if (degree < 1 || degree > 15) return 0;

    poly_t p;
    poly_init(&p);
    for (int i = 0; i <= degree; ++i) {
        mpq_ptr v = mpq_new();
        mpq_set_num(v, coeffs[i]);
        mpz_set_ui(mpq_denref(v), 1);
        mpq_canonicalize(v);
        poly_set_coeff(&p, i, v);
        mpq_drop(&v);
    }
    poly_normalize(&p);
    if (poly_degree(&p) < 1) { poly_clear(&p); return 0; }

    poly_t seq[16];
    int slen = sturm_chain_build(seq, &p);
    if (slen < 2) {
        for (int i = 0; i < slen; ++i) poly_clear(&seq[i]);
        poly_clear(&p);
        return 0;
    }

    mpz_ptr a_n = mpz_new_si(lo_num), a_d = mpz_new_si(lo_den);
    mpz_ptr b_n = mpz_new_si(hi_num), b_d = mpz_new_si(hi_den);
    int n = sturm_count(seq, slen, a_n, a_d, b_n, b_d);

    int ok = 0;
    if (n == 1) {
        int count = 0;
        iv_t* result = isolate_roots(seq, slen, a_n, a_d, b_n, b_d,
                                     tol_bits, &count);
        if (count == 1) {
            *lo_out_num = mpz_new_copy(result[0].lo_num);
            *lo_out_den = mpz_new_copy(result[0].lo_den);
            *hi_out_num = mpz_new_copy(result[0].hi_num);
            *hi_out_den = mpz_new_copy(result[0].hi_den);
            ok = 1;
        }
        for (int i = 0; i < count; ++i) {
            mpz_drop(&result[i].lo_num); mpz_drop(&result[i].lo_den);
            mpz_drop(&result[i].hi_num); mpz_drop(&result[i].hi_den);
        }
        free(result);
    }

    mpz_drop(&a_n); mpz_drop(&a_d); mpz_drop(&b_n); mpz_drop(&b_d);
    for (int i = 0; i < slen; ++i) poly_clear(&seq[i]);
    poly_clear(&p);
    return ok;
}
