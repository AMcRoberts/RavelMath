#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "ravel/proof/first_return_completeness_automation.hpp"
#include "ravel/proof/first_return_dimension_extension.hpp"
#include "ravel/proof/first_return_joint_product.hpp"
#include "ravel/proof/first_return_joint_role_quotient.hpp"
#include "ravel/proof/first_return_universal_composition.hpp"
#include "ravel/proof/first_return_proof_strategist.hpp"
#include "ravel/proof/residual_signature_transfer.hpp"
#include "ravel/proof/symbolic_residual_formula.hpp"
#include "ravel/proof/realized_first_return_completeness.hpp"
#include "ravel/proof/reflective_boundary_grammar.hpp"

namespace {
std::vector<std::filesystem::path> default_files(std::size_t n) {
    std::vector<std::filesystem::path> files;
    files.emplace_back("out/cover_tube_defect_splice/n" + std::to_string(n) + "_M2.json");
    if (n == 3) files.emplace_back("out/cover_tube_defect_splice/n3_M3.json");
    return files;
}
}

int main(int argc, char** argv) {
    using namespace ravel::proof;
    try {
        if (argc < 2) {
            std::cerr << "usage: ravel-prove <first-return-completeness|first-return-joint-product|first-return-joint-quotient|first-return-dimension-extension|first-return-universal|first-return-moonshot|residual-signature-transfer|symbolic-residual-formula> ...\n";
            return 64;
        }
        const std::string command = argv[1];
        if (command == "derive-boundary-grammar") {
            const std::size_t max_n = argc > 2 ? std::stoull(argv[2]) : 32;
            const std::filesystem::path lean_out = argc > 3
                ? argv[3]
                : "lean/generated/reflective_boundary_grammar.lean";
            std::uint64_t laws = 0;
            for (std::size_t n = 3; n <= max_n; ++n) {
                for (std::size_t d = 1; d <= n; ++d) {
                    laws += derive_boundary_queue_grammar(n, d).laws.size();
                }
            }
            std::filesystem::create_directories(lean_out.parent_path());
            std::ofstream out(lean_out);
            out << emit_boundary_grammar_lean();
            std::cout << "REFLECTIVE_BOUNDARY_GRAMMAR\n"
                      << "max_dimension=" << max_n << "\n"
                      << "derived_laws=" << laws << "\n"
                      << "lean=" << lean_out << "\n";
            return out ? 0 : 3;
        }
        if (command == "realized-first-return-completeness-lean") {
            const std::filesystem::path lean_out = argc > 2 ? argv[2] : "lean/generated/realized_first_return_completeness.lean";
            std::filesystem::create_directories(lean_out.parent_path());
            std::ofstream out(lean_out);
            out << render_realized_first_return_completeness_lean();
            std::cout << "lean=" << lean_out << "\n";
            return out ? 0 : 3;
        }
        if (command == "symbolic-residual-formula") {
            const std::size_t n = argc > 2 ? std::stoull(argv[2]) : 3;
            const std::filesystem::path lean_out = argc > 3
                ? argv[3]
                : "lean/generated/generic_residual_formula.lean";
            const auto product = build_first_return_joint_product_from_files(n, default_files(n));
            const auto proof = derive_symbolic_residual_formula_proof(product);
            std::cout << render_symbolic_residual_formula_report(proof);
            if (proof.valid) {
                std::filesystem::create_directories(lean_out.parent_path());
                std::ofstream out(lean_out);
                out << render_generic_residual_formula_lean();
                std::cout << "lean=" << lean_out << "\n";
            }
            return proof.valid ? 0 : 3;
        }
        if (command == "residual-signature-transfer") {
            const std::size_t n = argc > 2 ? std::stoull(argv[2]) : 3;
            const auto product = build_first_return_joint_product_from_files(n, default_files(n));
            const auto proof = derive_residual_signature_transfer(product);
            std::cout << render_residual_signature_transfer_report(proof);
            return proof.valid ? 0 : 3;
        }
        if (command == "first-return-joint-product") {
            const std::size_t n = argc > 2 ? std::stoull(argv[2]) : 3;
            auto files = default_files(n);
            const auto product = build_first_return_joint_product_from_files(n, files);
            std::cout << "FIRST_RETURN_JOINT_PRODUCT\n"
                      << "dimension=" << product.dimension << "\n"
                      << "witnesses=" << product.witness_count << "\n"
                      << "reachable_joint_states=" << product.reachable.size() << "\n"
                      << "transitions=" << product.transitions.size() << "\n"
                      << "replayed=" << (product.replayed ? "true" : "false") << "\n";
            if (!product.failure.empty()) std::cout << "failure=" << product.failure << "\n";
            return product.replayed ? 0 : 3;
        }
        if (command == "first-return-joint-quotient") {
            const std::size_t n = argc > 2 ? std::stoull(argv[2]) : 3;
            const auto product = build_first_return_joint_product_from_files(n, default_files(n));
            const auto quotient = synthesize_reachable_joint_role_quotient(product);
            std::cout << render_joint_role_quotient_report(quotient);
            return quotient.exact_on_reachable_product ? 0 : 3;
        }
        if (command == "first-return-dimension-extension") {
            const std::size_t n = argc > 2 ? std::stoull(argv[2]) : 3;
            const auto lower_product = build_first_return_joint_product_from_files(n, default_files(n));
            const auto upper_product = build_first_return_joint_product_from_files(n + 1, default_files(n + 1));
            const auto lower = synthesize_reachable_joint_role_quotient(lower_product);
            const auto upper = synthesize_reachable_joint_role_quotient(upper_product);
            const auto extension = synthesize_dimension_extension(lower_product, lower, upper_product, upper);
            std::cout << render_dimension_extension_report(extension);
            return extension.proved ? 0 : 3;
        }
        if (command == "first-return-moonshot") {
            const std::size_t min_n = argc > 2 ? std::stoull(argv[2]) : 3;
            const std::size_t max_n = argc > 3 ? std::stoull(argv[3]) : 4;
            const std::filesystem::path root = argc > 4 ? argv[4] : "out/first_return_moonshot";
            bool all_fixed = true;
            for (std::size_t n = min_n; n <= max_n; ++n) {
                const auto product = build_first_return_joint_product_from_files(n, default_files(n));
                const auto quotient = synthesize_reachable_joint_role_quotient(product);
                const auto result = run_first_return_proof_strategist(product, quotient, root / ("n" + std::to_string(n)));
                std::cout << render_joint_role_quotient_report(quotient)
                          << render_first_return_strategy_report(result);
                all_fixed = all_fixed && result.fixed_dimension_relational_induction;
            }
            return all_fixed ? 0 : 3;
        }
        if (command == "first-return-universal") {
            const auto lower_product = build_first_return_joint_product_from_files(3, default_files(3));
            const auto upper_product = build_first_return_joint_product_from_files(4, default_files(4));
            const auto lower = synthesize_reachable_joint_role_quotient(lower_product);
            const auto upper = synthesize_reachable_joint_role_quotient(upper_product);
            const auto extension = synthesize_dimension_extension(lower_product, lower, upper_product, upper);
            const auto composition = compose_first_return_universal_proof(lower, extension);
            std::cout << render_joint_role_quotient_report(lower)
                      << render_joint_role_quotient_report(upper)
                      << render_dimension_extension_report(extension)
                      << render_universal_composition_report(composition);
            return composition.universal_theorem_emitted ? 0 : 3;
        }
        if (command != "first-return-completeness") {
            std::cerr << "unknown command: " << command << "\n";
            return 64;
        }
        std::size_t min_n = argc > 2 ? std::stoull(argv[2]) : 3;
        std::size_t max_n = argc > 3 ? std::stoull(argv[3]) : 4;
        std::filesystem::path checkpoint = argc > 4
            ? argv[4]
            : "out/first_return_completeness.checkpoint";

        auto result = run_first_return_completeness_automation(min_n, max_n);
        write_automation_checkpoint(result, checkpoint);
        std::cout << render_automation_report(result);
        std::cout << "checkpoint=" << checkpoint << "\n";
        return result.universal_theorem_emitted ? 0 : 3;
    } catch (const std::exception& e) {
        std::cerr << "ravel-prove: " << e.what() << '\n';
        return 70;
    }
}
