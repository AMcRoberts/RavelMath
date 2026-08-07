// Finding 39/41's GENERAL case, wired through the reflection pipeline:
// covers a mixed in-run/out-of-run pair the same-chain lemma cannot.
// digits (1,0,0,0,1) -> canonical images:
//   sigma(0)=[0,1], sigma(1)=[2], sigma(2)=[3], sigma(3)=[4], sigma(4)=[0]
// (t_0=1: sigma(0)=0^1(1)=[0,1]; t_1=t_2=t_3=0: pass-through;
//  t_4=1 (N-1 case): sigma(4)=0^1=[0]).
// firstLetterMap: 0->0 (fixed point, since sigma(0)[0]=0), 1->2, 2->3,
// 3->4, 4->0. Letter 0 is NOT in any zero-run (t_0=1); letter 2 IS
// mid-run. Orbit of 0 stays at 0 forever; orbit of 2 reaches 0 after 3
// steps (2->3->4->0) -- a genuine mixed pair.

#include <cassert>
#include <fstream>
#include <iostream>

#include "math/proof_reflection.hpp"
#include "ravel/proof/first_letter_orbit_certificate.hpp"
#include "ravel/proof/reflective_lean_renderer.hpp"

using namespace ravel::proof;

int main() {
    std::array<std::vector<long long>, 5> images = {
        std::vector<long long>{0, 1}, std::vector<long long>{2}, std::vector<long long>{3},
        std::vector<long long>{4}, std::vector<long long>{0}};

    mathlib::reflection::Trace trace("first_letter_orbit_batch");
    {
        mathlib::reflection::ScopedTrace scope(&trace);

        // Mixed pair: letter 0 (not in a run) vs letter 2 (mid-run).
        auto r1 = certify_first_letter_orbit_collision<5>(images, 0, 2);
        assert(r1.found);
        assert(r1.k == 3);
        std::cout << "(0,2): " << r1.note << "\n";

        // Same-chain pair, for comparison: 1 vs 3.
        auto r2 = certify_first_letter_orbit_collision<5>(images, 1, 3);
        assert(r2.found);
        std::cout << "(1,3): " << r2.note << "\n";
    }

    auto nodes = trace.find<mathlib::reflection::FirstLetterOrbitCertificate>();
    std::cout << "trace recorded " << nodes.size() << " FirstLetterOrbitCertificate nodes\n";
    assert(nodes.size() == 2);

    std::string lean = render_reflective_lean_module(trace);
    assert(lean.find("first_letter_orbit_collision_forces_coincidence") != std::string::npos);
    assert(lean.find("first_letter_orbit_instance_1") != std::string::npos);

    std::ofstream out("/tmp/first_letter_orbit_generated.lean");
    out << lean;
    out.close();

    std::cout << "first_letter_orbit_reflection_test: PASS\n";
    return 0;
}
