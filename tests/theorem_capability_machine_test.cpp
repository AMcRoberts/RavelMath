#include <cassert>
#include <string>
#include "ravel/proof/theorem_capability_machine.hpp"

int main() {
    using namespace ravel::proof::truth;
    auto index = CapabilityIndex::load_tsv("config/theorem_capabilities.tsv");
    assert(index.size() >= 200);
    assert(index.find("RavelGenerated.nbonacci_universal_n") != nullptr);
    assert(index.find("nbonacci_block_identity_of_charpoly") != nullptr);
    assert(index.find("perron_column_difference") != nullptr);

    TruthMachine machine(std::move(index));
    auto universal = machine.ask("What is the universal nbonacci determinant polynomial?");
    assert(universal.has_kernel_checked_conclusion());
    assert(!universal.derivation_chains.empty());

    auto fast = machine.ask("Can the nbonacci charpoly reduce high matrix powers fast?");
    assert(!fast.derivation_chains.empty());
    bool fast_kernel = false;
    for (std::size_t i = 0; i < fast.derivation_chains.size(); ++i)
        if (fast.derivation_chains[i].rule_id == "charpoly-to-fast-power-reduction")
            fast_kernel = fast.derivation_kernel_checked[i];
    assert(!fast_kernel);

    auto perron = machine.ask("derive Perron coordinate gap order");
    assert(!perron.derivation_chains.empty());

    auto unknown = machine.ask("prove an unrelated elliptic curve conjecture");
    assert(!unknown.unresolved_obligations.empty());
    return 0;
}
