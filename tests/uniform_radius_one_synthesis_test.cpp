#include <cassert>
#include <iostream>
#include <vector>

#include "ravel/proof/uniform_radius_one_synthesis.hpp"

int main() {
    using namespace ravel::proof;

    const auto cert = synthesize_radius_one_controller(
        3,
        {-1, 0, 1, 0},
        {{0, 1}},
        {{2, -1}});
    assert(cert.closed);
    assert(cert.translation_windows.size() == 5);
    assert(cert.adjusted_digits.size() == 4);

    const auto impossible = synthesize_radius_one_controller(
        0, {}, {}, {});
    assert(!impossible.closed);

    std::cout << "uniform radius-one synthesis PASS\n";
}
