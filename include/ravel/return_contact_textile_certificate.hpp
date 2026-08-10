// Local matching/biregularity certificate for a return/contact quotient.
//
// Unlike a permutation-cover certificate, this keeps each quotient-channel
// relation rectangular and reports whether its concrete edges are regular.
// This is the finite textile/branched-extension seam needed when fibre sizes
// vary across quotient states.
#pragma once

#include <algorithm>
#include <cstddef>
#include <map>
#include <tuple>
#include <vector>

#include "ravel/return_offset_fibre_certificate.hpp"

namespace ravel {

struct ReturnContactTextilePair {
    std::size_t source_class = 0;
    std::size_t target_class = 0;
    std::size_t source_size = 0;
    std::size_t target_size = 0;
    std::size_t edges = 0;
    std::size_t source_degree = 0;
    std::size_t target_degree = 0;
    bool biregular = false;
    bool permutation_channel = false;
};

struct ReturnContactTextileCertificate {
    std::size_t quotient_classes = 0;
    std::size_t concrete_states = 0;
    std::size_t concrete_edges = 0;
    std::size_t nonempty_pairs = 0;
    std::size_t biregular_pairs = 0;
    std::size_t permutation_pairs = 0;
    std::size_t irregular_pairs = 0;
    std::vector<ReturnContactTextilePair> pairs;
    bool finite = false;
    bool all_pairs_biregular = false;
    bool has_branching_channels = false;
};

template <std::size_t d>
ReturnContactTextileCertificate derive_return_contact_textile_certificate(
        const ReturnContactLift<d>& lift, const ReturnPhaseSystem& phases) {
    using Key = std::tuple<std::size_t, std::size_t, std::size_t,
                           std::size_t, long long, long long>;
    std::map<Key, std::size_t> class_ids;
    std::vector<std::size_t> state_class(lift.states.size(), 0);
    for (std::size_t state_id = 0; state_id < lift.states.size(); ++state_id) {
        const auto& state = lift.states[state_id];
        const auto& left = phases.states.at(state.left_phase);
        const auto& right = phases.states.at(state.right_phase);
        const Key key{left.return_word, left.offset, right.return_word,
                      right.offset, state.contact.i, state.contact.j};
        auto [it, inserted] = class_ids.emplace(key, class_ids.size());
        (void)inserted;
        state_class[state_id] = it->second;
    }
    std::vector<std::vector<std::size_t>> members(class_ids.size());
    for (std::size_t state = 0; state < state_class.size(); ++state)
        members[state_class[state]].push_back(state);

    using Pair = std::pair<std::size_t, std::size_t>;
    std::map<Pair, std::vector<std::pair<std::size_t, std::size_t>>> edges;
    for (const auto& edge : lift.edges)
        edges[{state_class.at(edge.source), state_class.at(edge.destination)}]
            .push_back({edge.source, edge.destination});

    ReturnContactTextileCertificate out;
    out.quotient_classes = members.size();
    out.concrete_states = lift.states.size();
    out.concrete_edges = lift.edges.size();
    out.nonempty_pairs = edges.size();
    for (const auto& [pair, concrete] : edges) {
        const auto& source_members = members[pair.first];
        const auto& target_members = members[pair.second];
        std::vector<std::size_t> source_degree(source_members.size(), 0);
        std::vector<std::size_t> target_degree(target_members.size(), 0);
        std::map<std::size_t, std::size_t> source_local, target_local;
        for (std::size_t i = 0; i < source_members.size(); ++i)
            source_local[source_members[i]] = i;
        for (std::size_t i = 0; i < target_members.size(); ++i)
            target_local[target_members[i]] = i;
        for (const auto [source, target] : concrete) {
            ++source_degree.at(source_local.at(source));
            ++target_degree.at(target_local.at(target));
        }
        const bool source_regular = !source_degree.empty() &&
            std::all_of(source_degree.begin(), source_degree.end(),
                        [&](std::size_t x) { return x == source_degree.front(); });
        const bool target_regular = !target_degree.empty() &&
            std::all_of(target_degree.begin(), target_degree.end(),
                        [&](std::size_t x) { return x == target_degree.front(); });
        const bool biregular = source_regular && target_regular;
        const bool permutation = biregular && source_members.size() ==
            target_members.size() && source_degree.front() == 1;
        out.pairs.push_back({pair.first, pair.second, source_members.size(),
                             target_members.size(), concrete.size(),
                             source_regular ? source_degree.front() : 0,
                             target_regular ? target_degree.front() : 0,
                             biregular, permutation});
        if (biregular) ++out.biregular_pairs;
        else ++out.irregular_pairs;
        if (permutation) ++out.permutation_pairs;
        if (biregular && source_degree.front() > 1)
            out.has_branching_channels = true;
    }
    out.finite = true;
    out.all_pairs_biregular = out.biregular_pairs == out.nonempty_pairs;
    return out;
}

}  // namespace ravel
