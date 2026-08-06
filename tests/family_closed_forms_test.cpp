#include <cassert>
#include <cstddef>
#include <stdexcept>

#include "ravel/family_closed_forms.hpp"

int main() {
    // Wide exact comparison against the independent generic neighbor generator.
    for (std::size_t a = 1; a <= 4096; ++a) {
        for (std::size_t b : {std::size_t{1}, std::size_t{2}, std::size_t{3}, std::size_t{7}, std::size_t{31}}) {
            assert(ravel::class_ii_adjacent_swap_count_closed_form(a, b) == 3);
            assert(ravel::certify_class_ii_adjacent_swap_count(a, b));
        }
    }

    for (std::size_t n = 2; n <= 127; ++n) {
        assert(ravel::nbonacci_adjacent_swap_count_closed_form(n) == n - 1);
        assert(ravel::certify_nbonacci_adjacent_swap_count(n));
    }

    bool threw = false;
    try { (void)ravel::class_ii_adjacent_swap_count_closed_form(0, 1); }
    catch (const std::invalid_argument&) { threw = true; }
    assert(threw);

    threw = false;
    try { (void)ravel::nbonacci_adjacent_swap_count_closed_form(1); }
    catch (const std::invalid_argument&) { threw = true; }
    assert(threw);
}
