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
        out << "def " << stem << "_pair_depths : List Int := [";
        for (std::size_t i = 0; i < node->pair_depths.size(); ++i) {
            if (i) out << ", ";
            out << node->pair_depths[i];
        }
        out << "]\n";
        out << "def " << stem << "_resolution_depths : List Int := [";
        for (std::size_t i = 0; i < node->pair_resolution_depths.size(); ++i) {
            if (i) out << ", ";
            out << node->pair_resolution_depths[i];
        }
        out << "]\n";
        out << "theorem " << stem << "_summary :\n"
            << "    " << stem << "_images.length = " << node->images.size() << " ∧\n"
            << "    " << stem << "_pair_depths.length = "
            << node->pair_depths.size() << " ∧\n"
            << "    " << stem << "_resolution_depths.length = "
            << node->pair_resolution_depths.size() << " ∧\n"
            << "    " << stem << "_resolution_depths = " << stem
            << "_pair_depths ∧\n"
            << "    " << node->depth_reached << " ≤ " << node->max_depth << " ∧\n"
            << "    " << (node->holds ? "True" : "False") << " := by decide\n\n";
    }
    return out.str();
}

inline std::string render_strong_coincidence_prefix_closure_instances(
    const mathlib::reflection::Trace& trace) {
    std::ostringstream out;
    long long counter = 0;
    for (const auto& [id, node] :
         trace.find<mathlib::reflection::StrongCoincidencePrefixClosureCertificate>()) {
        (void)id;
        const std::string stem = "strong_coincidence_prefix_closure_" +
                                 std::to_string(counter++);
        out << "/-- Concrete finite prefix-closure run; suffix resolution is not claimed. -/\n";
        out << "def " << stem << "_resolution_depths : List Int := [";
        for (std::size_t i = 0; i < node->pair_resolution_depths.size(); ++i) {
            if (i) out << ", ";
            out << node->pair_resolution_depths[i];
        }
        out << "]\n";
        out << "def " << stem << "_terminal_letters : List Int := [";
        for (std::size_t i = 0; i < node->pair_terminal_letters.size(); ++i) {
            if (i) out << ", ";
            out << node->pair_terminal_letters[i];
        }
        out << "]\n";
        out << "def " << stem << "_vectors : List (List Int) := [";
        for (std::size_t i = 0; i < node->pair_vectors.size(); ++i) {
            if (i) out << ", ";
            out << "[";
            for (std::size_t j = 0; j < node->pair_vectors[i].size(); ++j) {
                if (j) out << ", ";
                out << node->pair_vectors[i][j];
            }
            out << "]";
        }
        out << "]\n";
        out << "def " << stem << "_matrix : List Int := [";
        for (std::size_t i = 0; i < node->matrix.size(); ++i) {
            if (i) out << ", ";
            out << node->matrix[i];
        }
        out << "]\n";
        out << "theorem " << stem << "_summary :\n"
            << "    " << stem << "_resolution_depths.length = "
            << node->pair_resolution_depths.size() << " ∧\n"
            << "    " << stem << "_terminal_letters.length = "
            << node->pair_terminal_letters.size() << " ∧\n"
            << "    " << stem << "_vectors.length = " << node->pair_vectors.size() << " ∧\n"
            << "    " << stem << "_matrix.length = " << node->matrix.size() << " ∧\n"
            << "    " << node->depth_reached << " ≤ " << node->max_depth << " ∧\n"
            << "    " << (node->holds ? "True" : "False") << " := by decide\n\n";
    }
    return out.str();
}

inline std::string render_strong_coincidence_closure_instances(
    const mathlib::reflection::Trace& trace) {
    std::ostringstream out;
    long long counter = 0;
    for (const auto& [id, node] :
         trace.find<mathlib::reflection::StrongCoincidenceClosureCertificate>()) {
        (void)id;
        const std::string stem = "strong_coincidence_closure_" + std::to_string(counter++);
        out << "/-- Concrete finite full strong-coincidence closure run. -/\n";
        out << "def " << stem << "_resolution_depths : List Int := [";
        for (std::size_t i = 0; i < node->pair_resolution_depths.size(); ++i) {
            if (i) out << ", ";
            out << node->pair_resolution_depths[i];
        }
        out << "]\n";
        out << "def " << stem << "_terminal_letters : List Int := [";
        for (std::size_t i = 0; i < node->pair_terminal_letters.size(); ++i) {
            if (i) out << ", ";
            out << node->pair_terminal_letters[i];
        }
        out << "]\n";
        out << "def " << stem << "_vectors : List (List Int) := [";
        for (std::size_t i = 0; i < node->pair_vectors.size(); ++i) {
            if (i) out << ", ";
            out << "[";
            for (std::size_t j = 0; j < node->pair_vectors[i].size(); ++j) {
                if (j) out << ", ";
                out << node->pair_vectors[i][j];
            }
            out << "]";
        }
        out << "]\n";
        out << "def " << stem << "_from_suffix : List Bool := [";
        for (std::size_t i = 0; i < node->pair_from_suffix.size(); ++i) {
            if (i) out << ", ";
            out << (node->pair_from_suffix[i] ? "true" : "false");
        }
        out << "]\n";
        out << "def " << stem << "_matrix : List Int := [";
        for (std::size_t i = 0; i < node->matrix.size(); ++i) {
            if (i) out << ", ";
            out << node->matrix[i];
        }
        out << "]\n";
        out << "theorem " << stem << "_summary :\n"
            << "    " << stem << "_resolution_depths.length = "
            << node->pair_resolution_depths.size() << " ∧\n"
            << "    " << stem << "_terminal_letters.length = "
            << node->pair_terminal_letters.size() << " ∧\n"
            << "    " << stem << "_vectors.length = " << node->pair_vectors.size() << " ∧\n"
            << "    " << stem << "_from_suffix.length = " << node->pair_from_suffix.size() << " ∧\n"
            << "    " << stem << "_matrix.length = " << node->matrix.size() << " ∧\n"
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
