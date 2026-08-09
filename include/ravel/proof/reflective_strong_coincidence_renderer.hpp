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

} // namespace ravel::proof
