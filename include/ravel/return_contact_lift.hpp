// return_contact_lift.hpp
//
// Sparse reachable coupling of the type-1 contact graph with two copies
// of a return-phase tower.  A contact edge
//
//   [i,x,j] --(p,q)--> [i',x',j']
//
// selects the occurrence at positions |p| and |q| in sigma(i') and
// sigma(j').  Consequently a parent phase a' is compatible with a child
// phase a precisely when phase_images[a'][|p|] == a.  The construction
// below explores only phase/contact states reachable from caller-supplied
// seeds; it never forms |G_B| times |P|^2 eagerly.

#pragma once

#include <cstddef>
#include <cstdint>
#include <deque>
#include <limits>
#include <map>
#include <set>
#include <stdexcept>
#include <utility>
#include <vector>

#include "ravel/corona.hpp"
#include "ravel/return_substitution.hpp"

namespace ravel {

inline std::int8_t return_phase_letter(
        const ReturnPhaseSystem& phases, std::size_t state) {
    if (state >= phases.states.size()) {
        throw std::out_of_range("return_phase_letter: state out of range");
    }
    const auto& phase = phases.states[state];
    return phases.induced.words.at(phase.return_word).at(phase.offset);
}

template <std::size_t d>
struct ReturnContactState {
    SNode<d> contact{};
    std::size_t left_phase = 0;
    std::size_t right_phase = 0;

    bool operator==(const ReturnContactState& other) const {
        return contact == other.contact
            && left_phase == other.left_phase
            && right_phase == other.right_phase;
    }

    bool operator<(const ReturnContactState& other) const {
        if (contact < other.contact) return true;
        if (other.contact < contact) return false;
        if (left_phase != other.left_phase) {
            return left_phase < other.left_phase;
        }
        return right_phase < other.right_phase;
    }
};

struct ReturnContactLimits {
    std::size_t max_states = 1 << 20;
    std::size_t max_edges = 1 << 22;
    // Logical storage of queued state indices.  This bound is portable;
    // deque implementation overhead is deliberately not guessed.
    std::size_t max_queue_bytes = 64 << 20;
};

enum class ReturnContactStop {
    complete,
    state_cap,
    edge_cap,
    queue_cap
};

struct ReturnContactEdge {
    std::size_t source = 0;
    std::size_t destination = 0;
    std::size_t bare_source = 0;
    std::size_t bare_destination = 0;
    std::size_t left_position = 0;
    std::size_t right_position = 0;
};

template <std::size_t d>
struct ReturnContactLift {
    std::vector<ReturnContactState<d>> states;
    std::vector<ReturnContactEdge> edges;
    ReturnContactStop stop = ReturnContactStop::complete;
    std::size_t peak_queue_bytes = 0;
    std::size_t projected_node_count = 0;
    std::size_t projected_edge_count = 0;
    // Number of lifted edges above each bare ordered edge.
    std::map<std::pair<std::size_t, std::size_t>, std::size_t>
        projected_edge_multiplicity;

    bool complete() const {
        return stop == ReturnContactStop::complete;
    }
};

template <std::size_t d>
std::vector<ReturnContactState<d>> compatible_return_contact_seeds(
        const SNode<d>& contact, const ReturnPhaseSystem& phases) {
    std::vector<std::size_t> left;
    std::vector<std::size_t> right;
    for (std::size_t phase = 0; phase < phases.states.size(); ++phase) {
        const auto letter = return_phase_letter(phases, phase);
        if (letter == contact.i) left.push_back(phase);
        if (letter == contact.j) right.push_back(phase);
    }
    std::vector<ReturnContactState<d>> result;
    if (!left.empty()
        && right.size() > std::numeric_limits<std::size_t>::max()
                               / left.size()) {
        throw std::overflow_error(
            "compatible_return_contact_seeds: size overflow");
    }
    result.reserve(left.size() * right.size());
    for (std::size_t a : left) {
        for (std::size_t b : right) {
            result.push_back({contact, a, b});
        }
    }
    return result;
}

namespace detail {

struct ReturnPhaseParentKey {
    std::int8_t letter = 0;
    std::size_t position = 0;
    std::size_t child_phase = 0;

    bool operator<(const ReturnPhaseParentKey& other) const {
        if (letter != other.letter) return letter < other.letter;
        if (position != other.position) return position < other.position;
        return child_phase < other.child_phase;
    }
};

inline std::map<ReturnPhaseParentKey, std::vector<std::size_t>>
return_phase_parent_index(const ReturnPhaseSystem& phases) {
    std::map<ReturnPhaseParentKey, std::vector<std::size_t>> index;
    for (std::size_t parent = 0; parent < phases.states.size(); ++parent) {
        const auto letter = return_phase_letter(phases, parent);
        for (std::size_t position = 0;
             position < phases.phase_images.at(parent).size(); ++position) {
            index[{letter, position, phases.phase_images[parent][position]}]
                .push_back(parent);
        }
    }
    return index;
}

}  // namespace detail

template <std::size_t d>
ReturnContactLift<d> build_reachable_return_contact_lift(
        const Substitution<d>& substitution,
        const std::vector<SNode<d>>& bare_nodes,
        const ReturnPhaseSystem& phases,
        const std::vector<ReturnContactState<d>>& seeds,
        const ReturnContactLimits& limits = {}) {
    if (limits.max_states == 0 || limits.max_edges == 0
        || limits.max_queue_bytes < sizeof(std::size_t)) {
        throw std::invalid_argument(
            "build_reachable_return_contact_lift: unusable limits");
    }

    ReturnContactLift<d> result;
    std::map<SNode<d>, std::size_t> bare_index;
    for (std::size_t i = 0; i < bare_nodes.size(); ++i) {
        if (!bare_index.emplace(bare_nodes[i], i).second) {
            throw std::invalid_argument(
                "build_reachable_return_contact_lift: duplicate bare node");
        }
    }

    using LabelledTarget = std::pair<
        std::size_t,
        std::pair<std::vector<long long>, std::vector<long long>>>;
    std::vector<std::vector<LabelledTarget>> outgoing(bare_nodes.size());
    for (std::size_t source = 0; source < bare_nodes.size(); ++source) {
        for (const auto& [destination, labels] :
             simple_forward_targets<d>(substitution, bare_nodes[source])) {
            const auto found = bare_index.find(destination);
            if (found != bare_index.end()) {
                outgoing[source].push_back({found->second, labels});
            }
        }
    }

    const auto parents = detail::return_phase_parent_index(phases);
    std::map<ReturnContactState<d>, std::size_t> state_index;
    std::deque<std::size_t> queue;
    const std::size_t max_queued =
        limits.max_queue_bytes / sizeof(std::size_t);

    auto enqueue = [&](const ReturnContactState<d>& state)
            -> std::pair<std::size_t, bool> {
        const auto existing = state_index.find(state);
        if (existing != state_index.end()) {
            return {existing->second, false};
        }
        if (result.states.size() >= limits.max_states) {
            result.stop = ReturnContactStop::state_cap;
            return {0, false};
        }
        if (queue.size() >= max_queued) {
            result.stop = ReturnContactStop::queue_cap;
            return {0, false};
        }
        const std::size_t index = result.states.size();
        result.states.push_back(state);
        state_index.emplace(state, index);
        queue.push_back(index);
        result.peak_queue_bytes =
            std::max(result.peak_queue_bytes,
                     queue.size() * sizeof(std::size_t));
        return {index, true};
    };

    for (const auto& seed : seeds) {
        const auto bare = bare_index.find(seed.contact);
        if (bare == bare_index.end()) {
            throw std::invalid_argument(
                "build_reachable_return_contact_lift: seed not in bare graph");
        }
        if (return_phase_letter(phases, seed.left_phase) != seed.contact.i
            || return_phase_letter(phases, seed.right_phase)
                   != seed.contact.j) {
            throw std::invalid_argument(
                "build_reachable_return_contact_lift: incompatible seed");
        }
        enqueue(seed);
        if (!result.complete()) return result;
    }

    while (!queue.empty() && result.complete()) {
        const std::size_t source_state = queue.front();
        queue.pop_front();
        const auto state = result.states[source_state];
        const std::size_t bare_source =
            bare_index.find(state.contact)->second;
        for (const auto& [bare_destination, labels] :
             outgoing[bare_source]) {
            const auto& destination = bare_nodes[bare_destination];
            const auto left_it = parents.find({
                static_cast<std::int8_t>(destination.i),
                labels.first.size(), state.left_phase});
            const auto right_it = parents.find({
                static_cast<std::int8_t>(destination.j),
                labels.second.size(), state.right_phase});
            if (left_it == parents.end() || right_it == parents.end()) {
                continue;
            }
            for (std::size_t left_parent : left_it->second) {
                for (std::size_t right_parent : right_it->second) {
                    if (result.edges.size() >= limits.max_edges) {
                        result.stop = ReturnContactStop::edge_cap;
                        break;
                    }
                    ReturnContactState<d> next{
                        destination, left_parent, right_parent};
                    const auto [destination_state, inserted] = enqueue(next);
                    (void)inserted;
                    if (!result.complete()) break;
                    result.edges.push_back({
                        source_state, destination_state,
                        bare_source, bare_destination,
                        labels.first.size(), labels.second.size()});
                    ++result.projected_edge_multiplicity[
                        {bare_source, bare_destination}];
                }
                if (!result.complete()) break;
            }
            if (!result.complete()) break;
        }
    }

    std::set<std::size_t> projected_nodes;
    for (const auto& state : result.states) {
        projected_nodes.insert(bare_index.find(state.contact)->second);
    }
    result.projected_node_count = projected_nodes.size();
    result.projected_edge_count =
        result.projected_edge_multiplicity.size();
    return result;
}

}  // namespace ravel
