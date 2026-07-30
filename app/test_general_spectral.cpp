#include <cstdio>
#include <vector>
#include "ravel/survey.hpp"

using namespace ravel;
using V = std::vector<std::vector<long long>>;

void report(const char* name, const V& M) {
    auto r = classify_matrix_spectral(M);
    std::printf("%-14s n=%zu beta=%.6f b2=%.6f irred=%d pisot=%d\n",
                name, M.size(), r.beta, r.b2, (int)r.irred, (int)r.pisot);
}

int main() {
    // Tribonacci (3x3): known beta ~ 1.839286755214161 (from the
    // earlier conversation's AUDIT.md cross-check table).
    report("Tribonacci", {{1,1,1},{1,0,0},{0,1,0}});

    // Tetrabonacci (4x4), sigma(0)=(0,1) sigma(1)=(0,2) sigma(2)=(0,3) sigma(3)=(0):
    // M[r][c] = count of r in image of c. Known beta ~ 1.927562 (independently verified
    // via the fixed cylinder_measure.cpp).
    report("Tetrabonacci", {{1,1,1,1},{1,0,0,0},{0,1,0,0},{0,0,1,0}});

    // rnd13 (4x4): known beta ~ 5.623559, |det|=2 (independently verified
    // via the fixed cylinder_measure.cpp).
    // sigma(0)=(0,0,1,2,3,3) sigma(1)=(0,0,2,3,3) sigma(2)=(0,0,3,3) sigma(3)=(0,0,0,2,3,3)
    report("rnd13", {{2,2,2,3},{1,0,0,0},{1,1,0,1},{2,2,2,2}});

    return 0;
}
