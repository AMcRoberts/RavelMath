#include <cassert>
#include <cstdlib>
#include <fstream>
#include <iostream>

#include "adelic/coincidence_and_property_f.hpp"
#include "adelic/prefix_automaton.hpp"
#include "math/charpoly.hpp"
#include "math/linalg_qbeta.hpp"
#include "ravel/proof/property_f_finite_run_certificate.hpp"
#include "ravel/proof/reflective_lean_renderer.hpp"

namespace {
std::vector<std::vector<long long>> transpose(const std::vector<std::vector<long long>>& m) {
    std::vector<std::vector<long long>> out(m[0].size(), std::vector<long long>(m.size()));
    for (std::size_t i = 0; i < m.size(); ++i)
        for (std::size_t j = 0; j < m[i].size(); ++j) out[j][i] = m[i][j];
    return out;
}
}  // namespace

int main() {
    std::array<std::vector<long long>, 2> images = {
        std::vector<long long>{0, 1}, std::vector<long long>{0}};
    const std::vector<std::vector<long long>> matrix = {{1, 1}, {1, 0}};
    mathlib::QBetaRing ring(mathlib::charpoly_faddeev_leverrier(matrix));
    auto eig = mathlib::right_eigenvector_via_qbeta(transpose(matrix), ring);
    assert(eig.ok);
    auto automaton = adelic::build_prefix_automaton<2>(images, eig.v, ring);
    adelic::PropertyFGraph graph;
    auto result = adelic::check_property_f<2>(automaton, 300000, nullptr, nullptr, nullptr, nullptr, &graph);
    assert(result.holds && !result.inconclusive);

    mathlib::reflection::Trace trace("property_f_reflection_test");
    {
        mathlib::reflection::ScopedTrace scope(&trace);
        assert(ravel::proof::stage_property_f_finite_run(result, "Fibonacci"));
        assert(ravel::proof::stage_property_f_graph(result, graph, "Fibonacci"));
    }
    auto certificates = trace.find<mathlib::reflection::PropertyFFiniteRunCertificate>();
    assert(certificates.size() == 1);
    assert(certificates.front().second->nodes_explored == 8);
    const std::string lean = ravel::proof::render_reflective_lean_module(trace);
    assert(lean.find("property_f_finite_run_summary_0") != std::string::npos);
    assert(lean.find("2 + 6 = 8") != std::string::npos);
    assert(lean.find("property_f_graph_0_edges") != std::string::npos);
    assert(lean.find("property_f_graph_0_gamma_0") != std::string::npos);
    assert(lean.find("property_f_graph_0_digit_0_0") != std::string::npos);
    if (const char* dump_path = std::getenv("RAVEL_PROPERTY_F_LEAN_OUT")) {
        std::ofstream dump(dump_path);
        dump << lean;
    }
    std::cout << "property_f_reflection: typed finite-run summary emitted\n";
}
