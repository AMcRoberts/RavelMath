#include <array>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "ravel/ambient_graph.hpp"
#include "ravel/generalized_multinacci.hpp"
#include "ravel/proof/generalized_multinacci_symbolic_embedding.hpp"

using namespace ravel;

static void symbolic_range() {
    for (std::size_t D=2;D<=256;++D) {
        for (std::size_t m=1;m<=256;++m) {
            const auto p=proof::derive_generalized_multinacci_symbolic_embedding(D,m);
            if(!p.proved) throw std::runtime_error("symbolic schema failed: "+p.obstruction);
            const auto total=(D-1)*(m+1)+1;
            if(p.total_parent_occurrences!=total ||
               p.universal_macro_edges!=total*total || p.maximum_roof!=m)
                throw std::runtime_error("symbolic count identity failed");
        }
    }
    if(proof::derive_generalized_multinacci_symbolic_embedding(1,2).proved ||
       proof::derive_generalized_multinacci_symbolic_embedding(2,0).proved)
        throw std::runtime_error("invalid symbolic parameters accepted");
}

template<std::size_t D>
static void implementation_cross_check(std::size_t m) {
    const auto raw=generalized_multinacci_rule(D,m);
    std::array<std::vector<long long>,D> images;
    for(std::size_t i=0;i<D;++i)
        images[i]=std::vector<long long>(raw[i].begin(),raw[i].end());
    const auto p=proof::derive_generalized_multinacci_symbolic_embedding(D,m);
    std::size_t total=0;
    for(std::size_t inner=0;inner<D;++inner) {
        const auto got=parent_decompositions<D>(images,static_cast<long long>(inner));
        const auto expected=proof::generalized_multinacci_parent_choices(D,m,inner);
        if(got.size()!=expected.size())
            throw std::runtime_error("parent decomposition cardinality mismatch");
        for(std::size_t k=0;k<got.size();++k) {
            if(static_cast<std::size_t>(got[k].parent_letter)!=expected[k].parent ||
               got[k].p.size()!=expected[k].prefix_position)
                throw std::runtime_error("parent decomposition schema mismatch");
            for(auto x:got[k].p) if(x!=0)
                throw std::runtime_error("generalized multinacci prefix is not all zero");
        }
        total+=got.size();
    }
    if(total!=p.total_parent_occurrences)
        throw std::runtime_error("total occurrence implementation mismatch");
}

int main() {
    symbolic_range();
    for(std::size_t m=1;m<=32;++m) {
        implementation_cross_check<2>(m);
        implementation_cross_check<3>(m);
        implementation_cross_check<4>(m);
        implementation_cross_check<5>(m);
        implementation_cross_check<6>(m);
        implementation_cross_check<7>(m);
        implementation_cross_check<8>(m);
    }
    std::cout<<"generalized multinacci symbolic embedding PASS\n";
}
