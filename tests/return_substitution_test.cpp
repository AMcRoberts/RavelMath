#include <cstdio>
#include <vector>

#include "ravel/return_substitution.hpp"

using namespace ravel;

namespace {
int tests = 0;
int failures = 0;

void expect(bool condition, const char* label) {
    ++tests;
    if (condition) std::printf("  [ok]   %s\n", label);
    else {
        std::printf("  [FAIL] %s\n", label);
        ++failures;
    }
}
}  // namespace

int main() {
    SubstitutionRule sigma({{0, 1, 2}, {0, 2}, {0}});
    auto system = build_return_phase_system(sigma, 0);

    expect(system.induced.words
               == std::vector<ReturnWord>({{0, 1, 2}, {0, 2}, {0}}),
           "sigma_{1,1} return words are 012, 02, 0");
    expect(system.induced.derived_images
               == std::vector<std::vector<std::size_t>>(
                   {{0, 1, 2}, {0, 2}, {0}}),
           "derived return substitution is sigma_{1,1} under relabeling");
    expect(system.states.size() == 6,
           "phase tower has 3+2+1 = 6 states");
    expect(system.phase_images.size() == 6,
           "every phase state has an image");

    const auto matrix = system.incidence_matrix();
    expect(matrix.size() == 6 && matrix[0].size() == 6,
           "phase incidence matrix is 6x6");
    for (std::size_t state = 0; state < system.states.size(); ++state) {
        const auto& phase = system.states[state];
        std::size_t letter = static_cast<std::size_t>(
            system.induced.words[phase.return_word][phase.offset]);
        expect(system.phase_images[state].size() == sigma.image(letter).size(),
               "phase image projects with correct letter-image length");
    }

    std::printf("%d tests run, %d failed.\n", tests, failures);
    return failures == 0 ? 0 : 1;
}
