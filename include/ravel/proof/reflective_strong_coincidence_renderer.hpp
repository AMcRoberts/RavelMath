#pragma once

#include <sstream>
#include <string>

#include "math/proof_reflection.hpp"

namespace ravel::proof {

inline const char* strong_coincidence_path_semantics_lean() {
    return
        "set_option maxRecDepth 100000\n\n"
        "structure SCClosureEdge where\n"
        "  from_junction : Int\n"
        "  to_junction : Int\n"
        "  jump_size : Int\n"
        "  child_index : Int\n"
        "  landmark : List Int\n"
        "  chain : List Int\n\n"
        "def sc_checkPath : List SCClosureEdge → List Int → Int → Int → Int → Bool\n"
        "  | _, [], cursor, remaining, terminal => cursor = terminal && remaining = 0\n"
        "  | edges, index :: rest, cursor, remaining, terminal =>\n"
        "      if index < 0 then false else\n"
        "      match edges[index.toNat]? with\n"
        "      | none => false\n"
        "      | some edge =>\n"
        "          if edge.from_junction ≠ cursor then false\n"
        "          else if edge.jump_size ≤ remaining then\n"
        "            sc_checkPath edges rest edge.to_junction (remaining - edge.jump_size) terminal\n"
        "          else\n"
        "            rest = [] ∧ remaining > 0 ∧\n"
        "              edge.chain[(remaining - 1).toNat]? = some terminal\n\n"
        "def sc_vecAdd : List Int → List Int → List Int\n"
        "  | [], ys => ys\n"
        "  | xs, [] => xs\n"
        "  | x :: xs, y :: ys => (x + y) :: sc_vecAdd xs ys\n\n"
        "def sc_sumVectors : List (List Int) → List Int\n"
        "  | [] => []\n"
        "  | v :: rest => sc_vecAdd v (sc_sumVectors rest)\n\n"
        "def sc_at (xs : List Int) (i : Nat) : Int := xs.getD i 0\n\n"
        "def sc_dotMul (m n : List Int) (d row col k : Nat) : Int :=\n"
        "  match k with\n"
        "  | 0 => 0\n"
        "  | t + 1 => sc_dotMul m n d row col t + sc_at m (row * d + t) * sc_at n (t * d + col)\n\n"
        "def sc_dotVec (m v : List Int) (d row k : Nat) : Int :=\n"
        "  match k with\n"
        "  | 0 => 0\n"
        "  | t + 1 => sc_dotVec m v d row t + sc_at m (row * d + t) * sc_at v t\n\n"
        "def sc_rowMul (m n : List Int) (d row cols : Nat) : List Int :=\n"
        "  match cols with\n"
        "  | 0 => []\n"
        "  | t + 1 => sc_rowMul m n d row t ++ [sc_dotMul m n d row t d]\n\n"
        "def sc_matMul (m n : List Int) (d rows cols : Nat) : List Int :=\n"
        "  match rows with\n"
        "  | 0 => []\n"
        "  | t + 1 => sc_matMul m n d t cols ++ sc_rowMul m n d t cols\n\n"
        "def sc_identityRow (d row cols : Nat) : List Int :=\n"
        "  match cols with\n"
        "  | 0 => []\n"
        "  | t + 1 => sc_identityRow d row t ++ [if row = t then 1 else 0]\n\n"
        "def sc_identity (d rows : Nat) : List Int :=\n"
        "  match rows with\n"
        "  | 0 => []\n"
        "  | t + 1 => sc_identity d t ++ sc_identityRow d t d\n\n"
        "def sc_matPow (m : List Int) (d p : Nat) : List Int :=\n"
        "  match p with\n"
        "  | 0 => sc_identity d d\n"
        "  | t + 1 => sc_matMul m (sc_matPow m d t) d d d\n\n"
        "def sc_matVecAux (m v : List Int) (d rows : Nat) : List Int :=\n"
        "  match rows with\n"
        "  | 0 => []\n"
        "  | t + 1 => sc_matVecAux m v d t ++ [sc_dotVec m v d t d]\n\n"
        "def sc_matVec (m v : List Int) (d : Nat) : List Int :=\n"
        "  sc_matVecAux m v d d\n\n"
        "def sc_pathWeights (edges : List SCClosureEdge) (m : List Int)\n"
        "    (path : List Int) (cursor remaining d : Int) : List (List Int) :=\n"
        "  match path with\n"
        "  | [] => []\n"
        "  | index :: rest =>\n"
        "      match edges[index.toNat]? with\n"
        "      | none => []\n"
        "      | some edge =>\n"
        "          let weighted := sc_matVec (sc_matPow m d.toNat (remaining - 1).toNat) edge.landmark d.toNat\n"
        "          if edge.jump_size ≤ remaining then\n"
        "            weighted :: sc_pathWeights edges m rest edge.to_junction (remaining - edge.jump_size) d\n"
        "          else [weighted]\n\n";
}

inline void render_closure_path_lists(
    std::ostringstream& out, const std::string& stem,
    const std::vector<std::vector<long long>>& first,
    const std::vector<std::vector<long long>>& second) {
    auto render = [&out](const std::string& name,
                         const std::vector<std::vector<long long>>& paths) {
        out << "def " << name << " : List (List Int) := [";
        for (std::size_t i = 0; i < paths.size(); ++i) {
            if (i) out << ", ";
            out << "[";
            for (std::size_t j = 0; j < paths[i].size(); ++j) {
                if (j) out << ", ";
                out << paths[i][j];
            }
            out << "]";
        }
        out << "]\n";
    };
    render(stem + "_first_paths", first);
    render(stem + "_second_paths", second);
}

inline void render_closure_int_list(std::ostringstream& out, const std::string& name,
                                    const std::vector<long long>& values) {
    out << "def " << name << " : List Int := [";
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (i) out << ", ";
        out << values[i];
    }
    out << "]\n";
}

inline void render_closure_weighted_vectors(
    std::ostringstream& out, const std::string& name,
    const std::vector<std::vector<std::vector<long long>>>& values) {
    out << "def " << name << " : List (List (List Int)) := [";
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (i) out << ", ";
        out << "[";
        for (std::size_t j = 0; j < values[i].size(); ++j) {
            if (j) out << ", ";
            out << "[";
            for (std::size_t k = 0; k < values[i][j].size(); ++k) {
                if (k) out << ", ";
                out << values[i][j][k];
            }
            out << "]";
        }
        out << "]";
    }
    out << "]\n";
}

inline void render_closure_weight_checks(
    std::ostringstream& out, const std::string& stem,
    const std::vector<std::vector<long long>>& targets,
    const std::vector<std::vector<std::vector<long long>>>& weighted,
    const std::vector<std::vector<long long>>& paths) {
    for (std::size_t i = 0; i < weighted.size(); ++i) {
        if (i >= targets.size() || i >= paths.size() || paths[i].empty()) continue;
        out << "theorem " << stem << "_weight_check_" << i << " :\n"
            << "    sc_sumVectors [";
        for (std::size_t j = 0; j < weighted[i].size(); ++j) {
            if (j) out << ", ";
            out << "[";
            for (std::size_t k = 0; k < weighted[i][j].size(); ++k) {
                if (k) out << ", ";
                out << weighted[i][j][k];
            }
            out << "]";
        }
        out << "] = [";
        for (std::size_t k = 0; k < targets[i].size(); ++k) {
            if (k) out << ", ";
            out << targets[i][k];
        }
        out << "] := by decide\n\n";
    }
}

inline void render_closure_derived_weight_checks(
    std::ostringstream& out, const std::string& stem,
    const std::string& edge_stem,
    const std::string& matrix_stem,
    const std::vector<std::vector<std::vector<long long>>>& recorded,
    const std::vector<std::vector<long long>>& paths,
    const std::vector<long long>& junctions,
    const std::vector<long long>& remaining_depths,
    std::size_t dimension,
    const std::vector<bool>* from_suffix = nullptr) {
    for (std::size_t i = 0; i < recorded.size(); ++i) {
        if (i >= paths.size() || i >= junctions.size() || i >= remaining_depths.size() ||
            paths[i].empty() || junctions[i] < 0 ||
            remaining_depths[i] <= 0) continue;
        const std::string selected_edges =
            from_suffix && i < from_suffix->size() && (*from_suffix)[i]
                ? edge_stem + "_suffix_edges" : edge_stem + "_edges";
        out << "theorem " << stem << "_derived_weight_check_" << i << " :\n"
            << "    sc_pathWeights " << selected_edges
            << " " << matrix_stem << "_matrix [";
        for (std::size_t j = 0; j < paths[i].size(); ++j) {
            if (j) out << ", ";
            out << paths[i][j];
        }
        out << "] " << junctions[i] << " " << remaining_depths[i] << " "
            << dimension << " = [";
        for (std::size_t j = 0; j < recorded[i].size(); ++j) {
            if (j) out << ", ";
            out << "[";
            for (std::size_t k = 0; k < recorded[i][j].size(); ++k) {
                if (k) out << ", ";
                out << recorded[i][j][k];
            }
            out << "]";
        }
        out << "] := by decide\n\n";
    }
}

inline void render_closure_derived_sum_checks(
    std::ostringstream& out, const std::string& stem,
    const std::string& edge_stem, const std::string& matrix_stem,
    const std::vector<std::vector<long long>>& targets,
    const std::vector<std::vector<long long>>& paths,
    const std::vector<long long>& junctions,
    const std::vector<long long>& remaining_depths, std::size_t dimension,
    const std::vector<bool>* from_suffix = nullptr) {
    for (std::size_t i = 0; i < targets.size(); ++i) {
        if (i >= paths.size() || i >= junctions.size() || i >= remaining_depths.size() ||
            paths[i].empty() || junctions[i] < 0 || remaining_depths[i] <= 0) continue;
        const std::string selected_edges =
            from_suffix && i < from_suffix->size() && (*from_suffix)[i]
                ? edge_stem + "_suffix_edges" : edge_stem + "_edges";
        out << "theorem " << stem << "_derived_sum_check_" << i << " :\n"
            << "    sc_sumVectors (sc_pathWeights " << selected_edges << " "
            << matrix_stem << "_matrix [";
        for (std::size_t j = 0; j < paths[i].size(); ++j) {
            if (j) out << ", ";
            out << paths[i][j];
        }
        out << "] " << junctions[i] << " " << remaining_depths[i] << " "
            << dimension << ") = [";
        for (std::size_t j = 0; j < targets[i].size(); ++j) {
            if (j) out << ", ";
            out << targets[i][j];
        }
        out << "] := by decide\n\n";
    }
}

inline void render_closure_derived_coincidence_checks(
    std::ostringstream& out, const std::string& stem,
    const std::string& edge_stem, const std::string& matrix_stem,
    const std::vector<std::vector<long long>>& first_paths,
    const std::vector<std::vector<long long>>& second_paths,
    const std::vector<long long>& first_junctions,
    const std::vector<long long>& second_junctions,
    const std::vector<long long>& first_remaining_depths,
    const std::vector<long long>& second_remaining_depths,
    std::size_t dimension, const std::vector<bool>* from_suffix = nullptr) {
    for (std::size_t i = 0; i < first_paths.size(); ++i) {
        if (i >= second_paths.size() || i >= first_junctions.size() ||
            i >= second_junctions.size() || i >= first_remaining_depths.size() ||
            i >= second_remaining_depths.size() || first_paths[i].empty() ||
            second_paths[i].empty() || first_junctions[i] < 0 ||
            second_junctions[i] < 0 || first_remaining_depths[i] <= 0 ||
            second_remaining_depths[i] <= 0) continue;
        const bool suffix = from_suffix && i < from_suffix->size() && (*from_suffix)[i];
        const std::string selected_edges = suffix ? edge_stem + "_suffix_edges" : edge_stem + "_edges";
        auto path_term = [&](const std::vector<long long>& path, long long junction,
                             long long remaining) {
            std::ostringstream term;
            term << "sc_sumVectors (sc_pathWeights " << selected_edges << " "
                 << matrix_stem << "_matrix [";
            for (std::size_t j = 0; j < path.size(); ++j) {
                if (j) term << ", ";
                term << path[j];
            }
            term << "] " << junction << " " << remaining << " " << dimension << ")";
            return term.str();
        };
        out << "theorem " << stem << "_derived_coincidence_check_" << i << " :\n"
            << "    " << path_term(first_paths[i], first_junctions[i],
                                   first_remaining_depths[i])
            << " = " << path_term(second_paths[i], second_junctions[i],
                                   second_remaining_depths[i]) << " := by decide\n\n";
    }
}

inline void render_closure_path_checks(
    std::ostringstream& out, const std::string& edge_stem, const std::string& side,
    const std::vector<long long>& terminals,
    const std::vector<std::vector<long long>>& paths,
    const std::vector<long long>& junctions,
    const std::vector<long long>& remaining_depths,
    const std::vector<bool>* from_suffix = nullptr) {
    for (std::size_t i = 0; i < paths.size(); ++i) {
        if (i >= terminals.size() || i >= junctions.size() ||
            i >= remaining_depths.size() || paths[i].empty() ||
            junctions[i] < 0 || remaining_depths[i] <= 0) continue;
        const std::string selected_edges =
            from_suffix && i < from_suffix->size() && (*from_suffix)[i]
                ? edge_stem + "_suffix_edges" : edge_stem + "_edges";
        out << "theorem " << edge_stem << "_" << side << "_path_check_" << i << " :\n"
            << "    sc_checkPath " << selected_edges << " [";
        for (std::size_t j = 0; j < paths[i].size(); ++j) {
            if (j) out << ", ";
            out << paths[i][j];
        }
        out << "] " << junctions[i] << " " << remaining_depths[i]
            << " " << terminals[i] << " = true := by decide\n\n";
    }
}

template <typename Edge>
inline void render_closure_edges(std::ostringstream& out, const std::string& stem,
                                 const std::vector<Edge>& edges) {
    out << "def " << stem << "_edges : List SCClosureEdge := [";
    for (std::size_t i = 0; i < edges.size(); ++i) {
        if (i) out << ", ";
        const auto& edge = edges[i];
        out << "{from_junction := " << edge.from_junction
            << ", to_junction := " << edge.to_junction
            << ", jump_size := " << edge.jump_size
            << ", child_index := " << edge.child_index << ", landmark := [";
        for (std::size_t j = 0; j < edge.landmark.size(); ++j) {
            if (j) out << ", ";
            out << edge.landmark[j];
        }
        out << "], chain := [";
        for (std::size_t j = 0; j < edge.chain.size(); ++j) {
            if (j) out << ", ";
            out << edge.chain[j];
        }
        out << "]}";
    }
    out << "]\n";
}

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
        render_closure_edges(out, stem, node->edges);
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
        render_closure_path_lists(out, stem, node->pair_first_paths,
                                  node->pair_second_paths);
        render_closure_weighted_vectors(out, stem + "_first_weighted_vectors",
                                        node->pair_first_weighted_vectors);
        render_closure_weighted_vectors(out, stem + "_second_weighted_vectors",
                                        node->pair_second_weighted_vectors);
        render_closure_int_list(out, stem + "_first_junctions", node->pair_first_junctions);
        render_closure_int_list(out, stem + "_second_junctions", node->pair_second_junctions);
        render_closure_int_list(out, stem + "_first_remaining_depths",
                                node->pair_first_remaining_depths);
        render_closure_int_list(out, stem + "_second_remaining_depths",
                                node->pair_second_remaining_depths);
        render_closure_path_checks(out, stem, "first", node->pair_terminal_letters,
                                   node->pair_first_paths, node->pair_first_junctions,
                                   node->pair_first_remaining_depths);
        render_closure_path_checks(out, stem, "second", node->pair_terminal_letters,
                                   node->pair_second_paths, node->pair_second_junctions,
                                   node->pair_second_remaining_depths);
        render_closure_weight_checks(out, stem + "_first", node->pair_vectors,
                                     node->pair_first_weighted_vectors,
                                     node->pair_first_paths);
        render_closure_weight_checks(out, stem + "_second", node->pair_vectors,
                                     node->pair_second_weighted_vectors,
                                     node->pair_second_paths);
        out << "def " << stem << "_first_positions : List Int := [";
        for (std::size_t i = 0; i < node->pair_first_positions.size(); ++i) {
            if (i) out << ", ";
            out << node->pair_first_positions[i];
        }
        out << "]\n";
        out << "def " << stem << "_second_positions : List Int := [";
        for (std::size_t i = 0; i < node->pair_second_positions.size(); ++i) {
            if (i) out << ", ";
            out << node->pair_second_positions[i];
        }
        out << "]\n";
        out << "def " << stem << "_matrix : List Int := [";
        for (std::size_t i = 0; i < node->matrix.size(); ++i) {
            if (i) out << ", ";
            out << node->matrix[i];
        }
        out << "]\n";
        render_closure_derived_weight_checks(
            out, stem + "_first", stem, stem, node->pair_first_weighted_vectors,
            node->pair_first_paths, node->pair_first_junctions,
            node->pair_first_remaining_depths, node->images.size());
        render_closure_derived_weight_checks(
            out, stem + "_second", stem, stem, node->pair_second_weighted_vectors,
            node->pair_second_paths, node->pair_second_junctions,
            node->pair_second_remaining_depths, node->images.size());
        render_closure_derived_sum_checks(
            out, stem + "_first", stem, stem, node->pair_vectors,
            node->pair_first_paths, node->pair_first_junctions,
            node->pair_first_remaining_depths, node->images.size());
        render_closure_derived_sum_checks(
            out, stem + "_second", stem, stem, node->pair_vectors,
            node->pair_second_paths, node->pair_second_junctions,
            node->pair_second_remaining_depths, node->images.size());
        render_closure_derived_coincidence_checks(
            out, stem, stem, stem, node->pair_first_paths,
            node->pair_second_paths, node->pair_first_junctions,
            node->pair_second_junctions, node->pair_first_remaining_depths,
            node->pair_second_remaining_depths, node->images.size());
        out << "theorem " << stem << "_summary :\n"
            << "    " << stem << "_images.length = " << node->images.size() << " ∧\n"
            << "    " << stem << "_edges.length = " << node->edges.size() << " ∧\n"
            << "    " << stem << "_resolution_depths.length = "
            << node->pair_resolution_depths.size() << " ∧\n"
            << "    " << stem << "_terminal_letters.length = "
            << node->pair_terminal_letters.size() << " ∧\n"
            << "    " << stem << "_vectors.length = " << node->pair_vectors.size() << " ∧\n"
            << "    " << stem << "_first_paths.length = " << node->pair_first_paths.size() << " ∧\n"
            << "    " << stem << "_second_paths.length = " << node->pair_second_paths.size() << " ∧\n"
            << "    " << stem << "_first_weighted_vectors.length = "
            << node->pair_first_weighted_vectors.size() << " ∧\n"
            << "    " << stem << "_second_weighted_vectors.length = "
            << node->pair_second_weighted_vectors.size() << " ∧\n"
            << "    " << stem << "_first_junctions.length = "
            << node->pair_first_junctions.size() << " ∧\n"
            << "    " << stem << "_second_junctions.length = "
            << node->pair_second_junctions.size() << " ∧\n"
            << "    " << stem << "_first_remaining_depths.length = "
            << node->pair_first_remaining_depths.size() << " ∧\n"
            << "    " << stem << "_second_remaining_depths.length = "
            << node->pair_second_remaining_depths.size() << " ∧\n"
            << "    " << stem << "_first_positions.length = "
            << node->pair_first_positions.size() << " ∧\n"
            << "    " << stem << "_second_positions.length = "
            << node->pair_second_positions.size() << " ∧\n"
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
        render_closure_edges(out, stem, node->edges);
        render_closure_edges(out, stem + "_suffix", node->suffix_edges);
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
        render_closure_path_lists(out, stem, node->pair_first_paths,
                                  node->pair_second_paths);
        render_closure_weighted_vectors(out, stem + "_first_weighted_vectors",
                                        node->pair_first_weighted_vectors);
        render_closure_weighted_vectors(out, stem + "_second_weighted_vectors",
                                        node->pair_second_weighted_vectors);
        render_closure_int_list(out, stem + "_first_junctions", node->pair_first_junctions);
        render_closure_int_list(out, stem + "_second_junctions", node->pair_second_junctions);
        render_closure_int_list(out, stem + "_first_remaining_depths",
                                node->pair_first_remaining_depths);
        render_closure_int_list(out, stem + "_second_remaining_depths",
                                node->pair_second_remaining_depths);
        render_closure_path_checks(out, stem, "first", node->pair_terminal_letters,
                                   node->pair_first_paths, node->pair_first_junctions,
                                   node->pair_first_remaining_depths, &node->pair_from_suffix);
        render_closure_path_checks(out, stem, "second", node->pair_terminal_letters,
                                   node->pair_second_paths, node->pair_second_junctions,
                                   node->pair_second_remaining_depths, &node->pair_from_suffix);
        render_closure_weight_checks(out, stem + "_first", node->pair_vectors,
                                     node->pair_first_weighted_vectors,
                                     node->pair_first_paths);
        render_closure_weight_checks(out, stem + "_second", node->pair_vectors,
                                     node->pair_second_weighted_vectors,
                                     node->pair_second_paths);
        out << "def " << stem << "_from_suffix : List Bool := [";
        for (std::size_t i = 0; i < node->pair_from_suffix.size(); ++i) {
            if (i) out << ", ";
            out << (node->pair_from_suffix[i] ? "true" : "false");
        }
        out << "]\n";
        out << "def " << stem << "_first_positions : List Int := [";
        for (std::size_t i = 0; i < node->pair_first_positions.size(); ++i) {
            if (i) out << ", ";
            out << node->pair_first_positions[i];
        }
        out << "]\n";
        out << "def " << stem << "_second_positions : List Int := [";
        for (std::size_t i = 0; i < node->pair_second_positions.size(); ++i) {
            if (i) out << ", ";
            out << node->pair_second_positions[i];
        }
        out << "]\n";
        out << "def " << stem << "_matrix : List Int := [";
        for (std::size_t i = 0; i < node->matrix.size(); ++i) {
            if (i) out << ", ";
            out << node->matrix[i];
        }
        out << "]\n";
        render_closure_derived_weight_checks(
            out, stem + "_first", stem, stem, node->pair_first_weighted_vectors,
            node->pair_first_paths, node->pair_first_junctions,
            node->pair_first_remaining_depths, node->images.size(), &node->pair_from_suffix);
        render_closure_derived_weight_checks(
            out, stem + "_second", stem, stem, node->pair_second_weighted_vectors,
            node->pair_second_paths, node->pair_second_junctions,
            node->pair_second_remaining_depths, node->images.size(), &node->pair_from_suffix);
        render_closure_derived_sum_checks(
            out, stem + "_first", stem, stem, node->pair_vectors,
            node->pair_first_paths, node->pair_first_junctions,
            node->pair_first_remaining_depths, node->images.size(), &node->pair_from_suffix);
        render_closure_derived_sum_checks(
            out, stem + "_second", stem, stem, node->pair_vectors,
            node->pair_second_paths, node->pair_second_junctions,
            node->pair_second_remaining_depths, node->images.size(), &node->pair_from_suffix);
        render_closure_derived_coincidence_checks(
            out, stem, stem, stem, node->pair_first_paths,
            node->pair_second_paths, node->pair_first_junctions,
            node->pair_second_junctions, node->pair_first_remaining_depths,
            node->pair_second_remaining_depths, node->images.size(), &node->pair_from_suffix);
        out << "theorem " << stem << "_summary :\n"
            << "    " << stem << "_images.length = " << node->images.size() << " ∧\n"
            << "    " << stem << "_edges.length = " << node->edges.size() << " ∧\n"
            << "    " << stem << "_suffix_edges.length = "
            << node->suffix_edges.size() << " ∧\n"
            << "    " << stem << "_resolution_depths.length = "
            << node->pair_resolution_depths.size() << " ∧\n"
            << "    " << stem << "_terminal_letters.length = "
            << node->pair_terminal_letters.size() << " ∧\n"
            << "    " << stem << "_vectors.length = " << node->pair_vectors.size() << " ∧\n"
            << "    " << stem << "_first_paths.length = " << node->pair_first_paths.size() << " ∧\n"
            << "    " << stem << "_second_paths.length = " << node->pair_second_paths.size() << " ∧\n"
            << "    " << stem << "_first_weighted_vectors.length = "
            << node->pair_first_weighted_vectors.size() << " ∧\n"
            << "    " << stem << "_second_weighted_vectors.length = "
            << node->pair_second_weighted_vectors.size() << " ∧\n"
            << "    " << stem << "_first_junctions.length = "
            << node->pair_first_junctions.size() << " ∧\n"
            << "    " << stem << "_second_junctions.length = "
            << node->pair_second_junctions.size() << " ∧\n"
            << "    " << stem << "_first_remaining_depths.length = "
            << node->pair_first_remaining_depths.size() << " ∧\n"
            << "    " << stem << "_second_remaining_depths.length = "
            << node->pair_second_remaining_depths.size() << " ∧\n"
            << "    " << stem << "_from_suffix.length = " << node->pair_from_suffix.size() << " ∧\n"
            << "    " << stem << "_first_positions.length = "
            << node->pair_first_positions.size() << " ∧\n"
            << "    " << stem << "_second_positions.length = "
            << node->pair_second_positions.size() << " ∧\n"
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
