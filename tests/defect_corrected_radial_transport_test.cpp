#include <cassert>
#include <cstdint>
#include <iostream>
#include <vector>

#include "ravel/proof/defect_corrected_radial_transport.hpp"

using namespace ravel::proof;

static void enumerate(std::size_t n, std::size_t i,
                      std::vector<std::int64_t>& x,
                      std::size_t& checks) {
    if (i == n) {
        bool nonzero = false;
        for (auto v : x) nonzero |= v != 0;
        if (!nonzero) return;
        const auto cert = certify_defect_corrected_radial(x, n + 1);
        assert(cert.exact);
        for (std::size_t j = 0; j + 1 < n; ++j)
            assert(cert.formula_defect[j] ==
                   static_cast<std::int64_t>(n + 1) *
                   (signum(x[j + 1]) - signum(x[j])));
        ++checks;
        return;
    }
    for (auto v : {-2LL, 0LL, 3LL}) {
        x[i] = v;
        enumerate(n, i + 1, x, checks);
    }
}

int main() {
    std::size_t checks = 0;
    for (std::size_t n = 2; n <= 10; ++n) {
        std::vector<std::int64_t> x(n, 0);
        enumerate(n, 0, x, checks);
    }
    std::cout << "defect-corrected radial transport PASS checks=" << checks << "\n";
}
