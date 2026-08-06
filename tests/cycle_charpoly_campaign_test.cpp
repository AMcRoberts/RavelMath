#include <cassert>
#include <iostream>
#include <string>

#include "ravel/proof/cycle_charpoly_campaign.hpp"

int main() {
    const std::string lean = ravel::proof::render_cycle_charpoly_lean();
    assert(lean.find("canonicalCycleCore_charpoly") != std::string::npos);
    assert(lean.find("charpoly_leftMulMatrix") != std::string::npos);
    assert(lean.find("AdjoinRoot.minpoly_powerBasis_gen_of_monic") != std::string::npos);
    assert(lean.find(".leftMulMatrix") != std::string::npos);
    assert(lean.find("sorry") == std::string::npos);
    std::cout << lean;
}
