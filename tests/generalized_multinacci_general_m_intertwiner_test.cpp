#include <cassert>
#include <fstream>
#include <iostream>

#include "math/proof_reflection.hpp"
#include "ravel/proof/generalized_multinacci_general_m_intertwiner.hpp"
#include "ravel/proof/reflective_lean_renderer.hpp"

using namespace ravel::proof;

int main() {
    const NonnegativeMatrix qc{{1,0},{0,1}};
    const NonnegativeMatrix rc{{0,1},{1,0}};
    const NonnegativeMatrix qk{{1,0,0},{0,1,0},{0,0,2}};
    const NonnegativeMatrix rk{{0,1,0},{1,0,0},{0,0,1}};
    const NonnegativeMatrix P{{1,0,0},{0,1,0}};

    mathlib::reflection::Trace trace("generalized_multinacci_general_m_intertwiner_batch");
    {
        mathlib::reflection::ScopedTrace scope(&trace);
        for(std::size_t m=1;m<=128;++m) {
            const auto p=derive_generalized_multinacci_general_m_intertwiner(
                m,qc,rc,qk,rk,P);
            assert(p.proved);
            assert(p.symbolic_cut_states==(m+1)*(m+1));
            assert(p.words_checked==m+1);
            assert(p.polynomial_terms==(m+1)*(m+1));
            stage_generalized_multinacci_general_m_intertwiner(
                p, "m="+std::to_string(m)+" simultaneous Q/R intertwiner sweep");
        }

        auto bad=qc; bad[0][0]=3;
        assert(!derive_generalized_multinacci_general_m_intertwiner(
            2,bad,rc,qk,rk,P).proved);
        assert(!derive_generalized_multinacci_general_m_intertwiner(
            0,qc,rc,qk,rk,P).proved);
    }

    auto nodes = trace.find<mathlib::reflection::GeneralizedMultinacciGeneralMIntertwinerReflectionCertificate>();
    std::cout << "trace recorded " << nodes.size()
              << " GeneralizedMultinacciGeneralMIntertwinerReflectionCertificate nodes\n";
    assert(nodes.size() == 128);

    std::string lean = render_reflective_lean_module(trace);
    assert(lean.find("roof_word_intertwiner") != std::string::npos);
    assert(lean.find("generalized_multinacci_general_m_intertwiner_instance_0") != std::string::npos);

    std::ofstream out("lean/generated/generalized_multinacci_general_m_intertwiner.lean");
    out << lean;
    out.close();

    std::cout << "generalized multinacci general-m intertwiner PASS\n";
}
