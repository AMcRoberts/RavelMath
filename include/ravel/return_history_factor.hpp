// Probe whether a return-contact lift factors through return-word history
// after forgetting offsets.  A conflict is a history key whose phase-offset
// representatives expose different bare edge/position signatures.
#pragma once

#include <cstddef>
#include <map>
#include <set>
#include <tuple>

#include "ravel/return_contact_lift.hpp"

namespace ravel {

struct ReturnHistoryFactorCertificate {
    std::size_t history_keys = 0;
    std::size_t conflicting_keys = 0;
    bool factors_through_history = false;
};

template <std::size_t d>
ReturnHistoryFactorCertificate probe_return_history_factor(
        const ReturnContactLift<d>& lift, const ReturnPhaseSystem& phases) {
    using HistoryKey = std::tuple<std::size_t, std::size_t, std::size_t>;
    using EdgeSignature = std::tuple<std::size_t, std::size_t, std::size_t>;
    std::map<HistoryKey, std::set<EdgeSignature>> signatures;
    for (const auto& edge : lift.edges) {
        const auto& source = lift.states.at(edge.source);
        signatures[{edge.bare_source,
                    phases.states.at(source.left_phase).return_word,
                    phases.states.at(source.right_phase).return_word}]
            .insert({edge.bare_destination, edge.left_position,
                     edge.right_position});
    }
    ReturnHistoryFactorCertificate result;
    result.history_keys = signatures.size();
    for (const auto& [key, values] : signatures)
        if (values.size() > 1) ++result.conflicting_keys;
    result.factors_through_history = result.conflicting_keys == 0;
    return result;
}

}  // namespace ravel
