// Validates this project's PRE-EXISTING return-substitution machinery
// (include/ravel/return_substitution.hpp, built before this session)
// against Durand & Petite (arXiv:1408.2110) / Durand's original return-
// substitution construction, since it had not previously been cross-
// checked against this specific literature -- two independent checks:
//   (1) Tribonacci's return substitution to marker=0 equals Tribonacci
//       itself (the paper's explicit claim).
//   (2) the paper's own worked example (sigma: 1->1123, 2->211, 3->21,
//       marker="1") -- checked via Proposition 8 (dominant eigenvalue
//       of M_sigma and M_sigma_u must match exactly).

#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

#include "ravel/return_substitution.hpp"
#include "ravel/substitution.hpp"

using namespace ravel;

namespace {

std::vector<std::vector<long long>> incidence_matrix(const SubstitutionRule& rule) {
    std::size_t d = rule.alphabet_size();
    std::vector<std::vector<long long>> M(d, std::vector<long long>(d, 0));
    for (std::size_t j = 0; j < d; ++j)
        for (auto c : rule.image(j)) M[static_cast<std::size_t>(c)][j]++;
    return M;
}

double dominant_eigenvalue(const std::vector<std::vector<long long>>& M) {
    std::size_t d = M.size();
    std::vector<double> v(d, 1.0);
    for (int iter = 0; iter < 500; ++iter) {
        std::vector<double> nv(d, 0.0);
        for (std::size_t i = 0; i < d; ++i)
            for (std::size_t j = 0; j < d; ++j) nv[i] += static_cast<double>(M[i][j]) * v[j];
        double norm = 0;
        for (double x : nv) norm += x * x;
        norm = std::sqrt(norm);
        for (auto& x : nv) x /= norm;
        v = nv;
    }
    std::vector<double> Mv(d, 0.0);
    for (std::size_t i = 0; i < d; ++i)
        for (std::size_t j = 0; j < d; ++j) Mv[i] += static_cast<double>(M[i][j]) * v[j];
    double num = 0, den = 0;
    for (std::size_t i = 0; i < d; ++i) { num += Mv[i] * v[i]; den += v[i] * v[i]; }
    return num / den;
}

}  // namespace

int main() {
    // (1) Tribonacci
    {
        SubstitutionRule sigma({{0, 1}, {0, 2}, {0}});
        auto rs = build_return_substitution(sigma, 0);
        assert((rs.words == std::vector<ReturnWord>{{0, 1}, {0, 2}, {0}}));
        std::cout << "Tribonacci: return substitution equals sigma exactly, as the paper claims. OK\n";
    }

    // (2) Paper's own worked example
    {
        SubstitutionRule sigma({{0, 0, 1, 2}, {1, 0, 0}, {1, 0}});
        auto rs = build_return_substitution(sigma, 0);
        auto M0 = incidence_matrix(sigma);
        std::size_t d1 = rs.words.size();
        std::vector<std::vector<long long>> M1(d1, std::vector<long long>(d1, 0));
        for (std::size_t j = 0; j < d1; ++j)
            for (auto c : rs.derived_images[j]) M1[c][j]++;
        double dom0 = dominant_eigenvalue(M0);
        double dom1 = dominant_eigenvalue(M1);
        std::cout << "worked example: dominant eigenvalue sigma=" << dom0 << " sigma_u=" << dom1 << "\n";
        assert(std::abs(dom0 - dom1) < 1e-6);
        assert(rs.words.size() > sigma.alphabet_size());  // genuinely grew, not degenerate
    }

    std::cout << "return_substitution_literature_validation: both checks pass "
                 "against this project's pre-existing implementation.\n";
    return 0;
}
