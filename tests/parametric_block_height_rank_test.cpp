#include <cassert>
#include <iostream>
#include "ravel/proof/parametric_block_height_rank.hpp"
using namespace ravel::proof;

static WideRank rank_operation(const ShellState& x) {
    const auto f = derive_block_height_features(x);
    WideRank out = 11;
    for (auto v : x) out += static_cast<WideRank>(v) * v;
    out += 3 * f.forcing_support + 5 * f.forcing_gcd +
           7 * f.forcing_zero_count + f.moment_energy;
    return out;
}

int main() {
    // Non-collinear affine forcing vectors give a finite gcd period.
    const AffineShellRay source{{2,-1,1}, {5,-5,5}};
    const AffineShellRay target{{3,-2,2}, {10,-5,5}};
    const auto cert = derive_parametric_block_height_rank(source,target,rank_operation);
    assert(cert.eventual_feature_grammar);
    if (cert.all_radii_closed) {
        for (std::int64_t q=0;q<200;++q)
            assert(rank_operation(eval_affine_shell_ray(target,q)) >
                   rank_operation(eval_affine_shell_ray(source,q)));
    }
    // The operation must honestly reject collinear forcing rays whose gcd can
    // grow without a fixed determinant period.
    const AffineShellRay collinear{{1,1,1},{1,1,1}};
    const auto bad = derive_parametric_block_height_rank(collinear,collinear,rank_operation);
    assert(!bad.eventual_feature_grammar);
    std::cout << "parametric block-height rank PASS period=" << cert.period
              << " closed=" << cert.all_radii_closed << "\n";
}
