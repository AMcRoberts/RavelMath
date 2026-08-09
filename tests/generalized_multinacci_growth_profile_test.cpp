#include <cassert>
#include <iostream>

#include "adelic/generalized_multinacci_growth_profile.hpp"

int main() {
    const auto small = adelic::derive_generalized_multinacci_growth_profile(4, 6);
    const auto high = adelic::derive_generalized_multinacci_growth_profile(7, 2);
    assert(small.return_channels == 19);
    assert(small.coefficient_dimension == 4);
    assert(small.secondary_modulus < 1.0);
    assert(high.return_channels == 13);
    assert(high.coefficient_dimension == 7);
    assert(high.secondary_modulus < 1.0);
    assert(high.inverse_secondary_gap > small.inverse_secondary_gap);
    // The profile is intentionally cheap enough to sweep the next evidence
    // sheet even when exact Property-F BFS is not resource-feasible.
    std::size_t sweep_count = 0;
    for (std::size_t d = 2; d <= 8; ++d) {
        for (std::size_t m = 1; m <= 6; ++m) {
            const auto profile =
                adelic::derive_generalized_multinacci_growth_profile(d, m);
            assert(profile.return_channels == (d - 1) * m + 1);
            assert(profile.coefficient_dimension == d);
            assert(profile.secondary_modulus < 1.0);
            ++sweep_count;
        }
    }
    std::cout << "(4,6) channels=" << small.return_channels
              << " rho=" << small.secondary_modulus
              << " gap-factor=" << small.inverse_secondary_gap << "\n"
              << "(7,2) channels=" << high.return_channels
              << " rho=" << high.secondary_modulus
              << " gap-factor=" << high.inverse_secondary_gap << "\n"
              << "profile sweep points=" << sweep_count << "\n"
              << "generalized multinacci growth profile PASS\n";
}
