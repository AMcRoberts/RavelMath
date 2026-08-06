#include <iostream>
#include <stdexcept>
#include <tuple>
#include <vector>

#include "ravel/contact_boundary.hpp"
#include "ravel/d_cont_check.hpp"
#include "ravel/generalized_multinacci.hpp"
#include "ravel/proof/generalized_multinacci_admissible_subgrammar.hpp"

using namespace ravel;

static ravel::proof::NonnegativeMatrix matrix(std::initializer_list<std::initializer_list<long long>> rows) {
    ravel::proof::NonnegativeMatrix out;
    for(const auto& r:rows) out.emplace_back(r);
    return out;
}

template <std::size_t d>
void check(std::size_t m) {
    const auto raw=generalized_multinacci_rule(d,m);
    SubstitutionRule rule(raw);
    const auto beta=generalized_multinacci_beta(d,m);
    const auto subst=make_substitution<d>(rule,beta);
    const auto cands=search_D_cont<d>(subst,2);
    std::vector<std::tuple<long long,std::vector<long long>,long long>> dcont;
    for(const auto& c:cands)
        dcont.emplace_back(c.i,std::vector<long long>(c.x.begin(),c.x.end()),c.j);
    const auto rep=compute_contact_boundary<d>(rule,beta,0.0,dcont);
    const auto lift=ravel::proof::derive_generalized_multinacci_boundary_word_lift<d>(subst,rep,m);

    // A deliberately nontrivial common intertwiner.
    const auto Qc=matrix({{1,1},{0,1}});
    const auto Rc=matrix({{0,1},{0,0}});
    const auto Qk=matrix({{2,1},{0,1}});
    const auto Rk=matrix({{0,1},{0,1}});
    const auto P=matrix({{1,0},{0,1}});
    const auto proof=ravel::proof::derive_generalized_multinacci_admissible_subgrammar(
        lift,Qc,Rc,Qk,Rk,P);
    if(!proof.proved)
        throw std::runtime_error("D="+std::to_string(d)+" m="+std::to_string(m)+": "+proof.obstruction);
    std::cout << "D="<<d<<" m="<<m<<" states="<<proof.source_states
              <<" edges="<<proof.witnessed_edges<<" words="<<proof.words_checked<<"\n";
}

int main() {
    for(std::size_t m=1;m<=6;++m) {
        check<2>(m);
        check<3>(m);
        check<4>(m);
    }
    // One larger-D stress sample for each of the first three multiplicities.
    for(std::size_t m=1;m<=3;++m) check<5>(m);
    std::cout << "generalized multinacci admissible subgrammar PASS\n";
}
