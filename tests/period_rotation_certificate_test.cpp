// Finding 35, wired through the reflection pipeline end-to-end: C++
// certificate (period_rotation_certificate.hpp) records a
// PeriodRotationCertificate node the moment the coloring is verified
// exactly; the renderer (reflective_lean_renderer.hpp) mechanically
// emits a Lean corollary of the general, independently kernel-checked
// lemma `period_coloring_rotates_eigenvalue`
// (lean/period_rotation_forces_equal_modulus.lean), discharging the
// coloring hypothesis itself via a finite case split + `omega` -- no
// hand-authored Lean at the instance level.

#include <cassert>
#include <fstream>
#include <iostream>

#include "math/proof_reflection.hpp"
#include "ravel/proof/period_rotation_certificate.hpp"
#include "ravel/proof/reflective_lean_renderer.hpp"

using namespace ravel::proof;

int main() {
    mathlib::reflection::Trace trace("period_rotation_batch");
    {
        mathlib::reflection::ScopedTrace scope(&trace);

        // g=1 control: must NOT record a node.
        {
            std::array<std::vector<long long>, 3> images = {
                std::vector<long long>{0, 0, 1}, std::vector<long long>{2}, std::vector<long long>{0, 1}};
            auto cert = certify_period_rotation<3>(images);
            assert(!cert.applicable);
            assert(cert.g == 1);
        }

        // Small synthetic g=2, single-junction: 0 branches to 1 and 2,
        // each a single forced step directly back to 0 (jump_size=2
        // each: the choice step plus one forced step).
        {
            std::array<std::vector<long long>, 3> images = {
                std::vector<long long>{1, 2}, std::vector<long long>{0}, std::vector<long long>{0}};
            auto cert = certify_period_rotation<3>(images);
            assert(cert.applicable);
            assert(cert.g == 2);
        }

        // Finding 35's own g=2 multi-junction example.
        {
            std::array<std::vector<long long>, 8> images = {
                std::vector<long long>{1, 3}, std::vector<long long>{2}, std::vector<long long>{4, 5},
                std::vector<long long>{0}, std::vector<long long>{0}, std::vector<long long>{6},
                std::vector<long long>{7}, std::vector<long long>{0}};
            auto cert = certify_period_rotation<8>(images);
            assert(cert.applicable);
            assert(cert.g == 2);
        }

        // Finding 35's independently-constructed g=4 example.
        {
            std::array<std::vector<long long>, 7> images = {
                std::vector<long long>{1, 4}, std::vector<long long>{2}, std::vector<long long>{3},
                std::vector<long long>{0}, std::vector<long long>{5}, std::vector<long long>{6},
                std::vector<long long>{0}};
            auto cert = certify_period_rotation<7>(images);
            assert(cert.applicable);
            assert(cert.g == 4);
        }
    }

    auto nodes = trace.find<mathlib::reflection::PeriodRotationCertificate>();
    std::cout << "trace recorded " << nodes.size() << " PeriodRotationCertificate nodes\n";
    assert(nodes.size() == 3);  // the g=1 control correctly recorded nothing

    std::string lean = render_reflective_lean_module(trace);
    assert(lean.find("period_coloring_rotates_eigenvalue") != std::string::npos);
    assert(lean.find("period_rotation_instance_0") != std::string::npos);
    assert(lean.find("period_rotation_instance_1") != std::string::npos);
    assert(lean.find("period_rotation_instance_2") != std::string::npos);

    std::ofstream out("lean/generated/period_rotation_batch.lean");
    out << lean;
    out.close();

    std::cout << "period_rotation_certificate_test: PASS -- "
                 "g=1 control recorded nothing, 3 g>=2 instances recorded and rendered.\n";
    return 0;
}
