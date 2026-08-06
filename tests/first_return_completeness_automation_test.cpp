#include <cassert>
#include <iostream>
#include "ravel/proof/first_return_completeness_automation.hpp"

int main() {
    using namespace ravel::proof;
    const auto result = run_first_return_completeness_automation(3, 3, 2);
    assert(result.fixed_dimension_formulas_exact);
    assert(result.predecessor_closed);
    assert(!result.dimension_parametric);
    assert(!result.universal_theorem_emitted);
    assert(!result.obstruction.empty());
    std::cout << "first return completeness automation boundary PASS\n";
}
