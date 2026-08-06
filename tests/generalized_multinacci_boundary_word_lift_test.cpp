#include <iostream>
#include <stdexcept>
#include <tuple>
#include <vector>

#include "ravel/contact_boundary.hpp"
#include "ravel/d_cont_check.hpp"
#include "ravel/generalized_multinacci.hpp"
#include "ravel/proof/generalized_multinacci_boundary_word_lift.hpp"

using namespace ravel;

template <std::size_t d>
ContactBoundaryReport build_report(std::size_t m) {
    const auto raw = generalized_multinacci_rule(d, m);
    SubstitutionRule rule(raw);
    const double beta = generalized_multinacci_beta(d, m);
    const auto subst = make_substitution<d>(rule, beta);
    const auto cands = search_D_cont<d>(subst, 2);
    std::vector<std::tuple<long long,std::vector<long long>,long long>> dcont;
    for (const auto& c : cands)
        dcont.emplace_back(c.i, std::vector<long long>(c.x.begin(),c.x.end()), c.j);
    return compute_contact_boundary<d>(rule,beta,0.0,dcont);
}

template <std::size_t d>
void check_case() {
    constexpr std::size_t m=2;
    const auto raw=generalized_multinacci_rule(d,m);
    SubstitutionRule rule(raw);
    const auto beta=generalized_multinacci_beta(d,m);
    const auto subst=make_substitution<d>(rule,beta);
    const auto rep=build_report<d>(m);
    const auto proof=ravel::proof::derive_generalized_multinacci_boundary_word_lift<d>(subst,rep,m);
    if(!proof.proved) {
        std::cerr << "D="<<d<<" obstruction="<<proof.obstruction<<" nodes="<<rep.gb_matrix.size()<<" edges="<<proof.edges.size()<<"\n";
        std::size_t diffs=0;
        for(std::size_t i=0;i<rep.gb_matrix.size();++i) for(std::size_t j=0;j<rep.gb_matrix.size();++j)
            if(rep.gb_matrix[i][j]!=proof.projected_adjacency[i][j]) {
                if(diffs<20) std::cerr<<" diff "<<i<<"->"<<j<<" gb="<<rep.gb_matrix[i][j]<<" proj="<<proof.projected_adjacency[i][j]<<"\n";
                ++diffs;
            }
        std::size_t tdiffs=0; for(std::size_t i=0;i<rep.gb_matrix.size();++i) for(std::size_t j=0;j<rep.gb_matrix.size();++j) if(rep.gb_matrix[i][j]!=proof.projected_adjacency[j][i]) ++tdiffs; std::cerr<<"diff_count="<<diffs<<" transpose_diff="<<tdiffs<<"\n";
        throw std::runtime_error("D="+std::to_string(d)+": "+proof.obstruction);
    }
    for(const auto& [word,count]:proof.word_multiplicity)
        std::cout<<"D="<<d<<" word="<<word<<" multiplicity="<<count<<"\n";
}

int main(){ check_case<2>(); check_case<3>(); check_case<4>(); check_case<5>(); std::cout<<"generalized multinacci boundary word lift PASS\n"; }
