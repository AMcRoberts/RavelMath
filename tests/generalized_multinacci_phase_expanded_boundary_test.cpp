#include <iostream>
#include <stdexcept>
#include <tuple>
#include <vector>

#include "ravel/contact_boundary.hpp"
#include "ravel/d_cont_check.hpp"
#include "ravel/generalized_multinacci.hpp"
#include "ravel/proof/generalized_multinacci_boundary_word_lift.hpp"
#include "ravel/proof/positive_word_graph_expansion.hpp"

using namespace ravel;

template <std::size_t d>
void check_case() {
    constexpr std::size_t m = 2;
    const auto raw = generalized_multinacci_rule(d,m);
    SubstitutionRule rule(raw);
    const auto beta = generalized_multinacci_beta(d,m);
    const auto subst = make_substitution<d>(rule,beta);
    const auto cands = search_D_cont<d>(subst,2);
    std::vector<std::tuple<long long,std::vector<long long>,long long>> dcont;
    for(const auto& c:cands)
        dcont.emplace_back(c.i,std::vector<long long>(c.x.begin(),c.x.end()),c.j);
    const auto rep=compute_contact_boundary<d>(rule,beta,0.0,dcont);
    const auto lift=ravel::proof::derive_generalized_multinacci_boundary_word_lift<d>(subst,rep,m);
    if(!lift.proved) throw std::runtime_error("boundary lift failed: "+lift.obstruction);
    std::vector<ravel::proof::PositiveWordGraphEdge> macro;
    for(const auto& e:lift.edges) {
        std::vector<std::size_t> word;
        for(auto g:e.word) word.push_back(g==ravel::proof::PrefixPrimitiveGenerator::balanced_q?0:1);
        macro.push_back({e.source,e.target,std::move(word)});
    }
    const auto expansion=ravel::proof::derive_positive_word_graph_expansion(
        rep.boundary_nodes.size(),2,macro);
    if(!expansion.proved) throw std::runtime_error("phase expansion failed: "+expansion.obstruction);
    if(expansion.reconstructed_macro_adjacency!=rep.gb_matrix)
        throw std::runtime_error("first-return macro graph does not equal G_B");
    std::size_t rr=0;
    for(const auto& e:macro) if(e.word==std::vector<std::size_t>{1,1}) ++rr;
    if(expansion.expanded_node_count!=rep.boundary_nodes.size()+rr)
        throw std::runtime_error("unexpected number of private phase states");
    std::cout<<"D="<<d<<" base="<<rep.boundary_nodes.size()
             <<" macro_edges="<<macro.size()<<" phase_states="<<rr
             <<" expanded="<<expansion.expanded_node_count<<"\n";
}

int main(){check_case<2>();check_case<3>();check_case<4>();check_case<5>();std::cout<<"generalized multinacci phase-expanded boundary PASS\n";}
