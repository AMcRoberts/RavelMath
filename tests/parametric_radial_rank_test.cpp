#include <cassert>
#include <iostream>
#include "ravel/proof/parametric_radial_rank.hpp"

using namespace ravel::proof;

static WideRank synthetic_rank(const ShellState& x) {
    WideRank out = 7;
    for (std::size_t i = 0; i < x.size(); ++i) {
        const auto v = static_cast<WideRank>(x[i]);
        out += static_cast<WideRank>(i + 1) * v * v +
               static_cast<WideRank>(3 * i + 1) * v;
    }
    return out;
}

int main() {
    const AffineShellRay source{{1, -2, 0}, {1, -1, 1}};
    const AffineShellRay target{{2, -1, 1}, {2, -1, 1}};
    const auto cert = derive_parametric_radial_rank(source, target, synthetic_rank);
    assert(cert.quadratic_replay);
    assert(cert.all_radii_closed);
    for (std::int64_t q = 0; q < 100; ++q) {
        const auto gain = synthetic_rank(eval_affine_shell_ray(target, q)) -
                          synthetic_rank(eval_affine_shell_ray(source, q));
        assert(gain == eval_integer_quadratic(cert.gain, q));
        assert(gain > 0);
    }

    const auto bad = certify_positive_on_nonnegative_integers({1, 0, -1});
    assert(!bad.strictly_positive_on_nonnegative_integers);
    const auto dip = certify_positive_on_nonnegative_integers({1, -4, 1});
    assert(!dip.strictly_positive_on_nonnegative_integers);
    const auto linear = certify_positive_on_nonnegative_integers({2, 1, 0});
    assert(linear.strictly_positive_on_nonnegative_integers);
    std::cout << "parametric radial rank PASS\n";
}
