// gkw_sturm_certify.cpp
//
// Replaces the Elsner-perturbation-bound attempt (gkw_certified.cpp)
// with the right tool for this job: exact rational characteristic
// polynomial (Faddeev-LeVerrier over Q, using mini-mpq) + the
// project's own Sturm-sequence root isolator (exact_pisot.c, now
// exposing isolate_real_root_generic for arbitrary-degree integer
// polynomials). No perturbation theory, no dimension-dependent
// weakening -- Sturm bisection is exact and reaches any requested
// bit precision directly.
//
// Honest scope: this exactly certifies the top eigenvalues of ONE
// SPECIFIC, FIXED rational matrix -- the M-node Chebyshev collocation
// matrix, with each double-precision entry rationalized to a fixed
// power-of-2 denominator. It closes the "matrix -> eigenvalue" gap
// completely (arbitrary precision, proven). It does NOT by itself
// bound the remaining two gaps: (a) how far the rationalized matrix
// is from the true (irrational, zeta-valued) collocation matrix, and
// (b) how far the M-node truncation is from the actual infinite-
// dimensional operator. Both still need the nuclearity/Bernstein-
// ellipse analytic bound discussed earlier -- this closes the linear
// algebra step, not the operator-theory step.

#include <cstdio>
#include <cstring>
#include <cmath>
#include <vector>
#include "exact_pisot.h"
#include "mini-gmp/mini-mpq.h"

using namespace std;
const double PI = 3.14159265358979323846;

// ---------- Chebyshev collocation matrix (double), same as before ----------
struct Cheb { vector<double> x, w; };
Cheb cheb_nodes(int M) {
    Cheb c; c.x.resize(M); c.w.resize(M);
    for (int j = 0; j < M; ++j) {
        double theta = (2.0*j + 1.0) * PI / (2.0*M);
        double xj = cos(theta);
        c.x[j] = 0.5 * (xj + 1.0);
        c.w[j] = ((j % 2 == 0) ? 1.0 : -1.0) * sin(theta);
    }
    return c;
}
void lagrange_basis(const Cheb& c, double y, vector<double>& ell) {
    int M = c.x.size();
    ell.assign(M, 0.0);
    for (int j = 0; j < M; ++j) if (fabs(y - c.x[j]) < 1e-14) { ell[j] = 1.0; return; }
    double denom = 0.0; vector<double> num(M);
    for (int j = 0; j < M; ++j) { num[j] = c.w[j] / (y - c.x[j]); denom += num[j]; }
    for (int j = 0; j < M; ++j) ell[j] = num[j] / denom;
}
double psi1_asymp(double z) {
    static const double B[] = {1.0/6, -1.0/30, 1.0/42, -1.0/30, 5.0/66, -691.0/2730, 7.0/6};
    double s = 1.0/z + 1.0/(2*z*z); double zp = z*z*z;
    for (int k = 1; k <= 7; ++k) { s += B[k-1]/zp; zp *= z*z; }
    return s;
}
vector<vector<double>> build_L(int M, int Ntrunc = 200000) {
    Cheb c = cheb_nodes(M);
    vector<vector<double>> A(M, vector<double>(M, 0.0));
    vector<double> ell;
    for (int i = 0; i < M; ++i) {
        double xi = c.x[i];
        for (int n = 1; n <= Ntrunc; ++n) {
            double y = 1.0/(n+xi), weight = 1.0/((n+xi)*(n+xi));
            lagrange_basis(c, y, ell);
            for (int j = 0; j < M; ++j) A[i][j] += weight*ell[j];
        }
        double tailweight = psi1_asymp(Ntrunc + 1 + xi);
        lagrange_basis(c, 0.0, ell);
        for (int j = 0; j < M; ++j) A[i][j] += tailweight*ell[j];
    }
    return A;
}

// ---------- Rationalize: double -> mpq_t with denominator 2^SCALE ----------
const int SCALE_BITS = 62;
void double_to_mpq(mpq_t out, double v) {
    long long num = (long long)llround(ldexp(v, SCALE_BITS));
    mpz_t n, d;
    mpz_init_set_si(n, num);
    mpz_init(d);
    mpz_set_ui(d, 1);
    mpz_mul_2exp(d, d, SCALE_BITS);
    mpq_set_num(out, n);
    mpq_set_den(out, d);
    mpq_canonicalize(out);
    mpz_clear(n); mpz_clear(d);
}

// ---------- Faddeev-LeVerrier over Q: char poly of an MxM mpq matrix ----------
// p(x) = x^M + c[1] x^{M-1} + ... + c[M]  (c[0] implicit = 1)
vector<mpq_ptr> faddeev_leverrier(vector<vector<mpq_ptr>>& A, int M) {
    auto new_mat = [&](){
        vector<vector<mpq_ptr>> X(M, vector<mpq_ptr>(M));
        for (int i=0;i<M;++i) for(int j=0;j<M;++j){ X[i][j]=(mpq_ptr)malloc(sizeof(__mpq_struct)); mpq_init(X[i][j]); }
        return X;
    };
    auto identity = [&](){
        auto X = new_mat();
        for (int i=0;i<M;++i) mpq_set_ui(X[i][i], 1, 1);
        return X;
    };
    auto matmul = [&](vector<vector<mpq_ptr>>& X, vector<vector<mpq_ptr>>& Y){
        auto Z = new_mat();
        mpq_t t;
        mpq_init(t);
        for (int i=0;i<M;++i) for(int j=0;j<M;++j){
            mpq_set_ui(Z[i][j], 0, 1);
            for (int k=0;k<M;++k) { mpq_mul(t, X[i][k], Y[k][j]); mpq_add(Z[i][j], Z[i][j], t); }
        }
        mpq_clear(t);
        return Z;
    };
    auto trace = [&](vector<vector<mpq_ptr>>& X, mpq_t out){
        mpq_set_ui(out, 0, 1);
        for (int i=0;i<M;++i) mpq_add(out, out, X[i][i]);
    };
    auto free_mat = [&](vector<vector<mpq_ptr>>& X){
        for (int i=0;i<M;++i) for(int j=0;j<M;++j){ mpq_clear(X[i][j]); delete X[i][j]; }
    };

    vector<mpq_ptr> c(M+1);
    for (auto& p : c) { p = (mpq_ptr)malloc(sizeof(__mpq_struct)); mpq_init(p); }
    mpq_set_ui(c[0], 1, 1);

    auto Mk = identity();
    mpq_t tr, t;
    mpq_init(tr); mpq_init(t);

    for (int k = 1; k <= M; ++k) {
        auto Nk = matmul(A, Mk);
        trace(Nk, tr);
        mpq_set_ui(t, k, 1);
        mpq_div(tr, tr, t);
        mpq_neg(c[k], tr);       // c[k] = -trace(A*M_{k-1}) / k
        free_mat(Mk);
        // M_k = N_k + c[k]*I
        auto NewMk = new_mat();
        for (int i=0;i<M;++i) for(int j=0;j<M;++j) {
            mpq_set(NewMk[i][j], Nk[i][j]);
            if (i==j) mpq_add(NewMk[i][j], NewMk[i][j], c[k]);
        }
        free_mat(Nk);
        Mk = NewMk;
    }
    free_mat(Mk);
    mpq_clear(tr); mpq_clear(t);
    return c;  // c[0..M], c[0]=1 (leading coeff of x^M)
}

// Clear denominators: multiply all coeffs by LCM of denominators ->
// integer polynomial with the same real roots. Coefficients are
// arbitrary precision (they overflow int64 quickly for M>~6).
void clear_denominators(vector<mpq_ptr>& c, int M, vector<mpz_ptr>& out_coeffs) {
    mpz_t L, d;
    mpz_init_set_ui(L, 1);
    mpz_init(d);
    for (int i = 0; i <= M; ++i) {
        mpq_get_den(d, c[i]);
        mpz_lcm(L, L, d);
    }
    mpq_t scaled, Lq; mpq_init(scaled); mpq_init(Lq);
    mpz_set(mpq_numref(Lq), L); mpz_set_ui(mpq_denref(Lq), 1);
    out_coeffs.resize(M+1);
    for (int i = 0; i <= M; ++i) {
        mpq_mul(scaled, c[i], Lq);
        out_coeffs[M - i] = (mpz_ptr)malloc(sizeof(__mpz_struct));
        mpz_init(out_coeffs[M - i]);
        mpz_set(out_coeffs[M - i], mpq_numref(scaled));  // den==1 guaranteed by LCM
    }
    mpq_clear(scaled); mpq_clear(Lq);
    mpz_clear(L); mpz_clear(d);
}

// Decimal expansion of num/den to `digits` significant digits.
void print_decimal(const char* label, const mpz_t num, const mpz_t den, int digits) {
    mpz_t n, d, q, r, scale;
    mpz_init_set(n, num); mpz_init_set(d, den);
    mpz_init(q); mpz_init(r); mpz_init(scale);
    int neg = mpz_sgn(n) < 0;
    if (neg) mpz_neg(n, n);
    mpz_ui_pow_ui(scale, 10, digits);
    mpz_mul(n, n, scale);
    mpz_tdiv_qr(q, r, n, d);
    char* s = mpz_get_str(NULL, 10, q);
    int len = strlen(s);
    printf("%s%s", label, neg ? "-" : "");
    if (len <= digits) {
        printf("0.");
        for (int i = 0; i < digits - len; ++i) putchar('0');
        printf("%s\n", s);
    } else {
        int intlen = len - digits;
        for (int i = 0; i < intlen; ++i) putchar(s[i]);
        printf(".%s\n", s + intlen);
    }
    free(s);
    mpz_clear(n); mpz_clear(d); mpz_clear(q); mpz_clear(r); mpz_clear(scale);
}

int main() {
    int M = 12;   // keep char poly degree <= 15 (poly_t's capacity)
    printf("Sturm-certified GKW eigenvalues, M=%d collocation nodes\n", M);
    printf("Reference lambda2 = -0.30366300289873265859744812190155623429...\n\n");

    auto Ad = build_L(M);

    vector<vector<mpq_ptr>> Aq(M, vector<mpq_ptr>(M));
    for (int i=0;i<M;++i) for(int j=0;j<M;++j) {
        Aq[i][j] = (mpq_ptr)malloc(sizeof(__mpq_struct)); mpq_init(Aq[i][j]);
        double_to_mpq(Aq[i][j], Ad[i][j]);
    }

    auto c = faddeev_leverrier(Aq, M);
    vector<mpz_ptr> coeffs;
    clear_denominators(c, M, coeffs);

    printf("Exact characteristic polynomial (ascending, degree %d), digit counts:\n  ", M);
    for (int i = 0; i <= M; ++i) {
        char* s = mpz_get_str(NULL, 10, coeffs[i]);
        printf("%d ", (int)strlen(s));
        free(s);
    }
    printf("(shows these are far beyond int64 -- why the API needed to change)\n\n");

    // Certify root near 1 (lambda1) and near -0.30366 (lambda2), to
    // 300 bits (~90 decimal digits).
    struct Target { const char* name; long long lo_n, lo_d, hi_n, hi_d; };
    Target targets[] = {
        {"lambda1 (near 1)",    1, 2,   2, 1},     // bracket (0.5, 2)
        {"lambda2 (near -0.3)", -31, 100,  -3, 10},      // bracket (-0.31, -0.30)
    };

    vector<mpz_srcptr> coeffs_src(coeffs.begin(), coeffs.end());
    for (auto& t : targets) {
        mpz_ptr lo_n, lo_d, hi_n, hi_d;
        int ok = isolate_real_root_generic_mpz(coeffs_src.data(), M,
                                           t.lo_n, t.lo_d, t.hi_n, t.hi_d,
                                           300, &lo_n, &lo_d, &hi_n, &hi_d);
        if (!ok) { printf("%s: bracket did not isolate a unique root\n", t.name); continue; }
        printf("%s:\n", t.name);
        print_decimal("  lo = ", lo_n, lo_d, 90);
        print_decimal("  hi = ", hi_n, hi_d, 90);
        mpz_clear(lo_n); free(lo_n); mpz_clear(lo_d); free(lo_d);
        mpz_clear(hi_n); free(hi_n); mpz_clear(hi_d); free(hi_d);
    }
    return 0;
}
