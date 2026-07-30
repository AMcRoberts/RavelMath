#include <cstdio>
#include <cstring>
#include <cstdlib>
#include "ravel/rational_transcendentals.hpp"
#include "mini-gmp/mini-gmp.h"

using namespace ravel;

void print_decimal(const char* label, const mpq_t v, int digits) {
    mpz_t n, d, q, r, scale;
    mpz_init(n); mpz_init(d); mpz_init(q); mpz_init(r); mpz_init(scale);
    mpq_get_num(n, v); mpq_get_den(d, v);
    int neg = mpz_sgn(n) < 0;
    if (neg) mpz_neg(n, n);
    mpz_ui_pow_ui(scale, 10, digits);
    mpz_mul(n, n, scale);
    mpz_tdiv_qr(q, r, n, d);
    char* s = mpz_get_str(NULL, 10, q);
    int len = strlen(s);
    printf("%s%s", label, neg ? "-" : "");
    if (len <= digits) { printf("0."); for (int i=0;i<digits-len;++i) putchar('0'); printf("%s\n", s); }
    else { for (int i=0;i<len-digits;++i) putchar(s[i]); printf(".%s\n", s+(len-digits)); }
    free(s);
    mpz_clear(n); mpz_clear(d); mpz_clear(q); mpz_clear(r); mpz_clear(scale);
}

int main() {
    printf("Rigorous rational pi (Machin + alternating-series bound):\n");
    printf("Reference: 3.14159265358979323846264338327950288419716939937510...\n");
    for (int terms : {5, 10, 20, 40}) {
        mpq_t lo, hi, width;
        mpq_init(lo); mpq_init(hi); mpq_init(width);
        rational_pi(lo, hi, terms);
        mpq_sub(width, hi, lo);
        printf(" terms=%2d  ", terms);
        print_decimal("lo=", lo, 40);
        print_decimal("width=", width, 10);
        mpq_clear(lo); mpq_clear(hi); mpq_clear(width);
    }

    printf("\nsin/cos sanity checks (bracket should straddle the known value):\n");
    mpq_t pi_lo, pi_hi;
    mpq_init(pi_lo); mpq_init(pi_hi);
    rational_pi(pi_lo, pi_hi, 40);

    // pi/6 bracket (just use pi_lo/6 .. pi_hi/6, valid since dividing
    // a bracket by a positive constant preserves it)
    mpq_t six, pi6_lo, pi6_hi;
    mpq_init(six); mpq_set_si(six, 6, 1);
    mpq_init(pi6_lo); mpq_init(pi6_hi);
    mpq_div(pi6_lo, pi_lo, six);
    mpq_div(pi6_hi, pi_hi, six);

    mpq_t s_lo, s_hi, c_lo, c_hi;
    mpq_init(s_lo); mpq_init(s_hi); mpq_init(c_lo); mpq_init(c_hi);
    // Use the midpoint of the pi/6 bracket as the evaluation point
    // (fine for a sanity check; a fully rigorous composition would
    // widen by the pi-bracket's own width too, omitted here for brevity).
    mpq_t mid; mpq_init(mid);
    mpq_add(mid, pi6_lo, pi6_hi);
    mpq_t two; mpq_init(two); mpq_set_si(two, 2, 1);
    mpq_div(mid, mid, two);

    rational_sin(s_lo, s_hi, mid, 20);
    rational_cos(c_lo, c_hi, mid, 20);
    printf("sin(pi/6): "); print_decimal("lo=", s_lo, 30);
    printf("           "); print_decimal("hi=", s_hi, 30);
    printf("  (expect 0.5 inside)\n");
    printf("cos(pi/6): "); print_decimal("lo=", c_lo, 30);
    printf("           "); print_decimal("hi=", c_hi, 30);
    printf("  (expect 0.866025403784438646763723170752... inside)\n");

    return 0;
}
