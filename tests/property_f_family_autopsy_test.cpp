#include <cassert>
#include <iostream>
#include <sstream>

#include "adelic/property_f_family_autopsy.hpp"

int main() {
    const auto rows = adelic::run_property_f_family(100'000, 1'000'000);
    // Three orderings of 001 and six combinations of 001/03 are the
    // complete first bounded family; the canonical forms are included.
    assert(rows.size() == 9);
    assert(rows.front().name == "first_anchor");
    assert(rows[3].name == "second_anchor");
    for (const auto& row : rows) {
        assert(!row.inconclusive);
        assert(row.coincidence_holds);
        assert(row.property_f_holds);
        assert(row.trusted_padic);
        assert(row.property_f_nonzero_cycles == 0);
        assert(row.property_f_terminal_sccs > 0);
        assert(row.property_f_max_escape_height >= 0);
        assert(row.property_f_transient_nodes >= 0);
        assert(row.property_f_height_mass >= 0);
        assert(row.property_f_max_scc_size > 0);
        assert(row.property_f_cyclic_sccs >= 0);
        assert(row.property_f_mixed_cycle_components == 0);
        assert(row.property_f_zero_cycle_components >= 0);
        assert(row.property_f_self_loops >= 0);
        assert(row.prefix_states > 0);
        assert(row.return_words >= 0);
        assert(row.return_phase_states >= 0);
        assert(row.return_phase_edges >= 0);
        assert(row.return_transport_closed || row.return_words == 0);
        if (row.return_transport_closed) {
            assert(row.return_phase_sccs > 0);
            assert(row.return_phase_cycle_components >= 0);
        }
        const auto serialized = adelic::property_f_family_tsv_row(row);
        assert(serialized.find(row.name) != std::string::npos);
    }
    assert(rows[0].property_f_nodes == 10);
    assert(rows[3].property_f_nodes == 4277);
    std::cout << adelic::property_f_family_tsv_header();
    for (const auto& row : rows) std::cout << adelic::property_f_family_tsv_row(row);
    std::cout << "property_f_family_autopsy: PASS (" << rows.size() << " rows)\n";
}
