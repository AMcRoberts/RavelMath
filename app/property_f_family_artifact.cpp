#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

#include "adelic/property_f_family_autopsy.hpp"

int main(int argc, char** argv) {
    std::string output = "out/property_f_family.tsv";
    long long node_budget = 100'000;
    long long coincidence_budget = 1'000'000;
    if (argc > 1) output = argv[1];
    if (argc > 2) node_budget = std::stoll(argv[2]);
    if (argc > 3) coincidence_budget = std::stoll(argv[3]);

    const auto rows = adelic::run_property_f_family(node_budget, coincidence_budget);
    std::ofstream file(output);
    if (!file) {
        std::cerr << "property_f_family_artifact: cannot open " << output << '\n';
        return EXIT_FAILURE;
    }
    file << adelic::property_f_family_tsv_header();
    for (const auto& row : rows) file << adelic::property_f_family_tsv_row(row);
    std::cout << "wrote " << rows.size() << " family observations to " << output
              << " (node_budget=" << node_budget << ", coincidence_budget="
              << coincidence_budget << ")\n";
}
