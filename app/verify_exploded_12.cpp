#include <cstdio>
#include <vector>
#include <string>
#include "exact_pisot.h"

struct Case { const char* name; long long m[9]; };

int main() {
    Case cases[] = {
        {"rnd1_canon", {0, 1, 2, 1, 0, 1, 0, 2, 2}},
        {"rnd4_canon", {2, 0, 2, 1, 0, 0, 1, 1, 0}},
        {"rnd5_canon", {1, 2, 1, 2, 2, 2, 1, 0, 0}},
        {"rnd6_canon", {1, 2, 1, 1, 2, 0, 0, 2, 0}},
        {"rnd8_canon", {1, 1, 0, 1, 2, 1, 1, 0, 1}},
        {"rnd8_barge", {1, 1, 0, 1, 2, 1, 1, 0, 1}},
        {"rnd10_canon", {0, 0, 1, 1, 0, 1, 1, 2, 1}},
        {"rnd13_canon", {2, 2, 0, 2, 2, 1, 1, 2, 0}},
        {"rnd16_canon", {2, 2, 0, 0, 1, 1, 1, 0, 0}},
        {"rnd19_canon", {1, 2, 1, 0, 0, 1, 1, 0, 1}},
        {"rnd19_barge", {1, 2, 1, 0, 0, 1, 1, 0, 1}},
        {"rnd24_canon", {0, 1, 0, 0, 2, 2, 1, 0, 1}},
    };

    int n_pisot = 0, n_not = 0, n_fail = 0;
    for (auto& c : cases) {
        long long M[3][3];
        for (int i = 0; i < 3; ++i)
            for (int j = 0; j < 3; ++j)
                M[i][j] = c.m[i*3+j];

        pisot_info_t info;
        int rc = pisot_classify_3x3(M, &info);
        if (!rc) {
            printf("%-14s classify FAILED (not squarefree or other issue)\n", c.name);
            ++n_fail;
            continue;
        }
        char buf[64];
        mpq_class_str(buf, sizeof(buf), info.beta_lo_num, info.beta_lo_den);
        char dbuf[64];
        mpz_get_str(dbuf, 10, info.det_abs);
        printf("%-14s is_pisot=%d  beta~%s  |det|=%s  complex_mod<1=%d\n",
               c.name, info.is_pisot, buf, dbuf, info.is_complex_modulus_lt_1);
        if (info.is_pisot) ++n_pisot; else ++n_not;
        pisot_info_clear(&info);
    }
    printf("\n%d classified Pisot, %d classified NOT Pisot, %d classify() failed outright\n",
           n_pisot, n_not, n_fail);
    return 0;
}
