#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <map>
#include <set>
#include <stdexcept>
#include <tuple>
#include <utility>
#include <vector>

namespace ravel::proof {

using ControllerStateId = std::size_t;
using ResidualController = std::vector<bool>;

struct ControllerPlant {
    std::size_t state_count = 0;
    std::map<std::pair<ControllerStateId, std::int64_t>,
             std::vector<ControllerStateId>> successors;
};

inline ResidualController predecessor_controller(
    const ControllerPlant& plant,
    const ResidualController& continuation,
    std::int64_t input) {
    if (continuation.size() != plant.state_count)
        throw std::invalid_argument("continuation size does not match plant");

    ResidualController result(plant.state_count, false);
    for (ControllerStateId state = 0; state < plant.state_count; ++state) {
        const auto it = plant.successors.find({state, input});
        if (it == plant.successors.end()) continue;
        for (const auto next : it->second) {
            if (next >= plant.state_count)
                throw std::out_of_range("successor state");
            if (continuation[next]) {
                result[state] = true;
                break;
            }
        }
    }
    return result;
}

struct ContinuationControllerFamily {
    std::vector<ResidualController> members;
    std::map<ResidualController, std::size_t> member_ids;
    std::map<std::tuple<std::size_t, std::int64_t>, std::size_t>
        predecessor_table;
    std::set<std::size_t> terminal_generators;
    bool closed = false;
};

inline ContinuationControllerFamily close_controller_family(
    const ControllerPlant& plant,
    const std::vector<ResidualController>& generators,
    const std::vector<std::int64_t>& inputs) {
    ContinuationControllerFamily family;

    for (const auto& generator : generators) {
        if (generator.size() != plant.state_count)
            throw std::invalid_argument("generator size does not match plant");
        if (!family.member_ids.contains(generator)) {
            const auto id = family.members.size();
            family.member_ids.emplace(generator, id);
            family.members.push_back(generator);
            family.terminal_generators.insert(id);
        } else {
            family.terminal_generators.insert(
                family.member_ids.at(generator));
        }
    }

    for (std::size_t cursor = 0; cursor < family.members.size(); ++cursor) {
        for (const auto input : inputs) {
            const auto predecessor =
                predecessor_controller(plant, family.members[cursor], input);
            auto [it, inserted] =
                family.member_ids.emplace(predecessor, family.members.size());
            if (inserted)
                family.members.push_back(predecessor);
            family.predecessor_table.emplace(
                std::make_tuple(cursor, input), it->second);
        }
    }

    family.closed = true;
    for (std::size_t id = 0; id < family.members.size(); ++id) {
        for (const auto input : inputs) {
            const auto key = std::make_tuple(id, input);
            if (!family.predecessor_table.contains(key)) {
                family.closed = false;
                return family;
            }
            const auto expected =
                predecessor_controller(plant, family.members[id], input);
            const auto target = family.predecessor_table.at(key);
            if (target >= family.members.size() ||
                family.members[target] != expected) {
                family.closed = false;
                return family;
            }
        }
    }
    return family;
}

inline bool residual_accepts(
    const ResidualController& controller,
    ControllerStateId state) {
    if (state >= controller.size())
        throw std::out_of_range("controller state");
    return controller[state];
}

}  // namespace ravel::proof
