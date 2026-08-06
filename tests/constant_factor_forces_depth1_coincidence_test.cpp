// Cross-checks the constant-factor-forces-depth-1 certificate against
// the REAL check_strong_coincidence search, not just the hand-derived
// premise -- on Finding 5's whole AR-partial/non-AR family, plus a
// synthetic 4-letter case with a different constant letter (2, not 0)
// and a synthetic case with NO constant factor at all, to confirm the
// theorem is general rather than an artifact of sigma_{a,b}'s shape.

#include <cassert>
#include <iostream>

#include "adelic/coincidence_and_property_f.hpp"
#include "ravel/proof/constant_factor_forces_depth1_coincidence.hpp"

using namespace ravel::proof;

std::array<std::vector<long long>, 3> sigma_ab(int a, int b) {
    std::array<std::vector<long long>, 3> s;
    for (int i = 0; i < a; ++i) s[0].push_back(0);
    for (int i = 0; i < b; ++i) s[0].push_back(1);
    s[0].push_back(2);
    for (int i = 0; i < a; ++i) s[1].push_back(0);
    s[1].push_back(2);
    s[2] = {0};
    return s;
}

int main() {
    // Finding 5's family: a=0 is non-AR (no constant factor), a=1..5
    // is AR-partial (constant factor). The certificate must predict
    // depth-1 resolution exactly where it actually occurs.
    for (int a = 0; a <= 5; ++a) {
        auto images = sigma_ab(a, 1);
        auto cert = check_constant_factor_forces_depth1<3>(images);

        bool actually_all_depth1 = true;
        for (std::size_t i = 0; i < 3; ++i)
            for (std::size_t j = i + 1; j < 3; ++j)
                if (!adelic::pair_has_coincidence<3>(images[i], images[j])) actually_all_depth1 = false;

        std::cout << "a=" << a << ": has_constant_factor=" << cert.has_constant_factor
                  << " actual_all_depth1=" << actually_all_depth1 << "\n";
        // The theorem is one-directional (constant factor => depth 1),
        // so only that implication is asserted, not the converse.
        if (cert.has_constant_factor) assert(actually_all_depth1);
        // But for THIS specific family we also know empirically the
        // converse holds (a=0 is the only one without it and the
        // only one that needs depth > 1) -- checked, not assumed.
        assert(cert.has_constant_factor == actually_all_depth1);
    }

    // Synthetic 4-letter case, constant letter = 2 (not 0), to confirm
    // the theorem isn't secretly about letter 0 specifically.
    {
        std::array<std::vector<long long>, 4> images = {
            std::vector<long long>{2, 1, 3},
            std::vector<long long>{2, 0},
            std::vector<long long>{2, 2, 1, 0},
            std::vector<long long>{2, 3},
        };
        auto cert = check_constant_factor_forces_depth1<4>(images);
        assert(cert.has_constant_factor && cert.constant_letter == 2);
        for (std::size_t i = 0; i < 4; ++i)
            for (std::size_t j = i + 1; j < 4; ++j)
                assert(adelic::pair_has_coincidence<4>(images[i], images[j]));
        std::cout << "synthetic 4-letter, constant letter 2: predicted and actual both depth-1. OK\n";
    }

    // Synthetic case with NO constant factor, confirming the
    // certificate correctly declines to predict depth-1 (whether or
    // not coincidence happens to still hold some other way).
    {
        std::array<std::vector<long long>, 3> images = {
            std::vector<long long>{0, 1},
            std::vector<long long>{1, 0},
            std::vector<long long>{2, 0, 1},
        };
        auto cert = check_constant_factor_forces_depth1<3>(images);
        assert(!cert.has_constant_factor);
        std::cout << "synthetic no-constant-factor case: certificate correctly declines. OK\n";
    }

    std::cout << "constant_factor_forces_depth1_coincidence: all checks pass.\n";
    return 0;
}
