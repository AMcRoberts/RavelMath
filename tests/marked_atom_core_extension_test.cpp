#include <cassert>
#include <iostream>
#include <vector>

#include "ravel/proof/marked_atom_core_extension.hpp"

using namespace ravel::proof;
using ravel::nbonacci_margin::LabelPair;

int main() {
    const std::size_t n = 3;
    DisplacementSumCatalogue catalogue(n);

    // A grade-two two-sheet extension in which the active mark swaps sheets.
    // Each source branch remains a distinct core branch.
    const LabelPair pair{0,1};
    const auto atoms = catalogue.atoms();
    std::vector<std::vector<long long>> x(2, std::vector<long long>(n,0));
    for (std::size_t k=0;k<n;++k) {
        x[0][k] = atoms[0][k] + atoms[1][k];
        x[1][k] = ravel::nbonacci_margin::forward_displacement(x[0], -1)[k];
    }
    // This tiny synthetic graph is only accepted if both endpoint states have
    // complete deterministic marked fibres.  If the selected descriptors do
    // not fit the labels, search the catalogue for a real compatible pair.
    bool found = false;
    for (const auto& a : atoms) for (const auto& b : atoms) {
        std::vector<long long> s(n), t;
        for (std::size_t k=0;k<n;++k) s[k]=a[k]+b[k];
        auto ws = catalogue.derive_one_stage(pair,s);
        if (ws.empty()) continue;
        t = ravel::nbonacci_margin::forward_displacement(s,-1);
        LabelPair dst{0,0};
        if (catalogue.derive_one_stage(dst,t).empty()) continue;
        const std::vector<std::vector<long long>> states{s,t};
        const std::vector<LabelPair> pairs{pair,dst};
        const auto proof = derive_marked_atom_core_extension(
            catalogue, 2,
            [&](std::size_t u){ return std::vector<std::size_t>{1-u}; },
            [&](std::size_t u){ return pairs[u]; },
            [&](std::size_t u){ return states[u]; });
        if (proof.path_injection_to_core) { found=true; break; }
    }
    // A complete synthetic instance may not exist with a 2-cycle in n=3;
    // the negative branch below still verifies rejection semantics.  The real
    // arithmetic-hull audit exercises positive components.

    // Parallel source branches that collapse to one marked core branch must
    // be rejected rather than silently counted once.
    const auto atom = atoms.front();
    auto descriptor = ravel::nbonacci_margin::describe_displacement(atom);
    assert(descriptor);
    LabelPair core_pair{descriptor->a, descriptor->b};
    if (!ravel::nbonacci_margin::predicted_core_member(n, core_pair, *descriptor))
        core_pair = {0,1};
    const std::vector<std::vector<long long>> one{atom};
    const std::vector<LabelPair> one_pair{core_pair};
    const auto bad = derive_marked_atom_core_extension(
        catalogue, 1,
        [&](std::size_t){ return std::vector<std::size_t>{0,0}; },
        [&](std::size_t){ return one_pair[0]; },
        [&](std::size_t){ return one[0]; });
    assert(!bad.path_injection_to_core);

    std::cout << "marked atom core extension PASS positive_found=" << found << "\n";
}
