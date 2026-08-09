#include <cassert>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <tuple>
#include <vector>

#include "math/proof_reflection.hpp"
#include "ravel/contact_boundary.hpp"
#include "ravel/d_cont_check.hpp"
#include "ravel/generalized_multinacci.hpp"
#include "ravel/proof/generalized_multinacci_boundary_word_lift.hpp"
#include "ravel/proof/generalized_multinacci_general_m.hpp"
#include "ravel/proof/reflective_lean_renderer.hpp"

using namespace ravel;

template <std::size_t d>
void check_concrete(std::size_t m) {
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
    const auto symbolic=ravel::proof::derive_generalized_multinacci_general_m(m);
    if(!ravel::proof::general_m_concrete_twist_refines_symbolic_scheduler<d>(twist,symbolic))
        throw std::runtime_error("concrete graph does not refine symbolic general-m scheduler");
    std::cout << "D="<<d<<" m="<<m<<" states="<<twist.base_states
              <<" edges="<<twist.macro_edges<<" roof="<<twist.maximum_return_time<<"\n";
}

int main() {
    mathlib::reflection::Trace trace("generalized_multinacci_general_m_batch");
    {
        mathlib::reflection::ScopedTrace scope(&trace);
        // Fully symbolic proof: no graph enumeration and no fixed upper m.
        for(std::size_t m=1;m<=64;++m) {
            const auto p=ravel::proof::derive_generalized_multinacci_general_m(m);
            if(!p.proved) throw std::runtime_error("symbolic general-m proof failed");
            if(p.unsigned_scheduler_coefficients.at(0)!=m+1)
                throw std::runtime_error("bad Q coefficient");
            for(std::size_t d=1;d<=m;++d)
                if(p.unsigned_scheduler_coefficients.at(d)!=2*(m+1-d))
                    throw std::runtime_error("bad R^d coefficient");
            ravel::proof::stage_generalized_multinacci_general_m(
                p, "m="+std::to_string(m)+" symbolic scheduler closed-form check");
        }
        // Concrete exact boundary refinements beyond the old m<=3 sample.
        for(std::size_t m=1;m<=6;++m) {
            check_concrete<2>(m);
            check_concrete<3>(m);
            check_concrete<4>(m);
        }
    }

    auto nodes = trace.find<mathlib::reflection::GeneralizedMultinacciGeneralMReflectionCertificate>();
    std::cout << "trace recorded " << nodes.size()
              << " GeneralizedMultinacciGeneralMReflectionCertificate nodes\n";
    assert(nodes.size() == 64);

    std::string lean = ravel::proof::render_reflective_lean_module(trace);
    assert(lean.find("schedulerCoefficient_positive") != std::string::npos);
    assert(lean.find("generalized_multinacci_general_m_instance_0") != std::string::npos);

    std::ofstream out("lean/generated/generalized_multinacci_general_m.lean");
    out << lean;
    out.close();

    std::cout << "generalized multinacci general-m PASS\n";
}
