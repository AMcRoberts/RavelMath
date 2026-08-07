// Locks in Finding 35: g>1 (a genuine, nontrivial gcd-obstruction)
// implies the substitution's incidence matrix is NOT primitive, and
// therefore cannot be Pisot. Verified exactly (Wielandt-bound boolean
// matrix powers, no floating point) via math/perron_frobenius.hpp's
// certified is_primitive, on two independently-constructed g>1
// examples plus the g=1 control that has been used as this project's
// "genuine Pisot" reference case since Finding 26.

#include <array>
#include <cassert>
#include <iostream>
#include <numeric>
#include <vector>

#include "math/perron_frobenius.hpp"
#include "ravel/proof/coincidence_closure.hpp"

using namespace mathlib;
using namespace ravel::proof;

namespace {

template <std::size_t d>
long long jump_gcd(const std::array<std::vector<long long>, d>& images) {
    auto edges = build_junction_graph<d>(images);
    long long g = 0;
    for (auto& e : edges) g = std::gcd(g, e.jump_size);
    return g;
}

template <std::size_t d>
std::vector<std::vector<long long>> incidence(const std::array<std::vector<long long>, d>& images) {
    std::vector<std::vector<long long>> M(d, std::vector<long long>(d, 0));
    for (long long j = 0; j < static_cast<long long>(d); ++j)
        for (auto c : images[static_cast<std::size_t>(j)]) M[static_cast<std::size_t>(c)][static_cast<std::size_t>(j)] += 1;
    return M;
}

}  // namespace

int main() {
    // g=1 control: the genuine Pisot single-junction example
    // (x^3-2x^2-x+1's substitution) used since Finding 26.
    {
        std::array<std::vector<long long>, 3> images = {
            std::vector<long long>{0, 0, 1}, std::vector<long long>{2}, std::vector<long long>{0, 1}};
        assert(jump_gcd<3>(images) == 1);
        auto M = incidence<3>(images);
        assert(is_irreducible(M));
        assert(is_primitive(M));
    }

    // g=2 multi-junction example, used throughout Findings 25-27/34.
    {
        std::array<std::vector<long long>, 8> images = {
            std::vector<long long>{1, 3}, std::vector<long long>{2}, std::vector<long long>{4, 5},
            std::vector<long long>{0}, std::vector<long long>{0}, std::vector<long long>{6},
            std::vector<long long>{7}, std::vector<long long>{0}};
        assert(jump_gcd<8>(images) == 2);
        auto M = incidence<8>(images);
        assert(is_irreducible(M));
        assert(!is_primitive(M));  // the theorem's claim, checked exactly
    }

    // A second, independently-constructed g>1 example (g=4), so the
    // theorem isn't trusted on a single instance.
    {
        std::array<std::vector<long long>, 7> images = {
            std::vector<long long>{1, 4}, std::vector<long long>{2}, std::vector<long long>{3},
            std::vector<long long>{0}, std::vector<long long>{5}, std::vector<long long>{6},
            std::vector<long long>{0}};
        long long g = jump_gcd<7>(images);
        assert(g > 1);
        auto M = incidence<7>(images);
        assert(is_irreducible(M));
        assert(!is_primitive(M));
    }

    std::cout << "gcd_obstruction_implies_nonpisot: g>1 => imprimitive, exactly as proven, "
                 "on two independent g>1 examples; g=1 control is primitive.\n";
    return 0;
}
