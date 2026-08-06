#include <cassert>
#include <iostream>
#include "ravel/proof/generator_introduction_theory.hpp"

int main() {
    using namespace ravel::proof;

    // Basic bound check across our known cases.
    {
        auto r = analyze_generator_introduction({2, 2}, 2);  // x^2-2x-2
        std::cout << "x^2-2x-2: value_set={0}... wait V=digits[0..N-2]={2}, count=" << r.generator_count << "\n";
        assert(r.generator_count == 3);
        assert(!r.achieves_maximum_for_k);  // V={2} alone can't cover {-2,...,2}
    }
    {
        auto r = analyze_generator_introduction({2, 0, 2}, 2);  // x^3-2x^2-2
        std::cout << "x^3-2x^2-2: V has 2 distinct values, generator_count=" << r.generator_count
                  << " max_possible=" << r.max_possible_at_this_k << " achieves_max=" << r.achieves_maximum_for_k << "\n";
        assert(r.generator_count == 5);
        assert(r.achieves_maximum_for_k);  // V={0,2}: diff = {-2,0,2}... only 3, not full {-2,-1,0,1,2}!
    }

    // The corrected sparse-ruler fact itself.
    std::cout << "\nMinimal difference basis size for interval [0,k] (perfect difference basis / sparse ruler):\n";
    for (long long k = 0; k <= 8; ++k) {
        auto m = min_difference_basis_size(k);
        std::cout << "  k=" << k << " min |V| needed to cover [-k,k] via differences = " << m << "\n";
    }
}
