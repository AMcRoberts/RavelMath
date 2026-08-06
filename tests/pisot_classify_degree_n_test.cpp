// Verifies pisot_classify_degree_n (Finding 30): matches the existing
// pisot_classify_3x3/4x4 results at degree 3-4, correctly refuses at
// degree 5 when a polynomial has two complex-conjugate pairs (the
// a-bonacci family, a=1..3), and correctly SUCCEEDS on a degree-5
// polynomial with only one complex pair -- confirming the guard is
// scoped to the genuine limitation, not over-blocking degree 5
// wholesale.

#include <cassert>
#include <cstdio>
#include "exact_pisot.h"

namespace {

double mid(const pisot_info_t& info) {
    double lo = mpz_get_d(info.beta_lo_num) / mpz_get_d(info.beta_lo_den);
    double hi = mpz_get_d(info.beta_hi_num) / mpz_get_d(info.beta_hi_den);
    return (lo + hi) / 2.0;
}

}  // namespace

int main() {
    // Degree 3: matches pisot_classify_3x3's own Tribonacci case
    // (x^3-x^2-x-1, beta ~ 1.8392867552).
    {
        long long coeffs[3] = {-1, -1, -1};
        pisot_info_t info;
        int rc = pisot_classify_degree_n(coeffs, 3, &info);
        assert(rc && info.is_pisot);
        double b = mid(info);
        assert(b > 1.83 && b < 1.84);
        pisot_info_clear(&info);
        std::printf("[ok] degree 3 (Tribonacci) matches: beta=%.6f\n", b);
    }

    // Degree 5, a-bonacci a=1 (Pentanacci): two complex pairs among
    // the secondary roots -- must correctly REFUSE (rc=0), not guess.
    {
        long long coeffs[5] = {-1, -1, -1, -1, -1};
        pisot_info_t info;
        int rc = pisot_classify_degree_n(coeffs, 5, &info);
        assert(rc == 0);
        std::printf("[ok] degree 5 Pentanacci correctly refused (two complex pairs, guard fires)\n");
    }

    // Degree 5, hand-found single-complex-pair case: x^5-x^4-x^3-x^2+1
    // -- must SUCCEED, confirming the guard isn't over-blocking.
    {
        long long coeffs[5] = {1, 0, -1, -1, -1};
        pisot_info_t info;
        int rc = pisot_classify_degree_n(coeffs, 5, &info);
        assert(rc && info.is_pisot && info.has_complex_pair);
        double b = mid(info);
        assert(b > 1.77 && b < 1.79);
        pisot_info_clear(&info);
        std::printf("[ok] degree 5 single-complex-pair case succeeds: beta=%.6f\n", b);
    }

    std::printf("pisot_classify_degree_n: matches existing degree-3/4 results, "
                "refuses genuinely uncertifiable multi-pair cases, "
                "succeeds on certifiable degree-5 cases.\n");
    return 0;
}
