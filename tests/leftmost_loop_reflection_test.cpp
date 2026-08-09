// Finding 27, wired through the reflection pipeline: leftmost_loop_length
// records a LeftmostLoopCertificate when it finds a cycle; the renderer
// mechanically emits corollaries of periodic_point_iterate_mul for
// m=0..max_m, modeling the loop as a pure rotation on Fin L.

#include <array>
#include <cassert>
#include <fstream>
#include <iostream>

#include "math/proof_reflection.hpp"
#include "ravel/proof/coincidence_closure.hpp"
#include "ravel/proof/coincidence_converse_leftmost_loop.hpp"
#include "ravel/proof/reflective_lean_renderer.hpp"

using namespace ravel::proof;

int main() {
    // Finding 26/27's own g=2 multi-junction example: leftmost cycle
    // 0 -> 2 -> 0, length 4 (a proper multiple of g=2).
    std::array<std::vector<long long>, 8> images = {
        std::vector<long long>{1, 3}, std::vector<long long>{2}, std::vector<long long>{4, 5},
        std::vector<long long>{0}, std::vector<long long>{0}, std::vector<long long>{6},
        std::vector<long long>{7}, std::vector<long long>{0}};
    auto edges = build_junction_graph<8>(images);

    mathlib::reflection::Trace trace("leftmost_loop_batch");
    std::optional<long long> L;
    {
        mathlib::reflection::ScopedTrace scope(&trace);
        L = leftmost_loop_length<8>(edges, 0, 10000, 3);
    }
    assert(L.has_value());
    assert(*L == 4);
    std::cout << "leftmost loop length L=" << *L << "\n";

    auto nodes = trace.find<mathlib::reflection::LeftmostLoopCertificate>();
    std::cout << "trace recorded " << nodes.size() << " LeftmostLoopCertificate nodes\n";
    assert(nodes.size() == 1);

    std::string lean = render_reflective_lean_module(trace);
    assert(lean.find("periodic_point_iterate_mul") != std::string::npos);
    assert(lean.find("leftmost_loop_instance_3") != std::string::npos);  // m=0..3

    std::ofstream out("lean/generated/leftmost_loop_batch.lean");
    out << lean;
    out.close();

    std::cout << "leftmost_loop_reflection_test: PASS\n";
    return 0;
}
