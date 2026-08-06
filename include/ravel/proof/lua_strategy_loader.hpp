#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace ravel::proof {

struct ProofObligation {
    std::string id;
    std::string kind;
    std::string statement;
    std::vector<std::string> dependencies;
};

struct PivotCourse {
    std::string id;
    std::string title;
    std::string method;
    std::string success;
    std::vector<std::string> outputs;
    std::vector<std::string> next_on_success;
    std::vector<std::string> next_on_failure;
    std::vector<std::string> failure_signatures;
};

struct ObligationStrategy {
    std::string objective;
    std::vector<PivotCourse> courses;
};

struct ReflectionConfig {
    bool enabled = false;
    std::string mode;
    std::vector<std::string> pivots;
    std::vector<std::string> outputs;
};

struct UniversalProofPlan {
    std::string schema_id;
    std::string strategy_id;
    std::vector<ProofObligation> obligations;
    std::unordered_map<std::string, ObligationStrategy> strategies;
    ReflectionConfig reflection;
};

enum class FinIndexNormalForm { CastSucc, Succ };

struct FinIndexBranch {
    std::string deleted;
    std::string source;
    std::string condition;
    FinIndexNormalForm normal_form;
};

struct IndexBijectionSpec {
    std::string id;
    std::string source_family;
    std::string target_family;
    std::string deletion;
    std::string validator;
    std::string emitter;
    std::string source_entry_rule;
    std::string q_entry_rule;
    std::string r_entry_rule;
    std::vector<std::string> q_cofactor_columns;
    std::vector<std::string> r_cofactor_columns;
    std::string characteristic_orientation;
};

UniversalProofPlan load_universal_proof_plan(
    const std::filesystem::path& lua_root,
    const std::string& schema_module,
    const std::string& strategy_module);

void validate_universal_proof_plan(const UniversalProofPlan& plan);

IndexBijectionSpec characteristic_minor_maps_spec();
void validate_index_bijection(const IndexBijectionSpec& spec, std::size_t regression_max);
std::string emit_index_bijection_lean(const IndexBijectionSpec& spec);

} // namespace ravel::proof
