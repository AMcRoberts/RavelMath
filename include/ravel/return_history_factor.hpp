// Probe whether a return-contact lift factors through return-word history
// after forgetting offsets.  A conflict is a history key whose phase-offset
// representatives expose different bare edge/position signatures.
#pragma once

#include <algorithm>
#include <cstddef>
#include <map>
#include <set>
#include <tuple>

#include "ravel/return_contact_lift.hpp"

namespace ravel {

struct ReturnHistoryFactorCertificate {
    std::size_t history_keys = 0;
    std::size_t conflicting_keys = 0;
    std::size_t offset_variant_keys = 0;
    std::size_t offset_sensitive_keys = 0;
    std::size_t maximum_offset_variants = 0;
    bool factors_through_history = false;
    bool conflicts_are_offset_driven = false;
};

template <std::size_t d>
ReturnHistoryFactorCertificate probe_return_history_factor(
        const ReturnContactLift<d>& lift, const ReturnPhaseSystem& phases) {
    using HistoryKey = std::tuple<std::size_t, std::size_t, std::size_t>;
    using EdgeSignature = std::tuple<std::size_t, std::size_t, std::size_t>;
    using OffsetPair = std::pair<std::size_t, std::size_t>;
    std::map<HistoryKey, std::set<EdgeSignature>> signatures;
    std::map<HistoryKey, std::map<OffsetPair, std::set<EdgeSignature>>>
        signatures_by_offset;
    for (const auto& edge : lift.edges) {
        const auto& source = lift.states.at(edge.source);
        const auto key = HistoryKey{
            edge.bare_source,
            phases.states.at(source.left_phase).return_word,
            phases.states.at(source.right_phase).return_word};
        const EdgeSignature signature{
            edge.bare_destination, edge.left_position, edge.right_position};
        const OffsetPair offset_pair{
            phases.states.at(source.left_phase).offset,
            phases.states.at(source.right_phase).offset};
        signatures[key].insert(signature);
        signatures_by_offset[key][offset_pair].insert(signature);
    }
    ReturnHistoryFactorCertificate result;
    result.history_keys = signatures.size();
    for (const auto& [key, values] : signatures) {
        const auto& by_offset = signatures_by_offset[key];
        const auto variant_count = by_offset.size();
        if (variant_count > 1) ++result.offset_variant_keys;
        result.maximum_offset_variants =
            std::max(result.maximum_offset_variants, variant_count);
        if (values.size() > 1) ++result.conflicting_keys;
        if (variant_count > 1) {
            const auto& reference = by_offset.begin()->second;
            for (const auto& [offset, offset_signatures] : by_offset) {
                if (offset_signatures != reference) {
                    ++result.offset_sensitive_keys;
                    break;
                }
            }
        }
    }
    result.factors_through_history = result.conflicting_keys == 0;
    result.conflicts_are_offset_driven = result.offset_sensitive_keys > 0;
    return result;
}

}  // namespace ravel
