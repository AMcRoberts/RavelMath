#include <cassert>
#include <iostream>
#include "ravel/proof/condition_f_parent_pair_recurrence.hpp"
using namespace ravel::proof;
int main() {
    for (std::size_t n=2;n<=128;++n) {
        const auto p = derive_condition_f_parent_pair_recurrence(n);
        assert(p.proved);
        assert(p.old_block.size() == n*n);
        assert(p.new_boundary.size() == 2*n+1);
        std::size_t residual=0, pos=0, neg=0;
        for (const auto& e : p.new_boundary) if (e.generator==1) {
            ++residual; if (e.signed_defect>0) ++pos; else ++neg;
        }
        assert(residual==2 && pos==1 && neg==1);
    }
    assert(!derive_condition_f_parent_pair_recurrence(1).proved);
    std::cout << "Condition-F parent-pair recurrence PASS\n";
}
