// math/fft_strong.hpp
//
// "Strong FFT" interface for noise-sensitive applications.
//
// Three modes, chosen at the call site. Input is normal double-precision
// audio (or sensor samples) in all modes -- the choice is about
// the OUTPUT's certifiability, not the input's type.
//
//   FFTMode::Double        fast double-precision (via fft_inplace);
//                          returns std::array<std::complex<double>, N>.
//                          ~1e-13 roundoff for N=1024.  Suitable for
//                          audio and bulk signal work.
//
//   FFTMode::Exact         bit-exact rational (via fft_exact) for
//                          closed-form N in {2, 4}.  Returns
//                          std::array<ComplexRat, N>.
//                          Useful for certification: compare the
//                          double result to the exact one and
//                          report the worst-case error bound.
//
//   FFTMode::HighPrecision STUB.  Planned: bigfloat twiddles
//                          rounded to rat, with ball-arithmetic
//                          error envelope per bin.  Returns
//                          std::array<Ball, N>.  Each bin comes
//                          with a provable [lo, hi] interval that
//                          brackets the true DFT output.
//
// Normal sound files come in double precision.  The strong
// interface is about certifying the OUTPUT, not changing the
// input pipeline.
//
// The HighPrecision path is a stub as of 2026-07-28; calling it
// throws std::runtime_error.  Implementation plan:
//
//   1. For each stage `len` of the FFT, compute the twiddle factor
//      wlen = exp(±2πi / len) using bigfloat with user-specified
//      precision (default 64 bits).
//   2. Round to rat, record the rounding error as a ball radius.
//   3. Run the butterfly with ball-arithmetic add/sub/mul so the
//      error envelope accumulates correctly.
//   4. Return each bin as a Ball (lo, hi) with provable width.
//
// When implemented, this gives the EM-coil seeing project a
// certifiable forward model: each bin's interval is guaranteed to
// contain the true DFT output, which means the Moore-Penrose
// gradient step is provably correct in the right direction.

#pragma once

#include <array>
#include <complex>
#include <stdexcept>

#include "math/ball.hpp"
#include "math/fft.hpp"

namespace mathlib {

enum class FFTMode {
    Double,
    Exact,
    HighPrecision,
};

// -----------------------------------------------------------------------------
// FFTMode::Double
// -----------------------------------------------------------------------------
// Fastest path.  In-place radix-2 Cooley-Tukey, double precision.
// Returns the data array (transformed) by value (the input is moved
// in, so the caller can pass an rvalue to keep the no-alias
// guarantee on the in-place transform).
//
// Complexity: O(N log N) double-precision ops.  Roundoff: bounded
// by the same ~N * eps as the underlying fft_inplace.
template <std::size_t N>
std::array<std::complex<double>, N>
fft_strong_double(std::array<std::complex<double>, N> data) {
    fft_inplace<N>(data, /*inverse=*/false);
    return data;
}

// -----------------------------------------------------------------------------
// FFTMode::Exact
// -----------------------------------------------------------------------------
// Bit-exact rational.  Restricted to N in {2, 4} where the twiddle
// factors are closed-form radicals.  For general N, use
// HighPrecision (or Double, for the fast case).
//
// Use case: certification oracle.  Compute the exact DFT for a
// small N, then compare with the Double result to obtain a
// provable error bound on the Double pipeline.
template <std::size_t N>
std::array<ComplexRat, N>
fft_strong_exact(std::array<ComplexRat, N> data) {
    fft_exact<N>(data, /*inverse=*/false);
    return data;
}

// -----------------------------------------------------------------------------
// FFTMode::HighPrecision
// -----------------------------------------------------------------------------
// STUB.  Throws std::runtime_error if called.  Planned
// implementation will use:
//
//   - bigfloat (via math/bigfloat.hpp) for high-precision twiddle
//     factor computation;
//   - rat (via mini-gmp / mpq_t, already wired into ball.hpp) for
//     the butterfly arithmetic;
//   - ball (via math/ball.hpp) for the per-bin certified envelope
//     [lo, hi] that brackets the true DFT output.
//
// The default `precision_bits = 64` is the working target; for
// noise-sensitive applications, bumping this to 128 or 256 should be
// enough to drive
// the per-bin width to below the sensor noise floor.
//
// Throws on call so callers cannot accidentally rely on
// the stub output -- the compile error is intentional.
template <std::size_t N>
std::array<Ball, N>
fft_strong_high_precision(
    const std::array<std::complex<double>, N>& /*input*/,
    unsigned /*precision_bits*/ = 64) {
    throw std::runtime_error(
        "fft_strong_high_precision: not yet implemented. "
        "TODO: bigfloat twiddle computation rounded to rat, with "
        "ball-arithmetic error envelope per bin.  See "
        "math/fft_strong.hpp for the implementation plan.");
}

// -----------------------------------------------------------------------------
// Single-dispatch helper
// -----------------------------------------------------------------------------
// Picks the right implementation based on `mode`. The caller selects
// at the call site; the dispatcher does no implicit
// precision promotion.
template <std::size_t N>
struct StrongFFT {
    using DoubleResult = std::array<std::complex<double>, N>;
    using ExactResult   = std::array<ComplexRat, N>;
    using HighPrecisionResult = std::array<Ball, N>;

    static DoubleResult run(DoubleResult data, std::integral auto /*Double*/) {
        return fft_strong_double<N>(std::move(data));
    }
    static ExactResult run(ExactResult data, std::integral auto /*Exact*/) {
        return fft_strong_exact<N>(std::move(data));
    }
    static HighPrecisionResult run(
        const DoubleResult& data,
        std::integral auto /*HighPrecision*/,
        unsigned precision_bits = 64) {
        return fft_strong_high_precision<N>(data, precision_bits);
    }
};

}  // namespace mathlib
