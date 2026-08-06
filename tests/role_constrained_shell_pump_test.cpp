#include <cassert>
#include <iostream>

#include "ravel/proof/periodic_word_controller_cycle.hpp"
#include "ravel/proof/role_constrained_shell_pump.hpp"

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
    const auto lift = derive_role_constrained_controller_lift(
        product, {segment}, {{0, 1}});
    assert(lift.valid);

    const auto source = replay_carry_cycle(
        "role-shell.synthetic.source", {{2}, {2}}, {0});
    const auto cert = certify_role_constrained_shell_pump(
        "role-shell.synthetic", source, product, lift);
    assert(cert.valid);
    assert(cert.source_role_is_active_face);
    assert(cert.strict_pump.valid);
    assert(cert.strict_pump.lifted_radius == 3);

    auto bad_lift = lift;
    bad_lift.lap_roles = {{0, -1}};
    const auto bad = certify_role_constrained_shell_pump(
        "role-shell.synthetic.bad", source, product, bad_lift);
    assert(!bad.valid);

    std::cout << "role constrained shell pump PASS repetitions="
              << cert.repeated_source_cycle.digits.size() << "\n";
}
