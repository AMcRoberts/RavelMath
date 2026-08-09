#include <cassert>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include "math/proof_reflection.hpp"
#include "ravel/proof/coefficient_profile_parity_obstruction.hpp"
#include "ravel/proof/reflective_lean_renderer.hpp"

int main() {
    mathlib::reflection::Trace trace("coefficient_profile_parity_obstruction_batch");
    {
        mathlib::reflection::ScopedTrace scope(&trace);
        for (std::size_t D=3;D<=256;++D) {
            const auto proof=ravel::proof::derive_nearest_left_profile_parity_obstruction(D);
            if(!proof.proved) throw std::runtime_error(proof.obstruction);
            if(proof.cyclotomic_x_plus_one_factor != (D%2==0))
                throw std::runtime_error("parity mismatch");
            ravel::proof::stage_coefficient_profile_parity_obstruction(
                proof, "D="+std::to_string(D)+" nearest-left profile parity instance");
        }
    }

    auto nodes = trace.find<mathlib::reflection::CoefficientProfileParityObstructionReflectionCertificate>();
    std::cout << "trace recorded " << nodes.size()
              << " CoefficientProfileParityObstructionReflectionCertificate nodes\n";
    assert(nodes.size() == 254);

    std::string lean = ravel::proof::render_reflective_lean_module(trace);
    assert(lean.find("nearest_left_profile_odd_value") != std::string::npos);
    assert(lean.find("coefficient_profile_parity_obstruction_instance_0") != std::string::npos);

    std::ofstream out("lean/generated/coefficient_profile_parity_obstruction.lean");
    out << lean;
    out.close();

    std::cout<<"coefficient profile parity obstruction PASS\n";
}
