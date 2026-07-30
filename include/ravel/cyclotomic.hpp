// cyclotomic.hpp
//
// 12th cyclotomic ring: integers modulo d^4 = d^2 - 1.
// Header-only, no Lua dependency. The Lua bindings in lua_bindings.cpp
// convert between {a,b,c,e} Lua tables and the int64 ring elements
// defined here.

#pragma once

#include <array>
#include <cstdint>

namespace ravel {

struct Cyclotomic12D {
    std::int64_t a = 0;
    std::int64_t b = 0;
    std::int64_t c = 0;
    std::int64_t e = 0;

    constexpr Cyclotomic12D() = default;
    constexpr Cyclotomic12D(std::int64_t a_, std::int64_t b_,
                            std::int64_t c_, std::int64_t e_) noexcept
        : a(a_), b(b_), c(c_), e(e_) {}

    constexpr static Cyclotomic12D zero() noexcept { return {}; }
    constexpr static Cyclotomic12D one() noexcept { return {1, 0, 0, 0}; }
    constexpr static Cyclotomic12D d() noexcept { return {0, 1, 0, 0}; }
    constexpr static Cyclotomic12D d2() noexcept { return {0, 0, 1, 0}; }
    constexpr static Cyclotomic12D d3() noexcept { return {0, 0, 0, 1}; }

    constexpr bool operator==(const Cyclotomic12D& o) const noexcept {
        return a == o.a && b == o.b && c == o.c && e == o.e;
    }
    constexpr bool operator!=(const Cyclotomic12D& o) const noexcept {
        return !(*this == o);
    }

    constexpr Cyclotomic12D operator+(const Cyclotomic12D& o) const noexcept {
        return {a + o.a, b + o.b, c + o.c, e + o.e};
    }
    constexpr Cyclotomic12D operator-(const Cyclotomic12D& o) const noexcept {
        return {a - o.a, b - o.b, c - o.c, e - o.e};
    }
    constexpr Cyclotomic12D operator-() const noexcept {
        return {-a, -b, -c, -e};
    }
    constexpr Cyclotomic12D operator*(std::int64_t k) const noexcept {
        return {a * k, b * k, c * k, e * k};
    }

    Cyclotomic12D operator*(const Cyclotomic12D& o) const noexcept {
        // Multiplication in the 12th cyclotomic ring, reduced by
        // d^4 = d^2 - 1. See docs/THEOREM_STATUS.md section 4.
        const auto a1 = a, b1 = b, c1 = c, e1 = e;
        const auto a2 = o.a, b2 = o.b, c2 = o.c, e2 = o.e;
        const std::int64_t constant = a1 * a2 - (b1 * e2 + c1 * c2 + e1 * b2)
                                  - (c1 * e2 + e1 * c2) - e1 * e2;
        const std::int64_t d1 = a1 * b2 + b1 * a2 - (c1 * e2 + e1 * c2);
        const std::int64_t d2c = a1 * c2 + b1 * b2 + c1 * a2
                               + (b1 * e2 + c1 * c2 + e1 * b2);
        const std::int64_t d3 = a1 * e2 + b1 * c2 + c1 * b2 + e1 * a2
                              + (c1 * e2 + e1 * c2);
        return {constant, d1, d2c, d3};
    }
};

// 12th cyclotomic constants. These are the only place where the
// decimal mantissa of sqrt(3)/2 lives.  See
// include/ravel/cyclotomic_constants.hpp in the C++ project for
// the source-of-truth rationale.  We name them with the same names
// as the original project so the Lua wrappers read identically.
inline constexpr long double kHalf = 0.5L;
inline constexpr long double kOne = 1.0L;
inline constexpr long double kZero = 0.0L;
inline constexpr long double kSqrt3Over2 = 0.86602540378443864676L;

inline constexpr long double kDGeneratorX = kSqrt3Over2;
inline constexpr long double kDGeneratorY = kHalf;
inline constexpr long double kD2GeneratorX = kHalf;
inline constexpr long double kD2GeneratorY = kSqrt3Over2;
inline constexpr long double kD3GeneratorX = kZero;
inline constexpr long double kD3GeneratorY = kOne;

inline std::array<double, 2> cyclotomic_to_xy(const Cyclotomic12D& z) noexcept {
    const double root3_over_2 = static_cast<double>(kSqrt3Over2);
    return {
        static_cast<double>(z.a) + root3_over_2 * static_cast<double>(z.b)
            + static_cast<double>(kHalf) * static_cast<double>(z.c),
        static_cast<double>(z.e) + root3_over_2 * static_cast<double>(z.c)
            + static_cast<double>(kHalf) * static_cast<double>(z.b)
    };
}

}  // namespace ravel
