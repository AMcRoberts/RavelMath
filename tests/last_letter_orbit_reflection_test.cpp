// Finding 39/41's suffix-side general case, wired through the
// reflection pipeline: exact dual of first_letter_orbit_reflection_test.cpp.
// Uses the same digits (1,0,0,0,1) example: sigma(0)=[0,1], sigma(1)=[2],
// sigma(2)=[3], sigma(3)=[4], sigma(4)=[0]. lastLetterMap:
// 0->1 (images[0].back()=1), 1->2, 2->3, 3->4, 4->0.
// Orbit of 1: 1,2,3,4,0,1,2,3,4,0,... (period 5, since 0->1 too).
// Orbit of 3: 3,4,0,1,2,3,... (period 5). These are on the SAME cycle
// (since lastLetterMap here is a pure rotation: 0->1->2->3->4->0),
// so they collide once aligned: need k with (1+k)%5 == (3+k)%5 mod
// nothing -- actually a pure rotation never "collides" at equal
// values unless starting points already differ by a multiple of the
// cycle length (they don't, gap=2, period=5, gcd(2,5)=1 means they
// visit the SAME positions at different times but are never EQUAL at
// the same k, except... let's just let the certificate search find it
// (or correctly report not found within bound, which the test must
// tolerate) -- suffix collision is a genuinely different mechanism
// than prefix collision, not guaranteed to succeed on the same digits.

#include <cassert>
#include <fstream>
#include <iostream>

#include "math/proof_reflection.hpp"
#include "ravel/proof/last_letter_orbit_certificate.hpp"
#include "ravel/proof/reflective_lean_renderer.hpp"

using namespace ravel::proof;

int main() {
    std::array<std::vector<long long>, 5> images = {
        std::vector<long long>{0, 1}, std::vector<long long>{2}, std::vector<long long>{3},
        std::vector<long long>{4}, std::vector<long long>{0}};

    mathlib::reflection::Trace trace("last_letter_orbit_batch");
    long long recorded = 0;
    {
        mathlib::reflection::ScopedTrace scope(&trace);
        auto r1 = certify_last_letter_orbit_collision<5>(images, 1, 3);
        std::cout << "(1,3): found=" << r1.found << " " << r1.note << "\n";
        if (r1.found) ++recorded;
    }

    auto nodes = trace.find<mathlib::reflection::LastLetterOrbitCertificate>();
    std::cout << "trace recorded " << nodes.size() << " LastLetterOrbitCertificate nodes\n";
    assert(static_cast<long long>(nodes.size()) == recorded);

    if (recorded > 0) {
        std::string lean = render_reflective_lean_module(trace);
        assert(lean.find("last_letter_orbit_collision_forces_coincidence") != std::string::npos);
        std::ofstream out("lean/generated/last_letter_orbit_batch.lean");
        out << lean;
        out.close();
        std::cout << "wrote lean/generated/last_letter_orbit_batch.lean\n";
    } else {
        std::cout << "no suffix collision found within bound on this example -- "
                     "expected for a pure-rotation lastLetterMap with a coprime gap; "
                     "trying a different pair below.\n";
        // Fall back to a pair that DOES collide: 0 vs 1 (gap 1, still
        // coprime to 5 -- a pure rotation on a 5-cycle never collides
        // for ANY nonzero gap, since (a+k) mod 5 == (b+k) mod 5 iff
        // a==b). This digit example is structurally a bad fit for the
        // suffix mechanism (every letter maps to a DIFFERENT next
        // letter with no merging point) -- construct a proper merging
        // example instead.
        mathlib::reflection::Trace trace2("last_letter_orbit_batch2");
        std::array<std::vector<long long>, 3> images2 = {
            std::vector<long long>{1, 2}, std::vector<long long>{0}, std::vector<long long>{0}};
        {
            mathlib::reflection::ScopedTrace scope2(&trace2);
            auto r2 = certify_last_letter_orbit_collision<3>(images2, 1, 2);
            std::cout << "(1,2) on fallback example: found=" << r2.found << " " << r2.note << "\n";
            assert(r2.found);
        }
        auto nodes2 = trace2.find<mathlib::reflection::LastLetterOrbitCertificate>();
        assert(nodes2.size() == 1);
        std::string lean2 = render_reflective_lean_module(trace2);
        assert(lean2.find("last_letter_orbit_collision_forces_coincidence") != std::string::npos);
        std::ofstream out2("lean/generated/last_letter_orbit_batch.lean");
        out2 << lean2;
        out2.close();
        std::cout << "wrote lean/generated/last_letter_orbit_batch.lean (fallback example)\n";
    }

    std::cout << "last_letter_orbit_reflection_test: PASS\n";
    return 0;
}
