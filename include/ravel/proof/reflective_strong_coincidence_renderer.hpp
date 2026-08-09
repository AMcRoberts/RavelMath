#pragma once

#include <sstream>
#include <string>

#include "math/proof_reflection.hpp"

namespace ravel::proof {

inline std::string render_strong_coincidence_run_instances(
    const mathlib::reflection::Trace& trace) {
    std::ostringstream out;
    long long counter = 0;
    for (const auto& [id, node] : trace.find<mathlib::reflection::StrongCoincidenceRunCertificate>()) {
        (void)id;
        const std::string stem = "strong_coincidence_run_" + std::to_string(counter++);
        out << "/-- Concrete bounded strong-coincidence run; its limits remain explicit. -/\n";
        out << "def " << stem << "_images : List (List Nat) := [";
        for (std::size_t i = 0; i < node->images.size(); ++i) {
            if (i) out << ", ";
            out << "[";
            for (std::size_t j = 0; j < node->images[i].size(); ++j) {
                if (j) out << ", ";
                out << node->images[i][j];
            }
            out << "]";
        }
        out << "]\n";
        out << "theorem " << stem << "_summary :\n"
            << "    " << stem << "_images.length = " << node->images.size() << " ∧\n"
            << "    " << node->depth_reached << " ≤ " << node->max_depth << " ∧\n"
            << "    " << (node->holds ? "True" : "False") << " := by decide\n\n";
    }
    return out.str();
}

inline std::string render_strong_coincidence_pair_witness_instances(
    const mathlib::reflection::Trace& trace) {
    std::ostringstream out;
    long long counter = 0;
    for (const auto& [id, node] : trace.find<mathlib::reflection::StrongCoincidencePairWitnessCertificate>()) {
        (void)id;
        const std::string stem = "strong_coincidence_pair_witness_" + std::to_string(counter++);
        auto list = [&out](const std::string& name, const std::vector<long long>& values) {
            out << "def " << name << " : List Int := [";
            for (std::size_t i = 0; i < values.size(); ++i) {
                if (i) out << ", ";
                out << values[i];
            }
            out << "]\n";
        };
        list(stem + "_first_prefix", node->first_prefix);
        list(stem + "_second_prefix", node->second_prefix);
        list(stem + "_first_suffix", node->first_suffix);
        list(stem + "_second_suffix", node->second_suffix);
        out << "theorem " << stem << "_checked :\n"
            << "    (" << stem << "_first_prefix = " << stem << "_second_prefix) ∨\n"
            << "    (" << stem << "_first_suffix = " << stem << "_second_suffix) := by decide\n\n";
    }
    return out.str();
}

} // namespace ravel::proof
