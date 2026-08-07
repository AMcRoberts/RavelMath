// Cross-checks the constant-last-letter-forces-depth-1 certificate
// (the exact dual of Finding 17, proving the coincidence half of
// Finding 37's Barge-class non-unit examples rather than resting on
// their empirical depth-1 observation) against the REAL
// pair_has_coincidence function -- on all 10 of Finding 37's actual
// substitutions, plus a synthetic case with a different constant
// letter and a synthetic no-constant-last-letter control.

#include <cassert>
#include <iostream>

#include "adelic/coincidence_and_property_f.hpp"
#include "ravel/proof/constant_last_letter_forces_depth1_coincidence.hpp"

using namespace ravel::proof;

namespace {

template <std::size_t d>
void check(const char* name, const std::array<std::vector<long long>, d>& images, bool expect_constant_last) {
    auto cert = check_constant_last_letter_forces_depth1<d>(images);
    assert(cert.has_constant_last_letter == expect_constant_last);
    if (cert.has_constant_last_letter) {
        for (std::size_t i = 0; i < d; ++i)
            for (std::size_t j = i + 1; j < d; ++j)
                assert(adelic::pair_has_coincidence<d>(images[i], images[j]));
    }
    std::cout << name << ": has_constant_last_letter=" << cert.has_constant_last_letter << " OK\n";
}

}  // namespace

int main() {
    // Finding 37's own 10 non-unit Pisot substitutions in Barge's
    // class -- all genuinely satisfy "constant last letter" (that's
    // half of Barge's own hypothesis), and all showed depth-1
    // coincidence empirically. Now proved, not just observed.
    check<4>("ex1", std::array<std::vector<long long>, 4>{
        std::vector<long long>{2,3,1,0}, std::vector<long long>{1,0,3,0},
        std::vector<long long>{3,3,0,0}, std::vector<long long>{0,3,0}}, true);

    check<4>("ex2", std::array<std::vector<long long>, 4>{
        std::vector<long long>{3,2,0,0}, std::vector<long long>{2,3,3,0,0},
        std::vector<long long>{1,3,3,2,0}, std::vector<long long>{0,1,3,0}}, true);

    check<4>("ex3", std::array<std::vector<long long>, 4>{
        std::vector<long long>{3,1,0,0}, std::vector<long long>{1,2,0},
        std::vector<long long>{0,2,0}, std::vector<long long>{2,0,2,0}}, true);

    check<4>("ex4", std::array<std::vector<long long>, 4>{
        std::vector<long long>{3,1,2,0}, std::vector<long long>{2,1,2,0},
        std::vector<long long>{0,2,2,0}, std::vector<long long>{1,2,0}}, true);

    check<4>("ex5", std::array<std::vector<long long>, 4>{
        std::vector<long long>{2,1,0}, std::vector<long long>{3,2,0},
        std::vector<long long>{0,3,0}, std::vector<long long>{1,3,2,0}}, true);

    check<4>("ex6", std::array<std::vector<long long>, 4>{
        std::vector<long long>{3,0,1,0}, std::vector<long long>{1,3,2,0},
        std::vector<long long>{2,3,0}, std::vector<long long>{0,0,0}}, true);

    check<4>("ex7", std::array<std::vector<long long>, 4>{
        std::vector<long long>{0,1,2,0}, std::vector<long long>{2,3,2,0},
        std::vector<long long>{1,0,3,0}, std::vector<long long>{3,1,0}}, true);

    check<4>("ex8", std::array<std::vector<long long>, 4>{
        std::vector<long long>{2,3,1,0}, std::vector<long long>{0,1,3,0},
        std::vector<long long>{1,0,1,0}, std::vector<long long>{3,1,0}}, true);

    check<5>("ex9", std::array<std::vector<long long>, 5>{
        std::vector<long long>{1,1,3,0}, std::vector<long long>{2,4,0},
        std::vector<long long>{0,4,4,0}, std::vector<long long>{3,1,0},
        std::vector<long long>{4,3,0}}, true);

    check<5>("ex10", std::array<std::vector<long long>, 5>{
        std::vector<long long>{2,4,3,0}, std::vector<long long>{4,1,0,0},
        std::vector<long long>{3,4,4,0}, std::vector<long long>{1,2,3,0},
        std::vector<long long>{0,1,0}}, true);

    // Synthetic case, constant last letter = 2 (not 0), confirming
    // the theorem isn't secretly about letter 0.
    {
        std::array<std::vector<long long>, 4> images = {
            std::vector<long long>{1, 3, 2},
            std::vector<long long>{0, 2},
            std::vector<long long>{3, 1, 0, 2},
            std::vector<long long>{3, 2},
        };
        auto cert = check_constant_last_letter_forces_depth1<4>(images);
        assert(cert.has_constant_last_letter && cert.constant_letter == 2);
        for (std::size_t i = 0; i < 4; ++i)
            for (std::size_t j = i + 1; j < 4; ++j)
                assert(adelic::pair_has_coincidence<4>(images[i], images[j]));
        std::cout << "synthetic constant-last-letter=2: predicted and actual both depth-1. OK\n";
    }

    // Synthetic control: no constant last letter, certificate must
    // correctly decline.
    {
        std::array<std::vector<long long>, 3> images = {
            std::vector<long long>{0, 1},
            std::vector<long long>{1, 0},
            std::vector<long long>{2, 0, 1},
        };
        auto cert = check_constant_last_letter_forces_depth1<3>(images);
        assert(!cert.has_constant_last_letter);
        std::cout << "synthetic no-constant-last-letter case: certificate correctly declines. OK\n";
    }

    std::cout << "constant_last_letter_forces_depth1_coincidence: all checks pass.\n";
    return 0;
}
