#include <cassert>
#include <iostream>
#include <vector>
#include "ravel/proof/condition_f_cyclic_voltage_fourier.hpp"
using namespace ravel::proof;
int main() {
    std::vector<ConditionFCyclicVoltageChannel> channels{
        {0,0,0,0,2}, {1,1,0,0,1},
        {0,1,1,1,1}, {1,0,1,-1,1},
        {0,1,1,2,1}
    };
    auto p = derive_condition_f_cyclic_voltage_fourier(2,5,channels);
    assert(p.proved);
    assert(p.q[0][0] == 2 && p.q[1][1] == 1);
    assert(p.r[0][1] == 2 && p.r[1][0] == 1);
    assert(p.untwisted[0][1] == 2);
    auto twisted_q = channels;
    twisted_q.push_back({0,0,0,1,1});
    assert(derive_condition_f_cyclic_voltage_fourier(2,5,twisted_q).proved);
    auto bad = channels;
    bad.push_back({2,0,0,0,1});
    assert(!derive_condition_f_cyclic_voltage_fourier(2,5,bad).proved);
    std::cout << "Condition-F cyclic voltage Fourier PASS\n";
}
