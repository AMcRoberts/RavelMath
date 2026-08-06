#include <cstdio>
#include <vector>

#include "ravel/proof/adjacent_twisted_renewal.hpp"

int main() {
    using namespace ravel::proof;
    PhaseBellmanGrammar lower, upper;
    lower.phase_count = 2;
    lower.competitor_profiles = {{{0,1}}, {{1,0}}};
    lower.core_incoming_profiles = {{{1,1}}, {{1,1}}};
    lower.competitor_active = {true,true};
    lower.core_active = {true,true};
    lower.replayed = true;
    upper.phase_count = 3;
    upper.competitor_profiles = {{{0,0,1}}, {}, {{1,0,0}}};
    upper.core_incoming_profiles = {{{1,0,1}}, {}, {{1,0,1}}};
    upper.competitor_active = {true,false,true};
    upper.core_active = {true,false,true};
    upper.replayed = true;
    const auto proof = derive_adjacent_twisted_renewal(
        4, 2, 6, 14, true, true, true, true, true,
        lower, upper, {0,2}, {0,1}, 8);
    std::printf("adjacent twisted renewal %s block=%zu obstruction=%s\n",
        proof.proved ? "PASS" : "FAIL", proof.bellman.renewal_block,
        proof.obstruction.empty() ? "none" : proof.obstruction.c_str());
    return proof.proved ? 0 : 1;
}
