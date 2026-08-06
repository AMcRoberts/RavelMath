#include <iostream>
#include <vector>

#include "ravel/proof/twisted_bellman_transport.hpp"

using namespace ravel::proof;

static PhaseBellmanGrammar make_lower_competitor() {
    PhaseBellmanGrammar g;
    g.phase_count = 2;
    g.competitor_profiles = {{{0,1}}, {{1,0}}};
    g.core_incoming_profiles.resize(2);
    g.competitor_active = {true,true};
    g.core_active = {false,false};
    g.replayed = true;
    return g;
}

static PhaseBellmanGrammar make_lower_core() {
    PhaseBellmanGrammar g;
    g.phase_count = 2;
    g.competitor_profiles.resize(2);
    g.core_incoming_profiles = {{{1,1}}, {{1,1}}};
    g.competitor_active = {false,false};
    g.core_active = {true,true};
    g.replayed = true;
    return g;
}

static PhaseBellmanGrammar embed_comp(const PhaseBellmanGrammar& lower) {
    PhaseBellmanGrammar g;
    g.phase_count = 4;
    g.competitor_profiles.resize(4);
    g.core_incoming_profiles.resize(4);
    g.competitor_profiles[1] = {{0,0,0,1}};
    g.competitor_profiles[3] = {{0,1,0,0}};
    g.competitor_active = {false,true,false,true};
    g.core_active.assign(4,false);
    g.replayed = true;
    return g;
}

static PhaseBellmanGrammar embed_core(const PhaseBellmanGrammar& lower) {
    PhaseBellmanGrammar g;
    g.phase_count = 5;
    g.competitor_profiles.resize(5);
    g.core_incoming_profiles.resize(5);
    g.core_incoming_profiles[0] = {{1,0,1,0,0}};
    g.core_incoming_profiles[2] = {{1,0,1,0,0}};
    g.competitor_active.assign(5,false);
    g.core_active = {true,false,true,false,false};
    g.replayed = true;
    return g;
}

int main() {
    const auto lc = make_lower_competitor();
    const auto lk = make_lower_core();
    const auto uc = embed_comp(lc);
    const auto uk = embed_core(lk);
    const auto proof = derive_twisted_bellman_transport(
        lc, lk, uc, uk,
        {1,3}, {0,2},
        {0,1}, {1,0}, 8);
    std::cout << "twisted Bellman roles=" << proof.lower_roles
              << " competitor_profiles=" << proof.competitor_profiles_checked
              << " core_profiles=" << proof.core_profiles_checked
              << " block=" << proof.renewal_block
              << " result=" << (proof.proved ? "PASS" : "FAIL") << "\n";
    if (!proof.proved) {
        std::cerr << proof.obstruction << "\n";
        return 1;
    }
    return 0;
}
