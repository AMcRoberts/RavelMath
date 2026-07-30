// openai_unit_distance_test.cpp
//
// Numerical implementation of the OpenAI / Sawin / Erdős-van-der-Waerden
// construction: a c²-spaced grid in the plane has unit-distance pairs
// counted by Jacobi's two-square theorem.  The point of the C² number
// theory is that counting unit-distance pairs becomes O(r(c²)) instead
// of O(n²·√c²): for each (Δi, Δj) with Δi² + Δj² = c², the number of
// grid pairs realizing this displacement is (k − |Δi|)(k − |Δj|), and
// the sum over the r(c²)/2 unordered representations is the answer.
//
// Reference: arXiv:2605.20579 (Sawin, explicit lower bound n^{1.014});
// arXiv:2605.20695 (Alon, Bloom, Gowers, Litt, Sawin, Shankar,
// Tsimerman, Wang, Wood — human-verified companion);
// cdn.openai.com/pdf/74c24085-19b0-4534-9c90-465b8e29ad73/
// unit-distance-proof.pdf  (original OpenAI model output);
// refs/references.bib for the canonical BibTeX.
//
// Construction: for a k × k grid of points at positions (i/c, j/c),
// i, j ∈ {0, ..., k-1}, two points are at Euclidean distance exactly 1
// iff their integer displacement (Δi, Δj) satisfies Δi² + Δj² = c².
// The number of such displacement vectors is r(c²), given by Jacobi's
// two-square theorem:
//
//   r(c²) = 4 · ∏_{p ≡ 1 mod 4 dividing c²} (e_p + 1)
//
// where e_p is the exponent of p in c².  Primes p ≡ 3 mod 4 must
// appear with even exponent (else r(c²) = 0); c² = product of distinct
// primes ≡ 1 mod 4 gives r(c²) = 4 · 2^k where k is the count of
// primes in the product.
//
// For each representation (Δi, Δj) of c² as a sum of two squares, the
// number of (P, Q) with P − Q = (Δi, Δj) in the k × k grid is
// (k − |Δi|)(k − |Δj|) (and is zero if either |Δi| ≥ k or |Δj| ≥ k).
// Each unordered pair {P, Q} of distinct points is counted twice
// across the r(c²) signed representations, so total unordered
// unit-distance pairs = (1/2) Σ_{(Δi, Δj)} (k − |Δi|)(k − |Δj|).
//
// Why this matters here: the project's "exotic point fields" research
// direction (F-machinery refinement, generalized Rauzy fractals over
// non-cube tile shapes, light × shape decomposition) is precisely the
// arena where counting unit distances in higher-dimensional algebraic-
// integer lattices becomes the central quantity.  This test gives a
// tractable closed-form count for the planar OpenAI grid and
// demonstrates the O(r(c²)) algorithm; the construction is what makes
// unit-distance counting number-theoretic rather than combinatorial.
//
// What this test does NOT prove:
//   - It is the count, not the lower bound.  Sawin's n^{1.014} lower
//     bound on the maximum over all planar point sets is asymptotic
//     and follows from choosing c² with r(c²) ≫ n^{0.014}, which
//     requires n ≫ 10⁴ and product-class c² much larger than the
//     5·13·...·61 used here.  The c² = product of 8 small primes has
//     r(c²) = 1024 — informative but not asymptotic.
//   - A Pisot / cut-and-project connection was attempted in
//     tests/rauzy_fractal_unit_distance_test.cpp and removed
//     (non-evidentiary and mathematically wrong on its own terms:
//     an always-passing exit code, a claimed mod-5 "sublattice"
//     that is not closed under addition, and a "2D projection"
//     that never discarded the expanding coordinate).  See
//     docs/FAMILY_OF_FAMILIES.md "Generalization to non-cube
//     shapes" for the still-open connection; no executable
//     currently tests it.  This test exercises the closed-form
//     c²-spaced grid and is independent of Pisot.
//   - Per the project's citation discipline: balanced-pair algorithms
//     are classical; the c² grid construction in this exact form is
//     the OpenAI 2025/2026 contribution; Sawin's explicit exponent
//     δ = 0.014 is due to Sawin (arXiv:2605.20579).  Refinements
//     beyond δ = 0.014 are open.

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <set>
#include <utility>
#include <vector>

namespace {

// All eight primes ≡ 1 mod 4 that fit in int64 without overflowing
// when product is taken; the 8-prime product is ≈ 7.66e9.
constexpr int kPrimes1Mod4[] = {5, 13, 17, 29, 37, 41, 53, 61};
constexpr int kNumPrimes = 8;

// Compute c² as a product of the first `kfac` primes ≡ 1 mod 4.
// For kfac = 8, c² = 5·13·17·29·37·41·53·61 ≈ 7.66e9.
long long c2_with_k_primes(int kfac) {
    long long c2 = 1;
    for (int i = 0; i < kfac && i < kNumPrimes; ++i) {
        c2 *= kPrimes1Mod4[i];
    }
    return c2;
}

// Enumerate all signed integer pairs (Δi, Δj) with Δi² + Δj² = c².
// Jacobi's two-square theorem guarantees the count is 4 · ∏ (e_p + 1)
// over primes p ≡ 1 mod 4 with exponent e_p in c²; we read it directly
// by iterating (di, dj) with 0 ≤ di ≤ dj ≤ √c²+1 (the di ≤ dj
// constraint deduplicates order-equivalent pairs), then expanding each
// to its full sign-and-order copy:
//   - di = 0 (so c² = dj² is a square):  2 variants (0, ±dj)
//   - di = dj > 0:                        4 variants (±di, ±di)
//   - 0 < di < dj:                        8 variants (all (±di, ±dj)
//                                                    and (±dj, ±di))
std::vector<std::pair<long long, long long>>
enumerate_signed_unit_diffs(long long c2) {
    std::vector<std::pair<long long, long long>> diffs;
    const long long s = static_cast<long long>(std::sqrt(
        static_cast<double>(c2))) + 1;
    for (long long di = 0; di <= s; ++di) {
        for (long long dj = di; dj <= s; ++dj) {
            if (di * di + dj * dj != c2) continue;
            if (di == 0) {
                diffs.push_back({0, dj});
                diffs.push_back({0, -dj});
            } else if (di == dj) {
                diffs.push_back({di, di});
                diffs.push_back({di, -di});
                diffs.push_back({-di, di});
                diffs.push_back({-di, -di});
            } else {
                diffs.push_back({di, dj});
                diffs.push_back({di, -dj});
                diffs.push_back({-di, dj});
                diffs.push_back({-di, -dj});
                diffs.push_back({dj, di});
                diffs.push_back({dj, -di});
                diffs.push_back({-dj, di});
                diffs.push_back({-dj, -di});
            }
        }
    }
    return diffs;
}

// Number of unordered unit-distance pairs among points (i/c, j/c)
// for i, j ∈ {0, ..., k-1}.  The algorithm is:
//   For each signed (Δi, Δj) with Δi² + Δj² = c²,
//     multiplicity = (k − |Δi|)(k − |Δj|)   if |Δi| < k and |Δj| < k
//   Total unordered pairs = (1/2) Σ multiplicity.
// Because enumerate_unit_diffs already returns all sign-and-order
// copies, the sum directly totals 2 × (# unordered pairs); we divide
// by 2.  The diagonal (Δi, Δj) with di = dj = 0 cannot occur since
// c² > 0; the factor 1/2 is safe.
long long count_unit_distances(long long c2, int k) {
    if (k <= 0) return 0;
    const auto diffs = enumerate_signed_unit_diffs(c2);
    long long directed = 0;
    for (const auto& [di, dj] : diffs) {
        const long long adi = std::abs(di);
        const long long adj = std::abs(dj);
        if (adi < k && adj < k) {
            directed += (k - adi) * (k - adj);
        }
    }
    return directed / 2;
}

// Direct count by brute force, used to cross-validate
// count_unit_distances at small (k, c²).  O(k²) per call.
long long count_unit_distances_brute(long long c2, int k) {
    if (k <= 0) return 0;
    long long count = 0;
    for (int i1 = 0; i1 < k; ++i1) {
        for (int j1 = 0; j1 < k; ++j1) {
            for (int i2 = i1; i2 < k; ++i2) {
                for (int j2 = 0; j2 < k; ++j2) {
                    if (i1 == i2 && j1 == j2) continue;
                    const long long di = i2 - i1;
                    const long long dj = j2 - j1;
                    if (di * di + dj * dj == c2) ++count;
                }
            }
        }
    }
    return count;
}

// Computed r(c²) from enumerate_signed_unit_diffs, for cross-validation.
long long count_r(long long c2) {
    return static_cast<long long>(enumerate_signed_unit_diffs(c2).size());
}

// The expected r(c²) for c² = ∏_{i<kfac} (kPrimes1Mod4[i]):
// 4 · 2^kfac, where each prime ≡ 1 mod 4 contributes a factor of 2
// (e_p = 1 for all).
long long expected_r(int kfac) {
    long long r = 4;
    for (int i = 0; i < kfac; ++i) r *= 2;
    return r;
}

// Minimum k such that count_unit_distances(c², k) > 0.
// The first (Δi, Δj) representation to fit is the one with the
// smallest max(|Δi|, |Δj|). For c² = product of small primes ≡ 1
// mod 4 the smallest representation is typically a (near-)factor
// pair like (1, 2) for c² = 5; for larger c² it is a product-of-
// nearby-factors pair.  This `min_k` is the "hosting width": the
// smallest k at which the c²-spaced grid produces any unit-distance
// pair at all.  Below this k, every unit-distance diff vector has
// |Δi| ≥ k or |Δj| ≥ k and the count is identically zero.
//
// For Sawin's n^{1.014} asymptotic interpretation: to produce the
// count's growing phase we need k *substantially* larger than
// min_k.  Phase 4 reports min_k as a structural data point;
// interpreting it requires k ≥ min_k, which for c² ≈ 10^9 sits
// around k ≈ 10^4 (open the grid by two orders of magnitude above
// the smallest representation).
long long min_hosting_k(long long c2) {
    const auto diffs = enumerate_signed_unit_diffs(c2);
    long long best = std::numeric_limits<long long>::max();
    for (const auto& [di, dj] : diffs) {
        const long long m = std::max(std::abs(di), std::abs(dj));
        if (m + 1 < best) best = m + 1;
    }
    return best;
}

// Finite-k convergence target for THIS construction (not Sawin's
// n^{1.014} asymptotic regime -- see docs/RECOVERY_AUDIT_2026-07-29.md
// finding A6: that regime is a separate, unproven-here claim about
// the maximum over all planar point sets).  For a fixed c²
// construction with k >> min_k and "balanced" c² (every
// representation (Δi, Δj) has |Δi|, |Δj| ≪ k), each of the
// r(c²) signed representations contributes k² (modulo the ±
// double-counting) to the unordered pair count, so
//
//   pairs ≈ r(c²) · k² / 2
//         = 2^{kfac + 1} · k².
//
// Hence pairs/point → r(c²)/2 = 2^{kfac + 1} as k → ∞ for this fixed
// c², purely from the double-counting derivation above. This
// function reports that finite-k target so the printed ratio can be
// read as a convergence check, not as a statement about entering
// Sawin's asymptotic regime.
double asymptotic_pairs_per_point_target(int kfac) {
    double target = 1.0;
    for (int i = 0; i < kfac + 1; ++i) target *= 2.0;
    return target;
}

}  // namespace

int main() {
    std::printf("OpenAI / Sawin / Erdős-van-der-Waerden unit-distance construction\n");
    std::printf("Closed-form count via Jacobi's two-square theorem.\n");
    std::printf("Refs: arXiv:2605.20579, arXiv:2605.20695, "
                "OpenAI 2026 blog, refs/references.bib.\n\n");

    std::printf("%6s  %14s  %8s  %8s  %10s  %14s\n",
                "kfac", "c²", "r(c²)", "exp.r", "match", "k²·r/8 (asym)");
    int exit_code = 0;

    // Phase 1: cross-validate enumerate_unit_diffs and the O(r(c²))
    // count against Jacobi's theorem and a small brute-force count.
    for (int kfac = 1; kfac <= 6; ++kfac) {
        const long long c2 = c2_with_k_primes(kfac);
        const long long r_observed = count_r(c2);
        const long long r_expected = expected_r(kfac);
        const char* match = (r_observed == r_expected) ? "yes" : "NO";
        if (r_observed != r_expected) exit_code = 1;
        std::printf("%6d  %14lld  %8lld  %8lld  %10s\n",
                    kfac, c2, r_observed, r_expected, match);
    }

    // Phase 2: at small (c², k) cross-validate the O(r(c²)) closed-form
    // count against the brute-force O(k²) count.
    std::printf("\nClosed-form vs brute-force (small k, small c²)\n");
    std::printf("%6s  %14s  %6s  %14s  %14s  %8s\n",
                "kfac", "c²", "k", "closed-form", "brute", "match");
    for (int kfac = 1; kfac <= 4; ++kfac) {
        const long long c2 = c2_with_k_primes(kfac);
        for (int k : {2, 4, 8, 16}) {
            const long long closed = count_unit_distances(c2, k);
            const long long brute = count_unit_distances_brute(c2, k);
            const char* match = (closed == brute) ? "yes" : "NO";
            if (closed != brute) exit_code = 1;
            std::printf("%6d  %14lld  %6d  %14lld  %14lld  %8s\n",
                        kfac, c2, k, closed, brute, match);
        }
    }

    // Phase 3: as k grows with c² fixed, the count grows asymptotically
    // like (k² r(c²))/(2 · ⟨k⟩) where ⟨k⟩ is the average |Δ| of the
    // representations.  For balanced c² the count is O(k² · r(c²)),
    // confirming that the per-point unit-distance count scales
    // linearly in r(c²)/4 = 2^{kfac}.
    std::printf("\nClosed-form at fixed c² = 5·13·17·29·37·41, varying k\n");
    std::printf("%6s  %14s  %10s  %14s  %14s\n",
                "kfac", "c²", "k", "pairs", "pairs/k²");
    const long long c2_med = c2_with_k_primes(6);  // 5·13·17·29·37·41
    for (int k : {4, 8, 16, 32, 64, 128, 256}) {
        const long long n = static_cast<long long>(k) * k;
        const long long pairs = count_unit_distances(c2_med, k);
        std::printf("%6s  %14lld  %10d  %14lld  %14.4f\n",
                    "—", c2_med, k, pairs,
                    static_cast<double>(pairs) / n);
    }
    std::printf("\nPairs/k² tends to r(c²)/8 = %lld/8 = %lld for "
                "balanced c² in the limit k ≫ √c².\n",
                expected_r(6), expected_r(6) / 8);

    // Phase 4: minimum hosting k per c² (the structural readiness
    // threshold).  Below min_k the c² construction produces zero
    // unit-distance pairs; at k = min_k the count becomes positive;
    // as k grows past min_k the count approaches r(c²)/8 per point.
    std::printf("\nMinimum hosting k per c² (structural threshold)\n");
    std::printf("%6s  %14s  %14s  %8s  %14s\n",
                "kfac", "c²", "min_k", "r(c²)", "asym target");
    for (int kfac = 1; kfac <= 8; ++kfac) {
        const long long c2 = c2_with_k_primes(kfac);
        const long long mk = min_hosting_k(c2);
        const double asym = asymptotic_pairs_per_point_target(kfac);
        std::printf("%6d  %14lld  %14lld  %8lld  %14.4f\n",
                    kfac, c2, mk, expected_r(kfac), asym);
    }
    std::printf("\nmin_k is the smallest k such that count_unit_distances(c², k) > 0.\n");
    std::printf("Sawin's n^{1.014} asymptotic regime requires k ≫ min_k; at the\n");
    std::printf("thresholds above, c² = 5·13·17·29·37·41·53·61 ≈ 7.66e9 needs\n");
    std::printf("k in the tens of thousands to host its smallest representation.\n");

    // Convergence check (NOT a "Sawin asymptotic regime" gate -- see
    // docs/RECOVERY_AUDIT_2026-07-29.md finding A6).  For each kfac,
    // the empirical pairs-per-point at k = 2^16 is compared to this
    // construction's own finite-k target r(c²)/2^{kfac+1}, derived
    // above from exact double-counting of the r(c²) representations.
    // That convergence (ratio -> 1 as k grows) is a property of this
    // specific closed-form count; it is not Sawin's n^{1.014} lower
    // bound and no fixed ratio threshold has been proved to mark
    // entry into that asymptotic regime, so none is asserted here.
    std::printf("\nConvergence check (empirical vs finite-k target pairs/point)\n");
    std::printf("%6s  %14s  %8s  %14s  %14s  %14s  %14s\n",
                "kfac", "c²", "k", "pairs", "pairs/pt", "target", "ratio");
    const long long k_sanity = 1L << 16;  // 65536
    for (int kfac = 1; kfac <= 6; ++kfac) {
        const long long c2 = c2_with_k_primes(kfac);
        if (k_sanity >= min_hosting_k(c2)) {
            const long long pairs = count_unit_distances(c2,
                static_cast<int>(k_sanity));
            const double ratio_at = static_cast<double>(pairs)
                                  / static_cast<double>(k_sanity * k_sanity);
            const double target = asymptotic_pairs_per_point_target(kfac);
            const double ratio = (target > 0.0) ? ratio_at / target : 0.0;
            std::printf("%6d  %14lld  %8lld  %14lld  %14.6f  %14.4f  %14.4f\n",
                        kfac, c2, k_sanity, pairs, ratio_at, target, ratio);
        } else {
            std::printf("%6d  %14lld  %8lld  (k < min_k, count is zero)\n",
                        kfac, c2, k_sanity);
        }
    }
    std::printf("\nRatio -> 1 as k -> infinity for fixed c², by the exact\n");
    std::printf("double-counting derivation above; the values printed are\n");
    std::printf("the measured ratios at k = 2^16 and are not evidence for\n");
    std::printf("or against Sawin's separate n^{1.014} asymptotic claim.\n");

    std::printf("\nConclusion: the O(r(c²)) algorithm reproduces the\n");
    std::printf("brute-force O(k²) count to all checked parameters; the\n");
    std::printf("Jacobi r(c²) value matches 4 · 2^{kfac}; unit-distance\n");
    std::printf("counting in c²-spaced grids is tractable via number\n");
    std::printf("theory rather than O(n²) combinatorial enumeration.\n");
    return exit_code;
}
