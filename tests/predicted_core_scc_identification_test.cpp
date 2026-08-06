#include "ravel/proof/predicted_core_scc_identification.hpp"

#include <cassert>
#include <iostream>

int main() {
    using namespace ravel::proof;
    std::size_t witness_paths = 0;
    for (std::size_t n = 3; n <= 12; ++n) {
        const auto cert = certify_predicted_core_scc(n);
        assert(cert.replay_checked);
        if (n == 3) continue;
        for (std::size_t k = 0; k < cert.nodes.size(); ++k) {
            if (ravel::nbonacci_margin::lies_in_previous_alphabet_shadow(
                    n, cert.nodes[k])) continue;
            const auto outward = reconstruct_shadow_path(cert.from_shadow, k);
            const auto inward = reconstruct_shadow_path(cert.to_shadow, k);
            assert(replay_path_edges(cert, outward));
            assert(replay_path_edges(cert, inward));
            assert(outward.size() - 1 ==
                ravel::nbonacci_margin::predicted_distance_from_previous_shadow(
                    n, cert.nodes[k]));
            assert(inward.size() - 1 ==
                ravel::nbonacci_margin::predicted_distance_to_previous_shadow(
                    n, cert.nodes[k]));
            ++witness_paths;
        }
    }
    std::cout << "predicted-core SCC identification PASS; replayed "
              << witness_paths * 2 << " shadow witness paths\n";
}
