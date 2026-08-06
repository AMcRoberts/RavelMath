#include "ravel/proof/lua_strategy_loader.hpp"

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace fs = std::filesystem;
using ravel::proof::UniversalProofPlan;
using ravel::proof::characteristic_minor_maps_spec;
using ravel::proof::emit_index_bijection_lean;
using ravel::proof::validate_index_bijection;

namespace {

struct Options {
    fs::path lua_root = "lua/lua_src";
    fs::path run_root = "out/shoot_the_moon";
    std::string schema = "ravel.proof.nbonacci_charpoly_schema";
    std::string strategy = "ravel.proof.nbonacci_universal_strategy_tree";
    bool execute_foundation = false;
};

std::string timestamp() {
    const auto now = std::chrono::system_clock::now();
    const std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
#if defined(_WIN32)
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    std::ostringstream ss;
    ss << std::put_time(&tm, "%Y%m%d-%H%M%S");
    return ss.str();
}

std::string lua_quote(const std::string& s) {
    std::string out = "\"";
    for (char ch : s) {
        if (ch == '\\' || ch == '"') out.push_back('\\');
        if (ch == '\n') out += "\\n"; else out.push_back(ch);
    }
    out += '"';
    return out;
}

Options parse(int argc, char** argv) {
    Options o;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        auto value = [&](const std::string& prefix) { return arg.substr(prefix.size()); };
        if (arg.starts_with("--lua-root=")) o.lua_root = value("--lua-root=");
        else if (arg.starts_with("--run-root=")) o.run_root = value("--run-root=");
        else if (arg.starts_with("--schema=")) o.schema = value("--schema=");
        else if (arg.starts_with("--strategy=")) o.strategy = value("--strategy=");
        else if (arg == "--execute-foundation") o.execute_foundation = true;
        else if (arg == "--plan-only") o.execute_foundation = false;
        else throw std::runtime_error("unknown argument: " + arg);
    }
    return o;
}

void write_index_bijection_artifact(const fs::path& run_dir) {
    const auto spec = characteristic_minor_maps_spec();
    validate_index_bijection(spec, 12);
    std::ofstream lean(run_dir / "lean/generated/characteristic_minor_maps_direct.lean");
    lean << emit_index_bijection_lean(spec);
    std::ofstream report(run_dir / "reports/characteristic_minor_maps_direct.txt");
    report << "course: " << spec.id << "\n";
    report << "validator: " << spec.validator << "\n";
    report << "emitter: " << spec.emitter << "\n";
    report << "regression_max: 12\nstatus: PASS\n";
}

void write_plan(const UniversalProofPlan& plan, const fs::path& run_dir) {
    fs::create_directories(run_dir / "lean/generated");
    fs::create_directories(run_dir / "lua/generated");
    fs::create_directories(run_dir / "logs");

    std::ofstream md(run_dir / "NEXT_MOVES.md");
    md << "# Universal n-bonacci proof: generated move tree\n\n";
    md << "Schema: `" << plan.schema_id << "`  \nStrategy: `" << plan.strategy_id << "`\n\n";
    md << "The first course under each obligation is the default attack. On a classified failure, follow its listed pivots in order. Preserve every failed artifact.\n\n";
    for (const auto& obligation : plan.obligations) {
        const auto& strategy = plan.strategies.at(obligation.id);
        md << "## " << obligation.id << "\n\n" << strategy.objective << "\n\n";
        md << "Depends on: ";
        if (obligation.dependencies.empty()) md << "none";
        for (std::size_t i = 0; i < obligation.dependencies.size(); ++i)
            md << (i ? ", " : "") << '`' << obligation.dependencies[i] << '`';
        md << "\n\n";
        for (std::size_t i = 0; i < strategy.courses.size(); ++i) {
            const auto& c = strategy.courses[i];
            md << "### " << (i + 1) << ". " << c.title << " (`" << c.id << "`)\n\n";
            md << c.method << "\n\n**Success gate:** " << c.success << "\n\n";
            if (!c.failure_signatures.empty()) {
                md << "**Recognized failures:** ";
                for (std::size_t j = 0; j < c.failure_signatures.size(); ++j)
                    md << (j ? "; " : "") << c.failure_signatures[j];
                md << "\n\n";
            }
            if (!c.next_on_failure.empty()) {
                md << "**Pivot on failure:** ";
                for (std::size_t j = 0; j < c.next_on_failure.size(); ++j)
                    md << (j ? " -> " : "") << '`' << c.next_on_failure[j] << '`';
                md << "\n\n";
            }
        }
    }

    std::ofstream state(run_dir / "run_state.lua");
    state << "return {\n  schema = " << lua_quote(plan.schema_id)
          << ",\n  strategy = " << lua_quote(plan.strategy_id) << ",\n  obligations = {\n";
    for (const auto& obligation : plan.obligations) {
        const auto& first = plan.strategies.at(obligation.id).courses.front();
        state << "    [" << lua_quote(obligation.id) << "] = { status = \"ready\", active_course = "
              << lua_quote(first.id) << ", attempts = {} },\n";
    }
    state << "  },\n}\n";

    std::ofstream queue(run_dir / "COMMAND_QUEUE.txt");
    queue << "# Foundation commands (run from repository root)\n"
          << "make ravel_proof_runner\n"
          << "lua5.4 lua/scripts/run_lua_tests.lua\n"
          << "make nbonacci_covering_witness_test\n"
          << "make nbonacci_charmpoly_proof_general\n"
          << "rg -n 'sorry|admit|axiom' out/*.lean lean || true\n"
          << "make lean-check\n";
}

int run_command(const fs::path& run_dir, const std::string& name, const std::string& command) {
    const fs::path log = run_dir / "logs" / (name + ".log");
    const std::string wrapped = command + " > " + log.string() + " 2>&1";
    const int rc = std::system(wrapped.c_str());
    std::ofstream summary(run_dir / "reports" / (name + ".status.txt"));
    summary << "command: " << command << "\nexit_code: " << rc << "\nlog: " << log.string() << "\n";
    return rc;
}

void execute_foundation(const fs::path& run_dir) {
    struct Step { const char* name; const char* command; bool required; };
    const Step steps[] = {
        {"lua_schema_tests", "lua5.4 lua/scripts/run_lua_tests.lua", true},
        {"covering_witness_regression", "make nbonacci_covering_witness_test", true},
        {"finite_charpoly_generator", "make nbonacci_charmpoly_proof_general", true},
        {"proof_hole_scan", "sh -c \"! rg -n '^[[:space:]]*(sorry|admit|axiom)([[:space:]]|$)' out/*.lean lean --glob '!free_involution_perron_existence_draft.lean'\"", true},
        {"lean_check", "make lean-check", false},
    };
    std::ofstream frontier(run_dir / "FRONTIER.md");
    frontier << "# Execution frontier\n\n";
    for (const auto& step : steps) {
        const int rc = run_command(run_dir, step.name, step.command);
        frontier << "- " << step.name << ": " << (rc == 0 ? "PASS" : "BLOCKED")
                 << " (`reports/" << step.name << ".status.txt`)\n";
        if (rc != 0 && step.required) {
            frontier << "\nFirst required foundation failure: **" << step.name
                     << "**. Repair it before trusting downstream proof-generation results.\n";
            break;
        }
    }
}

} // namespace

int main(int argc, char** argv) {
    try {
        const Options options = parse(argc, argv);
        const auto plan = ravel::proof::load_universal_proof_plan(
            options.lua_root, options.schema, options.strategy);
        ravel::proof::validate_universal_proof_plan(plan);
        const fs::path run_dir = options.run_root / timestamp();
        write_plan(plan, run_dir);
        write_index_bijection_artifact(run_dir);
        if (options.execute_foundation) execute_foundation(run_dir);
        std::cout << "Validated " << plan.obligations.size() << " obligations and wrote run package to "
                  << run_dir << "\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "ravel-proof-runner: " << e.what() << "\n";
        return 1;
    }
}
