#include <cassert>
#include <iostream>

#include "adelic/generalized_multinacci_block_affine.hpp"
#include "math/bezout.hpp"
#include "math/qbeta.hpp"
#include "ravel/generalized_multinacci.hpp"

int main() {
    for (std::size_t d = 2; d <= 8; ++d) {
        for (std::size_t m = 1; m <= 6; ++m) {
            const auto maps =
                adelic::derive_generalized_multinacci_block_affine_maps(d, m);
            assert(maps.size() == (d - 1) * m + 1);
            for (const auto& map : maps) {
                assert(map.linear.size() == d);
                assert(map.offset.size() == d);
                for (const auto& row : map.linear) assert(row.size() == d);
            }
        }
    }
    const auto maps = adelic::derive_generalized_multinacci_block_affine_maps(4, 6);
    const auto schema = adelic::derive_generalized_multinacci_block_transport(4, 6);
    mathlib::QBetaRing ring(ravel::generalized_multinacci_polynomial(4, 6));
    const auto beta = ring.beta_k(1);
    const auto inverse = mathlib::invert_in_qbeta(beta, ring);
    assert(inverse.invertible);
    mathlib::QElem gamma(4);
    for (std::size_t i = 0; i < 4; ++i)
        gamma.coeffs_[i] = mathlib::Rat(static_cast<long long>(i + 1));
    for (std::size_t map_index = 0; map_index < maps.size(); ++map_index) {
        auto exact = gamma;
        for (const auto carry : schema.channels[map_index].carry_word)
            exact = ring.add(ring.mul(inverse.inverse, exact),
                             ring.from_int(carry));
        auto integer_image = adelic::multiply(maps[map_index].linear,
            adelic::IntegerVector{1, 2, 3, 4});
        for (std::size_t i = 0; i < 4; ++i)
            integer_image[i] += maps[map_index].offset[i];
        for (std::size_t i = 0; i < 4; ++i) {
            assert(mathlib::cmp(exact.coeffs_[i], mathlib::Rat(integer_image[i])) == 0);
        }
    }
    std::cout << "(4,6) endpoint maps=" << maps.size()
              << " dimension=" << maps.front().linear.size() << "\n"
              << "generalized multinacci block affine PASS\n";
}
