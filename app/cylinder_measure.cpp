// cylinder_measure.cpp
//
// Compute cylinder measures for primitive Pisot substitutions.
//
//   mu([w]) = lim_{n->inf} |sigma^n(w)| / beta^n
//
// FIX 1 (crash): the original computed this by literally building
// sigma^n(w) as a std::vector<int> and reading .size(). That is
// exponential in n (length ~ beta^n) and overflows both memory and
// a 64-bit counter for any interesting n. Here mu(w) is instead the
// limit of the stable linear recursion y <- M y / beta applied to
// the abundance vector of w, which is O(1) work per step, never
// overflows, and converges geometrically at rate (lambda2/beta)^n.
//
// FIX 2 (silent no-op test): sigma() is a concatenation homomorphism,
// so |sigma^n(a·b)| == |sigma^n(a)| + |sigma^n(b)| ALWAYS, by
// construction -- re-expanding the seed word "ab" can never test
// whether the block "ab" is over/under-represented as a factor
// inside long expansions (the actual Exact Regularity Property
// question). That requires counting literal occurrences of "ab" in
// one realized long word, which is what count_occurrences() is for.
// It is now actually called.
//
// FIX 3 (alphabet size): the original was hard-coded to d=4.  Per
// DIRECTION_AND_OPEN_THREADS.md thread C item 1 and
// RESEARCH_VECTORS.md #6, the cylinder_measure machinery has been
// generalized to arbitrary alphabet size d (template on d).  This
// enables the systematic Exact Regularity Property survey
// (RESEARCH_VECTORS.md #6) over the full 39-substitution and
// 87-candidate populations, not just 4-letter.

#include <array>
#include <cmath>
#include <cstdio>
#include <cstddef>
#include <string>
#include <vector>

#include "ravel/spectral.hpp"

using namespace std;

long long count_occurrences(const vector<int>& u, const vector<int>& v) {
    if (u.empty()) return (long long)v.size() + 1;
    if (u.size() > v.size()) return 0;
    long long count = 0;
    for (size_t i = 0; i + u.size() <= v.size(); ++i) {
        bool match = true;
        for (size_t j = 0; j < u.size(); ++j)
            if (v[i + j] != u[j]) { match = false; break; }
        if (match) ++count;
    }
    return count;
}

// General-d Subst: takes alphabet size d at construction time.  The
// matrix M is d x d, the abundance vector y has d components, and
// the apply/measure/realize methods are templated on d.  No d=4
// hardcoding.
template <std::size_t d>
struct Subst {
    static_assert(d >= 2, "Subst requires alphabet size >= 2");
    using Image = vector<int>;
    std::array<Image, d> images;
    long long M[d][d];
    double beta;

    // beta (and, incidentally, beta2/det/pisot) comes from the
    // shared spectral_invariants_general in spectral.hpp.  The v/u
    // eigenvectors this struct used to compute were never actually
    // read anywhere in this file (measure() below does its own
    // independent normalized iteration on the word's abundance
    // vector), so they're dropped rather than ported.
    Subst(const std::array<Image, d>& imgs) : images(imgs) {
        for (std::size_t i = 0; i < d; ++i)
            for (std::size_t j = 0; j < d; ++j) M[i][j] = 0;
        for (std::size_t j = 0; j < d; ++j)
            for (int c : images[j]) M[c][j] += 1;

        std::vector<std::vector<long long>> Mvec(d, std::vector<long long>(d));
        for (std::size_t i = 0; i < d; ++i)
            for (std::size_t j = 0; j < d; ++j) Mvec[i][j] = M[i][j];
        beta = ravel::spectral_invariants_general(Mvec).beta;
    }

    vector<int> apply(const vector<int>& w) const {
        vector<int> r;
        for (int c : w) r.insert(r.end(), images[c].begin(), images[c].end());
        return r;
    }

    // Stable |sigma^n(w)| / beta^n via normalized power iteration on
    // the abundance vector of w. O(n) work, no overflow, ever.
    double measure(const vector<int>& w, int n) const {
        std::array<double, d> y{};
        for (int c : w) y[c] += 1.0;
        for (int it = 0; it < n; ++it) {
            std::array<double, d> ny{};
            for (std::size_t i = 0; i < d; ++i) {
                double s = 0;
                for (std::size_t j = 0; j < d; ++j) s += M[i][j] * y[j];
                ny[i] = s / beta;
            }
            y = ny;
        }
        double s = 0;
        for (double c : y) s += c;
        return s;
    }

    // Materialize one realized word sigma^N(seed), capped so it stays
    // in memory (single pass, no wasted re-expansion).
    vector<int> realize(int seed, long long cap) const {
        vector<int> cur = {seed};
        while (true) {
            vector<int> next = apply(cur);
            if ((long long)next.size() > cap) return cur;
            cur = next;
        }
    }
};

template <std::size_t d>
void analyze_subst(const string& name, const Subst<d>& s) {
    printf("\n=== %s (beta=%.6f, d=%zu) ===\n", name.c_str(), s.beta, d);
    printf("  Substitution images:\n");
    for (std::size_t i = 0; i < d; ++i) {
        printf("    sigma(%zu) = (", i);
        for (int c : s.images[i]) printf("%d ", c);
        printf(") (length %zu)\n", s.images[i].size());
    }

    printf("\n  mu([{k}]) = |sigma^n({k})| / beta^n  (stable recursion, no crash for any n):\n");
    printf("  %6s |", "n");
    for (std::size_t k = 0; k < d; ++k) printf(" %10s", ("{"+to_string(k)+"}").c_str());
    printf("\n");
    for (int n : {5, 10, 20, 40, 80, 160, 320, 1000}) {
        printf("  %6d |", n);
        for (std::size_t k = 0; k < d; ++k) printf(" %10.6f", s.measure({(int)k}, n));
        printf("\n");
    }

    printf("\n  REAL factorization check (frequency of literal factor 'ab'\n");
    printf("  inside one realized long word, vs product of single-letter freqs):\n");
    long long cap = 20'000'000;
    vector<int> W = s.realize(0, cap);
    long long total = (long long)W.size();
    printf("  realized |W| = %lld letters (cap %lld)\n", total, cap);

    std::vector<double> freq1(d);
    for (std::size_t k = 0; k < d; ++k) {
        freq1[k] = (double)count_occurrences({(int)k}, W) / (double)total;
    }
    printf("  %-6s %10s %10s %10s\n", "(a,b)", "freq(ab)", "fa*fb", "ratio");
    for (std::size_t a = 0; a < d; ++a) {
        for (std::size_t b = 0; b < d; ++b) {
            long long c_ab = count_occurrences({(int)a, (int)b}, W);
            double freq_ab = (double)c_ab / (double)(total - 1);
            double prod = freq1[a] * freq1[b];
            double ratio = prod > 0 ? freq_ab / prod : NAN;
            printf("  (%zu,%zu)   %10.6f %10.6f %10.6f\n", a, b, freq_ab, prod, ratio);
        }
    }
}

int main() {
    // n=4 cases (the original 4-letter survey).
    Subst<4> tetra({
        vector<int>{0, 1}, vector<int>{0, 2}, vector<int>{0, 3}, vector<int>{0}
    });
    analyze_subst<4>("Tetrabonacci (unimodular, |det|=1)", tetra);

    Subst<4> rnd13({
        vector<int>{0, 0, 1, 2, 3, 3},
        vector<int>{0, 0, 2, 3, 3},
        vector<int>{0, 0, 3, 3},
        vector<int>{0, 0, 0, 2, 3, 3}
    });
    analyze_subst<4>("rnd13 (non-unimodular, |det|=2)", rnd13);

    // n=3 cases (the original 3-letter survey).  Per Item 4
    // (DIRECTION thread C), the Subst struct is now templated on
    // d; this verifies the d != 4 path works.  Tribonacci and
    // Plastic are the canonical n=3 Pisots.
    Subst<3> trib({
        vector<int>{0, 1}, vector<int>{0, 2}, vector<int>{0}
    });
    analyze_subst<3>("Tribonacci (unimodular, d=3)", trib);

    Subst<3> plastic({
        vector<int>{1}, vector<int>{2}, vector<int>{0, 1}
    });
    analyze_subst<3>("Plastic (unimodular, d=3)", plastic);

    return 0;
}
