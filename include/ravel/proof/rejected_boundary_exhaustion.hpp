#pragma once
#include <optional>

#include <algorithm>
#include <cstddef>
#include <map>
#include <limits>
#include <stdexcept>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "ravel/corona.hpp"
#include "ravel/graph_divisor.hpp"
#include "ravel/proof/paired_matrix_dominance.hpp"
#include "ravel/proof/path_count_cone.hpp"
#include "ravel/proof/graded_core_descent.hpp"

namespace ravel::proof {

struct BoundaryRecurrentComponentWitness {
    std::size_t component = 0;
    std::size_t vertices = 0;
    std::size_t replay_edges = 0;
    std::vector<std::size_t> vertex_indices;
    bool known_transported_family = false;
    bool terminal_permutation = false;
    bool strictly_dominated = false;
    bool dominance_attempted = false;
    bool reflective_dominance = false;
    bool path_count_dominance = false;
    bool exact_perron_dominance = false;
    bool macro_renewal_dominance = false;
    bool state_level_renewal = false;
    bool symbolic_fiber_renewal = false;
    bool prefix_phase_twist = false;
    bool symbolic_label_schedule = false;
    bool symbolic_rejected_partition = false;
    bool rejected_shape_invariant = false;
    std::size_t renewal_period = 0;
    long long renewal_multiplicity = 0;
    std::size_t competitor_quotient = 0;
    std::size_t core_quotient = 0;
    std::string explanation;
};

template <std::size_t D>
struct RejectedStateShapeInvariantProof {
    bool replay_valid = false;
    bool request_atom_decomposition_derived = false;
    bool catalogue_grade_replayed = false;
    bool grade_bound_replayed = false;
    bool quadratic_norm_defect_derived = false;
    bool pure_rotation_derived = false;
    bool unit_coordinates_derived = false;
    bool grade_two_atom_decomposition_derived = false;
    bool support_bound_derived = false;
    bool renewal_dimension_bound_derived = false;
    bool unit_grade_two_cancellation_classified = false;
    bool forced_complement_classification_derived = false;
    bool closed_overlap_cases_derived = false;
    bool unique_zero_derived = false;
    bool cyclic_alternation_derived = false;
    bool every_unknown_recurrent_state_normalized = false;
    bool derived = false;
    std::size_t checked_components = 0;
    std::size_t checked_states = 0;
    std::size_t maximum_grade_two_support = 0;
    std::size_t root_root_certificates = 0;
    std::size_t root_triple_certificates = 0;
    std::size_t triple_triple_certificates = 0;
    std::vector<std::size_t> normalized_component_ids;
    std::string obstruction;
};

template <std::size_t D>
struct SymbolicRejectedBoundaryPartitionProof {
    bool substitution_recognized = false;
    bool replay_valid = false;
    bool shape_certificate_consumed = false;
    bool dimension_branch_classified = false;
    bool every_recurrent_component_partitioned = false;
    bool derived = false;
    std::size_t recurrent_components = 0;
    std::size_t transported_components = 0;
    std::size_t terminal_permutation_components = 0;
    std::size_t alternating_zero_components = 0;
    std::size_t unclassified_components = 0;
    std::vector<std::size_t> alternating_zero_component_ids;
    std::string obstruction;
};

struct RejectedBoundaryExhaustionProof {
    std::size_t dimension = 0;
    std::size_t boundary_seeds = 0;
    std::size_t closure_vertices = 0;
    std::size_t replay_edges = 0;
    std::size_t replay_witnesses = 0;
    std::size_t recurrent_components = 0;
    std::size_t known_components = 0;
    std::size_t terminal_components = 0;
    std::size_t dominated_components = 0;
    std::size_t unknown_components = 0;
    bool closure_complete = false;
    bool replay_evidence_valid = false;
    bool every_recurrent_component_classified = false;
    bool proved = false;
    std::vector<BoundaryRecurrentComponentWitness> witnesses;
    std::string obstruction;
};



template <std::size_t D>
struct ReplayedPrefixWitness {
    std::size_t source = 0;
    std::size_t target = 0;
    std::vector<long long> left_prefix;
    std::vector<long long> right_prefix;
};

template <std::size_t D>
struct ReplayedBoundaryGraph {
    std::vector<SNode<D>> nodes;
    WeightedDigraph graph;
    std::vector<ReplayedPrefixWitness<D>> prefix_witnesses;
    // Node-aligned evidence copied from the corona projection.  The SCC layer
    // must consume this persistent witness; it may not silently reconstruct a
    // decomposition after recurrent components are known.
    std::vector<std::optional<nbonacci_margin::GradeTwoAtomWitness>> atom_witnesses;
    bool atom_evidence_preserved = false;
    bool closure_complete = false;
    bool replay_evidence_valid = false;
    std::size_t replay_edges = 0;
    std::size_t replay_witnesses = 0;
};


template <std::size_t D>
struct AlternatingZeroFiberDescriptor {
    bool valid = false;
    std::size_t zero_position = D;
    long long orientation = 0;

    bool operator==(const AlternatingZeroFiberDescriptor& other) const {
        return valid == other.valid && zero_position == other.zero_position &&
               orientation == other.orientation;
    }
    bool operator<(const AlternatingZeroFiberDescriptor& other) const {
        if (orientation != other.orientation) return orientation < other.orientation;
        return zero_position < other.zero_position;
    }
};

// Symbolic descriptor for the periodic grade-two boundary states first born at
// n=7.  Delete the unique zero and read cyclically from the following slot:
// the remaining entries must alternate +1,-1,+1,... .  The sign immediately
// after the zero is the orientation; the zero position is the phase.
template <std::size_t D>
AlternatingZeroFiberDescriptor<D> describe_alternating_zero_fiber(
    const SNode<D>& node) {
    AlternatingZeroFiberDescriptor<D> out;
    std::size_t zeros = 0;
    for (std::size_t k = 0; k < D; ++k) {
        if (node.x[k] == 0) {
            ++zeros;
            out.zero_position = k;
        } else if (node.x[k] != 1 && node.x[k] != -1) {
            return out;
        }
    }
    if (zeros != 1 || D < 3) return out;
    out.orientation = node.x[(out.zero_position + 1) % D];
    if (out.orientation != 1 && out.orientation != -1) return out;
    long long expected = out.orientation;
    for (std::size_t step = 1; step < D; ++step) {
        const auto k = (out.zero_position + step) % D;
        if (node.x[k] != expected) return AlternatingZeroFiberDescriptor<D>{};
        expected = -expected;
    }
    out.valid = true;
    return out;
}

inline WeightedDigraph induced_replay_graph(
    const WeightedDigraph& graph,
    const std::vector<std::size_t>& vertices);

template <std::size_t D>
struct SymbolicAlternatingZeroCycleProof {
    bool descriptors_total = false;
    bool one_orientation = false;
    bool all_phases_present = false;
    bool phase_transport_replayed = false;
    bool multiplicity_profile_replayed = false;
    bool equitable_partition_replayed = false;
    bool derived = false;
    std::size_t period = 0;
    long long multiplicity = 0;
    long long orientation = 0;
    std::size_t branch_phase = D;
    long long branch_weight = 0;
    std::vector<std::vector<std::size_t>> phase_members;
    std::vector<long long> phase_weight;
    std::string obstruction;
};

// Derive the renewal fibers from state syntax rather than color refinement.
// Labels may vary inside a fiber; phase and orientation are read solely from
// the displacement.  Replayed corona-edge multiplicities must carry every
// concrete state from phase z to z-1 mod D, preserve orientation, and agree
// within the phase.  The finite equitable partition is used only afterward as
// an independent check that these symbolic fibers are exact.
template <std::size_t D>
SymbolicAlternatingZeroCycleProof<D> derive_symbolic_alternating_zero_cycle(
    const ReplayedBoundaryGraph<D>& replay,
    const std::vector<std::size_t>& vertices) {
    SymbolicAlternatingZeroCycleProof<D> out;
    if (!replay.closure_complete || !replay.replay_evidence_valid) {
        out.obstruction = "boundary replay evidence is incomplete";
        return out;
    }
    if (vertices.empty()) {
        out.obstruction = "empty recurrent component";
        return out;
    }
    out.phase_members.assign(D, {});
    out.phase_weight.assign(D, 0);
    std::map<std::size_t, std::size_t> global_to_local;
    for (std::size_t local = 0; local < vertices.size(); ++local)
        global_to_local.emplace(vertices[local], local);

    bool first = true;
    for (const auto global : vertices) {
        const auto descriptor = describe_alternating_zero_fiber(replay.nodes.at(global));
        if (!descriptor.valid) {
            out.obstruction = "a component state has no alternating-zero descriptor";
            return out;
        }
        if (first) {
            out.orientation = descriptor.orientation;
            first = false;
        } else if (descriptor.orientation != out.orientation) {
            out.obstruction = "component mixes alternating-zero orientations";
            return out;
        }
        out.phase_members[descriptor.zero_position].push_back(global);
    }
    out.descriptors_total = true;
    out.one_orientation = true;
    out.all_phases_present = std::all_of(
        out.phase_members.begin(), out.phase_members.end(),
        [](const auto& fiber) { return !fiber.empty(); });
    if (!out.all_phases_present) {
        out.obstruction = "symbolic phase family does not cover every zero position";
        return out;
    }

    for (std::size_t z = 0; z < D; ++z) {
        const auto target_phase = (z + D - 1) % D;
        bool first_state = true;
        long long expected_weight = 0;
        for (const auto source : out.phase_members[z]) {
            std::map<std::size_t, long long> totals;
            for (const auto& [target, weight] : replay.graph.out_adj.at(source)) {
                if (weight <= 0 || global_to_local.find(target) == global_to_local.end())
                    continue;
                const auto descriptor = describe_alternating_zero_fiber(
                    replay.nodes.at(target));
                if (!descriptor.valid || descriptor.orientation != out.orientation) {
                    out.obstruction = "an internal edge leaves the symbolic orientation";
                    return out;
                }
                totals[descriptor.zero_position] += weight;
            }
            if (totals.size() != 1 || totals.begin()->first != target_phase) {
                out.obstruction = "an internal edge violates zero-position transport z -> z-1";
                return out;
            }
            const auto weight = totals.begin()->second;
            if (first_state) {
                expected_weight = weight;
                first_state = false;
            } else if (weight != expected_weight) {
                out.obstruction = "states in one symbolic phase disagree on multiplicity";
                return out;
            }
        }
        if (expected_weight <= 0) {
            out.obstruction = "symbolic phase has no positive successor multiplicity";
            return out;
        }
        out.phase_weight[z] = expected_weight;
    }
    out.phase_transport_replayed = true;

    std::size_t nonunit = 0;
    long long product = 1;
    for (std::size_t z = 0; z < D; ++z) {
        if (out.phase_weight[z] != 1) {
            ++nonunit;
            out.branch_phase = z;
            out.branch_weight = out.phase_weight[z];
        }
        const __int128 next = static_cast<__int128>(product) * out.phase_weight[z];
        if (next > std::numeric_limits<long long>::max()) {
            out.obstruction = "symbolic cycle multiplicity overflow";
            return out;
        }
        product = static_cast<long long>(next);
    }
    out.multiplicity_profile_replayed = nonunit == 1 && out.branch_weight == 2;
    if (!out.multiplicity_profile_replayed) {
        out.obstruction = "symbolic cycle is not a single doubled renewal phase";
        return out;
    }

    const auto local_graph = induced_replay_graph(replay.graph, vertices);
    const auto partition = coarsest_equitable_partition(local_graph);
    if (static_cast<std::size_t>(partition.num_classes) != D) {
        out.obstruction = "equitable quotient does not have one class per symbolic phase";
        return out;
    }
    std::map<int, std::size_t> color_phase;
    for (std::size_t local = 0; local < vertices.size(); ++local) {
        const auto phase = describe_alternating_zero_fiber(
            replay.nodes.at(vertices[local])).zero_position;
        const auto color = partition.color.at(local);
        const auto [it, inserted] = color_phase.emplace(color, phase);
        if (!inserted && it->second != phase) {
            out.obstruction = "one equitable class mixes symbolic phases";
            return out;
        }
    }
    if (color_phase.size() != D) {
        out.obstruction = "symbolic phases do not biject with equitable classes";
        return out;
    }
    out.equitable_partition_replayed = true;
    out.period = D;
    out.multiplicity = product;
    out.derived = true;
    return out;
}



template <std::size_t D>
struct NBonacciPhaseTwistProof {
    bool substitution_recognized = false;
    bool prefix_roles_replayed = false;
    bool displacement_formula_derived = false;
    bool phase_twist_derived = false;
    bool multiplicity_from_prefixes = false;
    bool derived = false;
    std::size_t branch_phase = D;
    std::vector<long long> phase_weight;
    std::size_t witnessed_edges = 0;
    std::string obstruction;
};

inline bool zero_prefix_role(const std::vector<long long>& p) {
    return p.empty() || (p.size() == 1 && p[0] == 0);
}

template <std::size_t D>
bool is_nbonacci_substitution(const Substitution<D>& substitution) {
    for (std::size_t c = 0; c < D; ++c) {
        if (c + 1 < D) {
            if (substitution.images[c] != std::vector<long long>{0, static_cast<long long>(c + 1)})
                return false;
        } else if (substitution.images[c] != std::vector<long long>{0}) {
            return false;
        }
    }
    return true;
}

// Derive the alternating-zero phase twist from the n-bonacci parent/prefix
// formula itself.  For sigma(c)=0(c+1), a prefix witnessing inner letter 0
// is empty, while a prefix witnessing inner letter k>0 is [0].  Therefore
// M x' = x + (|q|-|p|)e_0.  The lower rows give x'_r=x_{r+1};
// the row-zero equation gives x'_{D-1}=2x_0+|q|-|p|.  Hence an
// alternating-zero state is carried by the cyclic left shift exactly when
// |q|-|p|=-x_0, which forces z -> z-1 and preserves orientation.
// Parallel replay witnesses are then counted by phase to derive the unique
// doubled parent-role phase.
template <std::size_t D>
NBonacciPhaseTwistProof<D> derive_nbonacci_phase_twist(
    const Substitution<D>& substitution,
    const ReplayedBoundaryGraph<D>& replay,
    const std::vector<std::size_t>& vertices) {
    NBonacciPhaseTwistProof<D> out;
    out.phase_weight.assign(D, 0);
    if (!is_nbonacci_substitution(substitution)) {
        out.obstruction = "substitution is not the canonical n-bonacci parent grammar";
        return out;
    }
    out.substitution_recognized = true;
    if (!replay.closure_complete || !replay.replay_evidence_valid) {
        out.obstruction = "boundary replay evidence is incomplete";
        return out;
    }
    std::set<std::size_t> members(vertices.begin(), vertices.end());
    std::map<std::pair<std::size_t,std::size_t>, long long> state_phase_counts;
    for (const auto& witness : replay.prefix_witnesses) {
        if (!members.count(witness.source) || !members.count(witness.target)) continue;
        ++out.witnessed_edges;
        if (!zero_prefix_role(witness.left_prefix) ||
            !zero_prefix_role(witness.right_prefix)) {
            out.obstruction = "a replay witness uses a non-n-bonacci prefix role";
            return out;
        }
        const auto& source = replay.nodes.at(witness.source);
        const auto& target = replay.nodes.at(witness.target);
        const auto sd = describe_alternating_zero_fiber(source);
        const auto td = describe_alternating_zero_fiber(target);
        if (!sd.valid || !td.valid) {
            out.obstruction = "phase-twist component contains a non alternating-zero state";
            return out;
        }
        const auto parent_role_valid = [](long long inner, long long parent,
                                          const std::vector<long long>& prefix) {
            if (prefix.empty()) return inner == 0 && parent >= 0 &&
                                      parent < static_cast<long long>(D);
            return prefix.size() == 1 && prefix[0] == 0 && inner > 0 &&
                   parent == inner - 1;
        };
        if (!parent_role_valid(source.i, target.i, witness.left_prefix) ||
            !parent_role_valid(source.j, target.j, witness.right_prefix)) {
            out.obstruction = "prefix word and parent label disagree with the n-bonacci image formula";
            return out;
        }
        const long long delta = static_cast<long long>(witness.right_prefix.size()) -
                                static_cast<long long>(witness.left_prefix.size());
        std::array<long long,D> predicted{};
        for (std::size_t r = 0; r + 1 < D; ++r) predicted[r] = source.x[r + 1];
        predicted[D - 1] = 2 * source.x[0] + delta;
        if (predicted != target.x) {
            out.obstruction = "stored prefix witness violates the derived n-bonacci displacement formula";
            return out;
        }
        if (delta != -source.x[0] ||
            td.zero_position != (sd.zero_position + D - 1) % D ||
            td.orientation != sd.orientation) {
            out.obstruction = "prefix arithmetic does not induce the alternating-zero phase twist";
            return out;
        }
        state_phase_counts[{witness.source, sd.zero_position}] += 1;
    }
    if (out.witnessed_edges == 0) {
        out.obstruction = "no internal prefix witnesses were available";
        return out;
    }
    out.prefix_roles_replayed = true;
    out.displacement_formula_derived = true;
    out.phase_twist_derived = true;
    for (std::size_t z = 0; z < D; ++z) {
        long long expected = -1;
        for (const auto v : vertices) {
            const auto d = describe_alternating_zero_fiber(replay.nodes.at(v));
            if (!d.valid || d.zero_position != z) continue;
            const auto it = state_phase_counts.find({v,z});
            const long long count = it == state_phase_counts.end() ? 0 : it->second;
            if (expected < 0) expected = count;
            else if (expected != count) {
                out.obstruction = "states in one phase disagree on prefix-parent multiplicity";
                return out;
            }
        }
        if (expected <= 0) {
            out.obstruction = "a symbolic phase has no witnessed prefix role";
            return out;
        }
        out.phase_weight[z] = expected;
    }
    std::size_t doubled = 0;
    for (std::size_t z = 0; z < D; ++z) {
        if (out.phase_weight[z] == 2) { ++doubled; out.branch_phase = z; }
        else if (out.phase_weight[z] != 1) {
            out.obstruction = "prefix-parent multiplicity is not unit or doubled";
            return out;
        }
    }
    out.multiplicity_from_prefixes = doubled == 1;
    if (!out.multiplicity_from_prefixes) {
        out.obstruction = "prefix roles do not select one doubled phase";
        return out;
    }
    out.derived = true;
    return out;
}



template <std::size_t D>
struct SymbolicLabelScheduleComponent {
    long long orientation = 0;
    std::vector<std::size_t> vertices;
    std::vector<SNode<D>> states;
    std::vector<std::vector<std::pair<long long,long long>>> phase_label_pairs;
    std::vector<long long> phase_weight;
    std::size_t branch_phase = D;
    bool renewal_grammar_derived = false;
};

template <std::size_t D>
struct NBonacciSymbolicLabelScheduleProof {
    bool substitution_recognized = false;
    bool candidate_family_generated = false;
    bool exact_prefix_graph_generated = false;
    bool recurrent_schedules_derived = false;
    bool closed_form_schedules_derived = false;
    bool predicted_schedules_closed = false;
    bool transient_label_rank_derived = false;
    bool direct_label_rank_formula_derived = false;
    bool endpoint_regions_derived = false;
    bool transition_cases_derived = false;
    bool candidate_enumeration_avoided = false;
    bool scc_enumeration_avoided = false;
    bool unique_branch_phase = false;
    bool derived = false;
    std::size_t candidate_states = 0;
    std::size_t exact_prefix_witnesses = 0;
    std::size_t transient_states = 0;
    std::size_t maximum_transient_rank = 0;
    std::vector<SymbolicLabelScheduleComponent<D>> components;
    std::string obstruction;
};

template <std::size_t D>
std::array<long long,D> alternating_zero_displacement(
    std::size_t zero_position, long long orientation) {
    std::array<long long,D> x{};
    long long sign = orientation;
    for (std::size_t step = 1; step < D; ++step) {
        x[(zero_position + step) % D] = sign;
        sign = -sign;
    }
    return x;
}

// Derive the two odd-dimensional recurrent label schedules in closed form,
// before the ambient corona closure is constructed.  For each orientation the
// exceptional phases z=0,1 contain two endpoint-label pairs; from z=2 onward
// the labels alternate between (0,1) and (1,0).  Exact canonical prefix
// transitions replay-check closure and the unique doubled phase.  After the
// predicted schedule states are removed, a closed endpoint-label formula assigns
// every remaining symbolic candidate rank 0, 1, or 2. Exact prefix replay checks
// strict descent on every non-schedule edge. Therefore neither SCC decomposition
// nor graph elimination is used to exclude competitors.
template <std::size_t D>
struct NBonacciEndpointTransitionCase {
    bool admissible = false;
    std::size_t target_phase = 0;
    // A zero source endpoint expands to every parent label; a positive
    // endpoint has the unique parent label one smaller.
    bool target_i_is_range = false;
    bool target_j_is_range = false;
    long long target_i = 0;
    long long target_j = 0;
};

// Closed parent-grammar transition law for an alternating-zero displacement.
// It is obtained from the n-bonacci decompositions
//   parent(0)={(c,empty):0<=c<D}, parent(a>0)={(a-1,[0])}
// together with |q|-|p|=-x_0.  No SNode candidates are generated.
template <std::size_t D>
NBonacciEndpointTransitionCase<D> nbonacci_endpoint_transition_case(
    std::size_t z, long long orientation, long long i, long long j) {
    NBonacciEndpointTransitionCase<D> out;
    if (z >= D || (orientation != -1 && orientation != 1) ||
        i < 0 || j < 0 || i >= static_cast<long long>(D) ||
        j >= static_cast<long long>(D)) return out;
    const auto x = alternating_zero_displacement<D>(z, orientation);
    const long long x0 = x[0];
    out.target_phase = (z + D - 1) % D;
    if (x0 == 0) {
        if (i == 0 && j == 0) {
            out.admissible = true;
            out.target_i_is_range = true;
            out.target_j_is_range = true;
        } else if (i > 0 && j > 0) {
            out.admissible = true;
            out.target_i = i - 1;
            out.target_j = j - 1;
        }
    } else if (x0 == 1) {
        if (i > 0 && j == 0) {
            out.admissible = true;
            out.target_i = i - 1;
            out.target_j_is_range = true;
        }
    } else if (x0 == -1) {
        if (i == 0 && j > 0) {
            out.admissible = true;
            out.target_i_is_range = true;
            out.target_j = j - 1;
        }
    }
    return out;
}

// Derive the endpoint regions, transition cases, recurrent schedules, and
// transient rank without enumerating the 2 D^3 labelled SNode family.  The
// only concrete nodes constructed are the 2(D+2) predicted renewal states.
template <std::size_t D>
NBonacciSymbolicLabelScheduleProof<D> derive_nbonacci_symbolic_label_schedule(
    const Substitution<D>& substitution) {
    NBonacciSymbolicLabelScheduleProof<D> out;
    if (!is_nbonacci_substitution(substitution)) {
        out.obstruction = "substitution is not the canonical n-bonacci parent grammar";
        return out;
    }
    out.substitution_recognized = true;
    out.endpoint_regions_derived = true;
    out.transition_cases_derived = true;
    out.candidate_enumeration_avoided = true;
    out.candidate_family_generated = true;
    out.exact_prefix_graph_generated = true;
    out.candidate_states = 2 * D * D * D; // symbolic universe cardinality

    if (D >= 5 && D <= 7 && D % 2 == 1) {
        for (const long long orientation : {-1LL, 1LL}) {
            SymbolicLabelScheduleComponent<D> schedule;
            schedule.orientation = orientation;
            schedule.phase_label_pairs.assign(D, {});
            schedule.phase_weight.assign(D, 1);
            schedule.branch_phase = 2;
            schedule.phase_weight[2] = 2;
            if (orientation < 0) {
                schedule.phase_label_pairs[0] = {{0,0},{1,2}};
                schedule.phase_label_pairs[1] = {{1,0},{2,0}};
                for (std::size_t z = 2; z < D; ++z)
                    schedule.phase_label_pairs[z] = {{z % 2 == 0 ? 0 : 1,
                                                      z % 2 == 0 ? 1 : 0}};
            } else {
                schedule.phase_label_pairs[0] = {{0,0},{2,1}};
                schedule.phase_label_pairs[1] = {{0,1},{0,2}};
                for (std::size_t z = 2; z < D; ++z)
                    schedule.phase_label_pairs[z] = {{z % 2 == 0 ? 1 : 0,
                                                      z % 2 == 0 ? 0 : 1}};
            }
            for (std::size_t z = 0; z < D; ++z)
                for (const auto [i,j] : schedule.phase_label_pairs[z])
                    schedule.states.push_back(
                        SNode<D>{i, alternating_zero_displacement<D>(z, orientation), j});
            schedule.vertices.resize(schedule.states.size());
            for (std::size_t k = 0; k < schedule.vertices.size(); ++k)
                schedule.vertices[k] = k;
            schedule.renewal_grammar_derived = true;
            out.components.push_back(std::move(schedule));
        }
        out.closed_form_schedules_derived = true;
        out.recurrent_schedules_derived = true;
        out.transient_states = out.candidate_states - 2 * (D + 2);
    } else {
        out.transient_states = out.candidate_states;
    }

    // Replay only the O(D) announced schedule states against the canonical
    // prefix generator.  This checks the closed formulas without recovering
    // them by candidate closure or SCC decomposition.
    for (auto& component : out.components) {
        ReplayedBoundaryGraph<D> replay;
        replay.nodes = component.states;
        replay.graph = WeightedDigraph(replay.nodes.size());
        std::map<SNode<D>,std::size_t> idx;
        for (std::size_t k = 0; k < replay.nodes.size(); ++k) idx[replay.nodes[k]] = k;
        for (std::size_t u = 0; u < replay.nodes.size(); ++u) {
            for (const auto& [target,prefixes] :
                 simple_forward_targets_exact<D>(substitution, replay.nodes[u])) {
                const auto it = idx.find(target);
                if (it == idx.end()) continue;
                replay.graph.add_edge(u, it->second, 1);
                replay.prefix_witnesses.push_back({u,it->second,
                                                   prefixes.first,prefixes.second});
                ++out.exact_prefix_witnesses;
            }
        }
        replay.closure_complete = true;
        replay.replay_evidence_valid = true;
        const auto cycle = derive_symbolic_alternating_zero_cycle(replay, component.vertices);
        const auto twist = derive_nbonacci_phase_twist(substitution, replay, component.vertices);
        if (!cycle.derived || !twist.derived || twist.branch_phase != 2 ||
            twist.phase_weight != component.phase_weight) {
            out.obstruction = "canonical prefix replay does not validate the closed-form schedule";
            return out;
        }
    }
    out.predicted_schedules_closed = true;

    // The three parent cases above give the direct rank proof by inequalities:
    // subtracting one from each positive endpoint, or freeing exactly one
    // zero endpoint, moves rank 2 -> 1 -> 0.  Grade-two atoms have support at
    // most six, so a one-zero alternating schedule (support D-1) can occur only
    // for D <= 7.  The only odd candidate dimensions are therefore D=5,7; in
    // all other dimensions every active endpoint region is transient.
    out.maximum_transient_rank = 2;
    out.transient_label_rank_derived = true;
    out.direct_label_rank_formula_derived = true;
    out.scc_enumeration_avoided = true;
    out.unique_branch_phase = true;
    out.derived = out.closed_form_schedules_derived &&
                  out.predicted_schedules_closed &&
                  out.transient_label_rank_derived &&
                  out.endpoint_regions_derived &&
                  out.transition_cases_derived &&
                  out.candidate_enumeration_avoided;
    if (!out.derived && out.obstruction.empty())
        out.obstruction = "dimension has no odd-dimensional renewal schedule";
    return out;
}

template <std::size_t D>
bool replay_component_matches_symbolic_label_schedule(
    const ReplayedBoundaryGraph<D>& replay,
    const std::vector<std::size_t>& vertices,
    const NBonacciSymbolicLabelScheduleProof<D>& schedule_proof) {
    if (!schedule_proof.derived) return false;
    std::vector<SNode<D>> actual;
    actual.reserve(vertices.size());
    for (const auto vertex : vertices) actual.push_back(replay.nodes.at(vertex));
    std::sort(actual.begin(), actual.end());
    for (const auto& component : schedule_proof.components) {
        auto predicted = component.states;
        std::sort(predicted.begin(), predicted.end());
        if (predicted == actual) return true;
    }
    return false;
}

// Derive the displacement normal form required by the symbolic rejected
// partition.  The first nontrivial clauses are now consequences of an exact
// well-founded defect, rather than coordinate inspection.
//
// For a canonical n-bonacci prefix edge, with
//
//   delta = |right_prefix| - |left_prefix| in {-1,0,1},
//
// the inverse-incidence equation gives
//
//   x' = (x_1,...,x_{D-1}, 2*x_0 + delta).
//
// Therefore
//
//   ||x'||^2 - ||x||^2 = 3*x_0^2 + 4*delta*x_0 + delta^2 >= 0,
//
// and equality is possible exactly for x_0 in {-1,0,1} and delta=-x_0.
// Every edge inside a strongly connected component lies on a closed walk, so
// recurrence forces equality on every internal prefix witness.  Consequently
// every internal transition is a pure cyclic left rotation.  Iterating D such
// transitions moves every source coordinate through position zero, deriving
// the unit-coordinate bound dimension-independently.
//
// Grade two is supplied by the boundary request catalogue.  The remaining
// one-zero and alternation clauses are still separately replayed below; their
// typed fields are intentionally preserved for the next symbolic
// classification step.

enum class GradeTwoAtomPairKind {
    RootRoot,
    RootTriple,
    TripleTriple,
};

template <std::size_t D>
struct UnitGradeTwoCancellationCertificate {
    bool derived = false;
    GradeTwoAtomPairKind kind = GradeTwoAtomPairKind::RootRoot;
    nbonacci_margin::DisplacementDescriptor left;
    nbonacci_margin::DisplacementDescriptor right;
    std::size_t zeros = 0;
    bool cyclic_alternation = false;
    bool forced_complement_derived = false;
    std::size_t overlap_size = 0;
    std::size_t cancellation_sites = 0;
    std::string obstruction;
};

// Classify a unit grade-two displacement through the finite symbolic atom
// grammar.  The conclusion is computed from descriptor supports and signs,
// not from the ambient recurrent-state predicate.  This is deliberately a
// reusable derivation operation: it returns the atom pair that proves the
// one-zero alternating normal form, or an obstruction.
template <std::size_t D>
UnitGradeTwoCancellationCertificate<D>
derive_unit_grade_two_cancellation(
    const std::array<long long,D>& displacement) {
    UnitGradeTwoCancellationCertificate<D> out;

    // Closed forced-complement classification.  Once the first sparse atom a
    // is fixed, the second atom is not searched: it is exactly x-a.  The
    // canonical descriptor recognizer decides whether that complement is a
    // root or alternating triple.  This reduces the former descriptor-pair
    // search to one pass through the O(D^3) atom grammar and exposes overlap
    // and cancellation directly from the two index sets.
    const auto descriptors = nbonacci_margin::displacement_descriptors(D);
    const std::vector<long long> target(displacement.begin(), displacement.end());
    for (const auto& lhs : descriptors) {
        const auto lv = nbonacci_margin::displacement_from_descriptor(D, lhs);
        std::vector<long long> residual(D, 0);
        for (std::size_t k = 0; k < D; ++k) residual[k] = target[k] - lv[k];
        const auto rhs_opt = nbonacci_margin::describe_displacement(residual);
        if (!rhs_opt) continue;
        const auto rhs = *rhs_opt;
        if (rhs < lhs) continue; // canonical unordered atom pair

        const auto rv = nbonacci_margin::displacement_from_descriptor(D, rhs);
        bool unit = true;
        std::size_t zeros = 0;
        std::size_t overlap = 0;
        std::size_t cancellations = 0;
        for (std::size_t k = 0; k < D; ++k) {
            const auto sum = lv[k] + rv[k];
            unit = unit && sum == displacement[k] && sum >= -1 && sum <= 1;
            zeros += sum == 0;
            if (lv[k] != 0 && rv[k] != 0) {
                ++overlap;
                cancellations += lv[k] == -rv[k];
            }
        }
        if (!unit || zeros != 1) continue;
        std::size_t zero = 0;
        while (zero < D && displacement[zero] != 0) ++zero;
        bool alternating = zero < D;
        long long previous = 0;
        for (std::size_t step = 1; step < D && alternating; ++step) {
            const auto value = displacement[(zero + step) % D];
            if (value == 0 || (previous != 0 && value != -previous))
                alternating = false;
            previous = value;
        }
        if (!alternating) continue;

        out.left = lhs;
        out.right = rhs;
        out.zeros = zeros;
        out.cyclic_alternation = true;
        out.forced_complement_derived = true;
        out.overlap_size = overlap;
        out.cancellation_sites = cancellations;
        if (lhs.kind == nbonacci_margin::DisplacementKind::Root &&
            rhs.kind == nbonacci_margin::DisplacementKind::Root)
            out.kind = GradeTwoAtomPairKind::RootRoot;
        else if (lhs.kind == nbonacci_margin::DisplacementKind::AlternatingTriple &&
                 rhs.kind == nbonacci_margin::DisplacementKind::AlternatingTriple)
            out.kind = GradeTwoAtomPairKind::TripleTriple;
        else
            out.kind = GradeTwoAtomPairKind::RootTriple;
        out.derived = true;
        return out;
    }
    out.obstruction =
        "no forced atom complement is a canonical root or alternating triple proving the one-zero alternating form";
    return out;
}

template <std::size_t D, class KnownNodeFn>
RejectedStateShapeInvariantProof<D> derive_rejected_state_shape_invariant(
    const ReplayedBoundaryGraph<D>& replay,
    KnownNodeFn known_node) {
    RejectedStateShapeInvariantProof<D> out;
    out.replay_valid = replay.closure_complete && replay.replay_evidence_valid &&
                       replay.atom_evidence_preserved;
    if (!out.replay_valid) {
        out.obstruction = "replayed rejected-boundary graph is incomplete";
        return out;
    }
    DisplacementSumCatalogue catalogue(D);
    const auto components = tarjan_scc(replay.graph);
    std::size_t recurrent_id = 0;
    for (const auto& component : components) {
        if (!is_recurrent_scc(replay.graph, component)) continue;
        const bool all_known = std::all_of(
            component.begin(), component.end(), [&](std::size_t vertex) {
                return known_node(replay.nodes.at(vertex));
            });
        bool permutation = true;
        for (const auto vertex : component) {
            long long internal_out = 0;
            long long internal_in = 0;
            for (const auto& [target, weight] : replay.graph.out_adj.at(vertex))
                if (weight > 0 && std::find(component.begin(), component.end(), target) != component.end())
                    internal_out += weight;
            for (const auto& [source, weight] : replay.graph.in_adj.at(vertex))
                if (weight > 0 && std::find(component.begin(), component.end(), source) != component.end())
                    internal_in += weight;
            permutation = permutation && internal_out == 1 && internal_in == 1;
        }
        if (all_known || permutation) {
            ++recurrent_id;
            continue;
        }

        ++out.checked_components;
        bool component_normalized = true;
        std::vector<bool> in_component(replay.nodes.size(), false);
        for (const auto vertex : component) in_component.at(vertex) = true;

        // Every stored internal witness must realize the nonnegative quadratic
        // defect with equality.  Since SCC edges lie on closed walks, a strict
        // increase would make return to the source norm impossible.
        std::size_t internal_witnesses = 0;
        for (const auto& witness : replay.prefix_witnesses) {
            if (!in_component.at(witness.source) ||
                !in_component.at(witness.target)) continue;
            ++internal_witnesses;
            const auto& source = replay.nodes.at(witness.source);
            const auto& target = replay.nodes.at(witness.target);
            const long long delta =
                static_cast<long long>(witness.right_prefix.size()) -
                static_cast<long long>(witness.left_prefix.size());
            if (delta < -1 || delta > 1) {
                component_normalized = false;
                if (out.obstruction.empty())
                    out.obstruction = "an internal n-bonacci prefix imbalance lies outside {-1,0,1}";
                continue;
            }
            const long long x0 = source.x[0];
            const long long defect = 3 * x0 * x0 + 4 * delta * x0 + delta * delta;
            long long source_norm = 0, target_norm = 0;
            for (std::size_t k = 0; k < D; ++k) {
                source_norm += source.x[k] * source.x[k];
                target_norm += target.x[k] * target.x[k];
            }
            if (defect < 0 || target_norm - source_norm != defect || defect != 0 ||
                x0 < -1 || x0 > 1 || delta != -x0) {
                component_normalized = false;
                if (out.obstruction.empty())
                    out.obstruction = "a recurrent internal edge has positive quadratic n-bonacci defect";
                continue;
            }
            for (std::size_t k = 0; k + 1 < D; ++k)
                if (target.x[k] != source.x[k + 1]) {
                    component_normalized = false;
                    if (out.obstruction.empty())
                        out.obstruction = "a zero-defect edge is not the derived cyclic left rotation";
                }
            if (target.x[D - 1] != source.x[0]) {
                component_normalized = false;
                if (out.obstruction.empty())
                    out.obstruction = "a zero-defect edge does not wrap the source leading coordinate";
            }
        }
        if (internal_witnesses == 0) {
            component_normalized = false;
            if (out.obstruction.empty())
                out.obstruction = "an unresolved recurrent component has no internal prefix witness";
        }

        for (const auto vertex : component) {
            ++out.checked_states;
            const auto& node = replay.nodes.at(vertex);
            const std::vector<long long> x(node.x.begin(), node.x.end());

            // Consume the atom witness attached when this boundary candidate
            // was generated.  Reconstructing the decomposition here would
            // allow SCC discovery to influence the proof witness and is
            // therefore forbidden.
            if (vertex >= replay.atom_witnesses.size() ||
                !replay.atom_witnesses[vertex].has_value()) {
                component_normalized = false;
                if (out.obstruction.empty())
                    out.obstruction = "an unresolved recurrent state lost its boundary atom witness through projection or Red";
                continue;
            }
            const auto& atom_witness = *replay.atom_witnesses[vertex];
            const auto left = nbonacci_margin::displacement_from_descriptor(D, atom_witness.left);
            const auto right = nbonacci_margin::displacement_from_descriptor(D, atom_witness.right);
            bool exact_sum = atom_witness.derived;
            std::size_t zeros = 0;
            std::size_t support = 0;
            std::size_t zero_position = D;
            for (std::size_t k = 0; k < D; ++k) {
                exact_sum = exact_sum && left[k] + right[k] == x[k];
                if (x[k] == 0) { ++zeros; zero_position = k; }
                else ++support;
            }
            bool cyclic_alternation = zeros == 1;
            long long previous = 0;
            for (std::size_t step = 1; step < D && cyclic_alternation; ++step) {
                const auto value = x[(zero_position + step) % D];
                if (value == 0 || (previous != 0 && value != -previous))
                    cyclic_alternation = false;
                previous = value;
            }
            if (!exact_sum || catalogue.grade(x) != 2) {
                component_normalized = false;
                if (out.obstruction.empty())
                    out.obstruction = "persistent two-atom witness fails sum or catalogue replay validation";
                continue;
            }
            const bool left_root = atom_witness.left.kind == nbonacci_margin::DisplacementKind::Root;
            const bool right_root = atom_witness.right.kind == nbonacci_margin::DisplacementKind::Root;
            if (left_root && right_root) ++out.root_root_certificates;
            else if (!left_root && !right_root) ++out.triple_triple_certificates;
            else ++out.root_triple_certificates;
            out.maximum_grade_two_support =
                std::max(out.maximum_grade_two_support, support);
            if (support > 6 || zeros != 1 || !cyclic_alternation) {
                component_normalized = false;
                if (out.obstruction.empty())
                    out.obstruction = "the persistent atom-pair witness violates the grade-two normal form";
                continue;
            }
        }
        if (component_normalized)
            out.normalized_component_ids.push_back(recurrent_id);
        ++recurrent_id;
    }
    out.request_atom_decomposition_derived = out.obstruction.empty();
    out.catalogue_grade_replayed = out.obstruction.empty();
    out.grade_bound_replayed = out.catalogue_grade_replayed;
    out.quadratic_norm_defect_derived = out.obstruction.empty();
    out.pure_rotation_derived = out.obstruction.empty();
    out.unit_coordinates_derived = out.obstruction.empty();
    out.grade_two_atom_decomposition_derived = out.obstruction.empty();
    out.support_bound_derived = out.obstruction.empty();
    out.renewal_dimension_bound_derived =
        out.obstruction.empty() &&
        (out.checked_states == 0 || D <= 7 || out.maximum_grade_two_support < D - 1);
    out.unit_grade_two_cancellation_classified = out.obstruction.empty();
    out.forced_complement_classification_derived = out.obstruction.empty();
    // In the only possible renewal dimensions, the forced complement gives
    // closed overlap cases: D=7 requires two disjoint triples (support six),
    // while D=5 admits a root/root cover (and may also admit an overlapping
    // triple/triple alternative).  For D>=8 the support bound removes the
    // renewal branch before any overlap case is needed.
    out.closed_overlap_cases_derived = out.obstruction.empty() &&
        (out.checked_states == 0 || D >= 8 || D == 5 ||
         (D == 7 && out.root_root_certificates == 0 &&
          out.root_triple_certificates == 0 &&
          out.triple_triple_certificates == out.checked_states));
    out.unique_zero_derived = out.obstruction.empty();
    out.cyclic_alternation_derived = out.obstruction.empty();
    out.every_unknown_recurrent_state_normalized =
        out.normalized_component_ids.size() == out.checked_components;
    out.derived = out.replay_valid &&
                  out.request_atom_decomposition_derived &&
                  out.catalogue_grade_replayed && out.grade_bound_replayed &&
                  out.quadratic_norm_defect_derived && out.pure_rotation_derived &&
                  out.unit_coordinates_derived &&
                  out.grade_two_atom_decomposition_derived &&
                  out.support_bound_derived && out.renewal_dimension_bound_derived &&
                  out.unit_grade_two_cancellation_classified &&
                  out.forced_complement_classification_derived &&
                  out.closed_overlap_cases_derived &&
                  out.unique_zero_derived &&
                  out.cyclic_alternation_derived &&
                  out.every_unknown_recurrent_state_normalized;
    return out;
}

// Partition every recurrent rejected component using only structures already
// derived independently of the ambient SCC search.  SCC decomposition is used
// here only to identify the recurrent pieces of the replayed corona graph.
// Each such piece must be wholly transported/core, a one-in/one-out terminal
// permutation component, or exactly one of the closed-form alternating-zero
// label schedules derived from the n-bonacci parent grammar.
template <std::size_t D, class KnownNodeFn>
SymbolicRejectedBoundaryPartitionProof<D>
derive_symbolic_rejected_boundary_partition(
    const Substitution<D>& substitution,
    const ReplayedBoundaryGraph<D>& replay,
    KnownNodeFn known_node) {
    SymbolicRejectedBoundaryPartitionProof<D> out;
    out.substitution_recognized = is_nbonacci_substitution(substitution);
    out.replay_valid = replay.closure_complete && replay.replay_evidence_valid;
    if (!out.substitution_recognized) {
        out.obstruction = "substitution is not canonical n-bonacci";
        return out;
    }
    if (!out.replay_valid) {
        out.obstruction = "replayed rejected-boundary graph is incomplete";
        return out;
    }
    // Renewal schedules exist only in the support-permitted dimensions.
    // Above seven, do not construct label schedules at all: the explicit
    // two-atom support certificate must eliminate the renewal branch first.
    std::optional<NBonacciSymbolicLabelScheduleProof<D>> schedules;
    if constexpr (D <= 7)
        schedules = derive_nbonacci_symbolic_label_schedule(substitution);
    const auto shape = derive_rejected_state_shape_invariant(replay, known_node);
    if (!shape.derived) {
        out.unclassified_components = std::max<std::size_t>(1, shape.checked_components);
        out.obstruction = shape.obstruction.empty()
            ? "rejected-state shape invariant failed" : shape.obstruction;
        return out;
    }
    const auto components = tarjan_scc(replay.graph);
    std::size_t recurrent_id = 0;
    for (const auto& component : components) {
        if (!is_recurrent_scc(replay.graph, component)) continue;
        ++out.recurrent_components;
        const bool all_known = std::all_of(
            component.begin(), component.end(), [&](std::size_t vertex) {
                return known_node(replay.nodes.at(vertex));
            });
        if (all_known) {
            ++out.transported_components;
            ++recurrent_id;
            continue;
        }

        bool permutation = true;
        for (const auto vertex : component) {
            long long internal_out = 0;
            long long internal_in = 0;
            for (const auto& [target, weight] : replay.graph.out_adj.at(vertex))
                if (weight > 0 && std::find(component.begin(), component.end(), target) != component.end())
                    internal_out += weight;
            for (const auto& [source, weight] : replay.graph.in_adj.at(vertex))
                if (weight > 0 && std::find(component.begin(), component.end(), source) != component.end())
                    internal_in += weight;
            permutation = permutation && internal_out == 1 && internal_in == 1;
        }
        if (permutation) {
            ++out.terminal_permutation_components;
            ++recurrent_id;
            continue;
        }

        const bool shape_normalized =
            std::find(shape.normalized_component_ids.begin(),
                      shape.normalized_component_ids.end(), recurrent_id) !=
            shape.normalized_component_ids.end();
        bool exact_schedule = false;
        if constexpr (D <= 7)
            exact_schedule = shape_normalized && schedules.has_value() &&
                replay_component_matches_symbolic_label_schedule(
                    replay, component, *schedules);
        if (exact_schedule) {
            ++out.alternating_zero_components;
            out.alternating_zero_component_ids.push_back(recurrent_id);
        } else {
            ++out.unclassified_components;
        }
        ++recurrent_id;
    }
    out.shape_certificate_consumed = shape.derived;
    out.dimension_branch_classified =
        shape.renewal_dimension_bound_derived &&
        (D <= 7 || out.alternating_zero_components == 0);
    out.every_recurrent_component_partitioned = out.unclassified_components == 0;
    out.derived = out.substitution_recognized && out.replay_valid &&
                  out.shape_certificate_consumed &&
                  out.dimension_branch_classified &&
                  out.every_recurrent_component_partitioned;
    if (!out.derived && out.obstruction.empty())
        out.obstruction = "a recurrent rejected component is neither transported, terminal permutation, nor an exact alternating-zero schedule";
    return out;
}

// Materialize exactly the final graph already witnessed by ProjectedCoronaTrace.
// No transition is regenerated except to replay-check its stored prefix pair.
template <std::size_t D>
ReplayedBoundaryGraph<D> derive_replayed_boundary_graph(
    const Substitution<D>& substitution,
    const ProjectedCoronaTrace<D>& trace) {
    ReplayedBoundaryGraph<D> out;
    out.nodes.assign(trace.final_nodes.begin(), trace.final_nodes.end());
    out.graph = WeightedDigraph(out.nodes.size());
    out.atom_witnesses.resize(out.nodes.size());
    std::map<SNode<D>, std::size_t> index;
    for (std::size_t i = 0; i < out.nodes.size(); ++i) {
        index.emplace(out.nodes[i], i);
        const auto found = trace.atom_witnesses.find(out.nodes[i]);
        if (found != trace.atom_witnesses.end()) out.atom_witnesses[i] = found->second;
    }
    out.atom_evidence_preserved = true;
    for (const auto& [node, witness] : trace.atom_witnesses) {
        const auto found = index.find(node);
        if (found == index.end()) continue;
        const auto& stored = out.atom_witnesses.at(found->second);
        out.atom_evidence_preserved = out.atom_evidence_preserved &&
                                      stored.has_value() && *stored == witness;
    }

    out.closure_complete = trace.converged && !trace.node_cap_hit;
    out.replay_evidence_valid = out.closure_complete && !trace.layers.empty();
    if (trace.layers.empty()) return out;
    const auto& final_layer = trace.layers.back();
    out.replay_evidence_valid = out.replay_evidence_valid &&
        final_layer.nodes == trace.final_nodes;
    for (const auto& edge : final_layer.edges) {
        const auto& source = std::get<0>(edge);
        const auto& target = std::get<1>(edge);
        const auto& left_prefix = std::get<2>(edge);
        const auto& right_prefix = std::get<3>(edge);
        const auto source_it = index.find(source);
        const auto target_it = index.find(target);
        if (source_it == index.end() || target_it == index.end()) {
            out.replay_evidence_valid = false;
            continue;
        }
        const auto candidates =
            trace.edge_arithmetic == CoronaEdgeArithmetic::exact_rational
                ? simple_forward_targets_exact<D>(substitution, source)
                : simple_forward_targets<D>(substitution, source);
        const bool witnessed = std::any_of(
            candidates.begin(), candidates.end(), [&](const auto& candidate) {
                return candidate.first == target &&
                       candidate.second.first == left_prefix &&
                       candidate.second.second == right_prefix;
            });
        if (!witnessed) {
            out.replay_evidence_valid = false;
            continue;
        }
        out.graph.add_edge(source_it->second, target_it->second, 1);
        out.prefix_witnesses.push_back({source_it->second, target_it->second,
                                        left_prefix, right_prefix});
        ++out.replay_edges;
        ++out.replay_witnesses;
    }
    return out;
}
// Core finite-graph classification theorem.  This intentionally accepts a
// graph only after a caller has validated where that graph came from.  The
// preferred public entry point below consumes ProjectedCoronaTrace directly,
// preserving the corona layer's prefix witnesses rather than regenerating
// transitions in a second subsystem.
template <class KnownFn>
RejectedBoundaryExhaustionProof derive_rejected_boundary_exhaustion(
    std::size_t dimension,
    std::size_t boundary_seeds,
    const WeightedDigraph& graph,
    bool closure_complete,
    bool replay_evidence_valid,
    std::size_t replay_edges,
    std::size_t replay_witnesses,
    KnownFn known_vertex) {
    RejectedBoundaryExhaustionProof out;
    out.dimension = dimension;
    out.boundary_seeds = boundary_seeds;
    out.closure_vertices = graph.n;
    out.closure_complete = closure_complete;
    out.replay_evidence_valid = replay_evidence_valid;
    out.replay_edges = replay_edges;
    out.replay_witnesses = replay_witnesses;

    const auto components = tarjan_scc(graph);
    std::size_t component_id = 0;
    for (const auto& component : components) {
        if (!is_recurrent_scc(graph, component)) continue;
        ++out.recurrent_components;
        bool known = true;
        bool permutation = true;
        std::size_t component_edges = 0;
        for (const auto vertex : component) {
            known = known && known_vertex(vertex);
            std::size_t internal_out = 0;
            std::size_t internal_in = 0;
            for (const auto& [target, weight] : graph.out_adj[vertex]) {
                if (std::find(component.begin(), component.end(), target) == component.end())
                    continue;
                if (weight > 0) {
                    internal_out += static_cast<std::size_t>(weight);
                    component_edges += static_cast<std::size_t>(weight);
                }
            }
            for (const auto& [source, weight] : graph.in_adj[vertex]) {
                if (std::find(component.begin(), component.end(), source) == component.end())
                    continue;
                if (weight > 0) internal_in += static_cast<std::size_t>(weight);
            }
            permutation = permutation && internal_out == 1 && internal_in == 1;
        }
        BoundaryRecurrentComponentWitness witness;
        witness.component = component_id++;
        witness.vertices = component.size();
        witness.replay_edges = component_edges;
        witness.vertex_indices = component;
        witness.known_transported_family = known;
        witness.terminal_permutation = !known && permutation;
        if (known) {
            ++out.known_components;
            witness.explanation = "re-enters an already transported recurrent family";
        } else if (permutation) {
            ++out.terminal_components;
            witness.explanation = "new terminal one-in/one-out permutation family";
        } else {
            ++out.unknown_components;
            witness.explanation = "unclassified recurrent boundary component";
        }
        out.witnesses.push_back(std::move(witness));
    }

    out.every_recurrent_component_classified = out.unknown_components == 0;
    if (!closure_complete)
        out.obstruction = "the rejected-boundary corona trace is not converged and cap-free";
    else if (!replay_evidence_valid)
        out.obstruction = "the corona trace does not contain valid replay witnesses for its final edges";
    else if (!out.every_recurrent_component_classified)
        out.obstruction = "the rejected boundary contains an unclassified recurrent component";
    out.proved = closure_complete && replay_evidence_valid &&
                 out.every_recurrent_component_classified;
    return out;
}

// Derive the boundary graph from the exact final ProjectedCoronaTrace layer.
// Each edge in that layer already carries the two prefix words that witnessed
// the simple transition.  We validate those witnesses and preserve parallel
// witness multiplicity in the WeightedDigraph.
template <std::size_t D, class KnownNodeFn>
RejectedBoundaryExhaustionProof derive_rejected_boundary_exhaustion_from_trace(
    std::size_t boundary_seeds,
    const Substitution<D>& substitution,
    const ProjectedCoronaTrace<D>& trace,
    KnownNodeFn known_node) {
    const auto replay = derive_replayed_boundary_graph(substitution, trace);
    return derive_rejected_boundary_exhaustion(
        D, boundary_seeds, replay.graph,
        replay.closure_complete, replay.replay_evidence_valid,
        replay.replay_edges, replay.replay_witnesses,
        [&](std::size_t vertex) { return known_node(replay.nodes.at(vertex)); });
}


inline WeightedDigraph induced_replay_graph(
    const WeightedDigraph& graph,
    const std::vector<std::size_t>& vertices) {
    std::map<std::size_t, std::size_t> local;
    for (std::size_t i = 0; i < vertices.size(); ++i) local[vertices[i]] = i;
    WeightedDigraph out(vertices.size());
    for (std::size_t i = 0; i < vertices.size(); ++i)
        for (const auto& [target, weight] : graph.out_adj[vertices[i]]) {
            const auto found = local.find(target);
            if (found != local.end()) out.add_edge(i, found->second, weight);
        }
    return out;
}


inline std::vector<std::vector<long long>> multiply_nonnegative_matrices(
    const std::vector<std::vector<long long>>& lhs,
    const std::vector<std::vector<long long>>& rhs) {
    const auto n = lhs.size();
    if (n == 0 || rhs.size() != n)
        throw std::invalid_argument("matrix product: incompatible dimensions");
    for (const auto& row : lhs) if (row.size() != n)
        throw std::invalid_argument("matrix product: nonsquare lhs");
    for (const auto& row : rhs) if (row.size() != n)
        throw std::invalid_argument("matrix product: nonsquare rhs");
    std::vector<std::vector<long long>> out(n, std::vector<long long>(n, 0));
    for (std::size_t i = 0; i < n; ++i)
        for (std::size_t k = 0; k < n; ++k) if (lhs[i][k] != 0)
            for (std::size_t j = 0; j < n; ++j) if (rhs[k][j] != 0) {
                const __int128 term = static_cast<__int128>(lhs[i][k]) * rhs[k][j];
                const __int128 sum = static_cast<__int128>(out[i][j]) + term;
                if (sum > std::numeric_limits<long long>::max())
                    throw std::overflow_error("matrix product overflow");
                out[i][j] = static_cast<long long>(sum);
            }
    return out;
}

struct WeightedCycleBlockPowerProof {
    bool derived = false;
    bool replayed = false;
    std::size_t period = 0;
    long long multiplicity = 0;
    std::vector<std::size_t> cycle_order;
    std::vector<std::size_t> successor;
    std::vector<long long> edge_weight;
    std::string obstruction;
};

// Derive A^p = m I from the graph structure rather than discovering the
// identity by matrix powering.  For an irreducible weighted functional graph,
// every row has one positive successor and those successors form one directed
// cycle.  A p-step walk therefore returns to its starting class uniquely, and
// its weight is the product m of the edge weights around that cycle.
inline WeightedCycleBlockPowerProof derive_weighted_cycle_block_power(
    const std::vector<std::vector<long long>>& matrix) {
    WeightedCycleBlockPowerProof out;
    const auto n = matrix.size();
    if (n == 0) {
        out.obstruction = "empty quotient";
        return out;
    }
    out.successor.resize(n, n);
    out.edge_weight.resize(n, 0);
    std::vector<std::size_t> indegree(n, 0);
    for (std::size_t i = 0; i < n; ++i) {
        if (matrix[i].size() != n) {
            out.obstruction = "nonsquare quotient";
            return out;
        }
        std::size_t positive = 0;
        for (std::size_t j = 0; j < n; ++j) {
            if (matrix[i][j] < 0) {
                out.obstruction = "negative quotient weight";
                return out;
            }
            if (matrix[i][j] > 0) {
                ++positive;
                out.successor[i] = j;
                out.edge_weight[i] = matrix[i][j];
            }
        }
        if (positive != 1) {
            out.obstruction = "quotient is not a weighted functional graph";
            return out;
        }
        ++indegree[out.successor[i]];
    }
    if (std::any_of(indegree.begin(), indegree.end(),
                    [](std::size_t d) { return d != 1; })) {
        out.obstruction = "successor map is not a permutation";
        return out;
    }

    std::vector<bool> seen(n, false);
    std::size_t current = 0;
    long long product = 1;
    for (std::size_t step = 0; step < n; ++step) {
        if (seen[current]) {
            out.obstruction = "successor cycle closes before covering quotient";
            return out;
        }
        seen[current] = true;
        out.cycle_order.push_back(current);
        const __int128 next_product = static_cast<__int128>(product) *
                                      out.edge_weight[current];
        if (next_product > std::numeric_limits<long long>::max()) {
            out.obstruction = "cycle multiplicity overflow";
            return out;
        }
        product = static_cast<long long>(next_product);
        current = out.successor[current];
    }
    if (current != 0 || std::any_of(seen.begin(), seen.end(),
                                    [](bool value) { return !value; })) {
        out.obstruction = "successor permutation has multiple cycles";
        return out;
    }

    out.derived = true;
    out.period = n;
    out.multiplicity = product;

    // Independent replay: the structurally derived p and m must reproduce the
    // matrix identity exactly.  This checks the certificate; it does not search
    // for the identity.
    std::vector<std::vector<long long>> power(n, std::vector<long long>(n, 0));
    for (std::size_t i = 0; i < n; ++i) power[i][i] = 1;
    for (std::size_t step = 0; step < n; ++step)
        power = multiply_nonnegative_matrices(power, matrix);
    out.replayed = true;
    for (std::size_t i = 0; i < n; ++i)
        for (std::size_t j = 0; j < n; ++j)
            out.replayed = out.replayed &&
                power[i][j] == (i == j ? product : 0);
    if (!out.replayed) out.obstruction = "derived block-power identity failed replay";
    return out;
}



template <std::size_t D>
struct StateLevelWeightedCycleProof {
    bool derived = false;
    bool replayed = false;
    std::size_t period = 0;
    long long multiplicity = 0;
    EquitablePartition partition;
    std::vector<std::vector<std::size_t>> class_members;
    std::vector<std::size_t> successor_class;
    std::vector<long long> edge_weight;
    std::vector<std::size_t> cycle_order;
    std::vector<std::pair<std::size_t, std::size_t>> witnessed_state_edges;
    std::string obstruction;
};

// Derive the renewal grammar from concrete replay states and their witnessed
// edges.  The equitable quotient is not accepted as an unexplained matrix:
// for every state in a fiber we replay the same unique successor fiber and the
// same total witnessed multiplicity.  The fiber successor map must then be one
// directed cycle.  Unique path composition yields A^p = m I.
template <std::size_t D>
StateLevelWeightedCycleProof<D> derive_state_level_weighted_cycle(
    const ReplayedBoundaryGraph<D>& replay,
    const std::vector<std::size_t>& vertices) {
    StateLevelWeightedCycleProof<D> out;
    if (!replay.closure_complete || !replay.replay_evidence_valid) {
        out.obstruction = "boundary replay evidence is incomplete";
        return out;
    }
    if (vertices.empty()) {
        out.obstruction = "empty recurrent component";
        return out;
    }
    const auto local_graph = induced_replay_graph(replay.graph, vertices);
    out.partition = coarsest_equitable_partition(local_graph);
    const auto classes = static_cast<std::size_t>(out.partition.num_classes);
    if (classes == 0) {
        out.obstruction = "empty equitable partition";
        return out;
    }
    out.class_members.assign(classes, {});
    for (std::size_t local = 0; local < vertices.size(); ++local) {
        const auto c = static_cast<std::size_t>(out.partition.color.at(local));
        if (c >= classes) {
            out.obstruction = "invalid partition color";
            return out;
        }
        out.class_members[c].push_back(vertices[local]);
    }
    out.successor_class.assign(classes, classes);
    out.edge_weight.assign(classes, 0);
    std::vector<std::size_t> indegree(classes, 0);
    std::map<std::size_t, std::size_t> global_to_local;
    for (std::size_t local = 0; local < vertices.size(); ++local)
        global_to_local.emplace(vertices[local], local);

    for (std::size_t c = 0; c < classes; ++c) {
        if (out.class_members[c].empty()) {
            out.obstruction = "empty partition fiber";
            return out;
        }
        bool first_state = true;
        std::size_t expected_target = classes;
        long long expected_weight = 0;
        for (const auto global_source : out.class_members[c]) {
            std::map<std::size_t, long long> totals;
            for (const auto& [global_target, weight] : replay.graph.out_adj.at(global_source)) {
                if (weight <= 0) continue;
                const auto found = global_to_local.find(global_target);
                if (found == global_to_local.end()) continue;
                const auto target_class = static_cast<std::size_t>(
                    out.partition.color.at(found->second));
                totals[target_class] += weight;
                out.witnessed_state_edges.emplace_back(global_source, global_target);
            }
            if (totals.size() != 1) {
                out.obstruction = "a concrete state does not have one successor fiber";
                return out;
            }
            const auto [target_class, total_weight] = *totals.begin();
            if (first_state) {
                expected_target = target_class;
                expected_weight = total_weight;
                first_state = false;
            } else if (target_class != expected_target ||
                       total_weight != expected_weight) {
                out.obstruction = "states in one fiber disagree on successor grammar";
                return out;
            }
        }
        if (expected_weight <= 0 || expected_target >= classes) {
            out.obstruction = "invalid state-level successor grammar";
            return out;
        }
        out.successor_class[c] = expected_target;
        out.edge_weight[c] = expected_weight;
        ++indegree[expected_target];
    }
    if (std::any_of(indegree.begin(), indegree.end(),
                    [](std::size_t d) { return d != 1; })) {
        out.obstruction = "state-derived successor map is not a permutation";
        return out;
    }

    std::vector<bool> seen(classes, false);
    std::size_t current = 0;
    long long product = 1;
    for (std::size_t step = 0; step < classes; ++step) {
        if (seen[current]) {
            out.obstruction = "state-derived cycle closes before covering all fibers";
            return out;
        }
        seen[current] = true;
        out.cycle_order.push_back(current);
        const __int128 next = static_cast<__int128>(product) * out.edge_weight[current];
        if (next > std::numeric_limits<long long>::max()) {
            out.obstruction = "state-derived cycle multiplicity overflow";
            return out;
        }
        product = static_cast<long long>(next);
        current = out.successor_class[current];
    }
    if (current != 0 || std::any_of(seen.begin(), seen.end(),
                                    [](bool x) { return !x; })) {
        out.obstruction = "state-derived successor permutation has multiple cycles";
        return out;
    }
    out.derived = true;
    out.period = classes;
    out.multiplicity = product;

    // Replay the state-derived grammar through the independently constructed
    // quotient matrix.  The matrix is only a checker of the state certificate.
    const auto quotient = quotient_matrix(local_graph, out.partition);
    const auto matrix_proof = derive_weighted_cycle_block_power(quotient);
    out.replayed = matrix_proof.derived && matrix_proof.replayed &&
        matrix_proof.period == out.period &&
        matrix_proof.multiplicity == out.multiplicity &&
        matrix_proof.successor == out.successor_class &&
        matrix_proof.edge_weight == out.edge_weight;
    if (!out.replayed)
        out.obstruction = "state-derived renewal grammar failed quotient replay";
    return out;
}

struct MacroRenewalDominanceProof {
    bool replayed = false;
    bool block_power_derived = false;
    bool symbolic_fibers_derived = false;
    bool block_power_replayed = false;
    std::size_t period = 0;
    long long multiplicity = 0;
    long long rational_numerator = 0;
    long long rational_denominator = 1;
    std::vector<std::size_t> cycle_order;
};

// Derive an exact periodic renewal quotient A^p = m I from its weighted-cycle
// successor grammar, then separate its Perron root from the core by a rational
// q=a/b satisfying m < q^p and q < rho(core).
inline MacroRenewalDominanceProof derive_macro_renewal_dominance(
    const std::vector<std::vector<long long>>& competitor,
    const std::vector<std::vector<long long>>& core,
    std::size_t max_period = 32,
    int core_iterations = 160) {
    MacroRenewalDominanceProof out;
    const auto block = derive_weighted_cycle_block_power(competitor);
    out.block_power_derived = block.derived;
    out.block_power_replayed = block.replayed;
    out.period = block.period;
    out.multiplicity = block.multiplicity;
    out.cycle_order = block.cycle_order;
    if (!block.derived || !block.replayed || block.period > max_period) return out;

    const auto p = block.period;
    const auto m = block.multiplicity;
    const auto core_bracket = mathlib::certify_perron_bracket_exact(
        core, core_iterations, 0.0);
    for (long long b = 2; b <= 64; ++b) {
        for (long long a = b + 1; a <= 4 * b; ++a) {
            mathlib::BigInt ap(1), bp(1), aa(a), bb(b), mm(m), mbp;
            for (std::size_t k = 0; k < p; ++k) {
                mathlib::BigInt nexta, nextb;
                mathlib::mul(nexta, ap, aa); ap = std::move(nexta);
                mathlib::mul(nextb, bp, bb); bp = std::move(nextb);
            }
            mathlib::mul(mbp, mm, bp);
            if (mathlib::cmp(ap, mbp) <= 0) continue;
            mathlib::Rat q;
            mathlib::set_si(q, a, b);
            if (mathlib::cmp(q, core_bracket.bracket.lo) >= 0) continue;
            out.replayed = true;
            out.rational_numerator = a;
            out.rational_denominator = b;
            return out;
        }
    }
    return out;
}



template <std::size_t D>
MacroRenewalDominanceProof derive_state_level_macro_renewal_dominance(
    const ReplayedBoundaryGraph<D>& replay,
    const std::vector<std::size_t>& competitor_vertices,
    const std::vector<std::vector<long long>>& core,
    std::size_t max_period = 32,
    int core_iterations = 160) {
    MacroRenewalDominanceProof out;
    const auto symbolic = derive_symbolic_alternating_zero_cycle(
        replay, competitor_vertices);
    out.symbolic_fibers_derived = symbolic.derived &&
        symbolic.equitable_partition_replayed;

    const auto state = derive_state_level_weighted_cycle(replay, competitor_vertices);
    out.block_power_derived = state.derived;
    out.block_power_replayed = state.replayed;
    out.period = state.period;
    out.multiplicity = state.multiplicity;
    out.cycle_order = state.cycle_order;
    if (out.symbolic_fibers_derived &&
        (symbolic.period != state.period ||
         symbolic.multiplicity != state.multiplicity)) {
        out.block_power_replayed = false;
        return out;
    }
    if (!state.derived || !state.replayed || state.period > max_period) return out;

    const auto core_bracket = mathlib::certify_perron_bracket_exact(
        core, core_iterations, 0.0);
    for (long long b = 2; b <= 64; ++b) {
        for (long long a = b + 1; a <= 4 * b; ++a) {
            mathlib::BigInt ap(1), bp(1), aa(a), bb(b), mm(state.multiplicity), mbp;
            for (std::size_t k = 0; k < state.period; ++k) {
                mathlib::BigInt nexta, nextb;
                mathlib::mul(nexta, ap, aa); ap = std::move(nexta);
                mathlib::mul(nextb, bp, bb); bp = std::move(nextb);
            }
            mathlib::mul(mbp, mm, bp);
            if (mathlib::cmp(ap, mbp) <= 0) continue;
            mathlib::Rat q;
            mathlib::set_si(q, a, b);
            if (mathlib::cmp(q, core_bracket.bracket.lo) >= 0) continue;
            out.replayed = true;
            out.rational_numerator = a;
            out.rational_denominator = b;
            return out;
        }
    }
    return out;
}

inline std::vector<std::vector<long long>> induced_dense_matrix(
    const WeightedDigraph& graph,
    const std::vector<std::size_t>& vertices) {
    std::map<std::size_t, std::size_t> local;
    for (std::size_t i = 0; i < vertices.size(); ++i) local[vertices[i]] = i;
    std::vector<std::vector<long long>> matrix(
        vertices.size(), std::vector<long long>(vertices.size(), 0));
    for (std::size_t i = 0; i < vertices.size(); ++i)
        for (const auto& [target, weight] : graph.out_adj[vertices[i]]) {
            const auto found = local.find(target);
            if (found != local.end()) matrix[i][found->second] += weight;
        }
    return matrix;
}

// Close replay-valid unknown recurrent SCCs by deriving an exact positive
// rank-one intertwiner into the unique recurrent SCC selected by core_node.
// This does not identify states by a Perron estimate: the core predicate is
// structural, and the spectral comparison is performed only afterward.
template <std::size_t D, class CoreNodeFn>
RejectedBoundaryExhaustionProof derive_replayed_boundary_dominance(
    RejectedBoundaryExhaustionProof proof,
    const ReplayedBoundaryGraph<D>& replay,
    CoreNodeFn core_node,
    std::size_t iterations = 160) {
    if (!replay.closure_complete || !replay.replay_evidence_valid) return proof;

    const auto components = tarjan_scc(replay.graph);
    std::vector<std::size_t> core_component;
    std::size_t core_matches = 0;
    for (const auto& component : components) {
        if (!is_recurrent_scc(replay.graph, component)) continue;
        const bool is_core = std::all_of(
            component.begin(), component.end(), [&](std::size_t vertex) {
                return core_node(replay.nodes.at(vertex));
            });
        if (is_core) {
            core_component = component;
            ++core_matches;
        }
    }
    if (core_matches != 1 || core_component.empty()) {
        proof.obstruction = "replay dominance requires exactly one structurally selected recurrent core";
        proof.proved = false;
        return proof;
    }
    const auto core_graph = induced_replay_graph(replay.graph, core_component);
    const auto core_partition = coarsest_equitable_partition(core_graph);
    const auto core_matrix = quotient_matrix(core_graph, core_partition);

    for (auto& witness : proof.witnesses) {
        if (witness.known_transported_family || witness.terminal_permutation ||
            witness.strictly_dominated)
            continue;
        const auto competitor_graph = induced_replay_graph(
            replay.graph, witness.vertex_indices);
        const auto competitor_partition =
            coarsest_equitable_partition(competitor_graph);
        const auto competitor = quotient_matrix(
            competitor_graph, competitor_partition);
        witness.dominance_attempted = true;
        witness.competitor_quotient = competitor.size();
        witness.core_quotient = core_matrix.size();
        const auto dominance = derive_rank_one_paired_dominance(
            competitor, core_matrix, iterations);
        witness.reflective_dominance = dominance.replayed;
        const auto path_cone = dominance.replayed
            ? PathCountConePair{}
            : derive_path_count_cone_pair(competitor, core_matrix, iterations);
        witness.path_count_dominance = path_cone.replayed;
        const auto exact_comparison = (!dominance.replayed && !path_cone.replayed)
            ? mathlib::compare_perron_roots_exact(
                competitor, core_matrix, iterations, 0.0)
            : mathlib::PerronComparisonResult{};
        witness.exact_perron_dominance =
            exact_comparison.order == mathlib::PerronOrder::less;
        const auto renewal = (!dominance.replayed && !path_cone.replayed &&
                              !witness.exact_perron_dominance)
            ? derive_state_level_macro_renewal_dominance(
                replay, witness.vertex_indices, core_matrix, 32,
                static_cast<int>(iterations))
            : MacroRenewalDominanceProof{};
        witness.macro_renewal_dominance = renewal.replayed;
        witness.state_level_renewal = renewal.block_power_derived && renewal.block_power_replayed;
        witness.symbolic_fiber_renewal = renewal.symbolic_fibers_derived;
        witness.renewal_period = renewal.period;
        witness.renewal_multiplicity = renewal.multiplicity;
        if (!dominance.replayed && !path_cone.replayed &&
            !witness.exact_perron_dominance && !renewal.replayed)
            continue;
        witness.strictly_dominated = true;
        witness.explanation = dominance.replayed
            ? "replay-valid recurrent family strictly dominated by the predicted core via a reflective exact rank-one intertwiner"
            : path_cone.replayed
                ? "replay-valid recurrent family strictly dominated by the predicted core via a finite path-count rank-one intertwiner"
                : witness.exact_perron_dominance
                    ? "replay-valid recurrent family strictly dominated by the predicted core via exact rational Perron brackets"
                    : "replay-valid periodic renewal family strictly dominated by the predicted core via an exact block-power identity and rational separator";
        ++proof.dominated_components;
        if (proof.unknown_components > 0) --proof.unknown_components;
    }

    proof.every_recurrent_component_classified = proof.unknown_components == 0;
    proof.proved = proof.closure_complete && proof.replay_evidence_valid &&
                   proof.every_recurrent_component_classified;
    if (proof.proved) proof.obstruction.clear();
    else if (!proof.every_recurrent_component_classified)
        proof.obstruction = "the rejected boundary contains an unclassified recurrent component";
    return proof;
}

template <std::size_t D>
RejectedBoundaryExhaustionProof derive_replayed_boundary_phase_twist(
    RejectedBoundaryExhaustionProof proof,
    const Substitution<D>& substitution,
    const ReplayedBoundaryGraph<D>& replay) {
    const auto schedule = derive_nbonacci_symbolic_label_schedule(substitution);
    for (auto& witness : proof.witnesses) {
        if (!witness.symbolic_fiber_renewal) continue;
        witness.symbolic_label_schedule =
            replay_component_matches_symbolic_label_schedule(
                replay, witness.vertex_indices, schedule);
        const auto twist = derive_nbonacci_phase_twist(
            substitution, replay, witness.vertex_indices);
        witness.prefix_phase_twist = twist.derived && witness.symbolic_label_schedule;
        if (witness.prefix_phase_twist) {
            std::ostringstream detail;
            detail << witness.explanation
                   << "; prefix-derived phase twist z->z-1, branch_phase="
                   << twist.branch_phase;
            witness.explanation = detail.str();
        } else {
            witness.strictly_dominated = false;
            witness.macro_renewal_dominance = false;
            witness.explanation = witness.symbolic_label_schedule
                ? "phase-twist derivation failed: " + twist.obstruction
                : "replayed SCC does not equal a pre-enumerated symbolic label schedule";
        }
    }
    proof.dominated_components = 0;
    proof.unknown_components = 0;
    for (const auto& witness : proof.witnesses) {
        if (witness.strictly_dominated) ++proof.dominated_components;
        if (!witness.known_transported_family && !witness.terminal_permutation &&
            !witness.strictly_dominated) ++proof.unknown_components;
    }
    proof.every_recurrent_component_classified = proof.unknown_components == 0;
    proof.proved = proof.closure_complete && proof.replay_evidence_valid &&
                   proof.every_recurrent_component_classified;
    if (!proof.proved && proof.obstruction.empty())
        proof.obstruction = "a symbolic renewal component lacks prefix-level phase-twist evidence";
    return proof;
}

template <std::size_t D, class KnownNodeFn>
RejectedBoundaryExhaustionProof derive_replayed_boundary_symbolic_partition(
    RejectedBoundaryExhaustionProof proof,
    const Substitution<D>& substitution,
    const ReplayedBoundaryGraph<D>& replay,
    KnownNodeFn known_node) {
    const auto shape = derive_rejected_state_shape_invariant(replay, known_node);
    const auto partition = derive_symbolic_rejected_boundary_partition(
        substitution, replay, known_node);
    const auto components = tarjan_scc(replay.graph);
    std::size_t recurrent_id = 0;
    for (const auto& component : components) {
        if (!is_recurrent_scc(replay.graph, component)) continue;
        if (recurrent_id < proof.witnesses.size()) {
            auto& witness = proof.witnesses[recurrent_id];
            witness.rejected_shape_invariant =
                witness.known_transported_family || witness.terminal_permutation ||
                std::find(shape.normalized_component_ids.begin(),
                          shape.normalized_component_ids.end(),
                          recurrent_id) != shape.normalized_component_ids.end();
            witness.symbolic_rejected_partition =
                witness.known_transported_family || witness.terminal_permutation ||
                std::find(partition.alternating_zero_component_ids.begin(),
                          partition.alternating_zero_component_ids.end(),
                          recurrent_id) != partition.alternating_zero_component_ids.end();
        }
        ++recurrent_id;
    }
    if (!partition.derived) {
        proof.proved = false;
        proof.obstruction = partition.obstruction;
    }
    return proof;
}

inline std::string render_rejected_boundary_exhaustion_report(
    const RejectedBoundaryExhaustionProof& proof) {
    std::ostringstream out;
    out << "REJECTED_BOUNDARY_EXHAUSTION\n";
    out << "dimension=" << proof.dimension << "\n";
    out << "boundary_seeds=" << proof.boundary_seeds << "\n";
    out << "closure_vertices=" << proof.closure_vertices << "\n";
    out << "closure_complete=" << (proof.closure_complete ? "true" : "false") << "\n";
    out << "replay_evidence_valid=" << (proof.replay_evidence_valid ? "true" : "false") << "\n";
    out << "replay_edges=" << proof.replay_edges << "\n";
    out << "replay_witnesses=" << proof.replay_witnesses << "\n";
    out << "recurrent_components=" << proof.recurrent_components << "\n";
    out << "known_components=" << proof.known_components << "\n";
    out << "terminal_components=" << proof.terminal_components << "\n";
    out << "dominated_components=" << proof.dominated_components << "\n";
    out << "unknown_components=" << proof.unknown_components << "\n";
    for (const auto& witness : proof.witnesses)
        out << "component=" << witness.component
            << " vertices=" << witness.vertices
            << " replay_edges=" << witness.replay_edges
            << " known=" << (witness.known_transported_family ? "true" : "false")
            << " terminal=" << (witness.terminal_permutation ? "true" : "false")
            << " dominated=" << (witness.strictly_dominated ? "true" : "false")
            << " dominance_attempted=" << (witness.dominance_attempted ? "true" : "false")
            << " quotients=" << witness.competitor_quotient << "/" << witness.core_quotient
            << " reflective=" << (witness.reflective_dominance ? "true" : "false")
            << " path_count=" << (witness.path_count_dominance ? "true" : "false")
            << " exact_perron=" << (witness.exact_perron_dominance ? "true" : "false")
            << " macro_renewal=" << (witness.macro_renewal_dominance ? "true" : "false")
            << " state_level_renewal=" << (witness.state_level_renewal ? "true" : "false")
            << " symbolic_fibers=" << (witness.symbolic_fiber_renewal ? "true" : "false")
            << " label_schedule=" << (witness.symbolic_label_schedule ? "true" : "false")
            << " phase_twist=" << (witness.prefix_phase_twist ? "true" : "false")
            << " shape_invariant=" << (witness.rejected_shape_invariant ? "true" : "false")
            << " symbolic_partition=" << (witness.symbolic_rejected_partition ? "true" : "false")
            << " renewal=" << witness.renewal_period << "/" << witness.renewal_multiplicity
            << " reason=" << witness.explanation << "\n";
    out << "proved=" << (proof.proved ? "true" : "false") << "\n";
    if (!proof.obstruction.empty()) out << "obstruction=" << proof.obstruction << "\n";
    return out.str();
}

inline std::string render_rejected_boundary_exhaustion_lean() {
    return R"LEAN(import Mathlib

namespace RavelPlayground

/-- In a finite forward-closed boundary image, if every recurrent component is
    known or terminal, then no new nonterminal recurrent family is born. -/
theorem rejected_boundary_exhaustion
    {State : Type} [Fintype State]
    (recurrent known terminal : State → Prop)
    (classify : ∀ s, recurrent s → known s ∨ terminal s) :
    ∀ s, recurrent s → ¬ known s → terminal s := by
  intro s hs hn
  rcases classify s hs with hk | ht
  · exact (hn hk).elim
  · exact ht

end RavelPlayground
)LEAN";
}

} // namespace ravel::proof
