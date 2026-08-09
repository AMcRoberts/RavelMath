// End-to-end test of the Barge-Diamond reflection pipeline: run the
// certificate with tracing enabled, render the resulting trace to
// Lean, and write it out for kernel-checking (the actual Lean check
// is run separately via `lake env lean`, not from this binary -- see
// the accompanying shell step). This test only verifies the C++/
// reflection/rendering side: the certificate succeeds, records
// exactly one IntegerEigenvectorNoWitness node per substitution
// tested, and the renderer produces well-formed output referencing
// each one.

#include <cassert>
#include <fstream>
#include <iostream>

#include "math/proof_reflection.hpp"
#include "ravel/proof/barge_diamond_certificate.hpp"
#include "ravel/proof/reflective_lean_renderer.hpp"

using namespace ravel::proof;

int main() {
    mathlib::reflection::Trace trace("barge_diamond_batch");
    {
        mathlib::reflection::ScopedTrace scope(&trace);

        // sigma_{0,1} (d=3) and two 2-letter unimodular Pisot cases.
        std::array<std::vector<long long>, 3> sigma01 = {std::vector<long long>{1, 2}, std::vector<long long>{2}, std::vector<long long>{0}};
        auto cert1 = certify_barge_diamond<3>(sigma01);
        assert(cert1.beta_irrational);
        std::cout << "sigma_{0,1}: " << cert1.conclusion << "\n";

        std::array<std::vector<long long>, 2> two1 = {std::vector<long long>{1, 0, 0}, std::vector<long long>{0, 1}};
        auto cert2 = certify_barge_diamond<2>(two1);
        assert(cert2.beta_irrational);
        std::cout << "2letter_1: " << cert2.conclusion << "\n";

        std::array<std::vector<long long>, 2> two2 = {std::vector<long long>{1, 0}, std::vector<long long>{0}};
        auto cert3 = certify_barge_diamond<2>(two2);
        assert(cert3.beta_irrational);
        std::cout << "2letter_2: " << cert3.conclusion << "\n";
    }

    auto nodes = trace.find<mathlib::reflection::IntegerEigenvectorNoWitness>();
    std::cout << "trace recorded " << nodes.size() << " IntegerEigenvectorNoWitness nodes\n";
    assert(nodes.size() == 3);

    std::string lean = render_reflective_lean_module(trace);
    assert(lean.find("irrational_eigenvalue_has_no_integer_eigenvector") != std::string::npos);
    assert(lean.find("barge_diamond_instance_0") != std::string::npos);
    assert(lean.find("barge_diamond_instance_1") != std::string::npos);
    assert(lean.find("barge_diamond_instance_2") != std::string::npos);

    std::ofstream out("lean/generated/barge_diamond_batch.lean");
    out << lean;
    out.close();
    std::cout << "wrote lean/generated/barge_diamond_batch.lean (" << lean.size() << " bytes)\n";

    std::cout << "barge_diamond_certificate: reflection pipeline produces well-formed output "
                 "for all 3 substitutions.\n";
    return 0;
}
