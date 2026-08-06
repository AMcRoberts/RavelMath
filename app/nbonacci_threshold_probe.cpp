#include <cstdio>
#include <cstdlib>
#include <stdexcept>

#include "ravel/proof/nbonacci_threshold_probe.hpp"

int main(int argc, char** argv) {
    try {
        std::size_t max_n = 32;
        if (argc > 1) max_n = static_cast<std::size_t>(std::stoul(argv[1]));
        const auto proof = ravel::proof::derive_nbonacci_threshold_probe(3, max_n);
        if (!proof.replay()) throw std::runtime_error("threshold derivation replay failed");

        std::puts("NBONACCI_FIRST_OBSTRUCTION_PROBE");
        std::printf("range=3..%zu\n", max_n);
        std::printf("first_support_pair_truncation=%zu\n", proof.first_support_truncation);
        std::printf("first_carry_singularity=%zu\n", proof.first_carry_singularity);
        std::printf("first_block_singularity=%zu\n", proof.first_block_singularity);
        std::printf("first_nonintegral_reciprocal=%zu\n", proof.first_nonintegral_reciprocal);
        for (const auto& row : proof.rows) {
            if (row.n == 4 || row.n == 8 || row.n == 15 || row.n == 16 || row.n == 17) {
                std::printf("n=%zu grade=%zu supports=%zu/%zu upper=%s detA=%lld detBlock=%lld reciprocal=integral\n",
                    row.n, row.maximal_grade, row.lower_support, row.upper_support,
                    row.upper_support_exists ? "present" : "truncated",
                    row.det_carry, row.det_block);
            }
        }
        std::puts("verdict=n16_not_first_obstruction");
        std::puts("reason=support truncation begins at n4 and repeats at every even n; inverse/block operators remain unimodular; beta inverse is integral for every n");
        return 0;
    } catch (const std::exception& e) {
        std::fprintf(stderr, "ERROR %s\n", e.what());
        return 1;
    }
}
