#include <cstdio>
#include <map>
#include <set>
#include <tuple>
#include <vector>

#include "ravel/contact_boundary.hpp"
#include "ravel/d_cont_check.hpp"
#include "ravel/return_contact_lift.hpp"
#include "ravel/proof/finite_graph_correspondence.hpp"
#include "ravel/spectral.hpp"
#include "ravel/survey.hpp"

using namespace ravel;

namespace {

int tests = 0;
int failures = 0;

void expect(bool condition, const char* label) {
    ++tests;
    if (condition) {
        std::printf("  [ok]   %s\n", label);
    } else {
        std::printf("  [FAIL] %s\n", label);
        ++failures;
    }
}

std::vector<std::vector<long long>> incidence(
        const std::vector<std::vector<std::int8_t>>& images) {
    std::vector<std::vector<long long>> matrix(
        images.size(), std::vector<long long>(images.size(), 0));
    for (std::size_t source = 0; source < images.size(); ++source) {
        for (auto letter : images[source]) {
            ++matrix[static_cast<std::size_t>(letter)][source];
        }
    }
    return matrix;
}

std::vector<SNode<3>> boundary_nodes(
        const std::vector<std::vector<std::int8_t>>& images,
        double& beta) {
    SubstitutionRule rule(images);
    const auto spectral = classify_matrix_spectral(incidence(images));
    beta = spectral.beta;
    const auto substitution = make_substitution<3>(rule, beta);
    const auto candidates = search_D_cont<3>(substitution, 2);
    std::vector<std::tuple<long long, std::vector<long long>, long long>>
        d_cont;
    for (const auto& candidate : candidates) {
        d_cont.emplace_back(
            candidate.i,
            std::vector<long long>(
                candidate.x.begin(), candidate.x.end()),
            candidate.j);
    }
    const auto report = compute_contact_boundary_dispatch(
        rule, spectral.beta, spectral.b2, d_cont);
    std::vector<SNode<3>> result;
    for (const auto& [i, x, j] : report.boundary_nodes) {
        SNode<3> node{};
        node.i = i;
        node.j = j;
        for (std::size_t k = 0; k < 3; ++k) node.x[k] = x[k];
        result.push_back(node);
    }
    return result;
}

}  // namespace

int main() {
    const std::vector<std::vector<std::int8_t>> images{
        {0, 1, 2}, {0, 2}, {0}};
    double beta = 0.0;
    const auto bare = boundary_nodes(images, beta);
    const SubstitutionRule rule(images);
    const auto substitution = make_substitution<3>(rule, beta);
    const auto phases = build_return_phase_system(rule, 0);

    expect(bare.size() == 26,
           "sigma_{1,1} full contact boundary has the expected 26 nodes");
    expect(phases.states.size() == 6,
           "sigma_{1,1} return tower has six phases");

    std::vector<ReturnContactState<3>> seeds;
    for (const auto& node : bare) {
        auto compatible = compatible_return_contact_seeds(node, phases);
        seeds.insert(seeds.end(), compatible.begin(), compatible.end());
    }
    const auto lift = build_reachable_return_contact_lift(
        substitution, bare, phases, seeds);
    expect(lift.complete(), "uncapped sparse exploration completes");
    expect(!lift.states.empty(), "lift has reachable states");
    expect(!lift.edges.empty(), "lift has reachable edges");
    expect(lift.states.size() <= bare.size()
                                      * phases.states.size()
                                      * phases.states.size(),
           "state count cannot exceed the dense product");

    bool compatible_states = true;
    for (const auto& state : lift.states) {
        compatible_states =
            compatible_states
            && return_phase_letter(phases, state.left_phase)
                   == state.contact.i
            && return_phase_letter(phases, state.right_phase)
                   == state.contact.j;
    }
    expect(compatible_states,
           "every stored phase projects to its contact endpoint letter");

    bool valid_edges = true;
    for (const auto& edge : lift.edges) {
        const auto& source = lift.states[edge.source];
        const auto& destination = lift.states[edge.destination];
        valid_edges =
            valid_edges
            && phases.phase_images[destination.left_phase]
                    [edge.left_position] == source.left_phase
            && phases.phase_images[destination.right_phase]
                    [edge.right_position] == source.right_phase
            && destination.contact == bare[edge.bare_destination]
            && source.contact == bare[edge.bare_source];
    }
    expect(valid_edges,
           "every lifted edge obeys both occurrence-position equations");
    expect(lift.projected_node_count == bare.size(),
           "all-seed exploration projects onto every bare node");
    std::set<std::pair<std::size_t, std::size_t>> bare_edge_support;
    std::map<SNode<3>, std::size_t> bare_index;
    for (std::size_t i = 0; i < bare.size(); ++i) {
        bare_index[bare[i]] = i;
    }
    for (std::size_t source = 0; source < bare.size(); ++source) {
        for (const auto& [destination, labels] :
             simple_forward_targets(substitution, bare[source])) {
            (void)labels;
            const auto found = bare_index.find(destination);
            if (found != bare_index.end()) {
                bare_edge_support.insert({source, found->second});
            }
        }
    }
    expect(lift.projected_edge_count == bare_edge_support.size(),
           "lift projection reproduces complete bare edge support");

    ReturnContactLimits state_limit;
    state_limit.max_states = 1;
    const auto capped = build_reachable_return_contact_lift(
        substitution, bare, phases, seeds, state_limit);
    expect(capped.stop == ReturnContactStop::state_cap,
           "state cap is explicit rather than silently truncating");
    expect(capped.states.size() == 1,
           "state cap prevents excess allocation");

    ReturnContactLimits edge_limit;
    edge_limit.max_edges = 1;
    const auto edge_capped = build_reachable_return_contact_lift(
        substitution, bare, phases, seeds, edge_limit);
    expect(edge_capped.stop == ReturnContactStop::edge_cap,
           "edge cap is explicit rather than silently truncating");
    expect(edge_capped.edges.size() == 1,
           "edge cap prevents excess allocation");

    ReturnContactLimits queue_limit;
    queue_limit.max_queue_bytes = sizeof(std::size_t);
    const auto queue_capped = build_reachable_return_contact_lift(
        substitution, bare, phases, seeds, queue_limit);
    expect(queue_capped.stop == ReturnContactStop::queue_cap,
           "logical queue-byte cap is explicit");
    expect(queue_capped.peak_queue_bytes == sizeof(std::size_t),
           "reported queue peak respects the byte cap");

    bool bad_seed_rejected = false;
    if (!seeds.empty()) {
        auto bad = seeds.front();
        bad.left_phase = phases.states.size();
        try {
            (void)build_reachable_return_contact_lift(
                substitution, bare, phases, {bad});
        } catch (const std::out_of_range&) {
            bad_seed_rejected = true;
        }
    }
    expect(bad_seed_rejected, "out-of-range seed phase is rejected");

    const auto escape = ravel::proof::derive_finite_escape_boundary_certificate(
        {{1}, {2}, {}, {0}},
        {true, true, true, false},
        {false, false, true, false});
    expect(escape.acyclic && escape.every_live_vertex_reaches_terminal &&
               escape.max_terminal_distance == 2 &&
               escape.live_vertices_without_terminal_route == 0,
           "finite escape certificate proves a bounded route to the terminal boundary");
    const auto cyclic_escape =
        ravel::proof::derive_finite_escape_boundary_certificate(
            {{1}, {0}}, {true, true}, {false, false});
    expect(!cyclic_escape.acyclic &&
               cyclic_escape.live_vertices_without_terminal_route == 2,
           "finite escape certificate rejects a recurrent boundary obstruction");

    const auto escape_height =
        ravel::proof::derive_finite_escape_height_certificate(
            {{1}, {2}, {}, {0}}, {true, true, true, false},
            {false, false, true, false}, {2, 1, 0, 99});
    expect(escape_height.strictly_decreasing &&
               escape_height.terminals_absorbing &&
               escape_height.proves_acyclic &&
               escape_height.checked_edges == 2 &&
               escape_height.maximum_live_height == 2,
           "finite escape height certificate proves strict descent into the shell");
    const auto bad_height =
        ravel::proof::derive_finite_escape_height_certificate(
            {{1}, {0}}, {true, true}, {false, false}, {0, 1});
    expect(!bad_height.strictly_decreasing && !bad_height.proves_acyclic &&
               bad_height.height_violations > 0,
           "finite escape height certificate rejects a nondecreasing cycle");
    const auto leaking_terminal =
        ravel::proof::derive_finite_escape_height_certificate(
            {{1}, {}}, {true, true}, {true, false}, {0, 1});
    expect(!leaking_terminal.terminals_absorbing &&
               leaking_terminal.terminal_outgoing_edges == 1 &&
               !leaking_terminal.proves_acyclic,
           "finite escape height certificate rejects an outgoing terminal shell");

    std::printf(
        "  lift: %zu sparse states, %zu edges, %zu/%zu projected nodes\n",
        lift.states.size(), lift.edges.size(),
        lift.projected_node_count, bare.size());
    std::printf("%d tests run, %d failed.\n", tests, failures);
    return failures == 0 ? 0 : 1;
}
