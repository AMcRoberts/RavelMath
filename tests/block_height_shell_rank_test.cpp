#include <cassert>
#include <iostream>
#include <tuple>
#include <vector>

#include "ravel/proof/block_height_shell_rank.hpp"

using namespace ravel::proof;

static void check_feature_identities() {
    for (std::size_t n = 2; n <= 8; ++n) {
        ShellState x(n, 0);
        for (std::size_t i = 0; i < n; ++i) x[i] = static_cast<std::int64_t>(i) - 2;
        const auto c = block_forcing_dual_coefficients(x);
        assert(c.size() == n + 1);
        std::int64_t support = 0;
        for (auto v : c) support += std::llabs(v);
        assert(support == block_forcing_support(x));
        const auto moments = carry_krylov_moments(x);
        assert(moments.size() == n + 1);
    }
}

int main() {
    check_feature_identities();

    struct Job { std::size_t n; std::int64_t lo; std::int64_t hi; };
    const std::vector<Job> jobs{{3,2,8},{4,2,6},{5,2,4}};
    for (const auto& job : jobs) {
        for (const auto [block,height] : std::vector<std::pair<bool,bool>>{
                 {true,false},{false,true},{true,true}}) {
            const auto result = derive_block_height_shell_rank(
                job.n, job.lo, job.hi, block, height);
            assert(result.rank.feasible && result.rank.replay_checked);
            std::cout << "block-height shell rank n=" << job.n
                      << " M=" << job.lo << ".." << job.hi
                      << " block=" << block << " height=" << height
                      << " phases=" << result.rank.phases.size()
                      << " constraints=" << result.rank.constraints.size()
                      << " raw_edges=" << result.raw_edges << " PASS\n";
        }
    }
    return 0;
}
