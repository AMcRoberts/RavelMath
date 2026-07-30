// gb_bp_involution_general_n.cpp
//
// Extends `app/gb_bp_involution_check.cpp`'s hand-enumerated n=2..5
// cases (docs/DIRECTION_AND_OPEN_THREADS.md thread A) to a single
// programmatic sweep over n=2..MAX_DISPATCH_D (currently 8, the
// compile-time dispatch ceiling in contact_boundary.hpp), generating
// the n-bonacci rule and its Pisot constant from n directly rather
// than hardcoding a new run_case(...) line per n.
//
// This is deliberately NOT a new mechanism -- it reuses exactly the
// same involution check / quotient / exact-charpoly-division machinery
// as gb_bp_involution_check.cpp (graph_divisor.hpp's
// extract_recurrent_core / extract_dominant_recurrent_core +
// coarsest_equitable_partition + quotient_matrix, barge.hpp's
// charpoly_int, math/poly_z.hpp's divmod). The only thing new here is
// the n -> (rule, beta) generator, so that thread A item (1) ("a
// general, all-n argument that [i,x,j] <-> [j,-x,i] is always a free
// automorphism of G_B's dominant recurrent core for n-bonacci
// substitutions") has more than n=2..5 of numerical evidence to
// generalize FROM before anyone commits to a closed-form combinatorial
// proof. See the bottom of this file for what the run confirms and
// does not confirm, and docs/RESEARCH_STATUS.md for how the wrong-SCC bug
// this driver's diagnostics were built to investigate was found.

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <numeric>
#include <string>
#include <unordered_map>
#include <vector>

#include "math/ball.hpp"
#include "math/bigfloat.hpp"
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

// ===================================================================
// The switch: which Perron-root certification method to use, and (for
// the tunable method) how much precision/memory to spend. Set from
// argv in main() -- see the usage banner there. Global rather than
// threaded through every call site since this is a single-purpose
// diagnostic driver, not library code (CPP_DESIGN_PHILOSOPHY.md's
// templated-header rules apply to include/ravel/*.hpp, not to a
// driver in app/).
// ===================================================================
mathlib::PerronCertifyMethod g_method = mathlib::PerronCertifyMethod::ExactRationalBracket;
unsigned g_precision_bits = 256;
int g_max_iters = 200;

// Runs whichever method g_method selects on (bp_core, gb_core) and
// prints the result in a method-appropriate way. This is the single
// place both run_n_bonacci_case and run_named_case call into, so
// flipping the switch changes both without duplicating logic.
void print_perron_comparison(const std::vector<std::vector<long long>>& bp_core,
                              const std::vector<std::vector<long long>>& gb_core) {
    if (g_method == mathlib::PerronCertifyMethod::ExactRationalBracket) {
        // Bit-exact BigInt Collatz-Wielandt bracket (math/ball.hpp).
        // Can PROVE strict inequality (bracket separation) or equality
        // (both brackets collapse to the identical rational point --
        // only possible when the shared Perron root is itself
        // rational, e.g. Fibonacci's BP-core/G_B-core both equal 1).
        // For an irrational shared root (n>=3 n-bonacci, where the
        // charpoly-divisibility check already proves equality a
        // different way) this method will report "inconclusive"
        // forever, no matter how many iterations -- that is the
        // correct, honest answer for this method, not a bug.
        try {
            auto cmp = mathlib::compare_perron_roots_exact(bp_core, gb_core, g_max_iters, 1e-9);
            const char* verdict =
                cmp.order == mathlib::PerronOrder::equal ? "PROVEN EQUAL" :
                cmp.order == mathlib::PerronOrder::less ? "PROVEN: BP-core < G_B-core" :
                cmp.order == mathlib::PerronOrder::greater ? "PROVEN: BP-core > G_B-core" :
                "inconclusive (brackets overlap -- expected for an irrational shared root)";
            std::printf("  [exact-rational] Perron-root comparison: %s\n", verdict);
            std::printf("    BP-core bracket:  [%s, %s]  (%d iters)\n",
                        mpq_get_str(nullptr, 10, cmp.lhs.bracket.lo.get()),
                        mpq_get_str(nullptr, 10, cmp.lhs.bracket.hi.get()),
                        cmp.lhs.iterations_run);
            std::printf("    G_B-core bracket: [%s, %s]  (%d iters)\n",
                        mpq_get_str(nullptr, 10, cmp.rhs.bracket.lo.get()),
                        mpq_get_str(nullptr, 10, cmp.rhs.bracket.hi.get()),
                        cmp.rhs.iterations_run);
        } catch (const std::exception& e) {
            std::printf("  [exact-rational] Perron-root comparison: could not run (%s)\n", e.what());
        }
    } else {
        // Tunable-precision BigFloat bracket (math/bigfloat.hpp).
        // NOT a proof -- rounding v's components each step introduces
        // untracked (but bounded-by-construction) error -- but at
        // g_precision_bits >> 53 (double's mantissa) this is far
        // stronger numerical evidence than the double-precision ratio
        // check, at bounded (tunable) memory per number, unlike the
        // exact-rational bracket's unboundedly-growing fractions.
        try {
            auto bp_br = mathlib::certify_perron_bracket_bigfloat(bp_core, g_precision_bits, g_max_iters);
            auto gb_br = mathlib::certify_perron_bracket_bigfloat(gb_core, g_precision_bits, g_max_iters);
            double bp_lo = mathlib::bigfloat_to_double(bp_br.lo);
            double bp_hi = mathlib::bigfloat_to_double(bp_br.hi);
            double gb_lo = mathlib::bigfloat_to_double(gb_br.lo);
            double gb_hi = mathlib::bigfloat_to_double(gb_br.hi);
            double bp_mid = 0.5 * (bp_lo + bp_hi);
            double gb_mid = 0.5 * (gb_lo + gb_hi);
            // Report agreement to as many decimal digits as the
            // bracket widths themselves support, not a fixed 1e-4:
            // the point of this method is that the caller controls
            // how tight this gets via precision_bits.
            double diff = std::abs(bp_mid - gb_mid);
            mathlib::BigFloat bp_width = mathlib::bigfloat_sub(bp_br.hi, bp_br.lo, g_precision_bits);
            mathlib::BigFloat gb_width = mathlib::bigfloat_sub(gb_br.hi, gb_br.lo, g_precision_bits);
            std::printf("  [tuned BigFloat @ %u bits] Perron-root comparison: "
                        "BP-core mid=%.15f  G_B-core mid=%.15f  |diff (double-truncated)|=%.3e\n",
                        g_precision_bits, bp_mid, gb_mid, diff);
            std::printf("    bracket widths (exact fractions, NOT double-truncated): "
                        "BP-core=%s  G_B-core=%s\n",
                        mathlib::bigfloat_str(bp_width).c_str(),
                        mathlib::bigfloat_str(gb_width).c_str());
            std::printf("    BP-core bracket:  [%s, %s]  (%d iters)\n",
                        mathlib::bigfloat_str(bp_br.lo).c_str(),
                        mathlib::bigfloat_str(bp_br.hi).c_str(), bp_br.iterations_run);
            std::printf("    G_B-core bracket: [%s, %s]  (%d iters)\n",
                        mathlib::bigfloat_str(gb_br.lo).c_str(),
                        mathlib::bigfloat_str(gb_br.hi).c_str(), gb_br.iterations_run);
        } catch (const std::exception& e) {
            std::printf("  [tuned BigFloat] Perron-root comparison: could not run (%s)\n", e.what());
        }
    }
}

// n-bonacci rule, 0-indexed, matching the convention already used in
// gb_bp_involution_check.cpp's hardcoded Fibonacci..Pentanacci cases:
//   sigma(i) = [0, i+1]   for i = 0 .. n-2
//   sigma(n-1) = [0]
// (companion matrix with an all-ones first row).
std::vector<std::vector<std::int8_t>> n_bonacci_rule(std::size_t n) {
    std::vector<std::vector<std::int8_t>> sigma(n);
    for (std::size_t i = 0; i + 1 < n; ++i) {
        sigma[i] = {0, static_cast<std::int8_t>(i + 1)};
    }
    sigma[n - 1] = {0};
    return sigma;
}

// The n-bonacci Pisot constant: the unique real root > 1 of
//   x^n - x^{n-1} - ... - x - 1 = 0
// Bisection is enough here (beta is only used as report metadata in
// this pipeline -- Substitution<d>::compute_right_eigenvector power-
// iterates M^T directly and never reads the beta field -- but we
// still want a correct, independently-derived value rather than a
// placeholder, since it's printed and cross-checked against the
// pipeline's own converged eigenvalue).
double n_bonacci_beta(std::size_t n) {
    auto f = [n](double x) {
        double xn = std::pow(x, static_cast<double>(n));
        double s = 0.0;
        for (std::size_t k = 0; k < n; ++k) s += std::pow(x, static_cast<double>(k));
        return xn - s;  // x^n - (1 + x + ... + x^{n-1})
    };
    double lo = 1.0, hi = 2.0;
    for (int it = 0; it < 200; ++it) {
        double mid = 0.5 * (lo + hi);
        if (f(mid) < 0.0) lo = mid; else hi = mid;
    }
    return 0.5 * (lo + hi);
}

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

std::pair<int, int> check_involution_on_core(const ContactBoundaryReport& rep,
                                              bool print_unmatched = false) {
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
        bool ok = (it != node_index.end() && core_set.count(static_cast<std::size_t>(it->second)));
        if (ok) {
            ++matched;
        } else if (print_unmatched) {
            std::printf("    unmatched core node: [i=%lld, x=(", i);
            const auto& xv = std::get<1>(n);
            for (std::size_t k = 0; k < xv.size(); ++k) std::printf("%s%lld", k ? "," : "", xv[k]);
            std::printf("), j=%lld]  flip=[i=%lld, x=(", j, j);
            for (std::size_t k = 0; k < negx.size(); ++k) std::printf("%s%lld", k ? "," : "", negx[k]);
            bool flip_in_gb = (it != node_index.end());
            bool flip_in_core = flip_in_gb && core_set.count(static_cast<std::size_t>(it->second));
            std::printf("), i=%lld]  flip_in_G_B=%s flip_in_core=%s\n",
                        i, flip_in_gb ? "yes" : "NO", flip_in_core ? "yes" : "no");
        }
    }
    return {matched, static_cast<int>(orig_idx.size())};
}

// Exact, arbitrary-precision (not checked long-long): these
// quotients can be large. See
// docs/RECOVERY_AUDIT_2026-07-29.md queue item Q4.
mathlib::PolyZ gb_core_quotient_charpoly(const ContactBoundaryReport& rep, bool* is_pure_pairing) {
    auto gb_graph = WeightedDigraph::from_dense(rep.gb_matrix);
    auto [core, orig_idx] = extract_recurrent_core(gb_graph);
    (void)orig_idx;
    auto p = coarsest_equitable_partition(core);
    std::map<int, int> class_size;
    for (int c : p.color) class_size[c]++;
    *is_pure_pairing = true;
    for (const auto& [c, cnt] : class_size) {
        if (cnt != 2) *is_pure_pairing = false;
    }
    if (!*is_pure_pairing) return mathlib::PolyZ();
    auto Q = quotient_matrix(core, p);
    return charpoly_PolyZ(Q);
}

// Recurrent-core dense-matrix extraction, shared by bp_core_charpoly,
// gb_core_quotient_charpoly's sibling checks, and the exact
// Perron-root comparison below (which needs the raw matrix, not a
// charpoly).
std::vector<std::vector<long long>> recurrent_core_dense_matrix(
        const std::vector<std::vector<long long>>& full_matrix) {
    auto graph = WeightedDigraph::from_dense(full_matrix);
    auto [core, orig_idx] = extract_recurrent_core(graph);
    (void)orig_idx;
    std::vector<std::vector<long long>> dense(core.n, std::vector<long long>(core.n, 0));
    for (std::size_t u = 0; u < core.n; ++u) {
        for (const auto& e : core.out_adj[u]) dense[u][e.first] += e.second;
    }
    return dense;
}

mathlib::PolyZ bp_core_charpoly(const SubstitutionRule& rule) {
    auto bp = balanced_pair_transition_graph(rule);
    return charpoly_PolyZ(recurrent_core_dense_matrix(bp.matrix));
}

bool check_exact_factor(const char* name, const mathlib::PolyZ& whole,
                         const mathlib::PolyZ& factor) {
    if (whole.is_zero() || factor.is_zero()) return false;
    auto dm = mathlib::divmod(whole, factor);
    bool exact = dm.r.is_zero();
    std::printf("  %s: G_B-quotient charpoly exactly divides BP-core charpoly? %s\n",
                name, exact ? "YES" : "no");
    return exact;
}

// Shared diagnostic: checks whether extract_recurrent_core's
// "largest-by-node-count" choice actually IS the dominant recurrent
// SCC for this case's G_B (and for BP-core), using
// extract_dominant_recurrent_core (graph_divisor.hpp). If it isn't,
// re-runs the involution check and the exact charpoly-divisibility
// check on the CORRECTED dominant component instead, and reports both
// the old (possibly wrong) and new (corrected) results side by side so
// the difference is visible, not silently swapped. See
// docs/RESEARCH_STATUS.md for how the Hexanacci counterexample that
// motivated this function was found.
void diagnose_dominant_scc(const char* name, const ContactBoundaryReport& rep,
                            const SubstitutionRule& rule) {
    auto gb_graph = WeightedDigraph::from_dense(rep.gb_matrix);
    auto sccs = tarjan_scc(gb_graph);
    std::vector<std::size_t> recurrent_sizes;
    for (auto& c : sccs) if (is_recurrent_scc(gb_graph, c)) recurrent_sizes.push_back(c.size());
    std::sort(recurrent_sizes.begin(), recurrent_sizes.end(), std::greater<std::size_t>());
    std::printf("  %s: G_B recurrent SCC sizes (largest first):", name);
    for (auto s : recurrent_sizes) std::printf(" %zu", s);
    std::printf("  (%zu SCC%s)\n", recurrent_sizes.size(), recurrent_sizes.size() == 1 ? "" : "s");

    auto [largest_core, largest_idx] = extract_recurrent_core(gb_graph);
    auto [dom_core, dom_idx] = extract_dominant_recurrent_core(gb_graph, g_max_iters);
    bool same_choice = (largest_idx.size() == dom_idx.size()) &&
        std::set<std::size_t>(largest_idx.begin(), largest_idx.end()) ==
        std::set<std::size_t>(dom_idx.begin(), dom_idx.end());
    if (recurrent_sizes.size() <= 1) {
        std::printf("  %s: only one recurrent SCC -- extract_recurrent_core's choice is "
                    "trivially correct, no wrong-SCC risk here.\n", name);
        return;
    }
    std::printf("  %s: extract_recurrent_core's largest-by-size SCC (%zu nodes) %s "
                "extract_dominant_recurrent_core's choice (%zu nodes) -- %s\n",
                name, largest_idx.size(), same_choice ? "MATCHES" : "DIFFERS FROM",
                dom_idx.size(),
                same_choice ? "no correction needed" : "the earlier report used the WRONG SCC");
    if (same_choice) return;

    // Involution check on the CORRECTED dominant core.
    std::map<std::tuple<long long, std::vector<long long>, long long>, int> node_index;
    for (std::size_t i = 0; i < rep.boundary_nodes.size(); ++i) {
        node_index[rep.boundary_nodes[i]] = static_cast<int>(i);
    }
    std::set<std::size_t> dom_set(dom_idx.begin(), dom_idx.end());
    int dom_matched = 0;
    for (std::size_t u : dom_idx) {
        const auto& nd = rep.boundary_nodes[u];
        long long ii = std::get<0>(nd), jj = std::get<2>(nd);
        std::vector<long long> negx = std::get<1>(nd);
        for (auto& v : negx) v = -v;
        auto key = std::make_tuple(jj, negx, ii);
        auto it = node_index.find(key);
        if (it != node_index.end() && dom_set.count(static_cast<std::size_t>(it->second))) ++dom_matched;
    }
    std::printf("  %s: involution on the CORRECTED dominant SCC (%zu nodes): %d/%zu %s\n",
                name, dom_idx.size(), dom_matched, dom_idx.size(),
                (static_cast<std::size_t>(dom_matched) == dom_idx.size()) ? "(EXACT)" : "(still partial)");

    auto p = coarsest_equitable_partition(dom_core);
    std::map<int, int> class_size;
    for (int c : p.color) class_size[c]++;
    bool dom_is_pure_pairing = true;
    for (const auto& [c, cnt] : class_size) if (cnt != 2) dom_is_pure_pairing = false;
    if (!dom_is_pure_pairing) {
        std::printf("  %s: corrected dominant core's partition is NOT a pure pairing; "
                    "skipping charpoly-division check.\n", name);
        return;
    }
    auto Q = quotient_matrix(dom_core, p);
    auto dom_quotient_charpoly = charpoly_PolyZ(Q);

    // BP-core: check its own recurrent structure too, for the same reason.
    auto bp = balanced_pair_transition_graph(rule);
    auto bp_graph = WeightedDigraph::from_dense(bp.matrix);
    auto bp_sccs = tarjan_scc(bp_graph);
    int bp_recurrent_count = 0;
    for (auto& c : bp_sccs) if (is_recurrent_scc(bp_graph, c)) ++bp_recurrent_count;
    if (bp_recurrent_count > 1) {
        std::printf("  %s: BP-core ALSO has %d recurrent SCCs -- using "
                    "extract_dominant_recurrent_core there too.\n", name, bp_recurrent_count);
    }
    auto [bp_dom_core, bp_dom_idx] = extract_dominant_recurrent_core(bp_graph, g_max_iters);
    (void)bp_dom_idx;
    auto bpc = charpoly_PolyZ([&]() {
        std::vector<std::vector<long long>> dense(bp_dom_core.n,
                                                    std::vector<long long>(bp_dom_core.n, 0));
        for (std::size_t u = 0; u < bp_dom_core.n; ++u) {
            for (const auto& e : bp_dom_core.out_adj[u]) dense[u][e.first] += e.second;
        }
        return dense;
    }());
    char label[64];
    std::snprintf(label, sizeof(label), "%s (CORRECTED)", name);
    check_exact_factor(label, bpc, dom_quotient_charpoly);

    // Exact/tunable Perron-root comparison on the CORRECTED cores,
    // via the same switch as everywhere else in this file.
    std::vector<std::vector<long long>> bp_dense(bp_dom_core.n, std::vector<long long>(bp_dom_core.n, 0));
    for (std::size_t u = 0; u < bp_dom_core.n; ++u) {
        for (const auto& e : bp_dom_core.out_adj[u]) bp_dense[u][e.first] += e.second;
    }
    std::vector<std::vector<long long>> gb_dense(dom_core.n, std::vector<long long>(dom_core.n, 0));
    for (std::size_t u = 0; u < dom_core.n; ++u) {
        for (const auto& e : dom_core.out_adj[u]) gb_dense[u][e.first] += e.second;
    }
    print_perron_comparison(bp_dense, gb_dense);
}

// Runs one n-bonacci case, given a compile-time-known alphabet size D
// matching the runtime n (the switch in run_n_bonacci below is what
// turns runtime n into this compile-time D).
template <std::size_t D>
void run_n_bonacci_case(std::size_t n) {
    auto sigma = n_bonacci_rule(n);
    double beta = n_bonacci_beta(n);
    SubstitutionRule rule(sigma);
    auto rep = run_pipeline<D>(rule, beta);

    double ratio = rep.boundary_eigenvalue / rep.bp_rho_nc;
    bool partial_expected_diagnostic = true;  // always safe to print
    auto [matched, total] = check_involution_on_core(rep, false);

    char name[32];
    std::snprintf(name, sizeof(name), "n=%zu", n);
    std::printf("%s (beta=%.10f): BP-rho_nc=%.6f lambda(G_B)=%.6f ratio=%.6f (%s, double-precision)\n",
                name, beta, rep.bp_rho_nc, rep.boundary_eigenvalue, ratio,
                std::abs(ratio - 1.0) < 1e-4 ? "MATCH" : "mismatch");

    // Perron-root comparison for the SAME quantity the double-precision
    // ratio above checks with a fuzzy 1e-4 tolerance -- method and
    // precision controlled by the g_method/g_precision_bits switch (see
    // main()'s argv handling).
    //
    // Caveat (see the n=6 recurrent-core investigation): when G_B has
    // more than one recurrent SCC, extract_recurrent_core keeps only
    // the largest BY NODE COUNT, which need not be the one with the
    // largest spectral radius. This check therefore certifies "the
    // Perron root of G_B's largest-by-size recurrent SCC vs. BP-core's
    // own", which coincides with lambda(G_B) only when that SCC is
    // also the dominant one -- true for n=2..5 (single recurrent SCC)
    // but not yet verified in general for n>=6.
    {
        auto bp_core = recurrent_core_dense_matrix(balanced_pair_transition_graph(rule).matrix);
        auto gb_core = recurrent_core_dense_matrix(rep.gb_matrix);
        print_perron_comparison(bp_core, gb_core);
    }

    std::printf("  |G_B|=%zu recurrent core involution [i,x,j]<->[j,-x,i]: %d/%d %s\n",
                rep.boundary_size, matched, total, (matched == total) ? "(EXACT)" : "(partial)");
    if (partial_expected_diagnostic) {
        std::printf("  diagnostics: converged=%s corona_capped=%s closure_stopped_early=%s "
                     "convergence_rounds=%d max_a_size_reached=%zu d_cont=%zu pre_contact=%zu "
                     "contact=%zu signed_contact=%zu\n",
                     rep.converged ? "yes" : "NO", rep.corona_capped ? "YES" : "no",
                     rep.closure_stopped_early ? "YES" : "no", rep.convergence_rounds,
                     rep.max_a_size_reached, rep.d_cont_size, rep.pre_contact_size,
                     rep.contact_size, rep.signed_contact_size);
    }
    if (matched != total) {
        std::printf("  listing unmatched core nodes (up to first 40):\n");
        int shown = 0;
        auto gb_graph = WeightedDigraph::from_dense(rep.gb_matrix);
        auto [core, orig_idx] = extract_recurrent_core(gb_graph);
        std::map<std::tuple<long long, std::vector<long long>, long long>, int> node_index;
        for (std::size_t i = 0; i < rep.boundary_nodes.size(); ++i) {
            node_index[rep.boundary_nodes[i]] = static_cast<int>(i);
        }
        std::set<std::size_t> core_set(orig_idx.begin(), orig_idx.end());
        for (std::size_t u : orig_idx) {
            if (shown >= 40) break;
            const auto& nd = rep.boundary_nodes[u];
            long long i = std::get<0>(nd), j = std::get<2>(nd);
            std::vector<long long> negx = std::get<1>(nd);
            for (auto& v : negx) v = -v;
            auto key = std::make_tuple(j, negx, i);
            auto it = node_index.find(key);
            bool ok = (it != node_index.end() && core_set.count(static_cast<std::size_t>(it->second)));
            if (ok) continue;
            std::printf("    [i=%lld x=(", i);
            const auto& xv = std::get<1>(nd);
            for (std::size_t k = 0; k < xv.size(); ++k) std::printf("%s%lld", k ? "," : "", xv[k]);
            bool flip_in_gb = (it != node_index.end());
            bool flip_in_core = flip_in_gb && core_set.count(static_cast<std::size_t>(it->second));
            std::printf(") j=%lld]  flip_exists_in_G_B=%s flip_in_core=%s\n",
                        j, flip_in_gb ? "yes" : "NO", flip_in_core ? "yes" : "no");
            ++shown;
        }
    }

    {
        auto gb_graph = WeightedDigraph::from_dense(rep.gb_matrix);
        auto sccs = tarjan_scc(gb_graph);
        std::vector<std::size_t> recurrent_sizes;
        std::vector<std::size_t> all_recurrent_nodes;  // union across every recurrent SCC
        std::vector<int> scc_id_of(gb_graph.n, -1);
        int next_scc_id = 0;
        for (auto& c : sccs) {
            if (!is_recurrent_scc(gb_graph, c)) continue;
            recurrent_sizes.push_back(c.size());
            for (auto u : c) { all_recurrent_nodes.push_back(u); scc_id_of[u] = next_scc_id; }
            ++next_scc_id;
        }
        {
            std::vector<std::size_t> sorted_sizes = recurrent_sizes;
            std::sort(sorted_sizes.begin(), sorted_sizes.end(), std::greater<std::size_t>());
            std::printf("  recurrent SCC sizes (all, largest first):");
            for (auto s : sorted_sizes) std::printf(" %zu", s);
            std::printf("  (total recurrent nodes across all SCCs: %zu)\n",
                        std::accumulate(sorted_sizes.begin(), sorted_sizes.end(), std::size_t{0}));
        }

        // Re-check the involution across the UNION of all recurrent SCCs
        // (not just the single largest one extract_recurrent_core keeps),
        // and report, for each match, whether the partner is in the SAME
        // SCC or a DIFFERENT one -- this is the concrete test of "the 15
        // stragglers pair up across SCCs, they're not really unmatched."
        std::map<std::tuple<long long, std::vector<long long>, long long>, int> node_index;
        for (std::size_t i = 0; i < rep.boundary_nodes.size(); ++i) {
            node_index[rep.boundary_nodes[i]] = static_cast<int>(i);
        }
        std::set<std::size_t> all_recurrent_set(all_recurrent_nodes.begin(), all_recurrent_nodes.end());
        int matched_union = 0, matched_same_scc = 0, matched_cross_scc = 0;
        for (std::size_t u : all_recurrent_nodes) {
            const auto& nd = rep.boundary_nodes[u];
            long long i = std::get<0>(nd), j = std::get<2>(nd);
            std::vector<long long> negx = std::get<1>(nd);
            for (auto& v : negx) v = -v;
            auto key = std::make_tuple(j, negx, i);
            auto it = node_index.find(key);
            if (it == node_index.end()) continue;
            std::size_t partner = static_cast<std::size_t>(it->second);
            if (!all_recurrent_set.count(partner)) continue;
            ++matched_union;
            if (scc_id_of[partner] == scc_id_of[u]) ++matched_same_scc; else ++matched_cross_scc;
        }
        std::printf("  involution over the UNION of all recurrent SCCs: %d/%zu matched "
                     "(%d same-SCC, %d cross-SCC)\n",
                     matched_union, all_recurrent_nodes.size(), matched_same_scc, matched_cross_scc);
    }

    diagnose_dominant_scc(name, rep, rule);

    bool is_pure_pairing = false;
    auto gbq = gb_core_quotient_charpoly(rep, &is_pure_pairing);
    if (matched == total && !is_pure_pairing) {
        std::printf("  NOTE: involution is exact but the coarsest equitable partition of the "
                     "core is NOT a pure 2-class-per-orbit pairing -- the quotient-by-involution "
                     "step needs the explicit involution's own orbits, not the coarsest-partition "
                     "shortcut, for this n. Skipping the charpoly-division check.\n");
    }
    if (is_pure_pairing) {
        auto bpc = bp_core_charpoly(rule);
        check_exact_factor(name, bpc, gbq);
    }
}

// Runtime n -> compile-time D dispatch, n = 2..8 (MAX_DISPATCH_D in
// contact_boundary.hpp). Extend both this switch and MAX_DISPATCH_D
// together if a future need for n > 8 comes up.
void run_n_bonacci(std::size_t n) {
    switch (n) {
        case 2: run_n_bonacci_case<2>(n); return;
        case 3: run_n_bonacci_case<3>(n); return;
        case 4: run_n_bonacci_case<4>(n); return;
        case 5: run_n_bonacci_case<5>(n); return;
        case 6: run_n_bonacci_case<6>(n); return;
        case 7: run_n_bonacci_case<7>(n); return;
        case 8: run_n_bonacci_case<8>(n); return;
        default:
            std::printf("n=%zu: outside compile-time dispatch range [2,8]\n", n);
    }
}

void run_named_case(const char* name, std::vector<std::vector<std::int8_t>> sigma,
                     double beta, std::size_t alphabet) {
    SubstitutionRule rule(sigma);
    ContactBoundaryReport rep;
    switch (alphabet) {
        case 2: rep = run_pipeline<2>(rule, beta); break;
        case 3: rep = run_pipeline<3>(rule, beta); break;
        case 4: rep = run_pipeline<4>(rule, beta); break;
        case 5: rep = run_pipeline<5>(rule, beta); break;
        default: std::printf("%s: unsupported alphabet %zu\n", name, alphabet); return;
    }
    double ratio = rep.boundary_eigenvalue / rep.bp_rho_nc;
    std::printf("%s: BP-rho_nc=%.6f lambda(G_B)=%.6f ratio=%.6f (%s, double-precision)\n",
                name, rep.bp_rho_nc, rep.boundary_eigenvalue, ratio,
                std::abs(ratio - 1.0) < 1e-4 ? "MATCH" : "mismatch");
    {
        auto bp_core = recurrent_core_dense_matrix(balanced_pair_transition_graph(rule).matrix);
        auto gb_core = recurrent_core_dense_matrix(rep.gb_matrix);
        print_perron_comparison(bp_core, gb_core);
    }
    diagnose_dominant_scc(name, rep, rule);
}

}  // namespace

int main(int argc, char** argv) {
    // The switch: `./gb_bp_involution_general_n [exact|bigfloat] [precision_bits] [max_iters]`
    //   exact              -- math/ball.hpp's bit-exact rational Collatz-Wielandt
    //                         bracket (default). Proves inequality; for an
    //                         irrational shared root, "equal" never fires --
    //                         see print_perron_comparison's comment.
    //   bigfloat           -- math/bigfloat.hpp's tunable-precision bracket.
    //                         precision_bits (default 256) controls the mantissa
    //                         width, i.e. the memory-per-number / precision
    //                         tradeoff the caller is tuning; max_iters (default
    //                         200) is shared by both methods.
    if (argc >= 2) {
        std::string method_arg = argv[1];
        if (method_arg == "bigfloat") {
            g_method = mathlib::PerronCertifyMethod::TunedBigFloat;
        } else if (method_arg == "exact") {
            g_method = mathlib::PerronCertifyMethod::ExactRationalBracket;
        } else {
            std::fprintf(stderr,
                "usage: %s [exact|bigfloat] [precision_bits] [max_iters]\n", argv[0]);
            return 2;
        }
    }
    if (argc >= 3) g_precision_bits = static_cast<unsigned>(std::atoi(argv[2]));
    if (argc >= 4) g_max_iters = std::atoi(argv[3]);
    std::printf("=== switch: method=%s%s  max_iters=%d ===\n\n",
                g_method == mathlib::PerronCertifyMethod::ExactRationalBracket ? "exact" : "bigfloat",
                g_method == mathlib::PerronCertifyMethod::TunedBigFloat
                    ? (std::string("  precision_bits=") + std::to_string(g_precision_bits)).c_str()
                    : "",
                g_max_iters);

    std::printf("=== n-bonacci family, n=2..8, generated programmatically ===\n");
    std::printf("(n=2..5 reproduce app/gb_bp_involution_check.cpp's committed\n");
    std::printf(" numbers exactly; n=6,7,8 -- Hexanacci, Heptanacci, Octanacci -- are new)\n\n");
#ifdef SPECTRE_ONLY_N
    run_n_bonacci(SPECTRE_ONLY_N);
#else
    for (std::size_t n = 2; n <= 5; ++n) {
        run_n_bonacci(n);
    }
    std::printf("\n(n=6,7,8 omitted from this default run -- n=6 alone takes several\n"
                " minutes; see the separate SPECTRE_ONLY_N=6 diagnostic run and\n"
                " docs/RESEARCH_STATUS.md for the multi-SCC findings.)\n");
#endif

#ifndef SPECTRE_ONLY_N
    std::printf("\n=== non-simple-Parry unimodular Pisot: certification of the known mismatch ===\n");
    run_named_case("sigma_1", {{0, 0, 0, 1}, {0, 0, 2}, {0}}, 3.6273650847118, 3);
    run_named_case("sigma_2", {{0, 0, 1}, {0, 0, 0, 2}, {0}}, 3.0795956234914, 3);
#endif
    return 0;
}

// ---------------------------------------------------------------------
// What this driver is for, precisely (read before citing its output):
//
// It extends the NUMERICAL side of thread A item (1) -- more values of
// n confirming the involution is exact and the charpoly divides -- but
// it is still a per-n computation, same as the n=2..5 cases. Seeing
// n=6,7,8 also match does not by itself constitute "a general, all-n
// argument [...] derivable directly from the substitution's explicit
// combinatorial structure" (the actual ask in
// docs/DIRECTION_AND_OPEN_THREADS.md thread A item 1) -- it raises
// confidence that such an argument exists and rules out "the pattern
// breaks at n=6" as a concern, but the closed-form proof is a
// separate, still-open piece of work.
// ---------------------------------------------------------------------
