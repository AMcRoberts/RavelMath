#include <iostream>
#include <stdexcept>
#include <tuple>
#include <vector>

#include "ravel/contact_boundary.hpp"
#include "ravel/d_cont_check.hpp"
#include "ravel/generalized_multinacci.hpp"
#include "ravel/proof/generalized_multinacci_boundary_word_lift.hpp"
#include "ravel/proof/generalized_multinacci_primitive_intertwiner.hpp"

using namespace ravel;

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
    const auto p=ravel::proof::derive_generalized_multinacci_primitive_intertwiner<d>(rep,lift);
    if(!p.proved)
        throw std::runtime_error("D="+std::to_string(d)+" m="+std::to_string(m)+": "+p.obstruction);
    std::cout<<"D="<<d<<" m="<<m
             <<" boundary="<<p.boundary_expanded_states
             <<" universal="<<p.universal_expanded_states
             <<" macros="<<p.universal_macro_edges
             <<" phase="<<p.mapped_phase_states<<"\n";
}

int main() {
    for(std::size_t m=1;m<=6;++m) {
        check<2>(m); check<3>(m); check<4>(m);
    }
    for(std::size_t m=1;m<=3;++m) check<5>(m);
    std::cout<<"generalized multinacci primitive intertwiner PASS\n";
}
