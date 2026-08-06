#include <cstdio>
#include <vector>
#include "ravel/proof/reflective_weight_family.hpp"

int main() {
    const std::vector<std::vector<long long>> cycle3{
        {0,1,0}, {0,0,1}, {1,0,0}
    };
    const std::vector<std::vector<long long>> cycle3_relabelled{
        {0,0,1}, {1,0,0}, {0,1,0}
    };
    const std::vector<std::vector<long long>> fib{
        {1,1}, {1,0}
    };
    const auto corpus = ravel::proof::derive_reflective_weight_family_corpus(
        {cycle3, cycle3_relabelled, fib}, 24);
    if (!corpus.replayed || corpus.members.size() != 2 ||
        corpus.family_of[0] != corpus.family_of[1] ||
        corpus.family_of[0] == corpus.family_of[2] ||
        !corpus.profiles[0].permutation_component ||
        corpus.profiles[2].permutation_component) {
        std::fprintf(stderr, "reflective weight family FAIL\n");
        return 1;
    }
    std::printf("reflective weight family PASS families=%zu\n", corpus.members.size());
    return 0;
}
