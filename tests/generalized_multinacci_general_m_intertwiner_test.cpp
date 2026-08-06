#include <cassert>
#include <iostream>

#include "ravel/proof/generalized_multinacci_general_m_intertwiner.hpp"

using namespace ravel::proof;

int main() {
    const NonnegativeMatrix qc{{1,0},{0,1}};
    const NonnegativeMatrix rc{{0,1},{1,0}};
    const NonnegativeMatrix qk{{1,0,0},{0,1,0},{0,0,2}};
    const NonnegativeMatrix rk{{0,1,0},{1,0,0},{0,0,1}};
    const NonnegativeMatrix P{{1,0,0},{0,1,0}};

    for(std::size_t m=1;m<=128;++m) {
        const auto p=derive_generalized_multinacci_general_m_intertwiner(
            m,qc,rc,qk,rk,P);
        assert(p.proved);
        assert(p.symbolic_cut_states==(m+1)*(m+1));
        assert(p.words_checked==m+1);
        assert(p.polynomial_terms==(m+1)*(m+1));
    }

    auto bad=qc; bad[0][0]=3;
    assert(!derive_generalized_multinacci_general_m_intertwiner(
        2,bad,rc,qk,rk,P).proved);
    assert(!derive_generalized_multinacci_general_m_intertwiner(
        0,qc,rc,qk,rk,P).proved);

    std::cout << "generalized multinacci general-m intertwiner PASS\n";
}
