// Test: fft.hpp (radix-2 Cooley-Tukey FFT in mathlib).
//
// Verifies F^{-1} . F = I to 1e-9 absolute error on multi-component
// signals, plus a peak-bin check (the strongest single-frequency
// recovery test).

#include <array>
#include <cmath>
#include <complex>
#include <cstdio>
#include <cstdlib>

#include "math/fft.hpp"
#include "math/fft_strong.hpp"

#include "test_common.hpp"

using namespace mathlib;

int main() {
    std::printf("== fft: radix-2 Cooley-Tukey ==\n");

    // F^{-1} . F = I on 5 random multi-component signals of length 256.
    {
        constexpr std::size_t N = 256;
        bool ok = true;
        double worst = 0.0;
        for (std::size_t t = 0; t < 5; ++t) {
            std::array<std::complex<double>, N> data{};
            const double f1 = 3.0 + static_cast<double>(t);
            const double f2 = 17.0 + static_cast<double>(t * 2);
            const double f3 = 64.0;
            for (std::size_t n = 0; n < N; ++n) {
                data[n] = std::complex<double>(
                    std::sin(2.0 * 3.14159265358979323846 * f1 * n / N)
                    + 0.5 * std::sin(2.0 * 3.14159265358979323846
                                       * f2 * n / N)
                    + 0.3 * std::cos(2.0 * 3.14159265358979323846
                                       * f3 * n / N),
                    0.0);
            }
            std::array<std::complex<double>, N> original = data;
            fft_inplace<N>(data, /*inverse=*/false);
            fft_inplace<N>(data, /*inverse=*/true);
            for (std::size_t n = 0; n < N; ++n) {
                const double err = std::abs(data[n] - original[n]);
                if (err > worst) worst = err;
                if (err > 1e-9) ok = false;
            }
        }
        EXPECT(ok && worst < 1e-9, "F^{-1} . F = I within 1e-9");
        std::printf("  worst abs err over 5 trials: %.3e\n", worst);
    }

    // Single-frequency peak-bin recovery: a pure sine at bin 4 should
    // have its peak at bin 4 after a forward FFT.
    {
        constexpr std::size_t N = 64;
        constexpr std::size_t peak_bin = 4;
        std::array<std::complex<double>, N> data{};
        for (std::size_t n = 0; n < N; ++n) {
            data[n] = std::complex<double>(
                std::cos(2.0 * 3.14159265358979323846 * peak_bin * n / N),
                0.0);
        }
        fft_inplace<N>(data, /*inverse=*/false);
        std::size_t peak = 0;
        double peak_mag = 0.0;
        for (std::size_t k = 1; k < N; ++k) {
            const double m = std::abs(data[k]);
            if (m > peak_mag) { peak_mag = m; peak = k; }
        }
        EXPECT(peak == peak_bin, "single-frequency peak at expected bin");
        std::printf("  peak bin = %zu (expected %zu), magnitude = %.3e\n",
                    peak, peak_bin, peak_mag);
    }

    // Exact-rational FFT, N = 4, against the same input shape.
    // Bit-exact: a constant input [1, 1, 1, 1] has DFT [4, 0, 0, 0].
    {
        std::array<ComplexRat, 4> data{};
        for (auto& x : data) x = ComplexRat(Rat(1), Rat(0));
        fft_exact<4>(data, /*inverse=*/false);
        EXPECT(is_zero(data[0].re) == false, "bin 0 nonzero");
        EXPECT_EQ_BI(data[0].re, BigInt(4));
        EXPECT(is_zero(data[1].re), "bin 1 zero (real)");
        EXPECT(is_zero(data[2].re), "bin 2 zero (real)");
        EXPECT(is_zero(data[3].re), "bin 3 zero (real)");
        for (std::size_t k = 0; k < 4; ++k) {
            EXPECT(is_zero(data[k].im), "all im parts zero");
        }
    }
    // Inverse round-trip: F^{-1} · F = I on a non-trivial input.
    {
        std::array<ComplexRat, 4> data{};
        data[0] = ComplexRat(Rat(1), Rat(2));
        data[1] = ComplexRat(Rat(3), Rat(0));
        data[2] = ComplexRat(Rat(-1), Rat(1));
        data[3] = ComplexRat(Rat(2), Rat(-2));
        std::array<ComplexRat, 4> orig = data;
        fft_exact<4>(data, false);
        fft_exact<4>(data, true);
        for (std::size_t k = 0; k < 4; ++k) {
            EXPECT(cmp(data[k].re, orig[k].re) == 0, "real round-trip");
            EXPECT(cmp(data[k].im, orig[k].im) == 0, "imag round-trip");
        }
    }

    // Strong-FFT interface: double and exact paths agree on the
    // closed-form case.
    {
        // Constant input [1, 1, 1, 1]: DFT = [4, 0, 0, 0].
        std::array<std::complex<double>, 4> d{};
        for (auto& x : d) x = std::complex<double>(1.0, 0.0);
        const auto dout = fft_strong_double<4>(std::move(d));
        EXPECT(std::abs(dout[0] - std::complex<double>(4.0, 0.0)) < 1e-9,
               "double strong: bin 0 = 4.0");
        for (std::size_t k = 1; k < 4; ++k) {
            EXPECT(std::abs(dout[k]) < 1e-9, "double strong: zero bins");
        }
        std::array<ComplexRat, 4> e{};
        for (auto& x : e) x = ComplexRat(Rat(1), Rat(0));
        const auto eout = fft_strong_exact<4>(std::move(e));
        EXPECT(cmp(eout[0].re, BigInt(4)) == 0, "exact strong: bin 0 = 4");
        for (std::size_t k = 1; k < 4; ++k) {
            EXPECT(is_zero(eout[k].re), "exact strong: zero bin real");
            EXPECT(is_zero(eout[k].im), "exact strong: zero bin imag");
        }
    }

    std::printf("fft: all checks passed\n");
    return EXIT_SUCCESS;
}