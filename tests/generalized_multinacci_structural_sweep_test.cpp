#include <array>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "adelic/generalized_multinacci_block_transport.hpp"
#include "ravel/generalized_multinacci.hpp"
#include "ravel/proof/coincidence_closure.hpp"
#include "ravel/proof/generalized_multinacci_general_m.hpp"

template <std::size_t d>
void check_point(std::size_t m) {
    const auto raw = ravel::generalized_multinacci_rule(d, m);
    std::array<std::vector<long long>, d> images;
    std::array<std::array<long long, d>, d> matrix{};
    for (std::size_t j = 0; j < d; ++j) {
        for (const auto symbol : raw[j]) {
            images[j].push_back(symbol);
            ++matrix[symbol][j];
        }
    }
    if (!ravel::proof::check_strong_coincidence_closure<d>(
            images, matrix, 128, 1000000).holds)
        throw std::runtime_error("strong coincidence failed");
    if (!ravel::proof::derive_generalized_multinacci_general_m(m).proved)
        throw std::runtime_error("general-m scheduler failed");
    if (!adelic::derive_generalized_multinacci_block_transport(d, m).proved)
        throw std::runtime_error("return-block schema failed");
}

int main() {
    std::size_t points = 0;
    for (std::size_t d = 2; d <= 8; ++d) {
        for (std::size_t m = 1; m <= 6; ++m) {
            switch (d) {
                case 2: check_point<2>(m); break;
                case 3: check_point<3>(m); break;
                case 4: check_point<4>(m); break;
                case 5: check_point<5>(m); break;
                case 6: check_point<6>(m); break;
                case 7: check_point<7>(m); break;
                case 8: check_point<8>(m); break;
            }
            ++points;
        }
    }
    std::cout << "generalized multinacci structural sweep points=" << points
              << " PASS\n";
}
