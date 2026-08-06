#include <cassert>
#include <iostream>
#include "ravel/proof/symbolic_radius_one_controller.hpp"
#include "ravel/proof/uniform_radius_one_synthesis.hpp"

int main() {
    using namespace ravel::proof;
    for (const auto& state : enumerate_radius_one_states(2)) {
        for (const auto digit : {-1LL, 0LL, 1LL}) {
            const auto symbolic = symbolic_successors(state, digit);
            std::set<RadiusOneState> operational;
            for (const auto adjusted : {-1LL, 0LL, 1LL}) {
                const auto next = translation_step(state, adjusted - digit);
                if (is_radius_one(next)) operational.insert(next);
            }
            assert(symbolic == operational);
        }
    }
    std::cout << "symbolic controller normal form PASS\n";
}
