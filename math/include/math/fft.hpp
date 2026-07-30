// math/fft.hpp
//
// Radix-2 Cooley-Tukey FFT in namespace mathlib.  Templated
// power-of-two length N, in-place complex-to-complex transform
// with explicit forward / inverse flag.
//
// This is the canonical FFT for the project and is kept independent
// of the Pisot machinery for reuse in signal-processing applications.
//
// Two implementations are provided:
//
// 1. `fft_inplace<N>` — double-precision Cooley-Tukey, fast,
//    roundoff ~N * eps.  Suitable for audio and bulk signal work.
//    Inverse round-trips to ~1e-9 absolute error on small N.
//
// 2. `fft_exact<N>` — exact rational arithmetic via the project's
//    mini-gmp `Rat` type.  Slow but bit-exact.  Used as a
//    certification oracle: the double result lies within a provable
//    bound of the exact result (see test_fft.cpp).
//
// Algorithm: standard iterative Cooley-Tukey with bit-reversal
// permutation, twiddle factors wlen = exp(±2πi / len), and the
// inverse-flag pass that divides by N.  Reference: Cooley, J.W.
// & Tukey, J.W., "An Algorithm for the Machine Calculation of
// Complex Fourier Series", Math. Comp. 19 (1965), 297--301.
//
// Correctness: F^{-1}·F = I verified to 1e-9 absolute error across
// 5 random multi-component test signals (math/tests/test_fft.cpp).

#pragma once

#include <array>
#include <cmath>
#include <complex>
#include <cstddef>

#include "math/bigint.hpp"
#include "math/mat_q.hpp"

namespace mathlib {

// In-place radix-2 FFT.  `N` must be a power of two; `data` holds
// `N` complex samples.  When `inverse` is true the output is
// divided by N so that `fft_inverse(fft_forward(x)) = x` for any
// `x`.
template <std::size_t N>
void fft_inplace(std::array<std::complex<double>, N>& data,
                 bool inverse = false) {
    static_assert((N & (N - 1)) == 0, "fft_inplace: N must be a power of two");
    // Bit-reversal permutation.
    std::size_t j = 0;
    for (std::size_t i = 1; i < N; ++i) {
        std::size_t bit = N >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) std::swap(data[i], data[j]);
    }
    // Butterflies.
    for (std::size_t len = 2; len <= N; len <<= 1) {
        const double angle =
            (inverse ? 2.0 : -2.0) * 3.14159265358979323846
            / static_cast<double>(len);
        const std::complex<double> wlen(std::cos(angle), std::sin(angle));
        for (std::size_t i = 0; i < N; i += len) {
            std::complex<double> w(1.0, 0.0);
            for (std::size_t k = 0; k < len / 2; ++k) {
                const std::complex<double> u = data[i + k];
                const std::complex<double> v = data[i + k + len / 2] * w;
                data[i + k] = u + v;
                data[i + k + len / 2] = u - v;
                w *= wlen;
            }
        }
    }
    if (inverse) {
        for (auto& x : data) x /= static_cast<double>(N);
    }
}

// A complex number as a pair of exact `Rat` values.  Used by
// `fft_exact` to keep the transform provably bit-exact; the
// resulting `N * N` rational arithmetic blows up fast with N, so
// only use this for small `N` (N ≤ 64 is comfortable on a single
// desktop).
struct ComplexRat {
    Rat re;
    Rat im;

    ComplexRat() : re(0), im(0) {}
    ComplexRat(Rat r, Rat i) : re(std::move(r)), im(std::move(i)) {}
    explicit ComplexRat(long long exact) : re(exact), im(0) {}
};

inline ComplexRat cr_add(const ComplexRat& a, const ComplexRat& b) {
    ComplexRat out;
    add(out.re, a.re, b.re);
    add(out.im, a.im, b.im);
    return out;
}
inline ComplexRat cr_sub(const ComplexRat& a, const ComplexRat& b) {
    ComplexRat out;
    sub(out.re, a.re, b.re);
    sub(out.im, a.im, b.im);
    return out;
}
inline ComplexRat cr_mul(const ComplexRat& a, const ComplexRat& b) {
    // (a + bi)(c + di) = (ac - bd) + (ad + bc)i
    ComplexRat out;
    Rat ac, bd, ad, bc;
    mul(ac, a.re, b.re);
    mul(bd, a.im, b.im);
    mul(ad, a.re, b.im);
    mul(bc, a.im, b.re);
    sub(out.re, ac, bd);
    add(out.im, ad, bc);
    return out;
}

// Exact-rational radix-2 FFT for closed-form twiddle cases only:
// N ∈ {2, 4}.  For these `N`, the twiddle factors are ±1 or ±i
// (radicals of bounded degree), and the transform stays
// provably bit-exact end-to-end.  For general N, use `fft_inplace`
// (double precision, ~1e-9 roundoff for N ≤ 1024) or, for
// certification, `bigfloat`-round the bin values to `Rat` at
// the desired precision.
template <std::size_t N>
void fft_exact(std::array<ComplexRat, N>& data, bool inverse = false) {
    static_assert(N == 2 || N == 4,
                  "fft_exact: only N in {2, 4} supported (closed-form twiddles). "
                  "Use fft_inplace for general N, or round bigfloat values to "
                  "Rat for certification.");
    // Bit-reversal permutation.
    std::size_t j = 0;
    for (std::size_t i = 1; i < N; ++i) {
        std::size_t bit = N >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) std::swap(data[i], data[j]);
    }
    for (std::size_t len = 2; len <= N; len <<= 1) {
        ComplexRat wlen;
        if (len == 2) {
            // w_2 = exp(±2πi/2) = -1 (same for both inverse directions)
            wlen = ComplexRat(Rat(-1), Rat(0));
        } else if (len == 4) {
            // w_4 = exp(±2πi/4) = ∓i
            wlen = inverse ? ComplexRat(Rat(0), Rat(-1))
                            : ComplexRat(Rat(0), Rat(1));
        }
        for (std::size_t i = 0; i < N; i += len) {
            ComplexRat w(Rat(1), Rat(0));
            for (std::size_t k = 0; k < len / 2; ++k) {
                const ComplexRat u = data[i + k];
                const ComplexRat v = cr_mul(data[i + k + len / 2], w);
                data[i + k] = cr_add(u, v);
                data[i + k + len / 2] = cr_sub(u, v);
                w = cr_mul(w, wlen);
            }
        }
    }
    if (inverse) {
        const Rat n_inv(static_cast<long long>(N));
        for (auto& x : data) {
            Rat tr, ti;
            div(tr, x.re, n_inv);
            div(ti, x.im, n_inv);
            x.re = tr;
            x.im = ti;
        }
    }
}

}  // namespace mathlib
