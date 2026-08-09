#include <array>
#include <cassert>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "math/proof_reflection.hpp"
#include "ravel/ambient_graph.hpp"
#include "ravel/generalized_multinacci.hpp"
#include "ravel/proof/generalized_multinacci_symbolic_embedding.hpp"
#include "ravel/proof/reflective_lean_renderer.hpp"

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
static void implementation_cross_check(std::size_t m, int& staged_count) {
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
    proof::stage_generalized_multinacci_symbolic_embedding(
        p, "D="+std::to_string(D)+" m="+std::to_string(m)+" symbolic embedding cross-check");
    ++staged_count;
}

int main() {
    symbolic_range();

    mathlib::reflection::Trace trace("generalized_multinacci_symbolic_embedding_batch");
    int staged_count = 0;
    {
        mathlib::reflection::ScopedTrace scope(&trace);
        for(std::size_t m=1;m<=32;++m) {
            implementation_cross_check<2>(m, staged_count);
            implementation_cross_check<3>(m, staged_count);
            implementation_cross_check<4>(m, staged_count);
            implementation_cross_check<5>(m, staged_count);
            implementation_cross_check<6>(m, staged_count);
            implementation_cross_check<7>(m, staged_count);
            implementation_cross_check<8>(m, staged_count);
        }
    }

    auto nodes = trace.find<mathlib::reflection::GeneralizedMultinacciSymbolicEmbeddingReflectionCertificate>();
    std::cout << "trace recorded " << nodes.size()
              << " GeneralizedMultinacciSymbolicEmbeddingReflectionCertificate nodes"
                 " (staged " << staged_count << ")\n";
    assert(static_cast<int>(nodes.size()) == staged_count);

    std::string lean = proof::render_reflective_lean_module(trace);
    assert(lean.find("deletion_only_subsum") != std::string::npos);
    assert(lean.find("generalized_multinacci_symbolic_embedding_instance_0") != std::string::npos);

    std::ofstream out("lean/generated/generalized_multinacci_symbolic_embedding.lean");
    out << lean;
    out.close();

    std::cout<<"generalized multinacci symbolic embedding PASS\n";
}
