#include <iostream>
#include <stdexcept>

#include "ravel/proof/generalized_multinacci_prefix_phase.hpp"

int main() {
    using namespace ravel::proof;

    const auto nbonacci = derive_generalized_multinacci_prefix_phase(1);
    if (!nbonacci.proved || nbonacci.macro_generator_count != 2)
        throw std::runtime_error("m=1 prefix grammar did not reduce exactly to Q/R");
    if (prefix_primitive_word_string(nbonacci.macro_as_primitive_word[0]) != "Q" ||
        prefix_primitive_word_string(nbonacci.macro_as_primitive_word[1]) != "R")
        throw std::runtime_error("m=1 Q/R words are wrong");

    const auto doubled = derive_generalized_multinacci_prefix_phase(2);
    if (!doubled.proved)
        throw std::runtime_error("m=2 prefix phase proof failed: " + doubled.obstruction);
    if (doubled.prefix_state_count != 3 || doubled.channel_count != 9 ||
        doubled.macro_generator_count != 3)
        throw std::runtime_error("m=2 exact prefix enumeration is wrong");
    if (prefix_primitive_word_string(doubled.macro_as_primitive_word[0]) != "Q" ||
        prefix_primitive_word_string(doubled.macro_as_primitive_word[1]) != "R" ||
        prefix_primitive_word_string(doubled.macro_as_primitive_word[2]) != "RR")
        throw std::runtime_error("m=2 macro-to-Q/R reduction is wrong");

    std::size_t zero = 0, one = 0, two = 0;
    for (const auto& c : doubled.channels) {
        if (c.macro_generator == 0) ++zero;
        else if (c.macro_generator == 1) ++one;
        else if (c.macro_generator == 2) ++two;
        else throw std::runtime_error("unexpected m=2 defect magnitude");
    }
    if (zero != 3 || one != 4 || two != 2)
        throw std::runtime_error("m=2 macro multiplicities are wrong");

    const auto tripled = derive_generalized_multinacci_prefix_phase(3);
    if (!tripled.proved ||
        prefix_primitive_word_string(tripled.macro_as_primitive_word[3]) != "RRR")
        throw std::runtime_error("general multiplicity lift failed");

    std::cout << "generalized multinacci prefix phase PASS\n";
}
