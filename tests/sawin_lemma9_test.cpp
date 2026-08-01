// sawin_lemma9_test.cpp
//
// Checks Sawin's Lemma 9 class-number bound (arXiv 2605.20579),
// specialized to F=Q/K imaginary quadratic, against this project's own
// independently-computed class numbers (include/adelic/quadratic_class_group.hpp).
// Closes the loose end golod_shafarevich.hpp's header names: Lemma 9
// "needs class-group machinery this project doesn't have yet" -- now
// it does, for this base case.

#include <cstdio>
#include <vector>

#include "adelic/sawin_lemma9.hpp"

using namespace std;
using namespace mathlib;
using namespace adelic;

namespace {
int n_pass = 0, n_fail = 0;
#define CHECK(cond, msg) do { \
    if (cond) { ++n_pass; fprintf(stderr, "  [ok]   %s\n", (msg)); } \
    else { ++n_fail; fprintf(stderr, "  [FAIL] %s\n", (msg)); } \
} while (0)
}  // namespace

int main() {
    fprintf(stderr, "=== Lemma 9 bound holds for the nine Heegner numbers ===\n");
    std::vector<long long> heegner = {-3, -4, -7, -8, -11, -19, -43, -67, -163};
    for (long long D : heegner) {
        BigInt Dz(D);
        long long h = quadratic_class_number(Dz);
        BigFloat bound = sawin_lemma9_bound(Dz);
        double bound_d = bigfloat_to_double(bound);
        fprintf(stderr, "  D=%lld: h=%lld, bound=%.6g\n", D, h, bound_d);
        CHECK(sawin_lemma9_holds(Dz), "h(D) <= Lemma 9's bound");
        CHECK(bound_d > 0.0, "the bound itself is a positive, finite number (not a degenerate 0 or NaN)");
    }

    fprintf(stderr, "\n=== Lemma 9 bound holds for larger h>1 discriminants ===\n");
    std::vector<long long> larger = {-84, -164, -755, -1000};
    for (long long D : larger) {
        long long dm4 = ((D % 4) + 4) % 4;
        if (dm4 != 0 && dm4 != 1) continue;
        BigInt Dz(D);
        long long h = quadratic_class_number(Dz);
        BigFloat bound = sawin_lemma9_bound(Dz);
        double bound_d = bigfloat_to_double(bound);
        fprintf(stderr, "  D=%lld: h=%lld, bound=%.6g\n", D, h, bound_d);
        CHECK(sawin_lemma9_holds(Dz), "h(D) <= Lemma 9's bound");
    }

    fprintf(stderr, "\n%d/%d checks passed\n", n_pass, n_pass + n_fail);
    return n_fail == 0 ? 0 : 1;
}
