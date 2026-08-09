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
#include "ravel/proof/generalized_multinacci_signed_renewal_twist.hpp"
#include "ravel/proof/reflective_lean_renderer.hpp"

using namespace ravel;

template <std::size_t d>
void check_case(std::size_t m, int& staged_count) {
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
    ravel::proof::stage_generalized_multinacci_signed_renewal_twist(
        twist, "D="+std::to_string(d)+" m="+std::to_string(m)+" signed renewal twist instance");
    ++staged_count;
}

int main() {
    mathlib::reflection::Trace trace("generalized_multinacci_signed_renewal_twist_batch");
    int staged_count = 0;
    {
        mathlib::reflection::ScopedTrace scope(&trace);
        for (std::size_t m : {1u,2u,3u}) {
            check_case<2>(m, staged_count);
            check_case<3>(m, staged_count);
            check_case<4>(m, staged_count);
            check_case<5>(m, staged_count);
        }
    }

    auto nodes = trace.find<mathlib::reflection::GeneralizedMultinacciSignedRenewalTwistReflectionCertificate>();
    std::cout << "trace recorded " << nodes.size()
              << " GeneralizedMultinacciSignedRenewalTwistReflectionCertificate nodes"
                 " (staged " << staged_count << ")\n";
    assert(static_cast<int>(nodes.size()) == staged_count);

    std::string lean = ravel::proof::render_reflective_lean_module(trace);
    assert(lean.find("defect_roof_bounded") != std::string::npos);
    assert(lean.find("generalized_multinacci_signed_renewal_twist_instance_0") != std::string::npos);

    std::ofstream out("lean/generated/generalized_multinacci_signed_renewal_twist.lean");
    out << lean;
    out.close();

    std::cout << "generalized multinacci signed renewal twist PASS\n";
}
