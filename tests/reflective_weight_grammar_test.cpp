#include <cstdio>
#include <vector>
#include "ravel/proof/reflective_weight_grammar.hpp"

int main() {
    // Two interchangeable leaves feeding a common center.  The grammar must
    // discover the leaf role without being told the symmetry.
    const std::vector<std::vector<long long>> m{
        {0,1,0},
        {1,0,1},
        {0,1,0}
    };
    const auto g = ravel::proof::derive_reflective_weight_grammar(m, 16);
    if (!g.equitable || !g.lifted_replay || g.roles() != 2 ||
        g.role_of[0] != g.role_of[2] || g.role_of[0] == g.role_of[1]) {
        std::fprintf(stderr, "reflective weight grammar FAIL\n");
        return 1;
    }
    std::printf("reflective weight grammar PASS roles=%zu rounds=%zu\n",
                g.roles(), g.refinement_rounds);
    return 0;
}
