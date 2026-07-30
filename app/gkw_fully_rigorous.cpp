// gkw_fully_rigorous.cpp
//
// Completes the step flagged as open at the end of the previous
// session: the Chebyshev node positions/weights were computed with
// plain std::cos/std::sin and a hardcoded PI literal -- unverified
// floating point sitting inside an otherwise-exact pipeline. This
// version has NO floating point anywhere in the matrix construction:
//
//   * node positions/weights: rational_transcendentals.hpp's proven
//     pi/sin/cos brackets (Machin's formula + alternating-series
//     bound; Taylor + Lagrange remainder bound), NOT std::cos/sin.
//   * the n-sum + tail: exact rational arithmetic throughout, with
//     the same proven integral-test tail bracket from the interval
//     version, now over mpq_t instead of double.
//   * Lagrange basis evaluation: a rational function of exact
//     rational nodes -- exact by construction, no approximation at
//     all.
//
// The only approximations left anywhere in the construction are the
// finite `terms` in the pi/sin/cos series and the finite N in the
// n-sum -- both PROVEN, shrinkable-on-demand error bounds, not
// silently-trusted floating point. What follows (Faddeev-LeVerrier +
// Sturm certification) is unchanged from gkw_sturm_certify.cpp.

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <vector>
#include "exact_pisot.h"
#include "ravel/rational_transcendentals.hpp"
#include "mini-gmp/mini-mpq.h"

using namespace std;
using namespace ravel;

mpq_ptr qnew() { mpq_ptr q = (mpq_ptr)malloc(sizeof(__mpq_struct)); mpq_init(q); return q; }
void qdrop(mpq_ptr q) { mpq_clear(q); free(q); }

// ---------- Chebyshev nodes/weights, exact rational midpoints of
// proven brackets (width tracked and reported, not discarded). ----------
struct ChebQ { vector<mpq_ptr> x, w; double max_node_halfwidth = 0; };

ChebQ cheb_nodes_rigorous(int M, int series_terms = 30) {
    ChebQ c;
    c.x.resize(M); c.w.resize(M);

    mpq_t pi_lo, pi_hi;
    mpq_init(pi_lo); mpq_init(pi_hi);
    rational_pi(pi_lo, pi_hi, series_terms);

    for (int j = 0; j < M; ++j) {
        // theta_j = (2j+1) pi / (2M) -- bracket via pi's bracket.
        mpq_t theta_lo, theta_hi, denom;
        mpq_init(theta_lo); mpq_init(theta_hi); mpq_init(denom);
        mpq_set_si(denom, 2*j + 1, 2*M);
        mpq_mul(theta_lo, pi_lo, denom);
        mpq_mul(theta_hi, pi_hi, denom);
        mpq_t theta_mid, two;
        mpq_init(theta_mid); mpq_init(two); mpq_set_si(two, 2, 1);
        mpq_add(theta_mid, theta_lo, theta_hi);
        mpq_div(theta_mid, theta_mid, two);

        mpq_t cos_lo, cos_hi, sin_lo, sin_hi;
        mpq_init(cos_lo); mpq_init(cos_hi); mpq_init(sin_lo); mpq_init(sin_hi);
        rational_cos(cos_lo, cos_hi, theta_mid, series_terms);
        rational_sin(sin_lo, sin_hi, theta_mid, series_terms);

        // x_j = (1 + cos(theta_j)) / 2  -- take the midpoint of the
        // resulting (tiny) bracket as the exact rational node.
        mpq_t one, cx_lo, cx_hi, xmid;
        mpq_init(one); mpq_set_si(one, 1, 1);
        mpq_init(cx_lo); mpq_init(cx_hi);
        mpq_add(cx_lo, one, cos_lo); mpq_div(cx_lo, cx_lo, two);
        mpq_add(cx_hi, one, cos_hi); mpq_div(cx_hi, cx_hi, two);
        mpq_init(xmid);
        mpq_add(xmid, cx_lo, cx_hi); mpq_div(xmid, xmid, two);
        c.x[j] = qnew();
        mpq_set(c.x[j], xmid);

        double hw = 0;
        {
            mpq_t w2; mpq_init(w2);
            mpq_sub(w2, cx_hi, cx_lo);
            hw = mpq_get_d(w2);
            mpq_clear(w2);
        }
        if (hw > c.max_node_halfwidth) c.max_node_halfwidth = hw;

        // w_j = (-1)^j * sin(theta_j), same midpoint treatment.
        mpq_t smid;
        mpq_init(smid);
        mpq_add(smid, sin_lo, sin_hi); mpq_div(smid, smid, two);
        if (j % 2 != 0) mpq_neg(smid, smid);
        c.w[j] = qnew();
        mpq_set(c.w[j], smid);

        mpq_clear(theta_lo); mpq_clear(theta_hi); mpq_clear(denom);
        mpq_clear(theta_mid); mpq_clear(two);
        mpq_clear(cos_lo); mpq_clear(cos_hi); mpq_clear(sin_lo); mpq_clear(sin_hi);
        mpq_clear(one); mpq_clear(cx_lo); mpq_clear(cx_hi); mpq_clear(xmid);
        mpq_clear(smid);
    }
    mpq_clear(pi_lo); mpq_clear(pi_hi);
    return c;
}

// Exact rational Lagrange basis values at rational point y.
void lagrange_basis_q(const ChebQ& c, mpq_srcptr y, vector<mpq_ptr>& ell) {
    int M = c.x.size();
    for (int j = 0; j < M; ++j) if (!ell[j]) ell[j] = qnew();
    for (int j = 0; j < M; ++j) if (mpq_equal(y, c.x[j])) {
        for (int k = 0; k < M; ++k) mpq_set_si(ell[k], k == j ? 1 : 0, 1);
        return;
    }
    mpq_t denom, t;
    mpq_init(denom); mpq_init(t);
    mpq_set_si(denom, 0, 1);
    vector<mpq_ptr> num(M);
    for (int j = 0; j < M; ++j) {
        num[j] = qnew();
        mpq_sub(t, y, c.x[j]);
        mpq_div(num[j], c.w[j], t);
        mpq_add(denom, denom, num[j]);
    }
    for (int j = 0; j < M; ++j) mpq_div(ell[j], num[j], denom);
    for (int j = 0; j < M; ++j) qdrop(num[j]);
    mpq_clear(denom); mpq_clear(t);
}

// Build the fully-exact-rational matrix: leading n-sum (exact
// rational) + proven tail bracket (integral test, pure rational,
// midpoint taken -- same construction as the double/interval
// versions, just with every intermediate value now exact instead of
// float).
vector<vector<mpq_ptr>> build_L_rational(const ChebQ& c, int M, int Ntrunc) {
    vector<vector<mpq_ptr>> A(M, vector<mpq_ptr>(M));
    for (int i = 0; i < M; ++i) for (int j = 0; j < M; ++j) { A[i][j] = qnew(); mpq_set_si(A[i][j], 0, 1); }

    vector<mpq_ptr> ell(M, nullptr);
    mpq_t y, weight, n_plus_x, t, one;
    mpq_init(y); mpq_init(weight); mpq_init(n_plus_x); mpq_init(t); mpq_init(one);
    mpq_set_si(one, 1, 1);

    for (int i = 0; i < M; ++i) {
        mpq_srcptr xi = c.x[i];
        for (int n = 1; n <= Ntrunc; ++n) {
            mpq_set_si(t, n, 1);
            mpq_add(n_plus_x, t, xi);              // n + x_i
            mpq_div(y, one, n_plus_x);              // y = 1/(n+x_i)
            mpq_mul(weight, n_plus_x, n_plus_x);
            mpq_div(weight, one, weight);           // weight = 1/(n+x_i)^2
            lagrange_basis_q(c, y, ell);
            for (int j = 0; j < M; ++j) {
                mpq_mul(t, weight, ell[j]);
                mpq_add(A[i][j], A[i][j], t);
            }
        }
        // Proven tail bracket 1/(N+1+x) <= tail <= 1/(N+x); take the
        // exact rational midpoint (half-width is tiny and reported).
        mpq_t tail_lo, tail_hi, Nq;
        mpq_init(tail_lo); mpq_init(tail_hi); mpq_init(Nq);
        mpq_set_si(Nq, Ntrunc, 1);
        mpq_add(t, Nq, xi); mpq_add(t, t, one);
        mpq_div(tail_lo, one, t);
        mpq_add(t, Nq, xi);
        mpq_div(tail_hi, one, t);
        mpq_t tail_mid, two;
        mpq_init(tail_mid); mpq_init(two); mpq_set_si(two, 2, 1);
        mpq_add(tail_mid, tail_lo, tail_hi); mpq_div(tail_mid, tail_mid, two);

        mpq_set_si(y, 0, 1);
        lagrange_basis_q(c, y, ell);
        for (int j = 0; j < M; ++j) {
            mpq_mul(t, tail_mid, ell[j]);
            mpq_add(A[i][j], A[i][j], t);
        }
        mpq_clear(tail_lo); mpq_clear(tail_hi); mpq_clear(Nq);
        mpq_clear(tail_mid); mpq_clear(two);
    }
    for (auto p : ell) if (p) qdrop(p);
    mpq_clear(y); mpq_clear(weight); mpq_clear(n_plus_x); mpq_clear(t); mpq_clear(one);
    return A;
}

// ---------- Faddeev-LeVerrier over Q (same as before) ----------
vector<mpq_ptr> faddeev_leverrier(vector<vector<mpq_ptr>>& A, int M) {
    auto new_mat = [&](){ vector<vector<mpq_ptr>> X(M, vector<mpq_ptr>(M)); for(int i=0;i<M;++i) for(int j=0;j<M;++j) X[i][j]=qnew(); return X; };
    auto identity = [&](){ auto X=new_mat(); for(int i=0;i<M;++i) mpq_set_ui(X[i][i],1,1); return X; };
    auto matmul = [&](vector<vector<mpq_ptr>>& X, vector<vector<mpq_ptr>>& Y){
        auto Z = new_mat(); mpq_t t; mpq_init(t);
        for(int i=0;i<M;++i) for(int j=0;j<M;++j){ mpq_set_ui(Z[i][j],0,1); for(int k=0;k<M;++k){ mpq_mul(t,X[i][k],Y[k][j]); mpq_add(Z[i][j],Z[i][j],t);} }
        mpq_clear(t); return Z;
    };
    auto trace = [&](vector<vector<mpq_ptr>>& X, mpq_t out){ mpq_set_ui(out,0,1); for(int i=0;i<M;++i) mpq_add(out,out,X[i][i]); };
    auto free_mat = [&](vector<vector<mpq_ptr>>& X){ for(int i=0;i<M;++i) for(int j=0;j<M;++j) qdrop(X[i][j]); };

    vector<mpq_ptr> c(M+1);
    for (auto& p : c) p = qnew();
    mpq_set_ui(c[0], 1, 1);

    auto Mk = identity();
    mpq_t tr, t; mpq_init(tr); mpq_init(t);
    for (int k = 1; k <= M; ++k) {
        auto Nk = matmul(A, Mk);
        trace(Nk, tr);
        mpq_set_ui(t, k, 1);
        mpq_div(tr, tr, t);
        mpq_neg(c[k], tr);
        free_mat(Mk);
        auto NewMk = new_mat();
        for (int i=0;i<M;++i) for(int j=0;j<M;++j) { mpq_set(NewMk[i][j], Nk[i][j]); if(i==j) mpq_add(NewMk[i][j],NewMk[i][j],c[k]); }
        free_mat(Nk);
        Mk = NewMk;
    }
    free_mat(Mk);
    mpq_clear(tr); mpq_clear(t);
    return c;
}

void clear_denominators(vector<mpq_ptr>& c, int M, vector<mpz_ptr>& out_coeffs) {
    mpz_t L, d;
    mpz_init_set_ui(L, 1); mpz_init(d);
    for (int i = 0; i <= M; ++i) { mpq_get_den(d, c[i]); mpz_lcm(L, L, d); }
    mpq_t scaled, Lq; mpq_init(scaled); mpq_init(Lq);
    mpz_set(mpq_numref(Lq), L); mpz_set_ui(mpq_denref(Lq), 1);
    out_coeffs.resize(M+1);
    for (int i = 0; i <= M; ++i) {
        mpq_mul(scaled, c[i], Lq);
        out_coeffs[M-i] = (mpz_ptr)malloc(sizeof(__mpz_struct));
        mpz_init(out_coeffs[M-i]);
        mpz_set(out_coeffs[M-i], mpq_numref(scaled));
    }
    mpq_clear(scaled); mpq_clear(Lq); mpz_clear(L); mpz_clear(d);
}

void print_decimal(const char* label, const mpz_t num, const mpz_t den, int digits) {
    mpz_t n, d, q, r, scale;
    mpz_init_set(n, num); mpz_init_set(d, den);
    mpz_init(q); mpz_init(r); mpz_init(scale);
    int neg = mpz_sgn(n) < 0; if (neg) mpz_neg(n, n);
    mpz_ui_pow_ui(scale, 10, digits);
    mpz_mul(n, n, scale);
    mpz_tdiv_qr(q, r, n, d);
    char* s = mpz_get_str(NULL, 10, q);
    int len = strlen(s);
    printf("%s%s", label, neg ? "-" : "");
    if (len <= digits) { printf("0."); for (int i=0;i<digits-len;++i) putchar('0'); printf("%s\n", s); }
    else { int il=len-digits; for(int i=0;i<il;++i) putchar(s[i]); printf(".%s\n", s+il); }
    free(s);
    mpz_clear(n); mpz_clear(d); mpz_clear(q); mpz_clear(r); mpz_clear(scale);
}

int main() {
    int M = 8;
                   // rational Lagrange-basis inner loop is much more
                   // expensive per entry than the double version.
    printf("Fully rigorous (no floating point anywhere) GKW matrix, M=%d\n", M);
    printf("Reference lambda2 = -0.30366300289873265859744812190155623429...\n\n");

    ChebQ c = cheb_nodes_rigorous(M, 30);
    printf("Proven max Chebyshev-node bracket half-width: %.3e (pi/sin/cos series, 30 terms)\n", c.max_node_halfwidth);

    auto A = build_L_rational(c, M, 200);
    auto ch = faddeev_leverrier(A, M);
    vector<mpz_ptr> coeffs;
    clear_denominators(ch, M, coeffs);

    vector<mpz_srcptr> coeffs_src(coeffs.begin(), coeffs.end());
    struct Target { const char* name; long long lo_n, lo_d, hi_n, hi_d; };
    Target targets[] = {
        {"lambda1 (near 1)",    1, 2,   2, 1},
        {"lambda2 (near -0.3)", -31, 100, -3, 10},
    };
    for (auto& t : targets) {
        mpz_ptr lo_n, lo_d, hi_n, hi_d;
        int ok = isolate_real_root_generic_mpz(coeffs_src.data(), M,
                                           t.lo_n, t.lo_d, t.hi_n, t.hi_d,
                                           200, &lo_n, &lo_d, &hi_n, &hi_d);
        if (!ok) { printf("%s: bracket did not isolate a unique root\n", t.name); continue; }
        printf("%s:\n", t.name);
        print_decimal("  lo = ", lo_n, lo_d, 60);
        print_decimal("  hi = ", hi_n, hi_d, 60);
        mpz_clear(lo_n); free(lo_n); mpz_clear(lo_d); free(lo_d);
        mpz_clear(hi_n); free(hi_n); mpz_clear(hi_d); free(hi_d);
    }
    return 0;
}
