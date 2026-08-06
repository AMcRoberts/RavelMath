#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include "ravel/proof/typed_theorem_application.hpp"

int main(int argc, char** argv) {
    std::string output = "out/truth_machine_fast_power_reduction.lean";
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg.rfind("--output=", 0) == 0) output = arg.substr(9);
    }
    using namespace ravel::proof::truth;
    ApplicationRequest request;
    request.rule_id = "charpoly-to-fast-power-reduction";
    request.hypotheses.emplace("charpoly_identity", TypedHypothesis{
        "charpoly_identity", "hchar", "A.charpoly = nbonacciCharpoly n"});
    auto result = TypedTheoremApplicator{}.apply(request);
    if (!result.closed) {
        for (const auto& missing : result.missing) std::cerr << "missing: " << missing << "\n";
        return 2;
    }
    std::ofstream out(output);
    if (!out) throw std::runtime_error("cannot write " + output);
    out << result.lean_source;
    std::cout << "wrote " << output << "\n";
    return 0;
}
