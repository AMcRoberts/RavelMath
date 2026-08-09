#include <cassert>
#include <iostream>

#include "adelic/generalized_multinacci_cross_dimension.hpp"
#include "ravel/proof/generalized_multinacci_symbolic_embedding.hpp"

int main() {
    std::size_t dimension_lifts = 0;
    for (std::size_t d = 2; d <= 8; ++d) {
        for (std::size_t m = 1; m <= 6; ++m) {
            const auto sofic =
                ravel::proof::derive_generalized_multinacci_symbolic_embedding(d, m);
            assert(sofic.proved);
            assert(sofic.universal_suspension_embedding_all_dimensions);
            const auto lift =
                adelic::derive_generalized_multinacci_dimension_lift(d, m);
            assert(lift.proved);
            assert(lift.added_channels == m);
            ++dimension_lifts;
        }
    }
    std::size_t multiplicity_lifts = 0;
    for (std::size_t d = 2; d <= 8; ++d) {
        for (std::size_t m = 1; m < 6; ++m) {
            const auto lift =
                adelic::derive_generalized_multinacci_multiplicity_lift(d, m);
            assert(lift.proved);
            assert(lift.added_channels == d - 1);
            ++multiplicity_lifts;
        }
    }
    std::cout << "dimension lifts=" << dimension_lifts
              << " multiplicity lifts=" << multiplicity_lifts << "\n"
              << "generalized multinacci cross-dimension laws PASS\n";
}
