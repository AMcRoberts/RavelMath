#include <cstdio>
#include <vector>
#include "ravel/proof/path_count_cone.hpp"

int main() {
    const std::vector<std::vector<long long>> competitor{{0,1},{1,0}};
    const std::vector<std::vector<long long>> core{{1,1},{1,1}};
    const auto proof = ravel::proof::derive_path_count_cone_pair(competitor, core, 8);
    if (!proof.replayed || proof.first_separating_horizon != 0) {
        std::fprintf(stderr, "path-count cone FAIL horizon=%zu\n", proof.first_separating_horizon);
        return 1;
    }
    std::printf("path-count cone PASS horizon=%zu\n", proof.first_separating_horizon);
    return 0;
}
