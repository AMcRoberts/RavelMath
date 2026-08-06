// Thin trigger for the reflective proof machinery.
//
// This executable owns no matrix family, determinant algorithm, polynomial
// identity, pivot selection, or proof narration.  A Lua schema opts into the
// latent tracing woven through the exact math library.  The application merely
// presents an object to that library and writes the resulting provenance.

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>

#include "math/poly_matrix.hpp"
#include "math/proof_reflection.hpp"
#include "ravel/proof/reflection_declaration.hpp"
#include "ravel/proof/proof_campaign_engine.hpp"

namespace {

struct Options {
    std::filesystem::path schema_path = "lua/lua_src/ravel/proof/nbonacci_charpoly_schema.lua";
    std::size_t n = 8;
    std::string trace_out = "out/nbonacci_reflective_trace.txt";
    std::string lean_out = "out/nbonacci_reflective_trace.lean";
    std::string campaign_out = "out/nbonacci_proof_campaign.txt";
};

Options parse_options(int argc, char** argv) {
    Options options;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        const auto eq = arg.find('=');
        if (eq == std::string::npos) continue;
        const auto key = arg.substr(0, eq);
        const auto value = arg.substr(eq + 1);
        if (key == "--schema") options.schema_path = value;
        else if (key == "--n") options.n = std::stoull(value);
        else if (key == "--trace-out") options.trace_out = value;
        else if (key == "--lean-out") options.lean_out = value;
        else if (key == "--campaign-out") options.campaign_out = value;
    }
    if (options.n < 2) throw std::runtime_error("--n must be at least 2");
    return options;
}

void write_file(const std::string& path, const std::string& contents) {
    const auto parent = std::filesystem::path(path).parent_path();
    if (!parent.empty()) std::filesystem::create_directories(parent);
    std::ofstream out(path);
    if (!out) throw std::runtime_error("cannot write " + path);
    out << contents;
}

} // namespace

int main(int argc, char** argv) try {
    const auto options = parse_options(argc, argv);
    const auto declaration = ravel::proof::load_reflection_declaration(options.schema_path);
    if (!declaration.enabled)
        throw std::runtime_error("Lua schema did not enable mathematical reflection");

    mathlib::reflection::Trace trace("nbonacci.characteristic_polynomial.universal");
    {
        mathlib::reflection::ScopedTrace activate(&trace);
        const auto q = mathlib::nbonacci_q_matrix(options.n);
        const auto r = mathlib::nbonacci_r_matrix(options.n);
        const auto qdet = mathlib::det(q);
        const auto rdet = mathlib::det(r);
        const auto combined = mathlib::PolyZ({0, 1}) * qdet +
            ((options.n % 2 == 0) ? mathlib::PolyZ(1) : mathlib::PolyZ(-1)) * rdet;
        const auto expected = mathlib::nbonacci_charpoly_z(options.n);
        mathlib::reflection::record(mathlib::reflection::NodeKind::Certificate,
            mathlib::reflection::DeterminantIdentity{mathlib::reflection::no_node,
                combined == expected ? "nbonacci characteristic identity observed exactly"
                                     : "nbonacci characteristic identity failed"});
        if (combined != expected)
            throw std::runtime_error("reflected calculation did not satisfy the target identity");
    }

    ravel::proof::CampaignGenerator generator;
    ravel::proof::ProofCampaignExecutor executor;
    const auto campaign = executor.run(generator.generate(trace), trace);

    write_file(options.trace_out, trace.debug_report());
    write_file(options.campaign_out, campaign.report());
    write_file(options.lean_out, ravel::proof::render_closed_campaign_lean(campaign));
    std::printf("reflected %zu library events for n=%zu\nwrote %s\nwrote %s\nwrote %s\n",
                trace.events().size(), options.n,
                options.trace_out.c_str(), options.campaign_out.c_str(), options.lean_out.c_str());
    return 0; // execution completed; campaign report carries theorem-closure status
} catch (const std::exception& e) {
    std::fprintf(stderr, "nbonacci reflective trigger: %s\n", e.what());
    return 2;
}
