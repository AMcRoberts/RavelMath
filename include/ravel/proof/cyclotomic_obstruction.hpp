#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <numeric>
#include <optional>
#include <queue>
#include <stdexcept>
#include <string>
#include <vector>

#include "math/poly_z.hpp"
#include "math/charpoly.hpp"
#include "math/proof_reflection.hpp"
#include "ravel/simple_parry_profile.hpp"

namespace ravel::proof {

inline std::size_t euler_phi(std::size_t n) {
    if (n == 0) return 0;
    std::size_t result = n;
    for (std::size_t p = 2; p <= n / p; ++p) {
        if (n % p != 0) continue;
        while (n % p == 0) n /= p;
        result -= result / p;
    }
    if (n > 1) result -= result / n;
    return result;
}

inline mathlib::PolyZ x_power_minus_one(std::size_t n) {
    if (n == 0) return mathlib::PolyZ();
    mathlib::PolyZ out;
    out.ensure_size(n + 1);
    mathlib::set_si(out.coeff(0), -1);
    mathlib::set_si(out.coeff(n), 1);
    out.trim();
    return out;
}

// Exact long division by a monic polynomial.  This is deliberately separate
// from PolyZ pseudo-division: cyclotomic construction and factor certificates
// require the literal quotient p = q*d, not a leading-coefficient multiple.
inline std::optional<mathlib::PolyZ>
exact_divide_monic(const mathlib::PolyZ& dividend,
                   const mathlib::PolyZ& divisor) {
    if (divisor.is_zero()) return std::nullopt;
    const auto dd = dividend.degree();
    const auto dv = divisor.degree();
    if (dv < 0 || mathlib::cmp_si(divisor.coeff(static_cast<std::size_t>(dv)), 1) != 0)
        return std::nullopt;
    if (dd < dv) return dividend.is_zero() ? std::optional<mathlib::PolyZ>(mathlib::PolyZ())
                                          : std::nullopt;
    mathlib::PolyZ remainder = dividend;
    mathlib::PolyZ quotient;
    quotient.ensure_size(static_cast<std::size_t>(dd - dv + 1));
    for (long long degree = dd; degree >= dv; --degree) {
        const auto shift = static_cast<std::size_t>(degree - dv);
        const auto& lead = remainder.coeff(static_cast<std::size_t>(degree));
        if (mathlib::sgn(lead) == 0) continue;
        mathlib::set(quotient.coeff(shift), lead);
        for (long long j = 0; j <= dv; ++j) {
            mathlib::BigInt product;
            mathlib::mul(product, lead, divisor.coeff(static_cast<std::size_t>(j)));
            mathlib::sub(remainder.coeff(shift + static_cast<std::size_t>(j)),
                         remainder.coeff(shift + static_cast<std::size_t>(j)), product);
        }
    }
    remainder.trim();
    quotient.trim();
    if (!remainder.is_zero()) return std::nullopt;
    if (quotient * divisor != dividend) return std::nullopt;
    return quotient;
}

inline mathlib::PolyZ cyclotomic_polynomial(std::size_t n) {
    if (n == 0) throw std::invalid_argument("cyclotomic order must be positive");
    mathlib::PolyZ result = x_power_minus_one(n);
    for (std::size_t d = 1; d < n; ++d) {
        if (n % d != 0) continue;
        const auto phi_d = cyclotomic_polynomial(d);
        const auto quotient = exact_divide_monic(result, phi_d);
        if (!quotient) throw std::logic_error("cyclotomic exact division failed");
        result = *quotient;
    }
    if (result.degree() != static_cast<long long>(euler_phi(n)))
        throw std::logic_error("cyclotomic degree check failed");
    return result;
}

struct CyclotomicFactorCertificate {
    std::size_t order = 0;
    std::size_t multiplicity = 0;
    mathlib::PolyZ factor;
};

struct CyclotomicObstructionCertificate {
    mathlib::PolyZ polynomial;
    std::vector<CyclotomicFactorCertificate> factors;
    mathlib::PolyZ remaining_factor;
    std::size_t searched_max_order = 0;
    bool exact_reconstruction = false;
    bool proved = false;
    std::string obstruction;

    bool has_order(std::size_t n) const {
        return std::any_of(factors.begin(), factors.end(),
                           [n](const auto& f) { return f.order == n; });
    }
};

inline CyclotomicObstructionCertificate
derive_cyclotomic_obstruction_certificate(const mathlib::PolyZ& polynomial,
                                           std::size_t maximum_order = 0) {
    CyclotomicObstructionCertificate out;
    out.polynomial = polynomial;
    out.remaining_factor = polynomial;
    if (polynomial.is_zero()) {
        out.obstruction = "zero polynomial has every root and no finite factor certificate";
        return out;
    }
    const std::size_t degree = static_cast<std::size_t>(std::max<long long>(0, polynomial.degree()));
    if (maximum_order == 0) maximum_order = std::max<std::size_t>(16, 16 * degree * degree + 16);
    out.searched_max_order = maximum_order;

    for (std::size_t n = 1; n <= maximum_order; ++n) {
        if (euler_phi(n) > degree) continue;
        const auto phi = cyclotomic_polynomial(n);
        std::size_t multiplicity = 0;
        while (true) {
            const auto quotient = exact_divide_monic(out.remaining_factor, phi);
            if (!quotient) break;
            out.remaining_factor = *quotient;
            ++multiplicity;
        }
        if (multiplicity != 0)
            out.factors.push_back({n, multiplicity, phi});
    }

    mathlib::PolyZ reconstruction = out.remaining_factor;
    for (const auto& factor : out.factors)
        for (std::size_t k = 0; k < factor.multiplicity; ++k)
            reconstruction = reconstruction * factor.factor;
    out.exact_reconstruction = (reconstruction == polynomial);
    out.proved = out.exact_reconstruction;
    if (!out.proved) out.obstruction = "cyclotomic factor reconstruction failed";
    return out;
}

// Stages a `CyclotomicObstructionReflectionCertificate` -- gates on
// `cert.proved` AND an independent recomputation of p(-1) via plain
// Horner evaluation over the polynomial's own coefficients (not trusting
// `has_order(2)`'s internal factorization search), checking that
// eval_at_minus_one == 0 exactly matches has_order(2) before staging --
// this is the same fact `x_add_one_dvd_iff_eval_neg_one_zero` proves, so
// a mismatch here would mean either the certificate or the theorem is
// wrong, and the record must not happen.
inline void stage_cyclotomic_obstruction(
        const CyclotomicObstructionCertificate& cert,
        const std::string& description) {
    if (!cert.proved) return;
    const long long degree = cert.polynomial.degree();
    if (degree < 0) return;
    long long eval_at_minus_one = 0;
    std::vector<long long> coeffs;
    for (long long i = degree; i >= 0; --i)
        coeffs.push_back(std::stoll(mathlib::str(cert.polynomial.coeff(static_cast<std::size_t>(i)))));
    std::reverse(coeffs.begin(), coeffs.end());  // ascending: coeffs[0] = constant term
    for (long long i = degree; i >= 0; --i)
        eval_at_minus_one = eval_at_minus_one * (-1) + coeffs[static_cast<std::size_t>(i)];
    const bool has_order_two = cert.has_order(2);
    if ((eval_at_minus_one == 0) != has_order_two) return;
    if (!mathlib::reflection::enabled()) return;
    mathlib::reflection::CyclotomicObstructionReflectionCertificate node;
    node.coefficients = coeffs;
    node.has_order_two = has_order_two;
    node.eval_at_minus_one = eval_at_minus_one;
    node.description = description;
    mathlib::reflection::record(mathlib::reflection::NodeKind::LemmaApplication, node);
}

struct Z2CharacterSectorCertificate {
    mathlib::PolyZ trivial_characteristic;
    mathlib::PolyZ nontrivial_characteristic;
    CyclotomicObstructionCertificate trivial_factors;
    CyclotomicObstructionCertificate nontrivial_factors;
    bool nontrivial_sector_has_order_two = false;
    bool proved = false;
    std::string obstruction;
};

inline Z2CharacterSectorCertificate
derive_z2_character_sector_certificate(const mathlib::PolyZ& trivial_sector,
                                       const mathlib::PolyZ& nontrivial_sector) {
    Z2CharacterSectorCertificate out;
    out.trivial_characteristic = trivial_sector;
    out.nontrivial_characteristic = nontrivial_sector;
    out.trivial_factors = derive_cyclotomic_obstruction_certificate(trivial_sector);
    out.nontrivial_factors = derive_cyclotomic_obstruction_certificate(nontrivial_sector);
    out.nontrivial_sector_has_order_two = out.nontrivial_factors.has_order(2);
    out.proved = out.trivial_factors.proved && out.nontrivial_factors.proved;
    if (!out.proved) out.obstruction = "one or more character-sector factor certificates failed";
    return out;
}


inline Z2CharacterSectorCertificate
derive_z2_character_sector_certificate(
    const std::vector<std::vector<long long>>& identity_component,
    const std::vector<std::vector<long long>>& odd_component) {
    const std::size_t n = identity_component.size();
    if (n == 0 || odd_component.size() != n) {
        Z2CharacterSectorCertificate out;
        out.obstruction = "Z/2 group-ring components must be nonempty and equally sized";
        return out;
    }
    std::vector<std::vector<long long>> trivial(n, std::vector<long long>(n));
    std::vector<std::vector<long long>> alternating(n, std::vector<long long>(n));
    for (std::size_t i = 0; i < n; ++i) {
        if (identity_component[i].size() != n || odd_component[i].size() != n) {
            Z2CharacterSectorCertificate out;
            out.obstruction = "Z/2 group-ring components must be square";
            return out;
        }
        for (std::size_t j = 0; j < n; ++j) {
            trivial[i][j] = identity_component[i][j] + odd_component[i][j];
            alternating[i][j] = identity_component[i][j] - odd_component[i][j];
        }
    }
    return derive_z2_character_sector_certificate(
        mathlib::charpoly_faddeev_leverrier(trivial),
        mathlib::charpoly_faddeev_leverrier(alternating));
}

struct WeightedDirectedEdge {
    std::size_t source = 0;
    std::size_t target = 0;
    std::int64_t weight = 1;
};

struct WeightedCyclePeriodCertificate {
    std::size_t vertex_count = 0;
    std::vector<WeightedDirectedEdge> edges;
    std::uint64_t period = 0;
    bool strongly_connected = false;
    bool proved = false;
    std::string obstruction;
};

inline WeightedCyclePeriodCertificate
derive_weighted_cycle_period_certificate(std::size_t vertex_count,
                                         const std::vector<WeightedDirectedEdge>& edges) {
    WeightedCyclePeriodCertificate out;
    out.vertex_count = vertex_count;
    out.edges = edges;
    if (vertex_count == 0) {
        out.obstruction = "cycle period requires at least one vertex";
        return out;
    }
    std::vector<std::vector<std::size_t>> forward(vertex_count), reverse(vertex_count);
    for (const auto& edge : edges) {
        if (edge.source >= vertex_count || edge.target >= vertex_count || edge.weight <= 0) {
            out.obstruction = "invalid weighted directed edge";
            return out;
        }
        forward[edge.source].push_back(edge.target);
        reverse[edge.target].push_back(edge.source);
    }
    auto reaches_all = [&](const auto& graph) {
        std::vector<bool> seen(vertex_count, false);
        std::queue<std::size_t> queue;
        seen[0] = true; queue.push(0);
        while (!queue.empty()) {
            const auto u = queue.front(); queue.pop();
            for (auto v : graph[u]) if (!seen[v]) { seen[v] = true; queue.push(v); }
        }
        return std::all_of(seen.begin(), seen.end(), [](bool x) { return x; });
    };
    out.strongly_connected = reaches_all(forward) && reaches_all(reverse);
    if (!out.strongly_connected) {
        out.obstruction = "period certificate currently requires a strongly connected graph";
        return out;
    }

    std::vector<std::optional<std::int64_t>> potential(vertex_count);
    potential[0] = 0;
    std::queue<std::size_t> queue;
    queue.push(0);
    while (!queue.empty()) {
        const auto u = queue.front(); queue.pop();
        for (const auto& edge : edges) {
            if (edge.source != u || potential[edge.target]) continue;
            potential[edge.target] = *potential[u] + edge.weight;
            queue.push(edge.target);
        }
    }
    std::uint64_t g = 0;
    for (const auto& edge : edges) {
        const std::int64_t defect = *potential[edge.source] + edge.weight - *potential[edge.target];
        const auto magnitude = static_cast<std::uint64_t>(std::llabs(defect));
        g = std::gcd(g, magnitude);
    }
    out.period = g;
    out.proved = (g != 0);
    if (!out.proved) out.obstruction = "graph contains no positive-weight cycle defect";
    return out;
}

} // namespace ravel::proof
