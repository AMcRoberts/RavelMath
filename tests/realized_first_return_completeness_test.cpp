#include <fstream>
#include <iostream>
#include <string>
#include "ravel/proof/realized_first_return_completeness.hpp"

int main() {
    using namespace ravel::proof;
    const auto lean = render_realized_first_return_completeness_lean();
    if (lean.find("theorem strictShellPump_of_realized_language") == std::string::npos) return 1;
    if (lean.find("theorem recurrent_radius_le_one_of_realized_language") == std::string::npos) return 2;
    std::ofstream out("lean/generated/realized_first_return_completeness.lean");
    out << lean;
    if (!out) return 3;
    std::cout << "realized first return completeness Lean emission PASS\n";
}
