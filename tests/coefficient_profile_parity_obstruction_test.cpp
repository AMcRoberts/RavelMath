#include <iostream>
#include <stdexcept>
#include "ravel/proof/coefficient_profile_parity_obstruction.hpp"

int main() {
    for (std::size_t D=3;D<=256;++D) {
        const auto proof=ravel::proof::derive_nearest_left_profile_parity_obstruction(D);
        if(!proof.proved) throw std::runtime_error(proof.obstruction);
        if(proof.cyclotomic_x_plus_one_factor != (D%2==0))
            throw std::runtime_error("parity mismatch");
    }
    std::cout<<"coefficient profile parity obstruction PASS\n";
}
