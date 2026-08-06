#pragma once

#include <algorithm>
#include <cctype>
#include <fstream>
#include <map>
#include <optional>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace ravel::proof::truth {

enum class TrustLevel {
    KernelCheckedExport,
    LeanDeclaration,
    Draft,
};

inline const char* trust_name(TrustLevel t) {
    switch (t) {
        case TrustLevel::KernelCheckedExport: return "kernel-checked-export";
        case TrustLevel::LeanDeclaration: return "lean-declaration";
        case TrustLevel::Draft: return "draft";
    }
    return "unknown";
}

struct Capability {
    std::string name;
    std::string kind;
    std::string file;
    std::size_t line = 0;
    std::string compartment;
    TrustLevel trust = TrustLevel::LeanDeclaration;
    std::set<std::string> tags;
    std::string signature;
};

struct DerivationRule {
    std::string rule_id;
    std::vector<std::string> premises;
    std::string conclusion;
    std::vector<std::string> theorem_names;
    std::set<std::string> query_terms;
    std::string compartment_path;
};

struct Answer {
    std::string question;
    std::vector<Capability> direct_matches;
    std::vector<DerivationRule> derivation_chains;
    std::vector<bool> derivation_kernel_checked;
    std::vector<std::string> unresolved_obligations;

    bool has_kernel_checked_conclusion() const {
        return std::any_of(derivation_kernel_checked.begin(), derivation_kernel_checked.end(),
                           [](bool x) { return x; });
    }

    std::string render() const {
        std::ostringstream out;
        out << "question: " << question << "\n";
        out << "classification: ";
        if (has_kernel_checked_conclusion()) out << "kernel-checked conclusion available\n";
        else if (!derivation_chains.empty()) out << "theorem-backed derivation path available\n";
        else if (!direct_matches.empty()) out << "relevant theorem capabilities available\n";
        else out << "unresolved: no supported theorem path found\n";

        if (!derivation_chains.empty()) {
            out << "\nderivation chains:\n";
            for (std::size_t chain_index = 0; chain_index < derivation_chains.size(); ++chain_index) {
                const auto& chain = derivation_chains[chain_index];
                out << "- " << chain.rule_id << " [" << chain.compartment_path << ", "
                    << (derivation_kernel_checked.at(chain_index) ? "kernel-checked chain" : "declared theorem chain")
                    << "]\n";
                for (const auto& premise : chain.premises) out << "    requires: " << premise << "\n";
                for (const auto& theorem : chain.theorem_names) out << "    theorem: " << theorem << "\n";
                out << "    yields: " << chain.conclusion << "\n";
            }
        }

        if (!direct_matches.empty()) {
            out << "\nranked theorem capabilities:\n";
            for (const auto& c : direct_matches) {
                out << "- " << c.name << " [" << trust_name(c.trust) << ", "
                    << c.compartment << "]\n"
                    << "    " << c.file << ":" << c.line << "\n"
                    << "    " << c.signature << "\n";
            }
        }

        if (!unresolved_obligations.empty()) {
            out << "\nunclosed obligations:\n";
            for (const auto& gap : unresolved_obligations) out << "- " << gap << "\n";
        }
        return out.str();
    }
};

inline std::string lowercase(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return text;
}

inline std::set<std::string> tokens(const std::string& text) {
    std::set<std::string> result;
    std::string current;
    for (unsigned char c : lowercase(text)) {
        if (std::isalnum(c) || c == '_') current.push_back(static_cast<char>(c));
        else if (!current.empty()) { result.insert(current); current.clear(); }
    }
    if (!current.empty()) result.insert(current);
    static const std::set<std::string> stop{
        "a","an","and","are","can","does","for","from","how","i","in","is","it",
        "of","on","prove","show","the","this","to","unrelated","what","when","where","which","why","with"
    };
    for (const auto& s : stop) result.erase(s);
    return result;
}

inline std::vector<std::string> split(const std::string& text, char delimiter) {
    std::vector<std::string> parts;
    std::stringstream ss(text);
    std::string item;
    while (std::getline(ss, item, delimiter)) parts.push_back(item);
    return parts;
}

class CapabilityIndex {
public:
    static CapabilityIndex load_tsv(const std::string& path) {
        std::ifstream in(path);
        if (!in) throw std::runtime_error("cannot open theorem capability index: " + path);
        CapabilityIndex result;
        std::string line;
        std::getline(in, line); // header
        while (std::getline(in, line)) {
            auto fields = split(line, '\t');
            if (fields.size() < 8) continue;
            Capability c;
            c.name = fields[0]; c.kind = fields[1]; c.file = fields[2];
            c.line = static_cast<std::size_t>(std::stoull(fields[3]));
            c.compartment = fields[4];
            if (fields[5] == "kernel-checked-export") c.trust = TrustLevel::KernelCheckedExport;
            else if (fields[5] == "draft") c.trust = TrustLevel::Draft;
            else c.trust = TrustLevel::LeanDeclaration;
            for (const auto& tag : split(fields[6], ',')) if (!tag.empty()) c.tags.insert(tag);
            c.signature = fields[7];
            result.by_name_[c.name] = result.capabilities_.size();
            result.capabilities_.push_back(std::move(c));
        }
        return result;
    }

    const Capability* find(const std::string& name) const {
        const auto it = by_name_.find(name);
        if (it == by_name_.end()) return nullptr;
        return &capabilities_.at(it->second);
    }

    std::vector<Capability> search(const std::string& question, std::size_t limit = 12) const {
        const auto q = tokens(question);
        std::vector<std::pair<int, std::size_t>> ranked;
        for (std::size_t i = 0; i < capabilities_.size(); ++i) {
            const auto& c = capabilities_[i];
            const auto hay = tokens(c.name + " " + c.compartment + " " + c.signature);
            int score = 0;
            for (const auto& term : q) {
                if (hay.contains(term)) score += 5;
                if (c.tags.contains(term)) score += 4;
                if (lowercase(c.name).find(term) != std::string::npos) score += 3;
            }
            if (c.trust == TrustLevel::KernelCheckedExport) score += 2;
            if (c.trust == TrustLevel::Draft) score -= 3;
            if (score >= 5) ranked.emplace_back(score, i);
        }
        std::sort(ranked.begin(), ranked.end(), [&](auto a, auto b) {
            if (a.first != b.first) return a.first > b.first;
            return capabilities_[a.second].name < capabilities_[b.second].name;
        });
        std::vector<Capability> answer;
        for (std::size_t i = 0; i < ranked.size() && answer.size() < limit; ++i)
            answer.push_back(capabilities_[ranked[i].second]);
        return answer;
    }

    std::size_t size() const { return capabilities_.size(); }

private:
    std::vector<Capability> capabilities_;
    std::map<std::string, std::size_t> by_name_;
};

inline std::vector<DerivationRule> standard_derivation_rules() {
    return {
        {
            "nbonacci-universal-determinant",
            {"n ≥ 2", "the matrix is RavelGenerated.nbonacciCharacteristicMatrix (n+1)"},
            "determinant equals X + X^2 + ... + X^(n+1) - 1",
            {"RavelGenerated.qMatrix_det_recurrence", "RavelGenerated.qMatrix_det_closed_form",
             "RavelGenerated.nbonacci_characteristic_split", "RavelGenerated.nbonacci_universal_n"},
            {"nbonacci","universal","determinant","characteristic","polynomial"},
            "matrix → polynomial"
        },
        {
            "charpoly-to-fast-power-reduction",
            {"A.charpoly = nbonacciCharpoly n"},
            "A^(n+1) = 2*A - 1",
            {"nbonacci_geomSum_of_charpoly", "nbonacci_block_identity_matrix",
             "nbonacci_block_identity_of_charpoly"},
            {"charpoly","power","matrix","fast","reduction","geometric","sum"},
            "matrix → number-theory"
        },
        {
            "perron-columns-to-gap-order",
            {"left Perron eigen-equations", "positive Perron root", "family-specific terminal equation"},
            "strict coordinate ordering for tetrabonacci, pentanacci, or hexanacci",
            {"perron_column_difference", "nbonacci_gap_step",
             "positive_gap_of_positive_scaled_gap", "tetrabonacci_gap_order",
             "pentanacci_gap_order", "hexanacci_gap_order"},
            {"perron","gap","coordinate","order","eigenvector","nbonacci"},
            "spectral → number-theory"
        },
        {
            "class-ii-perron-to-window",
            {"a ≥ 2", "q ≤ a-1", "β > a > 0", "β satisfies the Class-II cubic"},
            "the two universal Class-II window inequalities",
            {"class_ii_perron_gap_lt_one", "class_ii_perron_gap_gt_half",
             "class_ii_perron_gap_gt_two_thirds", "class_ii_perron_window_bounds"},
            {"class","ii","perron","window","bounds","gap","cubic"},
            "spectral → window"
        },
        {
            "affine-catalogue-to-boundary-support",
            {"edge belongs to the explicit neighbor catalogue", "parameter a is supplied"},
            "affine edge transport and exact boundary-layer support counts",
            {"affine_catalogue_affine", "neighbor0_boundary_layer_support",
             "neighbor1_boundary_layer_support", "neighbor2_boundary_layer_support",
             "all_neighbors_boundary_layer_support"},
            {"affine","catalogue","boundary","support","neighbor","edge"},
            "affine-catalogue → graph"
        },
        {
            "shell-forward-validity",
            {"shell kind", "round parameter", "Class-II Perron hypotheses"},
            "shell propagation, injectivity, and window validity",
            {"shellNode_propagates", "shellNode_in_open_signed_strip",
             "shellNode_injective_at_round", "class_ii_shell_endpoint_valid"},
            {"shell","propagation","injective","window","valid","endpoint"},
            "affine-catalogue → window"
        }
    };
}

class TruthMachine {
public:
    explicit TruthMachine(CapabilityIndex index,
                          std::vector<DerivationRule> rules = standard_derivation_rules())
        : index_(std::move(index)), rules_(std::move(rules)) {}

    Answer ask(const std::string& question) const {
        Answer answer;
        answer.question = question;
        answer.direct_matches = index_.search(question);
        const auto q = tokens(question);
        std::vector<std::pair<std::size_t, const DerivationRule*>> candidates;
        std::size_t best_overlap = 0;
        for (const auto& rule : rules_) {
            std::size_t overlap = 0;
            for (const auto& term : rule.query_terms) if (q.contains(term)) ++overlap;
            if (overlap >= 2 || (overlap == 1 && q.size() <= 3)) {
                candidates.push_back({overlap, &rule});
                best_overlap = std::max(best_overlap, overlap);
            }
        }
        for (const auto& [overlap, rule_ptr] : candidates) {
            if (overlap != best_overlap) continue;
            const auto& rule = *rule_ptr;
            bool all_found = true;
            for (const auto& theorem : rule.theorem_names) {
                if (!index_.find(theorem)) { all_found = false; break; }
            }
            if (all_found) {
                bool all_kernel = true;
                for (const auto& theorem : rule.theorem_names) {
                    const auto* capability = index_.find(theorem);
                    all_kernel = all_kernel && capability &&
                                 capability->trust == TrustLevel::KernelCheckedExport;
                }
                answer.derivation_chains.push_back(rule);
                answer.derivation_kernel_checked.push_back(all_kernel);
            }
        }
        if (answer.direct_matches.empty() && answer.derivation_chains.empty()) {
            answer.unresolved_obligations.push_back(
                "No indexed theorem or curated derivation chain matches the question terms.");
            answer.unresolved_obligations.push_back(
                "A new bridge theorem, synonym map, or compartment adapter is required.");
        } else if (!answer.derivation_chains.empty()) {
            answer.unresolved_obligations.push_back(
                "Instantiate every listed hypothesis before emitting a theorem application.");
            answer.unresolved_obligations.push_back(
                "Kernel-check any newly composed Lean file; theorem discovery alone is not proof closure.");
        }
        return answer;
    }

    std::size_t capability_count() const { return index_.size(); }

private:
    CapabilityIndex index_;
    std::vector<DerivationRule> rules_;
};

} // namespace ravel::proof::truth
