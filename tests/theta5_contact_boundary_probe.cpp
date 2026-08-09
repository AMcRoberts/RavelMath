#include <cassert>
#include <cstdio>
#include <vector>

#include "math/qbeta.hpp"
#include "math/sturm.hpp"
#include "ravel/canonical_beta_substitution.hpp"
#include "ravel/contact_boundary.hpp"

using namespace mathlib;
using namespace ravel;

namespace {

QBetaRing theta5_ring() {
    // x^6 - x^5 - x^4 + x^2 - 1, low-first {-1,-1,0,1,0,-1}.
    PolyZ p;
    p.ensure_size(7);
    const long long low_first[] = {-1, -1, 0, 1, 0, -1};
    for (std::size_t i = 0; i < 6; ++i)
        set_si(p.coeff(5 - static_cast<long long>(i)), low_first[i]);
    set_si(p.coeff(6), 1);
    return QBetaRing(p);
}

double midpoint(const RootInterval& interval) {
    Rat sum, mid, two;
    add(sum, interval.lo, interval.hi);
    set_si(two, 2, 1);
    div(mid, sum, two);
    return mpq_get_d(mid.get());
}

}  // namespace

int main() {
    const auto ring = theta5_ring();
    const auto interval = isolate_beta(ring);
    const auto expansion = exact_greedy_beta_expansion_of_one(ring, interval, 128);
    assert(expansion.terminated || expansion.period_len != 0);
    auto derived = canonical_beta_substitution_from_digits(expansion.digits);
    std::vector<std::vector<std::int8_t>> images;
    for (const auto& image : derived) {
        std::vector<std::int8_t> converted;
        for (const auto letter : image)
            converted.push_back(static_cast<std::int8_t>(letter));
        images.push_back(std::move(converted));
    }
    SubstitutionRule rule(images);
    assert(rule.alphabet_size() == 8);

    ContactBoundaryLimits limits;
    limits.max_rho_pairs = 200;
    limits.max_rho_len = 2000;
    limits.closure_cap = 500;
    limits.corona_cap = 2000;
    limits.max_corona_rounds = 3;
    const auto report = compute_contact_boundary_from_subst_dispatch(
        rule, midpoint(interval), 0.9, 1, limits);
    assert(report.d_cont_size > 0);
    assert(report.boundary_size > 0);
    assert(report.closure_stopped_early || report.corona_capped);
    std::printf("theta5 bounded contact probe: alphabet=%zu d_cont=%zu boundary=%zu "
                 "closure_early=%d corona_capped=%d\n",
                 rule.alphabet_size(), report.d_cont_size, report.boundary_size,
                 report.closure_stopped_early, report.corona_capped);
}
