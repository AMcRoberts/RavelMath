// Extends Finding 39/41's retrofit (first_letter_orbit_certificate.hpp)
// to the EVENTUALLY-PERIODIC family specifically (Finding 41's own
// extension past terminating expansions) -- the certificate doesn't
// depend on the terminating structure at all, so this is genuine new
// coverage, not a repeat. Uses one of Finding 41's own wraparound
// stress-test cases (preperiod={1}, period={0,1,0}), where a naive
// flat-only zero-run computation is specifically known to give a
// WRONG answer -- confirming the mechanism handles the wraparound
// case correctly, not just the simpler terminating one.

#include <cassert>
#include <fstream>
#include <iostream>

#include "math/proof_reflection.hpp"
#include "ravel/canonical_beta_substitution.hpp"
#include "ravel/proof/first_letter_orbit_certificate.hpp"
#include "ravel/proof/reflective_lean_renderer.hpp"

using namespace ravel::proof;

int main() {
    auto sigma_vec = ravel::canonical_beta_substitution_eventually_periodic({1}, {0, 1, 0});
    assert(sigma_vec.size() == 4);
    std::array<std::vector<long long>, 4> images;
    for (std::size_t i = 0; i < 4; ++i) images[i] = sigma_vec[i];

    mathlib::reflection::Trace trace("first_letter_orbit_finding41_batch");
    {
        mathlib::reflection::ScopedTrace scope(&trace);

        // Mixed pair: letter 0 (fixed point, not itself mid-chain) vs
        // letter 3 (needs 3 steps through the periodic wraparound).
        auto r1 = certify_first_letter_orbit_collision<4>(images, 0, 3);
        assert(r1.found);
        std::cout << "(0,3): " << r1.note << "\n";

        // Same-chain-flavored pair: 1 vs 3, both inside the wraparound.
        auto r2 = certify_first_letter_orbit_collision<4>(images, 1, 3);
        assert(r2.found);
        std::cout << "(1,3): " << r2.note << "\n";
    }

    auto nodes = trace.find<mathlib::reflection::FirstLetterOrbitCertificate>();
    std::cout << "trace recorded " << nodes.size() << " FirstLetterOrbitCertificate nodes\n";
    assert(nodes.size() == 2);

    std::string lean = render_reflective_lean_module(trace);
    assert(lean.find("first_letter_orbit_collision_forces_coincidence") != std::string::npos);

    std::ofstream out("/tmp/first_letter_orbit_finding41_generated.lean");
    out << lean;
    out.close();

    std::cout << "first_letter_orbit_finding41_test: PASS\n";
    return 0;
}
