#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "ravel/proof/first_return_joint_product.hpp"
#include "ravel/proof/parametric_predicate_updates.hpp"

namespace ravel::proof {

/*
 * Exact symbolic transfer algebra for residual predecessor operations.
 *
 * Controller states are partitioned by dimension-free shift-register
 * observables.  The partition is refined only when a concrete residual is not
 * constant on a signature class.  Once constant, predecessor is computed on
 * signatures by the existential inverse-image relation and replayed against
 * every concrete residual bitset.
 */
struct ControllerSignature {
    std::int64_t first = 0;
    std::int64_t last = 0;
    std::size_t changes = 0;
    std::vector<std::size_t> counts;
    std::vector<std::size_t> period_defects;
    std::vector<std::size_t> factor_counts;
    std::vector<std::int64_t> prefix;
    std::vector<std::int64_t> suffix;

    friend bool operator<(const ControllerSignature& a,
                          const ControllerSignature& b) {
        return std::tie(a.first, a.last, a.changes, a.counts,
                        a.period_defects, a.factor_counts,
                        a.prefix, a.suffix) <
               std::tie(b.first, b.last, b.changes, b.counts,
                        b.period_defects, b.factor_counts,
                        b.prefix, b.suffix);
    }
};

struct ResidualSignatureTransferProof {
    std::size_t dimension = 0;
    std::size_t controller_states = 0;
    std::size_t residuals = 0;
    std::size_t signature_classes = 0;
    std::size_t refinement_rounds = 0;
    std::size_t window = 0;
    std::vector<std::size_t> signature_of_state;
    std::vector<std::vector<std::size_t>> states_in_signature;
    std::map<std::pair<std::size_t, std::int64_t>, std::set<std::size_t>>
        signature_successors;
    std::vector<std::vector<bool>> residual_on_signature;
    std::map<std::tuple<std::size_t, std::int64_t, std::size_t>, bool>
        symbolic_predecessor_membership;
    bool membership_constant = false;
    bool transfer_relation_exact = false;
    bool predecessor_replay_exact = false;
    bool local_update_replayed = false;
    bool valid = false;
    std::string obstruction;
};

namespace detail {

inline void enumerate_words(std::size_t length,
                            std::vector<SymbolicControllerState>& out) {
    std::size_t total = 1;
    for (std::size_t i = 0; i < length; ++i) total *= 3;
    out.reserve(out.size() + total);
    for (std::size_t code = 0; code < total; ++code) {
        auto q = code;
        SymbolicControllerState word(length);
        for (std::size_t i = 0; i < length; ++i) {
            word[length - 1 - i] = static_cast<std::int64_t>(q % 3) - 1;
            q /= 3;
        }
        out.push_back(std::move(word));
    }
}

inline ControllerSignature controller_signature(
    const SymbolicControllerState& state,
    std::size_t window) {
    if (state.empty()) throw std::invalid_argument("empty controller state");
    ControllerSignature sig;
    sig.first = state.front();
    sig.last = state.back();
    sig.changes = change_count(state);
    for (const auto a : {-1LL, 0LL, 1LL})
        sig.counts.push_back(symbol_count(state, a));
    for (std::size_t p = 1; p <= std::min(window, state.size()); ++p)
        sig.period_defects.push_back(period_defect_count(state, p));

    for (std::size_t length = 1; length <= std::min(window, state.size()); ++length) {
        std::vector<SymbolicControllerState> words;
        enumerate_words(length, words);
        for (const auto& word : words)
            sig.factor_counts.push_back(factor_count(state, word));
    }
    const auto keep = std::min(window, state.size());
    sig.prefix.assign(state.begin(), state.begin() + static_cast<std::ptrdiff_t>(keep));
    sig.suffix.assign(state.end() - static_cast<std::ptrdiff_t>(keep), state.end());
    return sig;
}

inline bool residual_constant_on_partition(
    const std::vector<ResidualController>& residuals,
    const std::vector<std::vector<std::size_t>>& classes) {
    for (const auto& residual : residuals) {
        for (const auto& cls : classes) {
            if (cls.empty()) continue;
            const bool value = residual.at(cls.front());
            for (const auto state : cls)
                if (residual.at(state) != value) return false;
        }
    }
    return true;
}

} // namespace detail

inline ResidualSignatureTransferProof derive_residual_signature_transfer(
    const FirstReturnJointProduct& product,
    std::size_t initial_window = 1) {
    ResidualSignatureTransferProof proof;
    proof.dimension = product.dimension;
    proof.controller_states = product.controller_states.size();
    proof.residuals = product.residual_members.size();
    if (!product.replayed) {
        proof.obstruction = "joint product did not replay";
        return proof;
    }
    if (product.controller_states.empty() || product.residual_members.empty()) {
        proof.obstruction = "joint product omitted controller algebra";
        return proof;
    }

    const auto max_window = product.dimension;
    for (std::size_t window = std::max<std::size_t>(1, initial_window);
         window <= max_window; ++window) {
        ++proof.refinement_rounds;
        std::map<ControllerSignature, std::size_t> ids;
        std::vector<std::size_t> class_of(product.controller_states.size());
        std::vector<std::vector<std::size_t>> classes;
        for (std::size_t s = 0; s < product.controller_states.size(); ++s) {
            const auto sig = detail::controller_signature(product.controller_states[s], window);
            auto [it, inserted] = ids.emplace(sig, ids.size());
            if (inserted) classes.emplace_back();
            class_of[s] = it->second;
            classes[it->second].push_back(s);
        }
        if (!detail::residual_constant_on_partition(product.residual_members, classes))
            continue;
        proof.window = window;
        proof.signature_of_state = std::move(class_of);
        proof.states_in_signature = std::move(classes);
        proof.membership_constant = true;
        break;
    }
    if (!proof.membership_constant) {
        proof.obstruction = "no bounded local signature made every residual constant";
        return proof;
    }
    proof.signature_classes = proof.states_in_signature.size();

    proof.local_update_replayed = true;
    for (std::size_t s = 0; s < product.controller_states.size(); ++s) {
        for (const auto tail : {-1LL, 0LL, 1LL}) {
            const auto local = derive_local_word_update_proof(
                product.controller_states[s], tail, proof.window, proof.window);
            proof.local_update_replayed &= local.valid;
        }
    }

    for (const auto& [key, targets] : product.controller_plant.successors) {
        const auto source_signature = proof.signature_of_state.at(key.first);
        auto& out = proof.signature_successors[{source_signature, key.second}];
        for (const auto target : targets)
            out.insert(proof.signature_of_state.at(target));
    }

    proof.transfer_relation_exact = true;
    for (std::size_t s = 0; s < product.controller_states.size(); ++s) {
        for (const auto digit : {-1LL, 0LL, 1LL}) {
            std::set<std::size_t> concrete;
            const auto it = product.controller_plant.successors.find({s, digit});
            if (it != product.controller_plant.successors.end())
                for (const auto t : it->second)
                    concrete.insert(proof.signature_of_state.at(t));
            const auto symbolic = proof.signature_successors[
                {proof.signature_of_state.at(s), digit}];
            if (concrete != symbolic) {
                proof.transfer_relation_exact = false;
                proof.obstruction = "signature successor relation is not source-class constant";
                return proof;
            }
        }
    }

    proof.residual_on_signature.assign(
        product.residual_members.size(),
        std::vector<bool>(proof.signature_classes, false));
    for (std::size_t r = 0; r < product.residual_members.size(); ++r) {
        for (std::size_t q = 0; q < proof.signature_classes; ++q)
            proof.residual_on_signature[r][q] =
                product.residual_members[r][proof.states_in_signature[q].front()];
    }

    proof.predecessor_replay_exact = true;
    for (std::size_t r = 0; r < product.residual_members.size(); ++r) {
        for (const auto digit : {-1LL, 0LL, 1LL}) {
            const auto target_residual = product.residual_predecessor.at({r, digit});
            for (std::size_t q = 0; q < proof.signature_classes; ++q) {
                bool symbolic = false;
                const auto sit = proof.signature_successors.find({q, digit});
                if (sit != proof.signature_successors.end()) {
                    for (const auto tq : sit->second)
                        symbolic |= proof.residual_on_signature[r][tq];
                }
                proof.symbolic_predecessor_membership[{r, digit, q}] = symbolic;
                const bool concrete = proof.residual_on_signature[target_residual][q];
                if (symbolic != concrete) {
                    proof.predecessor_replay_exact = false;
                    std::ostringstream o;
                    o << "symbolic predecessor mismatch residual=" << r
                      << " digit=" << digit << " signature=" << q;
                    proof.obstruction = o.str();
                    return proof;
                }
            }
        }
    }

    proof.valid = proof.membership_constant && proof.transfer_relation_exact &&
                  proof.predecessor_replay_exact && proof.local_update_replayed;
    return proof;
}

inline std::string render_residual_signature_transfer_report(
    const ResidualSignatureTransferProof& p) {
    std::ostringstream o;
    o << "RESIDUAL_SIGNATURE_TRANSFER\n";
    o << "dimension=" << p.dimension << "\n";
    o << "controller_states=" << p.controller_states << "\n";
    o << "residuals=" << p.residuals << "\n";
    o << "signature_classes=" << p.signature_classes << "\n";
    o << "window=" << p.window << "\n";
    o << "refinement_rounds=" << p.refinement_rounds << "\n";
    o << "membership_constant=" << (p.membership_constant ? "true" : "false") << "\n";
    o << "transfer_relation_exact=" << (p.transfer_relation_exact ? "true" : "false") << "\n";
    o << "predecessor_replay_exact=" << (p.predecessor_replay_exact ? "true" : "false") << "\n";
    o << "local_update_replayed=" << (p.local_update_replayed ? "true" : "false") << "\n";
    o << "valid=" << (p.valid ? "true" : "false") << "\n";
    if (!p.obstruction.empty()) o << "obstruction=" << p.obstruction << "\n";
    return o.str();
}

} // namespace ravel::proof
