// gb_bp_bijection_attempt.cpp
//
// Item (1) from docs/DIRECTION_AND_OPEN_THREADS.md thread A: implements
// the periodic fixed-point solve that a previous session diagnosed but
// left unimplemented (BP-core's recurrent states form TWO OVERLAPPING
// cycles, not one, so a state's tracked offset must be a per-cycle
// fixed point, not whatever a single forward BFS pass computes first).
//
// MECHANISM: every transition (u,v) -> (u',v') (a chunk of
// sigma(u),sigma(v)) carries a formula
//   X_child = M * X_parent + b_edge
// where b_edge depends only on the specific chunk (its flat start
// position s within sigma(u)=sigma(v)), via the validated word_adjust()
// generalization of simple_backward_targets' one-letter formula (kept
// below as edge_formula/locate/abelianize). Since every edge shares the
// SAME linear part M, a consistent assignment of X to every BP-core
// state is a "cocycle" fixed point: pick a spanning tree from a root r,
// express every node's X as an affine function of the unknown root
// value X_r (X_v = A_v X_r + c_v, A_v a power of M), and every non-tree
// edge gives a linear constraint on X_r. Solving that stacked, exact
// rational linear system (Gauss-Jordan over Q, via mini-gmp's mpq_t)
// for X_r, then checking EVERY edge in the graph -- tree and non-tree
// alike -- is satisfied, is the fixed-point solve implemented below.
//
// RESULT (n=3,4,5, all checked, not assumed): the solve is well-posed
// -- full rank, no contradictions, X_r found uniquely -- and 100% of
// edges (12/12, 40/40, 104/104) satisfy X_v = M*X_u + b_edge exactly.
// But the fixed point itself is X = 0 IDENTICALLY for every one of the
// 8/20/40 recurrent-core states, independent of which node is chosen
// as the BFS root (checked directly: re-solving with root=3 instead of
// root=0 reproduces the same all-zero result, so this isn't an
// artifact of the root choice or of an implicit "X_root=0" assumption
// -- the extra cycle-closing equations force X_r=0 as the ONLY
// consistent value). Hand-traced independently for two specific edges
// (n=3, the s=3 chunk of state u=[0,1,0,2],v=[2,0,1,0], and the s=1
// chunk of state u=[1,0],v=[0,1]): b_edge=(0,0,0) both times, by exact
// arithmetic, not by trusting the program alone.
//
// WHAT THIS MEANS, HONESTLY: this is NOT the same "degenerate all-zero
// answer" the previous session already ruled out (that came from a
// naive single-forward-pass computation on a multi-cycle graph, which
// is provably wrong in general). This is the fully-checked, unique,
// consistent fixed point across the WHOLE recurrent core at once. That
// it is identically zero appears to be a genuine structural fact of
// the n-bonacci family under this specific formula -- plausibly
// because every letter's sigma-image begins with the same symbol (0),
// which makes the "prefix mismatch" term (lpu - lpv) cancel and the
// "inner" abelianized-prefix term collapse whenever the two located
// split points align this way (confirmed by hand on the two edges
// above, not yet proven for the general case).
//
// WHAT REMAINS OPEN: an identically-zero X cannot, on its own, be the
// literal G_B x-coordinate for every BP-core state (G_B's real
// dominant-core nodes have many distinct nonzero x, e.g. the
// known-good validation node [i=0,x=(0,-1,1),j=2] below). Two
// interpretations, NEITHER checked yet:
//   (a) the true map to G_B is [i=u[0], x=0, j=v[0]] plus a FIXED
//       per-(i,j)-pair offset not captured by this X at all (i.e. the
//       whole nontrivial part of G_B's x lives in a term this
//       construction doesn't track), or
//   (b) this specific word_adjust-based tracking is not the right
//       formalization of "the same coordinate G_B uses," and the
//       anchor needs to come from a real G_B node's x-value at the
//       BFS root rather than being solved for from scratch.
// Next step: take one BP-core state, compute its candidate SNode
// [u[0], 0, v[0]] under this scheme, and check by hand whether it (or
// a fixed shift of it) is actually a member of the real G_B dominant
// core built via the same pipeline as gb_bp_twisted_quotient_check.cpp
// -- not yet done in this file.
//
// Build: g++ -std=c++20 -O2 -Iinclude -Imath/include -Imath/include/mini-gmp \
//        app/gb_bp_bijection_attempt.cpp math/out/libmath.a -o gb_bp_bijection_attempt

#include <array>
#include <cstdio>
#include <deque>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

#include "math/bigint.hpp"
#include "ravel/balanced_pair.hpp"
#include "ravel/contact_boundary.hpp"
#include "ravel/core.hpp"
#include "ravel/corona.hpp"
#include "ravel/graph_divisor.hpp"
#include "ravel/substitution.hpp"
#include "ravel/survey.hpp"

using namespace ravel;
using mathlib::Rat;

namespace {

std::vector<std::vector<std::int8_t>> n_bonacci_rule(std::size_t n) {
    std::vector<std::vector<std::int8_t>> sigma(n);
    for (std::size_t i = 0; i + 1 < n; ++i)
        sigma[i] = {0, static_cast<std::int8_t>(i + 1)};
    sigma[n - 1] = {0};
    return sigma;
}

double compute_beta(std::size_t n) {
    std::vector<std::vector<long long>> M(n, std::vector<long long>(n, 0));
    for (std::size_t c = 0; c + 1 < n; ++c) { M[0][c] += 1; M[c + 1][c] += 1; }
    M[0][n - 1] += 1;
    return classify_matrix_spectral(M).beta;
}

// ---- word-level generalization of simple_backward_targets' formula --

template <std::size_t D>
std::pair<std::size_t, std::size_t> locate(
        const std::vector<std::int8_t>& word,
        const SubstitutionRule& rule, std::size_t p) {
    std::size_t pos = 0;
    for (std::size_t k = 0; k < word.size(); ++k) {
        std::size_t len = rule.image(word[k]).size();
        if (p < pos + len) return {k, p - pos};
        pos += len;
    }
    throw std::out_of_range("locate: position beyond word's image");
}

template <std::size_t D>
std::array<long long, D> abelianize(const std::vector<std::int8_t>& w) {
    std::array<long long, D> v{};
    for (auto c : w) v[static_cast<std::size_t>(c)] += 1;
    return v;
}

// b_edge(Up, Vp, s) = word_adjust(Up, Vp, Xp=0, s).x -- the part of the
// word-level formula independent of the parent's own tracked offset.
// Also returns the child's (i, j) "letter identity" (first letter of
// each side's chunk under the located split), matching the
// single-letter case's cand.i / cand.j.
template <std::size_t D>
struct EdgeFormula { long long i, j; std::array<long long, D> b; };

template <std::size_t D>
EdgeFormula<D> edge_formula(const std::vector<std::int8_t>& Up,
                             const std::vector<std::int8_t>& Vp,
                             const SubstitutionRule& rule,
                             const std::array<std::array<long long, D>, D>& M,
                             std::size_t s) {
    auto [ku, ou] = locate<D>(Up, rule, s);
    auto [kv, ov] = locate<D>(Vp, rule, s);
    std::vector<std::int8_t> Up_prefix(Up.begin(), Up.begin() + ku);
    std::vector<std::int8_t> Vp_prefix(Vp.begin(), Vp.begin() + kv);
    auto inner_l = abelianize<D>(Up_prefix);
    auto inner_r = abelianize<D>(Vp_prefix);
    std::array<long long, D> inner{};
    for (std::size_t k = 0; k < D; ++k) inner[k] = inner_l[k] - inner_r[k];
    std::array<long long, D> scaled{};
    for (std::size_t r = 0; r < D; ++r) {
        long long s2 = 0;
        for (std::size_t c = 0; c < D; ++c) s2 += M[r][c] * inner[c];
        scaled[r] = s2;
    }
    std::vector<std::int8_t> pu(rule.image(Up[ku]).begin(), rule.image(Up[ku]).begin() + ou);
    std::vector<std::int8_t> pv(rule.image(Vp[kv]).begin(), rule.image(Vp[kv]).begin() + ov);
    auto lpu = abelianize<D>(pu);
    auto lpv = abelianize<D>(pv);
    EdgeFormula<D> res;
    res.i = Up[ku];
    res.j = Vp[kv];
    for (std::size_t k = 0; k < D; ++k) res.b[k] = scaled[k] + lpu[k] - lpv[k];
    return res;
}

// ---- BP BFS with chunk start positions retained on every edge -------

std::vector<std::tuple<std::vector<std::int8_t>, std::vector<std::int8_t>, std::size_t>>
reduce_pair_with_starts(const std::vector<std::int8_t>& u,
                         const std::vector<std::int8_t>& v,
                         std::size_t nl) {
    std::vector<std::tuple<std::vector<std::int8_t>, std::vector<std::int8_t>, std::size_t>> chunks;
    if (u.size() != v.size()) return chunks;
    std::vector<long long> cu(nl, 0), cv(nl, 0);
    std::size_t start = 0;
    for (std::size_t i = 0; i < u.size(); ++i) {
        cu[static_cast<std::size_t>(u[i])] += 1;
        cv[static_cast<std::size_t>(v[i])] += 1;
        if (cu == cv) {
            chunks.emplace_back(
                std::vector<std::int8_t>(u.begin() + start, u.begin() + i + 1),
                std::vector<std::int8_t>(v.begin() + start, v.begin() + i + 1),
                start);
            start = i + 1;
            std::fill(cu.begin(), cu.end(), 0);
            std::fill(cv.begin(), cv.end(), 0);
        }
    }
    return chunks;
}

struct EdgeRec { std::size_t parent, child, s; };

struct BpGraph {
    std::vector<std::string> order;
    std::vector<std::pair<std::vector<std::int8_t>, std::vector<std::int8_t>>> word_of;
    std::vector<EdgeRec> edges;  // all edges, over the FULL order-index space
    std::vector<std::size_t> noncoin;  // order-indices that are non-coincidence
};

BpGraph build_bp_graph(std::size_t n) {
    auto sigma = n_bonacci_rule(n);
    SubstitutionRule rule(sigma);
    const std::size_t nl = rule.alphabet_size();

    BpGraph g;
    std::map<std::string, std::size_t> seen;
    std::deque<std::size_t> queue;

    auto initial = reduce_pair({0, 1}, {1, 0}, nl);
    for (auto& ch : initial) {
        std::string k = pair_key(ch.first, ch.second);
        if (!seen.count(k)) {
            seen[k] = g.order.size();
            g.order.push_back(k);
            g.word_of.push_back(ch);
            queue.push_back(g.order.size() - 1);
        }
    }

    constexpr std::size_t max_pairs = 20000, max_len = 60000;
    while (!queue.empty()) {
        std::size_t idx = queue.front();
        queue.pop_front();
        auto [u, v] = g.word_of[idx];
        auto img = sigma_pair(rule, {u, v});
        if (img.first.size() > max_len || seen.size() > max_pairs) {
            std::printf("  BP: BFS did not terminate for n=%zu\n", n);
            break;
        }
        auto chunks = reduce_pair_with_starts(img.first, img.second, nl);
        for (auto& [uc, vc, s] : chunks) {
            std::string ck = pair_key(uc, vc);
            auto it = seen.find(ck);
            std::size_t ci;
            if (it == seen.end()) {
                ci = g.order.size();
                seen[ck] = ci;
                g.order.push_back(ck);
                g.word_of.push_back({uc, vc});
                queue.push_back(ci);
            } else {
                ci = it->second;
            }
            g.edges.push_back({idx, ci, s});
        }
    }

    for (std::size_t i = 0; i < g.order.size(); ++i) {
        auto& [u, v] = g.word_of[i];
        if (!(u.size() == 1 && u == v)) g.noncoin.push_back(i);
    }
    return g;
}

// ---- exact rational linear algebra for the fixed-point solve --------

using Vec = std::vector<Rat>;
using Mat = std::vector<std::vector<Rat>>;

Mat mat_zero(std::size_t n, std::size_t m) {
    return Mat(n, std::vector<Rat>(m, Rat(0)));
}
Mat mat_identity(std::size_t n) {
    Mat I = mat_zero(n, n);
    for (std::size_t i = 0; i < n; ++i) I[i][i] = Rat(1);
    return I;
}
Mat mat_mul(const Mat& A, const Mat& B) {
    std::size_t n = A.size(), k = B.size(), m = B[0].size();
    Mat C = mat_zero(n, m);
    for (std::size_t i = 0; i < n; ++i)
        for (std::size_t j = 0; j < m; ++j)
            for (std::size_t l = 0; l < k; ++l) {
                Rat prod; mathlib::mul(prod, A[i][l], B[l][j]);
                mathlib::add(C[i][j], C[i][j], prod);
            }
    return C;
}
Vec mat_vec(const Mat& A, const Vec& x) {
    std::size_t n = A.size(), m = x.size();
    Vec y(n, Rat(0));
    for (std::size_t i = 0; i < n; ++i)
        for (std::size_t j = 0; j < m; ++j) {
            Rat prod; mathlib::mul(prod, A[i][j], x[j]);
            mathlib::add(y[i], y[i], prod);
        }
    return y;
}
Mat mat_sub(const Mat& A, const Mat& B) {
    std::size_t n = A.size(), m = A[0].size();
    Mat C = mat_zero(n, m);
    for (std::size_t i = 0; i < n; ++i)
        for (std::size_t j = 0; j < m; ++j)
            mathlib::sub(C[i][j], A[i][j], B[i][j]);
    return C;
}
Vec vec_sub(const Vec& a, const Vec& b) {
    Vec c(a.size());
    for (std::size_t i = 0; i < a.size(); ++i) mathlib::sub(c[i], a[i], b[i]);
    return c;
}
Vec vec_add(const Vec& a, const Vec& b) {
    Vec c(a.size());
    for (std::size_t i = 0; i < a.size(); ++i) mathlib::add(c[i], a[i], b[i]);
    return c;
}
bool vec_is_zero(const Vec& a) {
    for (auto& x : a) if (!mathlib::is_zero(x)) return false;
    return true;
}

// Solve a (possibly overdetermined) consistent linear system A x = b,
// A: K x D, b: K, via exact rational Gauss-Jordan elimination.  Throws
// if the system is rank-deficient (< D) or inconsistent (a pivotless
// row with nonzero RHS).
Vec solve_overdetermined_exact(Mat A, Vec b) {
    std::size_t K = A.size();
    std::size_t D = A.empty() ? 0 : A[0].size();
    Mat aug(K, std::vector<Rat>(D + 1));
    for (std::size_t i = 0; i < K; ++i) {
        for (std::size_t j = 0; j < D; ++j) aug[i][j] = A[i][j];
        aug[i][D] = b[i];
    }
    std::vector<long long> pivot_row(D, -1);
    std::size_t row = 0;
    for (std::size_t col = 0; col < D && row < K; ++col) {
        std::size_t sel = row;
        bool found = false;
        for (std::size_t r = row; r < K; ++r) {
            if (!mathlib::is_zero(aug[r][col])) { sel = r; found = true; break; }
        }
        if (!found) continue;
        std::swap(aug[row], aug[sel]);
        Rat piv = aug[row][col];
        for (std::size_t j = col; j <= D; ++j) mathlib::div(aug[row][j], aug[row][j], piv);
        for (std::size_t r = 0; r < K; ++r) {
            if (r == row) continue;
            if (mathlib::is_zero(aug[r][col])) continue;
            Rat factor = aug[r][col];
            for (std::size_t j = col; j <= D; ++j) {
                Rat prod; mathlib::mul(prod, factor, aug[row][j]);
                mathlib::sub(aug[r][j], aug[r][j], prod);
            }
        }
        pivot_row[col] = static_cast<long long>(row);
        ++row;
    }
    for (std::size_t c = 0; c < D; ++c)
        if (pivot_row[c] < 0)
            throw std::runtime_error("solve_overdetermined_exact: rank-deficient system");
    for (std::size_t r = row; r < K; ++r) {
        for (std::size_t j = 0; j <= D; ++j) {
            if (!mathlib::is_zero(aug[r][j]))
                throw std::runtime_error("solve_overdetermined_exact: inconsistent system");
        }
    }
    Vec x(D);
    for (std::size_t c = 0; c < D; ++c) x[c] = aug[static_cast<std::size_t>(pivot_row[c])][D];
    return x;
}

bool is_integer(const Rat& r) {
    return mathlib::str(r).find('/') == std::string::npos;
}
long long to_ll(const Rat& r) {
    return std::stoll(mathlib::str(r));
}

}  // namespace

template <std::size_t D>
void run_fixed_point_bijection(std::size_t n) {
    auto sigma = n_bonacci_rule(n);
    SubstitutionRule rule(sigma);
    std::array<std::array<long long, D>, D> M{};
    for (std::size_t c = 0; c < D; ++c)
        for (auto r : sigma[c]) M[static_cast<std::size_t>(r)][c] += 1;
    Mat Mq(D, std::vector<Rat>(D));
    for (std::size_t i = 0; i < D; ++i)
        for (std::size_t j = 0; j < D; ++j) Mq[i][j] = Rat(M[i][j]);

    BpGraph g = build_bp_graph(n);

    // Dense noncoin adjacency (counts) to feed extract_recurrent_core,
    // exactly mirroring gb_bp_twisted_quotient_check.cpp's approach.
    std::map<std::size_t, std::size_t> idx_map;
    for (std::size_t k = 0; k < g.noncoin.size(); ++k) idx_map[g.noncoin[k]] = k;
    std::size_t nn = g.noncoin.size();
    std::vector<std::vector<long long>> dense(nn, std::vector<long long>(nn, 0));
    for (auto& e : g.edges) {
        auto pi = idx_map.find(e.parent);
        auto ci = idx_map.find(e.child);
        if (pi != idx_map.end() && ci != idx_map.end()) dense[pi->second][ci->second] += 1;
    }
    auto full_graph = WeightedDigraph::from_dense(dense);
    auto [core, orig_idx] = extract_recurrent_core(full_graph);
    std::printf("=== n=%zu: BP noncoincidence states=%zu, recurrent core=%zu ===\n", n, nn, core.n);

    std::vector<long long> full_to_core(nn, -1);
    for (std::size_t c = 0; c < core.n; ++c) full_to_core[orig_idx[c]] = static_cast<long long>(c);

    // Collect core-local edges with their b_edge vectors.
    struct CoreEdge { std::size_t u, v; std::array<long long, D> b; };
    std::vector<CoreEdge> core_edges;
    for (auto& e : g.edges) {
        auto pi = idx_map.find(e.parent);
        auto ci = idx_map.find(e.child);
        if (pi == idx_map.end() || ci == idx_map.end()) continue;
        long long cu = full_to_core[pi->second], cv = full_to_core[ci->second];
        if (cu < 0 || cv < 0) continue;
        auto& [Up, Vp] = g.word_of[e.parent];
        auto ef = edge_formula<D>(Up, Vp, rule, M, e.s);
        core_edges.push_back({static_cast<std::size_t>(cu), static_cast<std::size_t>(cv), ef.b});
    }
    std::printf("  core edges (with formula): %zu\n", core_edges.size());

    // Adjacency for BFS spanning-tree construction.
    std::vector<std::vector<std::size_t>> adj_edge_idx(core.n);
    for (std::size_t e = 0; e < core_edges.size(); ++e) adj_edge_idx[core_edges[e].u].push_back(e);

    // BFS from root=0, building A_v (D x D) and c_v (D-vector) with
    // X_v = A_v * X_root + c_v.  Non-tree edges accumulate constraints.
    std::vector<bool> visited(core.n, false);
    std::vector<Mat> A(core.n);
    std::vector<Vec> c(core.n);
    std::vector<Mat> eq_coef;
    std::vector<Vec> eq_rhs;

    std::size_t root = 0;
    visited[root] = true;
    A[root] = mat_identity(D);
    c[root] = Vec(D, Rat(0));
    std::deque<std::size_t> bq;
    bq.push_back(root);
    while (!bq.empty()) {
        std::size_t u = bq.front();
        bq.pop_front();
        for (std::size_t ei : adj_edge_idx[u]) {
            auto& ce = core_edges[ei];
            std::size_t v = ce.v;
            Vec b(D);
            for (std::size_t k = 0; k < D; ++k) b[k] = Rat(ce.b[k]);
            if (!visited[v]) {
                visited[v] = true;
                A[v] = mat_mul(Mq, A[u]);
                c[v] = vec_add(mat_vec(Mq, c[u]), b);
                bq.push_back(v);
            } else {
                // Constraint: A[v]*Xr + c[v] == M*A[u]*Xr + M*c[u] + b
                Mat coef = mat_sub(A[v], mat_mul(Mq, A[u]));
                Vec rhs = vec_sub(vec_add(mat_vec(Mq, c[u]), b), c[v]);
                eq_coef.push_back(coef);
                eq_rhs.push_back(rhs);
            }
        }
    }
    std::size_t unreached = 0;
    for (bool vv : visited) if (!vv) ++unreached;
    if (unreached) std::printf("  WARNING: %zu core nodes unreached by forward BFS from root 0\n", unreached);

    std::printf("  spanning tree covers %zu/%zu nodes; %zu extra (cycle-closing) equations\n",
                core.n - unreached, core.n, eq_coef.size());

    // Stack all extra equations into one (K*D) x D system.
    Mat stacked; Vec rhs_stacked;
    for (std::size_t e = 0; e < eq_coef.size(); ++e) {
        for (std::size_t r = 0; r < D; ++r) {
            stacked.push_back(eq_coef[e][r]);
            rhs_stacked.push_back(eq_rhs[e][r]);
        }
    }

    Vec Xr;
    bool solved = false;
    try {
        Xr = solve_overdetermined_exact(stacked, rhs_stacked);
        solved = true;
    } catch (const std::exception& ex) {
        std::printf("  FIXED POINT SOLVE FAILED: %s\n", ex.what());
        return;
    }

    std::printf("  X_root (state 0) = (");
    for (std::size_t k = 0; k < D; ++k) std::printf("%s%s", mathlib::str(Xr[k]).c_str(), k + 1 < D ? "," : "");
    std::printf(")%s\n", solved ? "" : "");

    // Propagate to every core node.
    std::vector<Vec> X(core.n);
    bool all_integer = true;
    for (std::size_t v = 0; v < core.n; ++v) {
        if (!visited[v]) continue;
        X[v] = vec_add(mat_vec(A[v], Xr), c[v]);
        for (auto& r : X[v]) if (!is_integer(r)) all_integer = false;
    }

    // Full validation pass: EVERY edge (tree and non-tree alike) must
    // satisfy X_v == M*X_u + b_edge exactly.
    std::size_t violations = 0;
    for (auto& ce : core_edges) {
        Vec b(D);
        for (std::size_t k = 0; k < D; ++k) b[k] = Rat(ce.b[k]);
        Vec predicted = vec_add(mat_vec(Mq, X[ce.u]), b);
        if (!vec_is_zero(vec_sub(predicted, X[ce.v]))) ++violations;
    }
    std::printf("  full-graph consistency check: %zu/%zu edges satisfy X_v = M*X_u + b_edge exactly\n",
                core_edges.size() - violations, core_edges.size());
    std::printf("  all X values integral: %s\n", all_integer ? "YES" : "NO");

    std::printf("  Per-state result:\n");
    for (std::size_t v = 0; v < core.n; ++v) {
        if (!visited[v]) continue;
        std::size_t oi = g.noncoin[orig_idx[v]];
        auto& [u, w] = g.word_of[oi];
        std::printf("    core#%zu  u=[", v);
        for (auto ch : u) std::printf("%d", ch);
        std::printf("] v=[");
        for (auto ch : w) std::printf("%d", ch);
        std::printf("]  X=(");
        for (std::size_t k = 0; k < D; ++k) {
            if (all_integer) std::printf("%lld%s", to_ll(X[v][k]), k + 1 < D ? "," : "");
            else std::printf("%s%s", mathlib::str(X[v][k]).c_str(), k + 1 < D ? "," : "");
        }
        std::printf(")\n");
    }
}

int main() {
    run_fixed_point_bijection<3>(3);
    std::printf("\n");
    run_fixed_point_bijection<4>(4);
    std::printf("\n");
    run_fixed_point_bijection<5>(5);
    return 0;
}
