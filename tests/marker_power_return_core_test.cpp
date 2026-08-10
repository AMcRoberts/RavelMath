#include <cstdio>

#include "ravel/marker_power_return_core.hpp"
#include "ravel/simplest_nonunit_pisot_substitution.hpp"

using namespace ravel;

namespace {
int tests = 0;
int failures = 0;

void expect(bool value, const char* label) {
    ++tests;
    if (value) std::printf("  [ok]   %s\n", label);
    else { std::printf("  [FAIL] %s\n", label); ++failures; }
}
}

int main() {
    const auto non_ar = analyze_marker_power_return_core(
        SubstitutionRule({{1, 2}, {2}, {0}}), 0);
    expect(non_ar.holds, "sigma_{0,1} has a finite recurrent return core");
    expect(non_ar.power == 3, "sigma_{0,1} first marker-proper power is 3");
    expect(non_ar.return_words == 5 && non_ar.phase_states == 16,
           "sigma_{0,1} return core has 5 words and 16 phase states");

    const auto sibling = analyze_marker_power_return_core(
        SubstitutionRule({{1, 1, 2}, {2}, {0}}), 0);
    expect(sibling.holds, "sigma_{0,2} has a finite recurrent return core");
    expect(sibling.power == 3, "sigma_{0,2} first marker-proper power is 3");

    for (int b = 1; b <= 5; ++b) {
        std::vector<std::vector<std::int8_t>> images(3);
        for (int i = 0; i < b; ++i) images[0].push_back(1);
        images[0].push_back(2);
        images[1] = {2};
        images[2] = {0};
        const auto family = analyze_marker_power_return_core(
            SubstitutionRule(images), 0);
        expect(family.holds && family.power == 3,
               "sigma_{0,b}, b<=5, has a power-3 recurrent core");
    }

    const auto left_proper = analyze_marker_power_return_core(
        SubstitutionRule({{0, 1, 2}, {0, 2}, {0}}), 0);
    expect(left_proper.holds && left_proper.power == 1,
           "left-proper control closes at power 1");

    const auto nonunit = analyze_marker_power_return_core(
        SubstitutionRule(simplest_nonunit_pisot_rule()), 0);
    expect(nonunit.holds && nonunit.power == 1,
           "minimal non-unit rule has a finite power-1 core");

    std::printf("%d tests run, %d failed.\n", tests, failures);
    return failures == 0 ? 0 : 1;
}
