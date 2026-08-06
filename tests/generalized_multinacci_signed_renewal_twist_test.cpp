#include <iostream>
#include <stdexcept>
#include <tuple>
#include <vector>

#include "ravel/contact_boundary.hpp"
#include "ravel/d_cont_check.hpp"
#include "ravel/generalized_multinacci.hpp"
#include "ravel/proof/generalized_multinacci_boundary_word_lift.hpp"
#include "ravel/proof/generalized_multinacci_signed_renewal_twist.hpp"

using namespace ravel;

template <std::size_t d>
void check_case(std::size_t m) {
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
    if(!lift.proved) throw std::runtime_error("boundary lift failed: "+lift.obstruction);
    const auto twist=ravel::proof::derive_generalized_multinacci_signed_renewal_twist(lift);
    if(!twist.proved) throw std::runtime_error("signed renewal twist failed: "+twist.obstruction);
    if(twist.maximum_return_time>m && !twist.renewal_edges.empty())
        throw std::runtime_error("roof exceeds multiplicity");
    std::cout << "D="<<d<<" m="<<m
              <<" states="<<twist.base_states
              <<" macro="<<twist.macro_edges
              <<" expanded="<<twist.expanded_states
              <<" roof="<<twist.maximum_return_time
              <<" voltage(-,0,+)="<<twist.negative_voltage_edges<<","<<twist.zero_voltage_edges<<","<<twist.positive_voltage_edges
              <<"\n";
}

int main() {
    for (std::size_t m : {1u,2u,3u}) {
        check_case<2>(m);
        check_case<3>(m);
        check_case<4>(m);
        check_case<5>(m);
    }
    std::cout << "generalized multinacci signed renewal twist PASS\n";
}
