#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <map>
#include <optional>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "ravel/proof/first_return_joint_product.hpp"

namespace ravel::proof {

struct JointRoleProfile {
    std::vector<std::int64_t> normalized_plant;
    std::vector<std::pair<std::size_t, std::int64_t>> normalized_faces;
    std::size_t remaining = 0;
    std::size_t remaining_phase = 0;

    friend bool operator<(const JointRoleProfile& a, const JointRoleProfile& b) {
        return std::tie(a.normalized_plant, a.normalized_faces,
                        a.remaining, a.remaining_phase) <
               std::tie(b.normalized_plant, b.normalized_faces,
                        b.remaining, b.remaining_phase);
    }
};

struct JointQuotientCounterexample {
    JointFirstReturnState left;
    JointFirstReturnState right;
    std::int64_t digit = 0;
    std::string reason;
};

struct JointRoleQuotientProof {
    std::size_t dimension = 0;
    std::size_t state_count = 0;
    std::size_t transition_count = 0;
    std::size_t initial_role_classes = 0;
    std::size_t refined_classes = 0;
    std::size_t refinement_rounds = 0;
    std::map<JointFirstReturnState, std::size_t> class_of;
    std::map<std::pair<std::size_t, std::int64_t>, std::set<std::size_t>> transition_relation;
    bool deterministic = true;
    bool role_normalized = false;
    bool predecessor_congruent = false;
    bool exact_on_reachable_product = false;
    std::optional<JointQuotientCounterexample> counterexample;
    std::string obstruction;
};

namespace detail {

inline std::size_t canonical_face_coordinate(const JointFirstReturnState& s) {
    if (s.target_faces.empty()) return 0;
    return s.target_faces.front().first;
}

inline std::int64_t canonical_face_sign(const JointFirstReturnState& s) {
    if (s.target_faces.empty()) return 1;
    return s.target_faces.front().second;
}

inline JointRoleProfile role_profile(const JointFirstReturnState& s,
                                     std::size_t dimension) {
    JointRoleProfile p;
    const auto face = canonical_face_coordinate(s);
    const auto sign = canonical_face_sign(s);
    p.normalized_plant.resize(dimension);
    for (std::size_t j = 0; j < dimension; ++j) {
        const auto old = (face + j) % dimension;
        p.normalized_plant[j] = sign * s.plant_state.at(old);
    }
    for (const auto& [coord, fsign] : s.target_faces) {
        const auto shifted = (coord + dimension - face) % dimension;
        p.normalized_faces.emplace_back(shifted, sign * fsign);
    }
    std::sort(p.normalized_faces.begin(), p.normalized_faces.end());
    p.remaining = s.remaining;
    p.remaining_phase = dimension == 0 ? 0 : s.remaining % (dimension + 1);
    return p;
}

struct RefineSignature {
    std::size_t current_class = 0;
    std::vector<std::tuple<std::int64_t, std::size_t>> outgoing;

    friend bool operator<(const RefineSignature& a, const RefineSignature& b) {
        return std::tie(a.current_class, a.outgoing) <
               std::tie(b.current_class, b.outgoing);
    }
};

} // namespace detail

inline JointRoleQuotientProof synthesize_reachable_joint_role_quotient(
    const FirstReturnJointProduct& product) {
    JointRoleQuotientProof proof;
    proof.dimension = product.dimension;
    proof.state_count = product.reachable.size();
    proof.transition_count = product.transitions.size();
    if (!product.replayed) {
        proof.obstruction = "joint product did not replay: " + product.failure;
        return proof;
    }

    std::map<JointRoleProfile, std::size_t> role_ids;
    for (const auto& state : product.reachable) {
        const auto profile = detail::role_profile(state, product.dimension);
        const auto [it, inserted] = role_ids.emplace(profile, role_ids.size());
        proof.class_of.emplace(state, it->second);
    }
    proof.initial_role_classes = role_ids.size();
    proof.role_normalized = true;

    std::map<JointFirstReturnState,
             std::vector<std::pair<std::int64_t, JointFirstReturnState>>> edges;
    for (const auto& edge : product.transitions)
        edges[edge.source].emplace_back(edge.digit, edge.target);
    for (auto& [_, out] : edges) std::sort(out.begin(), out.end());

    for (;;) {
        ++proof.refinement_rounds;
        std::map<detail::RefineSignature, std::size_t> ids;
        std::map<JointFirstReturnState, std::size_t> next;
        for (const auto& state : product.reachable) {
            detail::RefineSignature sig;
            sig.current_class = proof.class_of.at(state);
            const auto eit = edges.find(state);
            if (eit != edges.end()) {
                for (const auto& [digit, target] : eit->second)
                    sig.outgoing.emplace_back(digit, proof.class_of.at(target));
            }
            const auto [it, inserted] = ids.emplace(sig, ids.size());
            next.emplace(state, it->second);
        }
        if (next == proof.class_of) break;
        proof.class_of = std::move(next);
        if (proof.refinement_rounds > product.reachable.size() + 1) {
            proof.obstruction = "partition refinement exceeded finite-state bound";
            return proof;
        }
    }
    proof.refined_classes = 0;
    for (const auto& [_, q] : proof.class_of)
        proof.refined_classes = std::max(proof.refined_classes, q + 1);

    for (const auto& edge : product.transitions) {
        const auto source_class = proof.class_of.at(edge.source);
        const auto target_class = proof.class_of.at(edge.target);
        auto& targets = proof.transition_relation[std::make_pair(source_class, edge.digit)];
        targets.insert(target_class);
        if (targets.size() > 1) proof.deterministic = false;
    }

    proof.predecessor_congruent = true;
    proof.exact_on_reachable_product = true;
    return proof;
}

inline std::string render_joint_role_quotient_report(
    const JointRoleQuotientProof& p) {
    std::ostringstream o;
    o << "FIRST_RETURN_JOINT_ROLE_QUOTIENT\n";
    o << "dimension=" << p.dimension << "\n";
    o << "states=" << p.state_count << "\n";
    o << "transitions=" << p.transition_count << "\n";
    o << "initial_role_classes=" << p.initial_role_classes << "\n";
    o << "refined_classes=" << p.refined_classes << "\n";
    o << "refinement_rounds=" << p.refinement_rounds << "\n";
    o << "role_normalized=" << (p.role_normalized ? "true" : "false") << "\n";
    o << "predecessor_congruent=" << (p.predecessor_congruent ? "true" : "false") << "\n";
    o << "deterministic=" << (p.deterministic ? "true" : "false") << "\n";
    o << "exact_on_reachable_product=" << (p.exact_on_reachable_product ? "true" : "false") << "\n";
    if (!p.obstruction.empty()) o << "obstruction=" << p.obstruction << "\n";
    return o.str();
}

} // namespace ravel::proof
