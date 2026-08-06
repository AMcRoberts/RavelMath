#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "ravel/proof/first_return_joint_product.hpp"

namespace ravel::proof {

struct SteppedFaceResidualSerialityCertificate {
    std::size_t dimension = 0;
    std::size_t controller_states = 0;
    std::size_t word_length = 0;
    std::vector<std::size_t> residual_chain;
    std::vector<std::size_t> fiber_sizes;
    bool terminal_is_face_monitor = false;
    bool predecessor_chain_replayed = false;
    bool every_step_serial = false;
    bool whole_segment_serial = false;
    bool valid = false;
    std::string obstruction;
};

/**
 * Derive the exact varying-fiber seriality theorem for one first-return word.
 *
 * If R_k is the residual accepting controller states that can complete the
 * suffix beginning at digit k, then
 *
 *   R_k = Pre_{d_k}(R_{k+1}).
 *
 * Hence every state in R_k has at least one d_k-successor in R_{k+1}.  This
 * is the stepped-face controller theorem at the correct abstraction: the
 * terminal fiber is a face monitor, and the intermediate fibers are its exact
 * linear/predecessor pullbacks.  No global fixed fiber is assumed.
 */
inline SteppedFaceResidualSerialityCertificate
 derive_stepped_face_residual_seriality(
    const FirstReturnJointProduct& product,
    const FirstReturnWitness& witness) {
    SteppedFaceResidualSerialityCertificate cert;
    cert.dimension = product.dimension;
    cert.controller_states = product.controller_states.size();
    cert.word_length = witness.digits.size();
    if (!product.replayed) {
        cert.obstruction = "joint product did not replay";
        return cert;
    }
    if (witness.dimension != product.dimension || witness.base_path.empty()) {
        cert.obstruction = "witness dimension/path mismatch";
        return cert;
    }

    const auto faces = detail::target_faces(witness.target, witness.bound);
    ResidualController terminal(product.controller_states.size(), false);
    for (std::size_t s = 0; s < product.controller_states.size(); ++s) {
        for (const auto& [coord, sign] : faces) {
            if (coord < product.controller_states[s].size() &&
                product.controller_states[s][coord] == sign) {
                terminal[s] = true;
                break;
            }
        }
    }
    const auto terminal_it = std::find(
        product.residual_members.begin(), product.residual_members.end(), terminal);
    if (terminal_it == product.residual_members.end()) {
        cert.obstruction = "terminal stepped-face monitor absent from residual family";
        return cert;
    }
    cert.terminal_is_face_monitor = true;

    cert.residual_chain.assign(witness.digits.size() + 1, 0);
    cert.residual_chain.back() = static_cast<std::size_t>(
        terminal_it - product.residual_members.begin());
    cert.predecessor_chain_replayed = true;
    for (std::size_t k = witness.digits.size(); k-- > 0;) {
        const auto it = product.residual_predecessor.find(
            {cert.residual_chain[k + 1], witness.digits[k]});
        if (it == product.residual_predecessor.end()) {
            cert.predecessor_chain_replayed = false;
            cert.obstruction = "missing residual predecessor";
            return cert;
        }
        cert.residual_chain[k] = it->second;
    }

    cert.fiber_sizes.reserve(cert.residual_chain.size());
    for (const auto r : cert.residual_chain) {
        std::size_t count = 0;
        for (const bool bit : product.residual_members.at(r)) count += bit;
        cert.fiber_sizes.push_back(count);
    }

    cert.every_step_serial = true;
    for (std::size_t k = 0; k < witness.digits.size(); ++k) {
        const auto& source_fiber = product.residual_members.at(cert.residual_chain[k]);
        const auto& target_fiber = product.residual_members.at(cert.residual_chain[k + 1]);
        for (std::size_t s = 0; s < source_fiber.size(); ++s) {
            if (!source_fiber[s]) continue;
            bool has_target = false;
            const auto it = product.controller_plant.successors.find(
                {s, witness.digits[k]});
            if (it != product.controller_plant.successors.end()) {
                for (const auto t : it->second)
                    has_target |= target_fiber.at(t);
            }
            cert.every_step_serial &= has_target;
        }
    }

    // Replay whole-word seriality by propagating every state in R_0.
    cert.whole_segment_serial = true;
    const auto& initial_fiber = product.residual_members.at(cert.residual_chain.front());
    const auto& final_fiber = product.residual_members.at(cert.residual_chain.back());
    for (std::size_t s = 0; s < initial_fiber.size(); ++s) {
        if (!initial_fiber[s]) continue;
        std::vector<bool> current(product.controller_states.size(), false);
        current[s] = true;
        for (const auto digit : witness.digits) {
            std::vector<bool> next(product.controller_states.size(), false);
            for (std::size_t q = 0; q < current.size(); ++q) {
                if (!current[q]) continue;
                const auto it = product.controller_plant.successors.find({q, digit});
                if (it != product.controller_plant.successors.end())
                    for (const auto t : it->second) next[t] = true;
            }
            current = std::move(next);
        }
        bool reaches = false;
        for (std::size_t q = 0; q < current.size(); ++q)
            reaches |= current[q] && final_fiber[q];
        cert.whole_segment_serial &= reaches;
    }

    cert.valid = cert.terminal_is_face_monitor &&
                 cert.predecessor_chain_replayed &&
                 cert.every_step_serial && cert.whole_segment_serial;
    if (!cert.valid && cert.obstruction.empty())
        cert.obstruction = "residual seriality replay failed";
    return cert;
}

} // namespace ravel::proof
