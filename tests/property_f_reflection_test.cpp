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
#include "ravel/proof/strong_coincidence_certificate.hpp"
#include "ravel/proof/strong_coincidence_pair_witness.hpp"

namespace {
std::vector<std::vector<long long>> transpose(const std::vector<std::vector<long long>>& m) {
    std::vector<std::vector<long long>> out(m[0].size(), std::vector<long long>(m.size()));
    for (std::size_t i = 0; i < m.size(); ++i)
        for (std::size_t j = 0; j < m[i].size(); ++j) out[j][i] = m[i][j];
    return out;
}
}  // namespace

int main() {
    // Cyclotomic-lift regression: the 7-letter third-smallest Pisot
    // incidence matrix has a Phi_4 factor, so its Perron vector must be
    // solved over the degree-5 minimal Pisot field rather than the full
    // incidence ring.  The reduced-factor helper tries minors and verifies
    // every row exactly.
    {
        const std::vector<std::vector<long long>> matrix7 = {
            {1, 0, 0, 1, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0},
            {0, 1, 0, 0, 0, 0, 0},
            {0, 0, 1, 0, 0, 0, 0},
            {0, 0, 0, 1, 0, 0, 0},
            {0, 0, 0, 0, 1, 0, 0},
            {0, 0, 0, 0, 0, 1, 0},
        };
        const mathlib::PolyZ minimal = {-1, 0, 1, -1, -1, 1};
        const mathlib::QBetaRing reduced_ring(minimal);
        const auto reduced = mathlib::left_eigenvector_via_qbeta_reduced_factor(
            matrix7, reduced_ring);
        assert(reduced.ok);
        assert(mathlib::verify_left_eigenvector(reduced.v, matrix7, reduced_ring));
    }

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
    const auto elided_result = adelic::check_property_f<2>(
        automaton, 300000, nullptr, nullptr, nullptr, nullptr, nullptr, false);
    assert(elided_result.holds && !elided_result.inconclusive);

    mathlib::reflection::Trace trace("property_f_reflection_test");
    {
        mathlib::reflection::ScopedTrace scope(&trace);
        assert(ravel::proof::stage_property_f_finite_run(result, "Fibonacci"));
        assert(ravel::proof::stage_property_f_graph(result, graph, ring, "Fibonacci"));
        assert(ravel::proof::stage_strong_coincidence_run(images, 20, 5'000'000,
                                                          "Fibonacci strong coincidence" )
               == ravel::proof::StrongCoincidenceStageResult::staged);
    }
    {
        auto tampered = graph;
        tampered.nodes[1].gamma_coefficients[0].first = "1";
        bool rejected = false;
        try {
            (void)ravel::proof::stage_property_f_graph(result, tampered, ring, "tampered");
        } catch (const std::invalid_argument&) {
            rejected = true;
        }
        assert(rejected);
        auto failed_result = result;
        failed_result.holds = false;
        assert(!ravel::proof::stage_property_f_graph(failed_result, graph, ring, "failed"));
        auto unclosed_result = result;
        unclosed_result.closure_reached = false;
        bool unclosed_rejected = false;
        try {
            (void)ravel::proof::stage_property_f_finite_run(unclosed_result, "unclosed");
        } catch (const std::invalid_argument&) {
            unclosed_rejected = true;
        }
        assert(unclosed_rejected);
        auto violating_graph = graph;
        violating_graph.nodes[2].successors.push_back(2);
        failed_result.violation_cycle_nodes = {2, 2};
        failed_result.violation_cycle_edges = {{2, 2}};
        mathlib::reflection::Trace violation_trace("property_f_reflection_violation_test");
        mathlib::reflection::ScopedTrace violation_scope(&violation_trace);
        assert(ravel::proof::stage_property_f_violation(failed_result, violating_graph, "synthetic failure"));
        const std::string violation_lean = ravel::proof::render_reflective_lean_module(violation_trace);
        assert(violation_lean.find("property_f_violation_0_closed") != std::string::npos);
        if (const char* violation_dump = std::getenv("RAVEL_PROPERTY_F_VIOLATION_LEAN_OUT")) {
            std::ofstream dump_violation(violation_dump);
            dump_violation << violation_lean;
        }
    }
    {
        std::array<std::vector<long long>, 3> images3 = {
            std::vector<long long>{0, 1, 2}, std::vector<long long>{0}, std::vector<long long>{1}};
        const std::vector<std::vector<long long>> matrix3 = {{1, 1, 0}, {1, 0, 1}, {1, 0, 0}};
        mathlib::QBetaRing ring3(mathlib::charpoly_faddeev_leverrier(matrix3));
        auto eig3 = mathlib::right_eigenvector_via_qbeta(transpose(matrix3), ring3);
        assert(eig3.ok);
        auto automaton3 = adelic::build_prefix_automaton<3>(images3, eig3.v, ring3);
        adelic::PropertyFGraph graph3;
        auto result3 = adelic::check_property_f<3>(automaton3, 300000, nullptr, nullptr, nullptr, nullptr, &graph3);
        assert(!result3.inconclusive);
        mathlib::reflection::Trace trace3("property_f_reflection_tribonacci");
        mathlib::reflection::ScopedTrace scope3(&trace3);
        assert(ravel::proof::stage_property_f_graph(result3, graph3, ring3, "Tribonacci"));
        assert(!trace3.find<mathlib::reflection::PropertyFGraphCertificate>().empty());
        const std::string lean3 = ravel::proof::render_reflective_lean_module(trace3);
        assert(lean3.find("property_f_graph_0_charpoly") != std::string::npos);
        assert(lean3.find("propertyFQ3Step") != std::string::npos);
        assert(lean3.find("property_f_graph_0_edge_0_0") != std::string::npos);
        assert(lean3.find("no_nonzero_internal_scc_edge") != std::string::npos);
        if (const char* dump_path3 = std::getenv("RAVEL_PROPERTY_F_LEAN_Q3_OUT")) {
            std::ofstream dump3(dump_path3);
            dump3 << lean3;
        }
    }
    auto certificates = trace.find<mathlib::reflection::PropertyFFiniteRunCertificate>();
    assert(certificates.size() == 1);
    assert(certificates.front().second->nodes_explored == 8);
    const auto coincidence_runs = trace.find<mathlib::reflection::StrongCoincidenceRunCertificate>();
    assert(coincidence_runs.size() == 1);
    assert(coincidence_runs.front().second->holds);
    assert(!coincidence_runs.front().second->inconclusive);
    assert(coincidence_runs.front().second->pair_depths.size() == 1);
    assert(coincidence_runs.front().second->pair_resolution_depths.size() == 1);
    assert(coincidence_runs.front().second->pair_resolution_depths.front() ==
           coincidence_runs.front().second->pair_depths.front());
    const auto coincidence_witnesses =
        trace.find<mathlib::reflection::StrongCoincidencePairWitnessCertificate>();
    assert(coincidence_witnesses.size() == 1);
    assert(coincidence_witnesses.front().second->prefix_match);
    const std::string lean = ravel::proof::render_reflective_lean_module(trace);
    assert(lean.find("property_f_finite_run_summary_0") != std::string::npos);
    assert(lean.find("2 + 6 = 8") != std::string::npos);
    assert(lean.find("property_f_graph_0_edges") != std::string::npos);
    assert(lean.find("property_f_graph_0_gamma_0") != std::string::npos);
    assert(lean.find("property_f_graph_0_digit_0_0") != std::string::npos);
    assert(lean.find("strong_coincidence_run_0_summary") != std::string::npos);
    assert(lean.find("strong_coincidence_run_0_resolution_depths") != std::string::npos);
    assert(lean.find("strong_coincidence_pair_witness_0_checked") != std::string::npos);
    assert(lean.find("property_f_graph_0_charpoly") != std::string::npos);
    assert(lean.find("propertyFQ2Step") != std::string::npos);
    assert(lean.find("property_f_graph_0_edge_0_0_topology") != std::string::npos);
    assert(lean.find("property_f_graph_0_edge_0_0_graph_recurrence") != std::string::npos);
    assert(lean.find("property_f_graph_0_nonzero_cycle_components") != std::string::npos);
    assert(lean.find("property_f_graph_0_scc_label_count_0") != std::string::npos);
    assert(lean.find("property_f_graph_0_letters_in_range") != std::string::npos);
    assert(lean.find("property_f_graph_0_edges_in_range") != std::string::npos);
    assert(lean.find("property_f_graph_0_zero_membership_0") != std::string::npos);
    assert(lean.find("property_f_graph_0_beta_inverse_matrix") != std::string::npos);
    assert(lean.find("property_f_graph_0_q_step") != std::string::npos);
    assert(lean.find("property_f_graph_0_beta_inverse_matrix_shape") != std::string::npos);
    assert(lean.find("property_f_graph_0_beta_inverse_denominator_0_0") != std::string::npos);
    assert(lean.find("property_f_graph_0_shape") != std::string::npos);
    if (const char* dump_path = std::getenv("RAVEL_PROPERTY_F_LEAN_OUT")) {
        std::ofstream dump(dump_path);
        dump << lean;
    }
    std::cout << "property_f_reflection: typed finite-run summary emitted\n";
}
