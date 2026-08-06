#include <cassert>
#include <iostream>

#include "ravel/proof/condition_f_pair_boundary_substitution.hpp"

using namespace ravel::proof;

int main() {
    for(std::size_t D=2;D<=256;++D) {
        const auto s=derive_condition_f_pair_boundary_substitution(D);
        assert(s.proved);
        assert(s.excursions.size()==8*D-1);
        assert(s.length_two==8*D-5);
        assert(s.length_three==4);
        assert(s.net_q+s.net_r==s.excursions.size());
    }

    // Joint pair inequality with a nontrivial rectangular intertwiner.
    // Competitor generators are coordinate restrictions of the core pair.
    const NonnegativeMatrix qc{{1,0},{0,1}};
    const NonnegativeMatrix rc{{0,1},{1,0}};
    const NonnegativeMatrix qk{{1,0,0},{0,1,0},{0,0,2}};
    const NonnegativeMatrix rk{{0,1,0},{1,0,0},{0,0,1}};
    const NonnegativeMatrix P{{1,0,0},{0,1,0}};
    const auto s=derive_condition_f_pair_boundary_substitution(7);
    const auto proof=derive_condition_f_pair_substitution_monotonicity(
        qc,rc,qk,rk,P,s);
    assert(proof.proved);
    assert(proof.words_checked==s.word_multiplicity.size());
    assert(proof.polynomial_terms==s.excursions.size());

    auto bad_qc=qc;
    bad_qc[0][0]=3;
    assert(!derive_condition_f_pair_substitution_monotonicity(
        bad_qc,rc,qk,rk,P,s).proved);

    std::cout << "Condition-F pair boundary substitution PASS words="
              << proof.words_checked << " terms=" << proof.polynomial_terms
              << "\n";
}
