#include <cassert>
#include <iostream>

#include "ravel/proof/role_constrained_controller_lift.hpp"
#include "ravel/proof/periodic_word_controller_cycle.hpp"

using namespace ravel::proof;

int main() {
    FirstReturnJointProduct product;
    product.dimension = 2;
    product.controller_states = nonzero_radius_one_controller_states(2);
    product.controller_plant = make_symbolic_controller_plant(product.controller_states);
    product.replayed = true;

    // Search a small exact example instead of hand-authoring controller edges.
    RoleConstrainedControllerLiftCertificate good;
    FirstReturnWitness witness;
    witness.dimension = 2;
    witness.bound = 1;
    witness.base_path = {{1, 0}, {1, 0}};
    witness.target = {1, 0};
    for (const auto digit : {-1LL, 0LL, 1LL}) {
        witness.digits = {digit};
        good = derive_role_constrained_controller_lift(
            product, {witness}, {{0, 1}});
        if (good.valid) break;
    }
    assert(good.valid);
    assert(good.controller_cycle.valid);
    assert(good.controller_run.valid);
    assert(good.junction_roles_replayed);

    const auto malformed = derive_role_constrained_controller_lift(
        product, {witness}, {{2, 1}});
    assert(!malformed.valid);

    // This role/word combination is permitted to fail, but must fail honestly
    // at the restricted relation rather than silently dropping the role.
    witness.digits = {-1};
    const auto constrained = derive_role_constrained_controller_lift(
        product, {witness}, {{0, -1}});
    if (!constrained.valid)
        assert(!constrained.obstruction.empty());

    std::cout << "role constrained controller lift PASS period_digits="
              << good.controller_run.base_word.size() << "\n";
}
