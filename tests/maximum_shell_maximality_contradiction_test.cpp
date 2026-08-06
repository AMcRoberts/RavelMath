#include <cassert>
#include <iostream>

#include "ravel/proof/maximum_shell_maximality_contradiction.hpp"
#include "ravel/proof/periodic_word_controller_cycle.hpp"

using namespace ravel::proof;

int main() {
    FirstReturnJointProduct product;
    product.dimension = 1;
    product.controller_states = {{-1}, {1}};
    product.controller_plant = make_symbolic_controller_plant(product.controller_states);
    product.replayed = true;

    FirstReturnWitness segment;
    segment.dimension = 1;
    segment.bound = 2;
    segment.digits = {0};
    segment.base_path = {{2}, {2}};
    segment.target = {2};
    auto lift = derive_role_constrained_controller_lift(product, {segment}, {{0, 1}});
    auto source = replay_carry_cycle("maximality.source", {{2}, {2}}, {0});
    auto pump = certify_role_constrained_shell_pump(
        "maximality.pump", source, product, std::move(lift));
    const auto cert = certify_maximum_shell_maximality_contradiction(2, pump);
    assert(cert.valid);
    assert(cert.lifted_radius == 3);

    const auto wrong = certify_maximum_shell_maximality_contradiction(3, pump);
    assert(!wrong.valid);
    std::cout << "maximum shell maximality contradiction PASS\n";
}
