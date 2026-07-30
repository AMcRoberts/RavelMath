// gb_bp_involution_check.cpp
//
// For the n-bonacci (simple Parry number) family -- where
// `docs/FINDINGS_FOR_CITATION.md` Finding 1 confirms `BP-rho_nc ==
// lambda(G_B)` exactly -- the map `[i,x,j] <-> [j,-x,i]` (swap the
// two contact faces, negate the translation) is an EXACT,
// fixed-point-free automorphism of `G_B`'s recurrent core (see
// `graph_divisor.hpp`'s `extract_recurrent_core` -- for n=2..5 this
// coincides with `extract_dominant_recurrent_core`, since each has a
// single dominant recurrent SCC; that stops being true at n=6, see
// `app/gb_bp_involution_general_n.cpp` and `docs/RESEARCH_STATUS.md`).
// Comparing G_B's FULL graph against the balanced-pair automaton, as
// `app/test_bp_gb_divisor.cpp` does, dilutes any real structural
// relationship with transient "feeder" nodes that contribute nothing
// to the eigenvalue.
//
// For the mismatching cases (sigma_1, sigma_2), the SAME map is only a
// PARTIAL match -- a precisely quantified failure, not the near-total
// absence of structure the full-graph comparison suggested.
//
// Since a free automorphism of an all-positive-weight graph fixes the
// Perron eigenvector, quotienting G_B's core by this involution gives
// a smaller graph with the SAME dominant eigenvalue. For Tribonacci
// and Tetrabonacci, this quotient's exact integer characteristic
// polynomial DIVIDES the balanced-pair automaton's own recurrent
// core's characteristic polynomial exactly (checked via
// mathlib::divmod, not floating point) -- this is the actual
// algebraic mechanism behind the numerical equality, not just a
// correlation. See docs/DIRECTION_AND_OPEN_THREADS.md thread A for
// what's still needed to turn this into a complete proof for general
// n, app/test_bp_gb_divisor.cpp for the (superseded) predecessor
// experiment this one replaces, and docs/RESEARCH_STATUS.md for how this
// mechanism and the recurrent-core-extraction fix were found.


#include <cstdio>
#include <vector>

#include "math/poly_z.hpp"
#include "ravel/ambient_graph.hpp"
#include "ravel/balanced_pair.hpp"
#include "ravel/contact_boundary.hpp"
#include "ravel/core.hpp"
#include "ravel/corona.hpp"
#include "ravel/d_cont_check.hpp"
#include "ravel/faces.hpp"
#include "ravel/graph_divisor.hpp"
#include "ravel/substitution.hpp"

using namespace ravel;

namespace {

// Runs the full pipeline for a substitution of known alphabet size,
// returning the report. Alphabet size is templated since
// compute_contact_boundary_dispatch's own dispatch is on the rule's
// runtime image count, but search_D_cont needs a compile-time d.
template <std::size_t d>
ContactBoundaryReport run_pipeline(const SubstitutionRule& rule, double beta) {
    auto d_cont_cands = search_D_cont<d>(make_substitution<d>(rule, beta), 2);
    std::vector<std::tuple<long long, std::vector<long long>, long long>> d_cont;
    for (const auto& c : d_cont_cands) {
        std::vector<long long> xv(c.x.begin(), c.x.end());
        d_cont.emplace_back(c.i, std::move(xv), c.j);
    }
    return compute_contact_boundary_dispatch(rule, beta, 0.0, d_cont);
}

// Checks the involution [i,x,j] <-> [j,-x,i] within G_B's recurrent
// core specifically (not the full graph -- see file header). Returns
// (matched, total) node counts.
std::pair<int, int> check_involution_on_core(const ContactBoundaryReport& rep) {
    auto gb_graph = WeightedDigraph::from_dense(rep.gb_matrix);
    auto [core, orig_idx] = extract_recurrent_core(gb_graph);

    std::map<std::tuple<long long, std::vector<long long>, long long>, int> node_index;
    for (std::size_t i = 0; i < rep.boundary_nodes.size(); ++i) {
        node_index[rep.boundary_nodes[i]] = static_cast<int>(i);
    }
    std::set<std::size_t> core_set(orig_idx.begin(), orig_idx.end());

    int matched = 0;
    for (std::size_t u : orig_idx) {
        const auto& n = rep.boundary_nodes[u];
        long long i = std::get<0>(n), j = std::get<2>(n);
        std::vector<long long> negx = std::get<1>(n);
        for (auto& v : negx) v = -v;
        auto key = std::make_tuple(j, negx, i);
        auto it = node_index.find(key);
        if (it != node_index.end() && core_set.count(static_cast<std::size_t>(it->second))) {
            ++matched;
        }
    }
    return {matched, static_cast<int>(orig_idx.size())};
}

// Given the equitable partition happens to be a pure pairing (every
// class size 2 -- the free-involution signature), builds the
// quotient's exact integer characteristic polynomial.
std::vector<long long> gb_core_quotient_charpoly(const ContactBoundaryReport& rep, bool* is_pure_pairing) {
    auto gb_graph = WeightedDigraph::from_dense(rep.gb_matrix);
    auto [core, orig_idx] = extract_recurrent_core(gb_graph);
    (void)orig_idx;
    auto p = coarsest_equitable_partition(core);
    std::map<int, int> class_size;
    for (int c : p.color) class_size[c]++;
    *is_pure_pairing = true;
    for (const auto& [c, n] : class_size) {
        if (n != 2) *is_pure_pairing = false;
    }
    if (!*is_pure_pairing) return {};
    auto Q = quotient_matrix(core, p);
    return charpoly_int(Q);
}

std::vector<long long> bp_core_charpoly(const SubstitutionRule& rule) {
    auto bp = balanced_pair_transition_graph(rule);
    auto bp_graph = WeightedDigraph::from_dense(bp.matrix);
    auto [core, orig_idx] = extract_recurrent_core(bp_graph);
    (void)orig_idx;
    std::vector<std::vector<long long>> dense(core.n, std::vector<long long>(core.n, 0));
    for (std::size_t u = 0; u < core.n; ++u) {
        for (const auto& e : core.out_adj[u]) dense[u][e.first] += e.second;
    }
    return charpoly_int(dense);
}

mathlib::PolyZ from_high_first(std::vector<long long> hf) {
    mathlib::PolyZ p;
    std::size_t n = hf.size();
    p.ensure_size(n);
    for (std::size_t i = 0; i < n; ++i) mathlib::set_si(p.coeff(i), hf[n - 1 - i]);
    return p;
}

// Checks whether `factor`'s polynomial (after stripping a trailing
// x^k trivial-zero-eigenvalue factor, which the core-quotient always
// carries whenever the quotient itself isn't fully recurrent) exactly
// divides `whole`. Returns true/false; prints the cofactor if exact.
bool check_exact_factor(const char* name, std::vector<long long> whole_hf,
                         std::vector<long long> factor_hf) {
    while (factor_hf.size() > 1 && factor_hf.back() == 0) factor_hf.pop_back();
    if (whole_hf.empty() || factor_hf.empty()) return false;
    auto whole = from_high_first(whole_hf);
    auto factor = from_high_first(factor_hf);
    auto dm = mathlib::divmod(whole, factor);
    bool exact = true;
    for (std::size_t i = 0; i < dm.r.coeffs_.size(); ++i) {
        if (mpz_sgn(dm.r.coeff(i).get()) != 0) { exact = false; break; }
    }
    std::printf("  %s: G_B-quotient charpoly exactly divides BP-core charpoly? %s\n",
                name, exact ? "YES" : "no");
    return exact;
}

void run_case(const char* name, std::vector<std::vector<std::int8_t>> sigma, double beta,
              std::size_t alphabet) {
    SubstitutionRule rule(sigma);
    ContactBoundaryReport rep;
    if (alphabet == 2) rep = run_pipeline<2>(rule, beta);
    else if (alphabet == 3) rep = run_pipeline<3>(rule, beta);
    else if (alphabet == 4) rep = run_pipeline<4>(rule, beta);
    else if (alphabet == 5) rep = run_pipeline<5>(rule, beta);
    else if (alphabet == 6) rep = run_pipeline<6>(rule, beta);
    else { std::printf("%s: unsupported alphabet size %zu\n", name, alphabet); return; }

    double ratio = rep.boundary_eigenvalue / rep.bp_rho_nc;
    auto [matched, total] = check_involution_on_core(rep);

    std::printf("%s: BP-rho_nc=%.6f lambda(G_B)=%.6f ratio=%.6f (%s)\n",
                name, rep.bp_rho_nc, rep.boundary_eigenvalue, ratio,
                std::abs(ratio - 1.0) < 1e-4 ? "MATCH" : "mismatch");
    std::printf("  involution [i,x,j]<->[j,-x,i] within G_B's recurrent core: %d/%d %s\n",
                matched, total, (matched == total) ? "(EXACT)" : "(partial)");

    bool is_pure_pairing = false;
    auto gbq = gb_core_quotient_charpoly(rep, &is_pure_pairing);
    if (is_pure_pairing) {
        auto bpc = bp_core_charpoly(rule);
        check_exact_factor(name, bpc, gbq);
    }
}

}  // namespace

int main() {
    std::printf("=== n-bonacci family (expect: MATCH, exact involution) ===\n");
    run_case("Fibonacci", {{0, 1}, {0}}, 1.618033988749895, 2);
    run_case("Tribonacci", {{0, 1}, {0, 2}, {0}}, 1.8392867552141612, 3);
    run_case("Tetrabonacci", {{0, 1}, {0, 2}, {0, 3}, {0}}, 1.9275619754829254, 4);
    run_case("Pentanacci", {{0, 1}, {0, 2}, {0, 3}, {0, 4}, {0}}, 1.9659482366454853, 5);

    std::printf("\n=== non-simple-Parry unimodular Pisot (expect: mismatch, partial involution) ===\n");
    run_case("sigma_1", {{0, 0, 0, 1}, {0, 0, 2}, {0}}, 3.6273650847118, 3);
    run_case("sigma_2", {{0, 0, 1}, {0, 0, 0, 2}, {0}}, 3.0795956234914, 3);

    return 0;
}
