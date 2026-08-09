// ravel/proof/coupled_winning_predicate_certificate.hpp
//
// Wires lean/coupled_automaton_characterization.lean (previously flat: its
// only consumer, coupled_winning_predicate.hpp's evaluate_predicate_tree,
// merely evaluates a tree -- it never checked the actual win-condition
// obligations synthesized_winning_predicate_sound requires) to the
// reflection pipeline. certify_synthesized_winning_predicate independently
// re-evaluates the predicate tree at every concrete state's feature vector
// (not trusting any pre-labeled "Win" set) and exhaustively checks
// hinit/hstep/haccept over the full finite state/input space.

#pragma once

#include <cstddef>
#include <set>
#include <string>
#include <tuple>
#include <vector>

#include "math/proof_reflection.hpp"
#include "ravel/proof/coupled_winning_predicate.hpp"

namespace ravel::proof {

struct WinningPredicateCertificate {
    std::string certificate_id;
    std::vector<PredicateTreeNode> tree;
    std::vector<long long> state_feature;  // one Int per state (single-feature trees)
    std::vector<std::size_t> init_states;
    std::vector<std::size_t> accept_states;
    std::vector<std::tuple<std::size_t, int, std::size_t>> transitions;  // (from, input, to)
    std::vector<int> inputs;
    bool hinit = false;
    bool hstep = false;
    bool haccept = false;
    bool valid = false;
    std::string unsupported_reason;
};

inline WinningPredicateCertificate certify_synthesized_winning_predicate(
    std::string certificate_id,
    std::vector<PredicateTreeNode> tree,
    std::vector<long long> state_feature,
    std::vector<std::size_t> init_states,
    std::vector<std::size_t> accept_states,
    std::vector<std::tuple<std::size_t, int, std::size_t>> transitions,
    std::vector<int> inputs) {
    WinningPredicateCertificate cert;
    cert.certificate_id = std::move(certificate_id);
    cert.tree = std::move(tree);
    cert.state_feature = std::move(state_feature);
    cert.init_states = std::move(init_states);
    cert.accept_states = std::move(accept_states);
    cert.transitions = std::move(transitions);
    cert.inputs = std::move(inputs);

    if (cert.init_states.empty()) {
        cert.unsupported_reason = "no initial states supplied";
        return cert;
    }

    const auto win = [&](std::size_t s) -> bool {
        return evaluate_predicate_tree(cert.tree, {cert.state_feature.at(s)});
    };

    cert.hinit = true;
    for (const auto s : cert.init_states)
        if (!win(s)) cert.hinit = false;

    cert.hstep = true;
    for (std::size_t s = 0; s < cert.state_feature.size(); ++s) {
        if (!win(s)) continue;
        for (const auto a : cert.inputs) {
            bool found = false;
            for (const auto& [from, input, to] : cert.transitions)
                if (from == s && input == a && win(to)) { found = true; break; }
            if (!found) cert.hstep = false;
        }
    }

    const std::set<std::size_t> accept_set(cert.accept_states.begin(), cert.accept_states.end());
    cert.haccept = true;
    for (std::size_t s = 0; s < cert.state_feature.size(); ++s)
        if (win(s) && !accept_set.count(s)) cert.haccept = false;

    cert.valid = cert.hinit && cert.hstep && cert.haccept;
    if (!cert.valid && cert.unsupported_reason.empty()) {
        if (!cert.hinit) cert.unsupported_reason = "an initial state is not winning";
        else if (!cert.hstep) cert.unsupported_reason = "a winning state lacks a winning successor for some input";
        else cert.unsupported_reason = "a winning state is not accepting";
    }
    return cert;
}

inline void stage_synthesized_winning_predicate(const WinningPredicateCertificate& cert) {
    if (!cert.valid) return;
    if (!mathlib::reflection::enabled()) return;
    mathlib::reflection::WinningPredicateReflectionCertificate node;
    node.certificate_id = cert.certificate_id;
    node.state_count = static_cast<long long>(cert.state_feature.size());
    for (std::size_t s = 0; s < cert.state_feature.size(); ++s)
        node.win.push_back(evaluate_predicate_tree(cert.tree, {cert.state_feature[s]}));
    for (const auto s : cert.init_states) node.init_states.push_back(static_cast<long long>(s));
    for (const auto s : cert.accept_states) node.accept_states.push_back(static_cast<long long>(s));
    for (const auto& [from, input, to] : cert.transitions)
        node.transitions.push_back({static_cast<long long>(from), input, static_cast<long long>(to)});
    for (const auto a : cert.inputs) node.inputs.push_back(a);
    mathlib::reflection::record(mathlib::reflection::NodeKind::LemmaApplication, node);
}

}  // namespace ravel::proof
