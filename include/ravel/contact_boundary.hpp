// contact_boundary.hpp
//
// Driver layer for the contact-boundary-map machinery: glues
// SubstitutionRule (the alphabet-size-agnostic representation
// used by the existing Lua bindings) to the templated
// Substitution<d> machinery in core.hpp / ambient_graph.hpp /
// corona.hpp / d_cont_check.hpp, and produces a single structured
// record covering the full Def 3.1 -> Def 3.5 -> Def 3.9 -> Alg 2
// pipeline:
//
//   D_cont -> backward_closure -> Red -> C -> ±C -> c_corona -> Red -> G_B
//
// plus the dominant eigenvalue of G_B's adjacency matrix (with
// multiplicity), and a separate `bp_rho_nc` slot for the
// Hollander-Solomyak balanced-pair growth rate.
//
// Per `docs/THEOREM_STATUS.md §3.1`, the two
// numbers (BP-`rho_nc` and `lambda(G_B)`) are NOT a theorem for
// the November-2025 family -- they're a research conjecture
// (σ_1: 2.286 vs 1.746).  This module therefore exposes them as
// a PAIR tagged "conjecture: equal?", never as a single equality.
//
// Currently dispatched on d = 2..9 (covering the canonical alphabets of the
// first six Pisot cases, including the fifth/sixth cases with 8/9 letters).
// Larger alphabets still error out with a clear runtime message; dispatch
// support does not by itself promise that an unbounded contact search will
// close within a finite resource budget.

#pragma once

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <map>
#include <numeric>
#include <set>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "ravel/core.hpp"
#include "ravel/substitution.hpp"
#include "ravel/spectral.hpp"
#include "ravel/ambient_graph.hpp"
#include "ravel/corona.hpp"
#include "ravel/d_cont_check.hpp"
#include "ravel/balanced_pair.hpp"

namespace ravel {

// Maximum backward-closure size before the algorithm stops and reports
// what it has.  Paper claims 14 for sigma_1; anything close to this
// is a real contact set; anything close to MAX_NODES is almost
// certainly an implementation bug (or a non-Pisot / near-Salem
// instance where the closure legitimately explodes).
// For d=3 unimodular Pisot (Tribonacci, sigma_1, sigma_2, ...) the
// closure lands at <= 26 nodes. For d=4 (Tetrabonacci, random 4x4
// Pisot candidates) the closure can legitimately reach 100-500+
// nodes -- bumped to 5000 to accommodate the random Pisot survey
// (which can produce candidates with complex contact structure).
//
// (The cap is now lifted to a runtime-configurable `closure_cap`
// field in `ContactBoundaryLimits`; the compile-time default below
// is the historical value used by every published run.  See
// `ContactBoundaryLimits` for the full set of runtime knobs.)
constexpr std::size_t kContactBoundaryMaxNodesDefault = 5000;

// All the caps the contact-boundary pipeline can hit, bundled so
// the implementing layer (i.e. the caller of
// `compute_contact_boundary_from_subst`) can specify them as a
// single struct rather than threading each one individually.  The
// field defaults here are the historical values used by every
// published run (the 39-/87-candidate non-unimodular Pisot survey
// and the original Hexanacci experiments) -- reproducing those
// results is just `ContactBoundaryLimits{}` with no overrides.
struct ContactBoundaryLimits {
    // Caps the BP-rho_nc BFS in the balanced-pair certify path:
    // stops when the BFS has explored more than this many distinct
    // (state, rule) pairs without converging.  Historical 20000.
    std::size_t max_rho_pairs = 20000;

    // Caps the BP-rho_nc BFS by the total word-length explored:
    // stops when any BFS path exceeds this length.  Historical
    // 60000 (corresponds to beta^15ish for beta ~ 1.8, which is
    // plenty for the cubic/4-letter Pisot families).
    std::size_t max_rho_len = 60000;

    // Caps the backward-closure pass that produces G_P from
    // D_cont.  Beyond this many nodes we assume the closure
    // legitimately exploded and bail out (the resulting G_B
    // is suspect and the report's `closure_stopped_early` is
    // set).  Historical 5000.
    std::size_t closure_cap = 5000;

    // Caps the corona iteration A_{p+1} = Red(corona(A_p, A_p))
    // for p = 2..max_rounds.  The corona can legitimately produce
    // tens of thousands of nodes before stabilizing for
    // 4-letter Pisot with complex contact structure; this cap
    // records whether it hit the cap (the report's
    // `corona_capped` flag) and returns the partial G_B.
    // Historical 50000.
    std::size_t corona_cap = 50000;

    // Maximum number of corona iteration rounds.  Pisot
    // substitutions typically converge in 2-4 rounds; 8 is
    // generous headroom for the (very rare) slow-converging
    // case.  Historical 8.
    int max_corona_rounds = 8;

    // Dense G_B spectral data is useful for settled small runs but scales as
    // |G_B|^2. Exploratory high-dimensional probes may retain only the node
    // set and cap evidence, avoiding a multi-gigabyte dense allocation.
    bool retain_boundary_matrix = true;
};

// Build a Substitution<d> from a SubstitutionRule + the dominant
// eigenvalue beta (already computed by classify_matrix_spectral
// or spectral_invariants_3x3).
template <std::size_t d>
Substitution<d> make_substitution(const SubstitutionRule& r, double beta) {
    std::array<std::vector<long long>, d> images{};
    if (r.alphabet_size() != d) {
        throw std::invalid_argument(
            "make_substitution: alphabet size mismatch (rule has "
            + std::to_string(r.alphabet_size()) + ", expected "
            + std::to_string(d) + ")");
    }
    for (std::size_t c = 0; c < d; ++c) {
        for (auto r_letter : r.image(c)) {
            images[c].push_back(static_cast<long long>(r_letter));
        }
    }
    return Substitution<d>(images, beta);
}

// Forward-iteration-counting record: how many times the corona
// fixed-point loop ran before stabilizing (1 = converged at
// round 2, i.e. A[2] == A[1]).
struct ContactBoundaryReport {
    // Substitution metadata
    std::size_t alphabet_size = 0;
    double beta = 0.0;
    double b2 = 0.0;  // second-largest-in-magnitude eigenvalue (Pisot dominance)

    // Pipeline counts (each layer)
    std::size_t d_cont_size = 0;       // |D_cont| (input from caller)
    std::size_t pre_contact_size = 0;  // |G_P| = backward_closure(D_cont)
    std::size_t contact_size = 0;      // |C| = Red(G_P)
    std::size_t signed_contact_size = 0;  // |±C| = C ∪ -C
    std::size_t boundary_size = 0;     // |G_B| = algorithm2(C) fixed point
    int convergence_rounds = 0;        // rounds of algorithm2 until convergence
    bool converged = false;            // true iff algorithm2 hit fixed point
    bool closure_stopped_early = false; // true iff backward_closure hit limits.closure_cap
    bool corona_capped = false;        // true iff corona iteration hit limits.corona_cap
    bool boundary_spectral_skipped = false;
    std::size_t max_a_size_reached = 0; // largest |A_p| observed (if corona capped)

    // The cap set the implementing layer actually used for this
    // run.  Recorded so a downstream consumer (a re-audit driver,
    // a results-dump script, a future session comparing the
    // pipeline's output across runs with different caps) can tell
    // at a glance whether a `closure_stopped_early` /
    // `corona_capped` flag is the pipeline hitting its cap (the
    // run is incomplete) or the run simply being at the cap
    // without hitting it.  Default-constructed `ContactBoundaryLimits`
    // gives the historical 20000/60000/5000/50000/8 values.
    ContactBoundaryLimits limits_used;

    // The two numbers, kept STRICTLY DISTINCT.
    // BP-`rho_nc` (balanced-pair growth rate) / σ_1: 2.286298.
    // λ(G_B) (dominant eigenvalue of G_B's adjacency matrix with
    // edge multiplicity) / σ_1: 1.7462.
    // Per ANSWERS_TO_QUESTIONS.txt §3.1, ρ_nc == λ(G_B) is an
    // open conjecture for the November-2025 family, NOT a
    // theorem.  Always report as a pair; never collapse.
    double bp_rho_nc = 0.0;
    double boundary_eigenvalue = 0.0;

    // G_B node set (as a list of (i, x[0..d-1], j) tuples).
    std::vector<std::tuple<long long, std::vector<long long>, long long>>
        boundary_nodes;

    // G_B adjacency matrix (integer entries, edge multiplicity).
    // Indexed in the same order as boundary_nodes.  Size |G_B| x |G_B|.
    // Populated by compute_contact_boundary<d>; left empty by
    // compute_contact_boundary_from_subst (which doesn't track the
    // matrix internally).
    std::vector<std::vector<long long>> gb_matrix;

    // Conjecture status: equality is NOT assumed; we just
    // surface the two numbers as data.
    // status = "OPEN CONJECTURE: BP-`rho_nc` may equal λ(G_B) but
    // is unproven for the November-2025 family."
    std::string conjecture_status;
};

// Structural analysis of a substitution rule.  Computed BEFORE the
// pipeline runs (so it's available even for EXPLODED cases).  Each
// field is a candidate feature for classifying whether the
// conjecture `bp_rho_nc == λ(G_B)` empirically holds.  See
// `lua/docs/FINDINGS_contact_boundary_survey.md` for the
// classification analysis built on top of these.
//
// Notation: β = dominant Pisot root; M = incidence matrix;
// |b₂| = second-largest-in-magnitude eigenvalue of M.
struct SubstitutionAnalysis {
    // Basic structure.
    std::size_t alphabet_size = 0;
    std::vector<std::size_t> image_lengths;  // |σ(i)| for each letter i
    std::vector<std::size_t> sigma_total_length = {0};  // sum of |σ(i)|

    // Pisot / matrix invariants.
    double beta = 0.0;                       // dominant eigenvalue
    double b2 = 0.0;                         // |second-largest|
    double b3 = 0.0;                         // |smallest| (for d=3)
    long long det_M = 0;                    // det of incidence matrix
    bool   is_pisot = false;                 // beta > 1, all others |z| < 1
    bool   is_unimodular = false;            // |det M| == 1
    bool   is_primitive = false;             // gcd of column lengths == 1
    bool   is_injective = false;             // distinct letters in each σ(i)
    double pisot_quality = 0.0;              // |b2| / beta

    // Secondary spectrum structure.  The "conformal" case (single
    // modulus in the secondary spectrum, e.g. Tribonacci/σ_{a,b}
    // with one complex conjugate pair) is structurally different
    // from the "non-conformal" case (multiple moduli), and the
    // conjectured identity is more likely to hold in the conformal
    // case (per `boundary_dimension_shortcut.py`'s docstring).
    bool   conformal = false;                // single modulus in secondary
    std::vector<double> secondary_eigenvalues;  // all non-β eigenvalues
    std::vector<double> secondary_moduli;       // distinct |z| values
    std::size_t moduli_count = 0;               // = secondary_moduli.size()

    // Barge-Diamond / Pisot-certification heuristics.
    // distinct_initials: σ(i) starts with a different letter for each i.
    // constant_finals: σ(i) ends with the same letter for all i.
    // constant_factor: ∃ letter c such that for all i, c appears at
    //   the same fixed position k in σ(i) — the **Laurent Pisot /
    //   "conformal" condition** that makes the BP-ρ_nc == λ(G_B)
    //   algebraic proof work for the σ_{a,b} family.  This is the
    //   feature we HYPOTHESIZE is the actual classifier.
    bool   distinct_initials = false;
    bool   constant_finals = false;
    bool   has_constant_factor = false;       // ∃c,k: σ(i)[k] == c ∀i
    std::vector<std::size_t> constant_factor_positions;  // k values for which ∃c

    // Symbolic entropy of the contact graph: h = log(λ(G_B)) / log(β).
    // (Computed downstream from ContactBoundaryReport.)
    double symbolic_entropy = 0.0;
};

// Compute structural invariants of a substitution rule.  Pure
// function of (σ, β): doesn't run the pipeline.  Returns the
// analysis even for non-Pisot substitutions (just marks is_pisot=false).
//
// For d > 3 (e.g. Tetrabonacci) the analysis only computes the
// 3-letter-relevant fields; secondary spectrum details are
// approximate.
inline SubstitutionAnalysis analyze_substitution(
        const SubstitutionRule& r, double beta, double b2) {
    SubstitutionAnalysis a;
    a.alphabet_size = r.alphabet_size();
    a.beta = beta;
    a.b2 = b2;
    a.sigma_total_length = {0};

    // Image lengths.
    for (std::size_t c = 0; c < r.alphabet_size(); ++c) {
        a.image_lengths.push_back(r.image(c).size());
        a.sigma_total_length[0] += r.image(c).size();
    }

    // Incidence matrix M[r][c] = count of letter r in image of c.
    std::vector<std::vector<long long>> M(r.alphabet_size(),
        std::vector<long long>(r.alphabet_size(), 0));
    for (std::size_t c = 0; c < r.alphabet_size(); ++c) {
        for (auto letter : r.image(c)) {
            M[static_cast<std::size_t>(letter)][c] += 1;
        }
    }
    // det(M) via closed-form for d=2,3; recursive cofactor for d=4.
    auto det_2 = [&](long long a, long long b, long long c, long long d) {
        return a*d - b*c;
    };
    auto det_3 = [&](const std::vector<std::vector<long long>>& m) {
        return m[0][0]*(m[1][1]*m[2][2] - m[1][2]*m[2][1])
             - m[0][1]*(m[1][0]*m[2][2] - m[1][2]*m[2][0])
             + m[0][2]*(m[1][0]*m[2][1] - m[1][1]*m[2][0]);
    };
    auto det_4 = [&](const std::vector<std::vector<long long>>& m) {
        // 4x4 determinant via Laplace expansion along the first row.
        // Slow (O(d!)) but correct.  For d=4 the matrix is small.
        long long d = 0;
        auto det3 = [&](const std::array<std::array<long long, 3>, 3>& mm) {
            return mm[0][0]*(mm[1][1]*mm[2][2] - mm[1][2]*mm[2][1])
                 - mm[0][1]*(mm[1][0]*mm[2][2] - mm[1][2]*mm[2][0])
                 + mm[0][2]*(mm[1][0]*mm[2][1] - mm[1][1]*mm[2][0]);
        };
        for (std::size_t j = 0; j < 4; ++j) {
            std::array<std::array<long long, 3>, 3> mm;
            for (std::size_t ii = 0, r = 0; ii < 4; ++ii) {
                if (ii == 0) continue;
                for (std::size_t jj = 0, c = 0; jj < 4; ++jj) {
                    if (jj == j) continue;
                    mm[r][c++] = m[ii][jj];
                }
                ++r;
            }
            d += (j % 2 == 0 ? 1 : -1) * m[0][j] * det3(mm);
        }
        return d;
    };
    if (r.alphabet_size() == 2) {
        a.det_M = det_2(M[0][0], M[0][1], M[1][0], M[1][1]);
    } else if (r.alphabet_size() == 3) {
        a.det_M = det_3(M);
    } else if (r.alphabet_size() == 4) {
        a.det_M = det_4(M);
    }
    a.is_unimodular = (std::abs(a.det_M) == 1);

    // Pisot test: beta > 1, all secondary |z| < 1.
    // (b2 is the SECOND-largest-in-magnitude; we use |b2| < 1.)
    a.is_pisot = (beta > 1.05) && (b2 < 1.0 - 1e-9);
    a.pisot_quality = (beta > 0) ? (b2 / beta) : 0.0;

    // Compute secondary eigenvalues via the characteristic polynomial.
    // For d=2: roots = (tr ± sqrt(tr^2 - 4*det)) / 2.
    // For d=3: use the closed-form cubic (matching spectral.hpp's
    // spectral_invariants_3x3 logic; we accept the precision
    // caveat for near-degenerate cases).
    if (r.alphabet_size() == 2) {
        double tr = static_cast<double>(M[0][0] + M[1][1]);
        double det = static_cast<double>(a.det_M);
        double disc = tr*tr - 4.0*det;
        if (disc >= 0) {
            double sq = std::sqrt(disc);
            a.secondary_eigenvalues.push_back((tr - sq) * 0.5);
            a.secondary_eigenvalues.push_back((tr + sq) * 0.5);
        } else {
            // complex pair, real part = tr/2
            double re = tr * 0.5;
            a.secondary_eigenvalues.push_back(re);
        }
        // After ordering by magnitude (we assume b2 is the second)
        a.secondary_moduli.push_back(b2);
        a.moduli_count = 1;
        a.conformal = (a.secondary_moduli.size() == 1);
    } else if (r.alphabet_size() == 3) {
        // Closed-form cubic (same as spectral.hpp; precision OK for
        // Pisot with |b2| < 0.95).
        double m_[3][3] = {
            {static_cast<double>(M[0][0]), static_cast<double>(M[0][1]), static_cast<double>(M[0][2])},
            {static_cast<double>(M[1][0]), static_cast<double>(M[1][1]), static_cast<double>(M[1][2])},
            {static_cast<double>(M[2][0]), static_cast<double>(M[2][1]), static_cast<double>(M[2][2])},
        };
        double tr = m_[0][0] + m_[1][1] + m_[2][2];
        double cf = m_[0][0]*m_[1][1] + m_[0][0]*m_[2][2] + m_[1][1]*m_[2][2]
                  - m_[0][1]*m_[1][0] - m_[0][2]*m_[2][0] - m_[1][2]*m_[2][1];
        double det = static_cast<double>(a.det_M);
        double p = (3.0 * cf - tr * tr) / 3.0;
        double q = (9.0 * tr * cf - 2.0 * tr * tr * tr - 27.0 * det) / 27.0;
        // Cardano discriminant: disc = q²/4 + p³/27.
        //   disc > 0 : casus irreducibilis, one real root + complex
        //              conjugate pair (the Pisot case).
        //   disc = 0 : multiple real roots.
        //   disc < 0 : three distinct real roots.
        double disc = q * q / 4.0 + p * p * p / 27.0;
        double tr3 = tr / 3.0;
        if (disc > 0) {
            // Pisot case: one real root (= β) + one complex conjugate
            // pair.  We only need the modulus of the complex pair as
            // a "secondary eigenvalue" -- both have the same |z|.
            double sd = std::sqrt(disc);
            double u = std::cbrt(-q / 2.0 + sd);
            double v = std::cbrt(-q / 2.0 - sd);
            double re_part = -(u + v) * 0.5 + tr3;
            double im_part = (std::sqrt(3.0) * 0.5) * (u - v);
            double mod = std::sqrt(re_part * re_part + im_part * im_part);
            // Sanity: should equal sqrt(|det|/β) for unimodular Pisot.
            // Push the modulus twice (for the conjugate pair).
            a.secondary_eigenvalues.push_back(mod);
            a.secondary_eigenvalues.push_back(mod);
        } else if (std::abs(disc) < 1e-15) {
            // Multiple real roots (rare for Pisot, but handle it).
            // The dominant root is β; the other two are equal.
            double u = std::cbrt(-q / 2.0);
            a.secondary_eigenvalues.push_back(-u + tr3);
            a.secondary_eigenvalues.push_back(-u + tr3);
        } else {
            // Three distinct real roots (Salem case or generic Pisot
            // with three real secondary eigenvalues).  The dominant
            // root is β; the other two are reported as-is.
            double r = std::sqrt(-p * p * p / 27.0);
            double phi = std::acos(std::clamp(-q / (2.0 * r), -1.0, 1.0));
            double m_cbrt = std::cbrt(r);
            a.secondary_eigenvalues.push_back(
                2.0 * m_cbrt * std::cos((phi + 2.0 * M_PI) / 3.0) + tr3);
            a.secondary_eigenvalues.push_back(
                2.0 * m_cbrt * std::cos((phi + 4.0 * M_PI) / 3.0) + tr3);
        }
        // b3 = |smallest|
        double bmin = std::abs(a.secondary_eigenvalues[0]);
        for (auto e : a.secondary_eigenvalues) bmin = std::min(bmin, std::abs(e));
        a.b3 = bmin;
        // Distinct moduli (with 1e-9 tolerance).
        std::map<double, bool> seen;
        for (auto e : a.secondary_eigenvalues) {
            double m = std::abs(e);
            bool new_modulus = true;
            for (auto& kv : seen) if (std::abs(kv.first - m) < 1e-9) new_modulus = false;
            if (new_modulus) seen[m] = true;
        }
        for (auto& kv : seen) a.secondary_moduli.push_back(kv.first);
        a.moduli_count = a.secondary_moduli.size();
        a.conformal = (a.moduli_count == 1);
    } else {
        // d=4: classify by moduli of the secondary 3 spectrum.  We
        // approximate by computing the 4 roots of the char poly
        // via the closed-form quartic (companion matrix eigenvalues
        // via Newton's identities).  For our purposes (Pisot
        // dominance) the dominant is β and the others are < β.
        // Just report |b2|, no conformal classification.
        a.conformal = false;
        a.moduli_count = 0;
    }

    // Primitive substitution: gcd of column lengths == 1.
    std::size_t g = 0;
    for (auto len : a.image_lengths) g = std::gcd(g, len);
    a.is_primitive = (g == 1);

    // Injective: σ is letter-injective (no letter appears twice in
    // any single σ(i)).
    a.is_injective = true;
    for (std::size_t c = 0; c < r.alphabet_size(); ++c) {
        std::set<long long> seen;
        for (auto l : r.image(c)) {
            if (!seen.insert(l).second) { a.is_injective = false; break; }
        }
        if (!a.is_injective) break;
    }

    // Barge-Diamond heuristics.
    // distinct_initials: σ(0)[0], σ(1)[0], ..., σ(d-1)[0] are all distinct.
    a.distinct_initials = true;
    if (r.alphabet_size() > 0) {
        std::set<long long> initials;
        for (std::size_t c = 0; c < r.alphabet_size(); ++c) {
            if (r.image(c).empty()) { a.distinct_initials = false; break; }
            if (!initials.insert(r.image(c)[0]).second) {
                a.distinct_initials = false; break;
            }
        }
    }
    // constant_finals: σ(i) ends with the same letter for all i.
    a.constant_finals = true;
    if (r.alphabet_size() > 0) {
        long long final = r.image(0).back();
        for (std::size_t c = 1; c < r.alphabet_size(); ++c) {
            if (r.image(c).back() != final) {
                a.constant_finals = false; break;
            }
        }
    }
    // constant_factor: ∃ letter c such that for all i, c appears at
    // the same fixed position k in σ(i).  This is the structural
    // condition under which the σ_{a,b} BP-rho_nc == λ(G_B) proof
    // works, and the leading HYPOTHESIS for what's special about
    // the HOLDS set.
    a.has_constant_factor = false;
    std::size_t max_len = 0;
    for (auto len : a.image_lengths) max_len = std::max(max_len, len);
    for (std::size_t k = 0; k < max_len; ++k) {
        long long common = -1;
        bool is_constant = true;
        for (std::size_t c = 0; c < r.alphabet_size(); ++c) {
            if (k >= r.image(c).size()) { is_constant = false; break; }
            long long at_k = r.image(c)[k];
            if (common == -1) common = at_k;
            else if (common != at_k) { is_constant = false; break; }
        }
        if (is_constant) {
            a.constant_factor_positions.push_back(k);
            a.has_constant_factor = true;
        }
    }

    return a;
}

// Compute the dominant eigenvalue of an adjacency-matrix-like
// square matrix (with multiplicity).  Uses power iteration
// with double precision and the local
// `core.hpp::compute_right_eigenvector`-style loop.  Returns
// the dominant eigenvalue as a non-negative double.
template <std::size_t d>
double dominant_eigenvalue(const std::vector<std::vector<double>>& A) {
    const std::size_t n = A.size();
    if (n == 0) return 0.0;
    // Cap iterations so a 1000-node G_B doesn't take 10+ seconds
    // computing the eigenvalue (it's O(n^2) per iteration).
    int max_iter = 20000;
    if (n > 500) max_iter = 2000;
    if (n > 1000) max_iter = 500;
    std::vector<double> v(n, 1.0 / static_cast<double>(n));
    std::vector<double> w(n, 0.0);
    for (int it = 0; it < max_iter; ++it) {
        for (std::size_t i = 0; i < n; ++i) {
            double s = 0.0;
            for (std::size_t j = 0; j < n; ++j) s += A[i][j] * v[j];
            w[i] = s;
        }
        double norm = 0.0;
        for (auto x : w) norm += x * x;
        norm = std::sqrt(norm);
        if (norm < 1e-15) return 0.0;
        for (auto& x : w) x /= norm;
        // Check convergence every 100 iterations.
        if (it > 100 && it % 100 == 0) {
            double err = 0.0;
            for (std::size_t i = 0; i < n; ++i) err += std::abs(w[i] - v[i]);
            if (err < 1e-12) break;
        }
        v = w;
    }
    // Final eigenvalue = v^T A v (Rayleigh quotient)
    double num = 0.0, den = 0.0;
    for (std::size_t i = 0; i < n; ++i) {
        double s = 0.0;
        for (std::size_t j = 0; j < n; ++j) s += A[i][j] * v[j];
        num += v[i] * s;
        den += v[i] * v[i];
    }
    return (den > 1e-15) ? (num / den) : 0.0;
}

// Build the G_B adjacency matrix from a set of boundary nodes and
// the simple-edge sweep restricted to those nodes.  Used by the
// two `compute_contact_boundary<d>` overloads above to compute
// `rep.boundary_eigenvalue` (and to populate `rep.gb_matrix` for
// downstream Q(beta) analysis).  Templated on d because the
// underlying SNode<d> type's x-vector length is d.
template <std::size_t d>
inline std::vector<std::vector<double>> gb_adjacency_matrix(
        const std::set<SNode<d>>& nodes,
        const std::vector<std::pair<SNode<d>, SNode<d>>>& edges) {
    std::vector<SNode<d>> node_list(nodes.begin(), nodes.end());
    std::map<SNode<d>, std::size_t> idx;
    for (std::size_t i = 0; i < node_list.size(); ++i) idx[node_list[i]] = i;
    std::vector<std::vector<double>> A(node_list.size(),
                                       std::vector<double>(node_list.size(), 0.0));
    for (const auto& [src, dest] : edges) {
        auto is = idx.find(src); auto id = idx.find(dest);
        if (is == idx.end() || id == idx.end()) continue;
        A[is->second][id->second] += 1.0;
    }
    return A;
}

// End-to-end driver: from a substitution alone, derive D_cont
// geometrically (bound = 2), then run the full pipeline.  This
// is the "automatic" entry point: no caller-supplied D_cont
// required.
//
// bound = 2 is sufficient for all the cubic Pisot substitutions
// the local project cares about (Fibonacci, Tribonacci, Plastic,
// Supergolden, σ_1, σ_2, σ_{a,b}).  For higher-degree Pisot
// substitutions (Tetrabonacci) you may need bound = 3 or 4 to
// find the full D_cont; the search is geometric and will find
// the right elements if they're in the box.
//
// max_rho_pairs / max_rho_len cap the BP-rho_nc BFS.  These default
// to the same values used by `compute_contact_boundary` (20000 /
// 60000) but can be reduced for fast batch runs over many
// substitutions where an approximate BP-rho_nc is sufficient.
//
// IMPORTANT: precision caveat.  The Python reference uses EXACT
// Q(β) arithmetic (via algebraic.py::NFElem) for in_H_sigma and
// face_intersection.  The C++ port uses double-precision power
// iteration for v, so spurious entries very close to the
// H_sigma boundary may slip through.  For the cubic Pisot
// substitutions the spurious count is small (~5-10% of the
// true D_cont) and doesn't materially affect the pipeline
// output: extra D_cont entries don't change |C|, |±C|, |G_B|
// by much (they're either absorbed by Red or filtered by the
// corona hop filter).  We surface `d_cont_size` in the report
// so callers can spot any anomaly.
template <std::size_t d>
ContactBoundaryReport compute_contact_boundary_from_subst(
        const SubstitutionRule& rule,
        double beta,
        double b2,
        long long search_bound = 2,
        const ContactBoundaryLimits& limits = ContactBoundaryLimits{}) {
    ContactBoundaryReport rep;
    rep.alphabet_size = d;
    rep.beta = beta;
    rep.b2 = b2;
    rep.limits_used = limits;

    auto subst = make_substitution<d>(rule, beta);
    auto d_cont_an = search_D_cont<d>(subst, search_bound);
    rep.d_cont_size = d_cont_an.size();

    // Convert to the triple format for compute_contact_boundary.
    std::vector<std::tuple<long long, std::vector<long long>, long long>>
        d_cont;
    d_cont.reserve(d_cont_an.size());
    for (const auto& c : d_cont_an) {
        std::vector<long long> xv(c.x.begin(), c.x.end());
        d_cont.emplace_back(c.i, std::move(xv), c.j);
    }
    auto rep_full = compute_contact_boundary<d>(rule, beta, b2, d_cont, limits);
    // We overrode the bp_rho_nc in compute_contact_boundary with the
    // default-capped one; if the caller passed different caps, the
    // rep_full already reflects them.  Just return it.
    return rep_full;
}

// Variant of compute_contact_boundary<d> that takes explicit caps
// on the BP-rho_nc BFS, the backward-closure pass, the corona
// iteration, and the max number of corona rounds.  All caps are
// bundled in `ContactBoundaryLimits`; the implementing layer (the
// caller of `compute_contact_boundary_from_subst`) can specify them
// as a single struct rather than threading each one individually.
// For batch runs over many Pisot substitutions the defaults (20000
// / 60000 / 5000 / 50000 / 8) are too slow on near-Salem or large
// n_irr instances; capping at 4000/10000/1000/10000/8 gives a
// usable approximation in a fraction of the time.
template <std::size_t d>
ContactBoundaryReport compute_contact_boundary(
        const SubstitutionRule& r,
        double beta,
        double b2,
        const std::vector<std::tuple<long long,
                                     std::vector<long long>,
                                     long long>>& d_cont_input,
        const ContactBoundaryLimits& limits = ContactBoundaryLimits{}) {
    ContactBoundaryReport rep;
    rep.alphabet_size = d;
    rep.beta = beta;
    rep.b2 = b2;
    rep.d_cont_size = d_cont_input.size();
    rep.limits_used = limits;

    auto subst = make_substitution<d>(r, beta);

    std::vector<ANode<d>> d_cont_an;
    d_cont_an.reserve(d_cont_input.size());
    for (const auto& [i, x, j] : d_cont_input) {
        ANode<d> n;
        n.i = i; n.j = j;
        for (std::size_t k = 0; k < d; ++k) n.x[k] = (k < x.size()) ? x[k] : 0;
        d_cont_an.push_back(n);
    }

    auto gp_nodes = backward_closure<d>(subst, d_cont_an, limits.closure_cap);
    rep.pre_contact_size = gp_nodes.size();
    rep.closure_stopped_early = (gp_nodes.size() >= limits.closure_cap);

    auto gp_edges = induced_restricted_edges<d>(subst, gp_nodes);
    std::set<ANode<d>> gp_set(gp_nodes.begin(), gp_nodes.end());
    auto red_gp = red_anode<d>(gp_set, gp_edges);
    auto& c_nodes_an = red_gp.first;
    rep.contact_size = c_nodes_an.size();

    std::set<SNode<d>> C;
    for (const auto& a : c_nodes_an) {
        SNode<d> s; s.i = a.i; s.j = a.j; s.x = a.x;
        C.insert(s);
    }

    auto pmC = build_signed_contact_set<d>(C);
    rep.signed_contact_size = pmC.size();

    auto A_prev = pmC;
    rep.convergence_rounds = 1;
    rep.converged = false;
    // The corona iteration A_p+1 = Red(corona(A_p, A_p)) is supposed
    // to converge quickly for Pisot substitutions (typically 2-4
    // rounds). For 4-letter Pisot candidates with large boundary
    // graphs (3000+ nodes), the corona can legitimately produce
    // tens of thousands of nodes before stabilizing.  The cap
    // records whether it was hit (the report's `corona_capped`
    // flag) and returns the partial G_B.
    std::set<SNode<d>> gb;
    bool corona_capped_hit = false;
    for (int p = 2; p <= limits.max_corona_rounds; ++p) {
        // Definition 3.9 composes the evolving layer with the fixed
        // signed contact set ±C.  The historical implementation passed
        // A_prev as the connector set too.  That happens to reach the
        // same fixed point on the audited Class-II cases, but produces
        // many unnecessary candidates and obscures the layer induction.
        bool corona_projection_capped = false;
        std::set<SNode<d>> corona_nodes;
        if (limits.corona_cap == 0 ||
            default_corona_execution_mode() == CoronaExecutionMode::legacy_materialized) {
            corona_nodes = c_corona<d>(subst, A_prev, pmC);
        } else {
            std::size_t accepted = 0;
            corona_nodes = c_corona_projected<d>(
                subst, A_prev, pmC,
                [&](const SNode<d>&) {
                    if (accepted >= limits.corona_cap) {
                        corona_projection_capped = true;
                        return false;
                    }
                    ++accepted;
                    return true;
                });
        }
        if (corona_projection_capped) {
            gb = A_prev;
            corona_capped_hit = true;
            rep.max_a_size_reached = limits.corona_cap + 1;
            break;
        }
        std::vector<std::tuple<SNode<d>, SNode<d>,
                               std::vector<long long>,
                               std::vector<long long>>> edges;
        std::vector<std::pair<SNode<d>, SNode<d>>> edges_simple;
        for (const auto& n : corona_nodes) {
            auto fwd = simple_forward_targets<d>(subst, n);
            for (const auto& [dest, pq] : fwd) {
                if (corona_nodes.count(dest) > 0) {
                    edges.push_back({n, dest, pq.first, pq.second});
                    edges_simple.push_back({n, dest});
                }
            }
        }
        auto red_result = red<d>(corona_nodes, edges);
        auto& A_cur = red_result.first;
        rep.convergence_rounds = p - 1;
        if (A_cur == A_prev) { gb = A_cur; rep.converged = true; break; }
        if (A_cur.size() > limits.corona_cap) {
            // Cap hit: keep the previous (smaller) layer as the best
            // approximation, and mark corona_capped.  This matches
            // the original hardcoded behavior -- when MAX_A_SIZE
            // was hit we kept the partial G_B rather than aborting.
            gb = A_prev;
            corona_capped_hit = true;
            rep.max_a_size_reached = A_cur.size();
            break;
        }
        A_prev = A_cur;
        if (p == limits.max_corona_rounds) { gb = A_cur; }
    }
    if (!rep.converged && !corona_capped_hit) {
        // Ran out of rounds without convergence or cap hit; use
        // whatever the last layer produced.
        gb = A_prev;
    }
    rep.boundary_size = gb.size();
    rep.corona_capped = corona_capped_hit;

    rep.boundary_nodes.reserve(gb.size());
    for (const auto& n : gb) {
        std::vector<long long> xvec(n.x.begin(), n.x.end());
        rep.boundary_nodes.emplace_back(n.i, std::move(xvec), n.j);
    }

    std::vector<std::pair<SNode<d>, SNode<d>>> gb_edges;
    for (const auto& n : gb) {
        auto fwd = simple_forward_targets<d>(subst, n);
        for (const auto& [dest, _pq] : fwd) {
            if (gb.count(dest) > 0) gb_edges.push_back({n, dest});
        }
    }
    if (!limits.retain_boundary_matrix) {
        rep.boundary_spectral_skipped = true;
    } else {
        auto A = gb_adjacency_matrix<d>(gb, gb_edges);
        rep.boundary_eigenvalue = dominant_eigenvalue<d>(A);
        // Also store the integer matrix (for downstream Q(beta) analysis).
        std::vector<SNode<d>> nodes(gb.begin(), gb.end());
        std::map<SNode<d>, std::size_t> idx;
        for (std::size_t i = 0; i < nodes.size(); ++i) idx[nodes[i]] = i;
        rep.gb_matrix.assign(nodes.size(),
                             std::vector<long long>(nodes.size(), 0));
        for (const auto& [src, dest] : gb_edges) {
            auto is = idx.find(src); auto id = idx.find(dest);
            if (is == idx.end() || id == idx.end()) continue;
            rep.gb_matrix[is->second][id->second] += 1;
        }
    }

    rep.bp_rho_nc = rho_nc(r, limits.max_rho_pairs, limits.max_rho_len);

    rep.conjecture_status =
        "OPEN CONJECTURE: BP-rho_nc may equal lambda(G_B) but is "
        "unproven for the November-2025 family; the two numbers "
        "are reported as DISTINCT measurements.";

    return rep;
}

// Maximum alphabet size the dispatch supports.  Bumping this value
// requires that `compute_contact_boundary<d>` and
// `compute_contact_boundary_from_subst<d>` be instantiated for the
// new d; the recursive templates below will pick them up
// automatically.
constexpr std::size_t MAX_DISPATCH_D = 9;

namespace contact_boundary_detail {

template <std::size_t d>
inline ContactBoundaryReport dispatch_by_d(
        const SubstitutionRule& r, double beta, double b2,
        const std::vector<std::tuple<long long,
                                     std::vector<long long>,
                                     long long>>& d_cont,
        const ContactBoundaryLimits& limits) {
    return compute_contact_boundary<d>(r, beta, b2, d_cont, limits);
}

// Recursive template: walks d from MAX_DISPATCH_D down to 2 and
// dispatches to the matching `compute_contact_boundary<d>`.  Each
// recursive step is `if constexpr` so only the matching d is
// instantiated.
template <std::size_t D>
inline ContactBoundaryReport dispatch_recurse(
        const SubstitutionRule& r, double beta, double b2,
        const std::vector<std::tuple<long long,
                                     std::vector<long long>,
                                     long long>>& d_cont,
        const ContactBoundaryLimits& limits) {
    if (r.alphabet_size() == D) {
        return dispatch_by_d<D>(r, beta, b2, d_cont, limits);
    }
    if constexpr (D > 2) {
        return dispatch_recurse<D - 1>(r, beta, b2, d_cont, limits);
    }
    throw std::invalid_argument(
        "compute_contact_boundary_dispatch: alphabet size "
        + std::to_string(r.alphabet_size())
        + " not in [2, " + std::to_string(MAX_DISPATCH_D) + "]");
}

template <std::size_t D>
inline ContactBoundaryReport dispatch_recurse_from_subst(
        const SubstitutionRule& rule, double beta, double b2,
        long long search_bound,
        const ContactBoundaryLimits& limits) {
    if (rule.alphabet_size() == D) {
        return compute_contact_boundary_from_subst<D>(rule, beta, b2,
                                                      search_bound, limits);
    }
    if constexpr (D > 2) {
        return dispatch_recurse_from_subst<D - 1>(
            rule, beta, b2, search_bound, limits);
    }
    throw std::invalid_argument(
        "compute_contact_boundary_from_subst_dispatch: alphabet size "
        + std::to_string(rule.alphabet_size())
        + " not in [2, " + std::to_string(MAX_DISPATCH_D) + "]");
}

}  // namespace contact_boundary_detail

// Runtime dispatcher: pick the right template instantiation by
// alphabet size.  Supports any alphabet size d in [2, MAX_DISPATCH_D]
// via a recursive template that walks d from MAX_DISPATCH_D down to 2
// and dispatches to the matching `compute_contact_boundary<d>`
// instantiation.  Each recursive step is `if constexpr` so only the
// matching d is instantiated.
//
// Bump MAX_DISPATCH_D to extend support to higher alphabet sizes
// (the recursive template will pick up new `compute_contact_boundary<D>`
// instantiations automatically as they're added).  The underlying
// `make_substitution<d>`, `search_D_cont<d>`, `c_corona<d>`,
// `algorithm2<d>`, and `gb_adjacency_matrix<d>` are all already
// templated on d, so the dispatch is the only place that needed
// editing.  `contact_boundary_test` covers d=2..8.
inline ContactBoundaryReport compute_contact_boundary_dispatch(
        const SubstitutionRule& r,
        double beta,
        double b2,
        const std::vector<std::tuple<long long,
                                     std::vector<long long>,
                                     long long>>& d_cont,
        const ContactBoundaryLimits& limits = ContactBoundaryLimits{}) {
    return contact_boundary_detail::dispatch_recurse<MAX_DISPATCH_D>(
        r, beta, b2, d_cont, limits);
}

// Runtime dispatcher for the variant that derives D_cont directly from
// the substitution.  Keep this alongside compute_contact_boundary_dispatch
// so non-template clients (notably the Lua binding) share the same supported
// alphabet range and limit handling.
inline ContactBoundaryReport compute_contact_boundary_from_subst_dispatch(
        const SubstitutionRule& rule,
        double beta,
        double b2,
        long long search_bound = 2,
        const ContactBoundaryLimits& limits = ContactBoundaryLimits{}) {
    return contact_boundary_detail::dispatch_recurse_from_subst<
        MAX_DISPATCH_D>(rule, beta, b2, search_bound, limits);
}

// One row in a batch run: a named substitution plus its precomputed
// spectral invariants plus a caller-supplied D_cont.  Used by the
// batch driver to crunch a family of substitutions at once and
// export the BP-rho_nc / lambda(G_B) pair for each.
struct ContactBoundaryBatchRow {
    std::string name;
    SubstitutionRule rule;
    double beta;
    double b2;
    std::vector<std::tuple<long long,
                           std::vector<long long>,
                           long long>> d_cont;
};

// Run the contact-boundary pipeline over a list of substitutions
// and return a list of reports in the same order.  Each report
// follows the same schema as `compute_contact_boundary_dispatch`.
//
// This is the "crunch other systems and spill out interesting
// data" hook: feed it a family (sigma_{a,b}, the survey rows in
// pisot_survey.jsonl, the named Pisot rings) and you get back a
// (BP-rho_nc, lambda(G_B)) pair per row, with the open-conjecture
// framing made explicit in `conjecture_status`.
//
// Errors for individual rows do not abort the batch; instead the
// `error` field of the returned record is set and the row is left
// otherwise empty.  Callers can filter on `record.error == nil`.
struct ContactBoundaryBatchRecord {
    std::string name;
    ContactBoundaryReport report;
    std::string error;
};

inline std::vector<ContactBoundaryBatchRecord>
compute_contact_boundary_batch(
        const std::vector<ContactBoundaryBatchRow>& rows) {
    std::vector<ContactBoundaryBatchRecord> out;
    out.reserve(rows.size());
    for (const auto& row : rows) {
        ContactBoundaryBatchRecord rec;
        rec.name = row.name;
        try {
            rec.report = compute_contact_boundary_dispatch(
                row.rule, row.beta, row.b2, row.d_cont);
        } catch (const std::exception& e) {
            rec.error = e.what();
        }
        out.push_back(std::move(rec));
    }
    return out;
}

}  // namespace ravel
