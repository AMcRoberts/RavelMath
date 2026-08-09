// A new reflection connection for Finding 23: sigma_{0,1}'s incidence
// matrix M satisfies M^3 = M + I exactly (the Cayley-Hamilton-style
// relation from its minimal polynomial x^3-x-1 having coefficients in
// {-1,0,1}, used in include/ravel/proof/single_junction_coincidence_
// composition.hpp's argument but previously not backed by any
// executable/checkable artifact). Computes M from the actual
// substitution images (not hardcoded), checks M^3=M+I by exact
// integer arithmetic, threads the concrete matrix; the renderer has
// Lean independently re-derive M^3 via Mathlib's own Matrix power and
// decide the identity.

#include <cassert>
#include <fstream>
#include <iostream>
#include <vector>

#include "math/proof_reflection.hpp"
#include "ravel/proof/cayley_hamilton_cubic_certificate.hpp"
#include "ravel/proof/reflective_lean_renderer.hpp"

int main() {
    // sigma_{0,1}: sigma(0)=[1,2], sigma(1)=[2], sigma(2)=[0] -- the
    // non-AR control from Finding 5/16/17, also used by
    // tests/coincidence_as_landmark_vector_cancellation_test.cpp.
    std::array<std::vector<long long>, 3> images = {
        std::vector<long long>{1, 2}, std::vector<long long>{2}, std::vector<long long>{0}};
    auto M = ravel::proof::incidence_matrix_3(images);

    // Independently confirm the matrix matches what the pre-existing
    // landmark-vector-cancellation test hardcodes (cross-check, not
    // assumed): {{0,0,1},{1,0,0},{1,1,0}}.
    ravel::proof::Mat3 expected = {{{0, 0, 1}, {1, 0, 0}, {1, 1, 0}}};
    assert(M == expected);

    mathlib::reflection::Trace trace("cayley_hamilton_cubic_batch");
    {
        mathlib::reflection::ScopedTrace scope(&trace);
        ravel::proof::stage_cayley_hamilton_cubic(
            M, "sigma_{0,1}'s incidence matrix (Finding 23's Cayley-Hamilton relation)");
        // Negative control: a matrix that does NOT satisfy M^3=M+I
        // should record nothing.
        ravel::proof::Mat3 not_ch = {{{1, 0, 0}, {0, 1, 0}, {0, 0, 1}}};
        ravel::proof::stage_cayley_hamilton_cubic(not_ch, "identity matrix (negative control)");
    }

    auto nodes = trace.find<mathlib::reflection::CayleyHamiltonCubicCertificate>();
    std::cout << "trace recorded " << nodes.size() << " CayleyHamiltonCubicCertificate nodes\n";
    assert(nodes.size() == 1);

    std::string lean = ravel::proof::render_reflective_lean_module(trace);
    assert(lean.find("cayley_hamilton_cubic_instance_0") != std::string::npos);
    assert(lean.find("cayley_hamilton_cubic_instance_1") == std::string::npos);

    std::ofstream out("lean/generated/cayley_hamilton_cubic_batch.lean");
    out << lean;
    out.close();

    std::cout << "cayley_hamilton_cubic_reflection_test: PASS\n";
    return 0;
}
