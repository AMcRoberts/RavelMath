#include "ravel/proof/predicted_core_scc_identification.hpp"

#include <cassert>
#include <iostream>

int main() {
    using namespace ravel;
    using namespace ravel::nbonacci_margin;
    using namespace ravel::proof;

    std::size_t states_checked = 0;
    for (std::size_t n = 4; n <= 32; ++n) {
        for (const auto pair : label_pairs(n)) {
            for (const auto descriptor : displacement_descriptors(n)) {
                if (!predicted_core_member(n, pair, descriptor)) continue;
                const CoreState state{pair, descriptor};
                const auto cert = derive_symbolic_shadow_paths(n, state);
                assert(cert.derived);
                assert(cert.edges_replay);
                assert(cert.lengths_exact);
                ++states_checked;
            }
        }
    }

    CoreState malformed{{0, 0},
        {DisplacementKind::Root, 1, 0, 1, 0}};
    const auto rejected = derive_symbolic_shadow_paths(5, malformed);
    assert(!rejected.derived);
    assert(!rejected.target_member);

    std::cout << "predicted-core symbolic induction PASS; checked "
              << states_checked << " formula states through n=32\n";
}
