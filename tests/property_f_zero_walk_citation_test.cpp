// Findings 18-21 (property F), wired into the reflection pipeline for
// the first time: check_property_f now records a LemmaApplication
// citing zeroWalk_eq_zero_iff (property_f_unconditional.hpp,
// independently kernel-checked in
// lean/generated/property_f_zero_walk.lean BEFORE the mechanical
// reflection pipeline existed) whenever it runs its corrected verdict
// logic. This is not a per-instance retrofit (the lemma is already
// fully general) -- it is making the EXECUTING code cite an
// already-verified fact via the trace, exactly the "assertion-checked"
// pattern AM asked for, applied to code that predates this session's
// retrofit work entirely.

#include <cassert>
#include <fstream>
#include <iostream>

#include "adelic/coincidence_and_property_f.hpp"
#include "math/charpoly.hpp"
#include "math/linalg_qbeta.hpp"
#include "math/proof_reflection.hpp"
#include "ravel/proof/reflective_lean_renderer.hpp"

namespace {
std::vector<std::vector<long long>> transpose(const std::vector<std::vector<long long>>& M) {
    std::size_t n = M.size();
    std::vector<std::vector<long long>> T(n, std::vector<long long>(n, 0));
    for (std::size_t i = 0; i < n; ++i)
        for (std::size_t j = 0; j < n; ++j) T[j][i] = M[i][j];
    return T;
}
}  // namespace

int main() {
    mathlib::reflection::Trace trace("property_f_fibonacci");
    adelic::PropertyFResult propf;
    {
        mathlib::reflection::ScopedTrace scope(&trace);

        std::array<std::vector<long long>, 2> images = {std::vector<long long>{0, 1}, std::vector<long long>{0}};
        std::vector<std::vector<long long>> M = {{1, 1}, {1, 0}};
        auto charpoly = mathlib::charpoly_faddeev_leverrier(M);
        mathlib::QBetaRing R(charpoly);
        auto eig = mathlib::right_eigenvector_via_qbeta(transpose(M), R);
        assert(eig.ok);
        auto automaton = adelic::build_prefix_automaton<2>(images, eig.v, R);
        propf = adelic::check_property_f<2>(automaton, 300000);
    }

    std::cout << "Fibonacci: holds=" << propf.holds << " nodes=" << propf.nodes_explored << "\n";
    assert(propf.holds);
    assert(propf.nodes_explored == 8);

    auto lemmas = trace.find<mathlib::reflection::LemmaApplication>();
    bool found = false;
    for (auto& [id, l] : lemmas) { (void)id; if (l->theorem_name == "zeroWalk_eq_zero_iff") found = true; }
    std::cout << "trace has zeroWalk_eq_zero_iff citation: " << found << "\n";
    assert(found);

    std::string lean = ravel::proof::render_reflective_lean_module(trace);
    assert(lean.find("zeroWalk_eq_zero_iff") != std::string::npos);
    assert(lean.find("zeroWalk_nonneg") != std::string::npos);

    std::ofstream out("/tmp/property_f_zero_walk_citation_generated.lean");
    out << lean;
    out.close();

    std::cout << "property_f_zero_walk_citation_test: PASS\n";
    return 0;
}
