// Locks in Finding 32: sigma_ab_matrix(0,2) (the matrix used as
// "sigma_{0,2}" throughout Findings 6.5/28/30/31) is NOT Pisot --
// its complex-conjugate pair has modulus > 1. Regression test so
// this misclassification can never silently creep back in.

#include <cassert>
#include <cstdio>
#include "exact_pisot.h"

int main() {
    long long M[3][3] = {{0, 0, 1}, {2, 0, 0}, {1, 1, 0}};
    pisot_info_t info;
    int rc = pisot_classify_3x3(M, &info);
    assert(rc != 0);  // classifier itself succeeds (squarefree, isolable)
    assert(info.is_pisot == 0);
    assert(info.has_complex_pair == 1);
    assert(info.is_complex_modulus_lt_1 == 0);
    std::printf("[ok] sigma_ab_matrix(0,2) correctly classified as NOT Pisot "
                "(complex-conjugate pair modulus > 1)\n");
    pisot_info_clear(&info);
    return 0;
}
