#include <cassert>
#include <iostream>

#include "ravel/proof/covering_translation_tube.hpp"

int main() {
    using namespace ravel::proof;

    const auto homogeneous = certify_defect_spliced_tube(
        "n3.homogeneous",
        2,
        1,
        {{1, -2, 1}, {-2, 1, 1}},
        {{-1, -1, -1}, {-1, -1, 1}},
        {-1},
        {0});
    assert(homogeneous.splice_recurrence_checked);
    assert(homogeneous.adjusted_digits_admissible);
    assert(homogeneous.transported_replay_checked);
    assert(homogeneous.first_return_transport);

    const auto defect = certify_defect_spliced_tube(
        "n3.defect",
        2,
        1,
        {{2, -1, 1}, {-1, 1, 2}},
        {{1, -1, 0}, {-1, 0, 1}},
        {0},
        {-1});
    assert(defect.splice_recurrence_checked);
    assert(defect.adjusted_digits == std::vector<std::int64_t>({-1}));
    assert(defect.transported_replay_checked);
    assert(defect.first_return_transport);

    std::cout << "defect-spliced tube certificate PASS\n";
}
