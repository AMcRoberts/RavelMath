#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include "ravel/proof/symbolic_residual_induction.hpp"

int main(int argc, char** argv) {
    using namespace ravel::proof;
    std::size_t n = 3;
    std::filesystem::path output = "lean/generated/first_return_residual_induction.lean";
    if (argc >= 2) n = static_cast<std::size_t>(std::stoull(argv[1]));
    if (argc >= 3) output = argv[2];

    const auto proof = derive_residual_induction_proof(n);
    std::cout << "dimension=" << n << " states=" << proof.states.size()
              << " residuals=" << proof.concrete_family.members.size()
              << " quotient=" << proof.quotient_members.size() << "\n";
    if (!proof.valid) {
        std::cerr << "PROOF FAILED: " << proof.failure << "\n";
        return 2;
    }
    std::filesystem::create_directories(output.parent_path());
    std::ofstream out(output);
    out << render_residual_induction_lean(proof);
    if (!out) throw std::runtime_error("failed to write Lean proof");
    std::cout << "PROOF EMITTED " << output << "\n";
    return 0;
}
