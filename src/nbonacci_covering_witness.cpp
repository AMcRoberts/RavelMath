// nbonacci_covering_witness.cpp
//
// Implementation of the n-bonacci covering-witness computation
// extracted from app/nbonacci_covering_witness_enumerator.cpp's
// main().  This is the C++ core that:
//   - builds the r/q matrix pieces (SignInt bareiss-like Laplace
//     expansion),
//   - enumerates (n-1)-pin subsets of {1, ..., n+L-1} and sign
//     assignments in {-1, +1},
//   - computes the simplest valid (in-box, covering) witness,
//   - exposes the result as a covering_witness_t struct.
//
// Used by:
//   - app/nbonacci_covering_witness_enumerator.cpp (the existing
//     C++ covering-witness JSON enumerator)
//   - the Lua binding in src/lua_bindings.cpp (the Lua path)
//   - app/nbonacci_covering_witness_proof.cpp (the proof probe)

#include "ravel/nbonacci_covering_witness.hpp"

#include <algorithm>
#include <cstddef>
#include <map>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace ravel {

namespace {

// SignInt: (num, denom) reduced to coprime form, denom > 0.
struct SignInt {
    long long num = 0;
    long long denom = 1;
    SignInt() = default;
    SignInt(long long n) : num(n), denom(1) {}
    SignInt(long long n, long long d) : num(n), denom(d) { normalize(); }
    static long long gcd(long long a, long long b) {
        if (a < 0) a = -a;
        if (b < 0) b = -b;
        while (b != 0) { long long t = b; b = a % b; a = t; }
        return a;
    }
    void normalize() {
        if (denom < 0) { denom = -denom; num = -num; }
        if (num == 0) { denom = 1; return; }
        long long g = gcd(num, denom);
        num /= g; denom /= g;
    }
    bool operator==(const SignInt& o) const { return num * o.denom == o.num * denom; }
    bool operator<(const SignInt& o) const { return num * o.denom < o.num * denom; }
    SignInt operator+(const SignInt& o) const {
        long long d = denom / gcd(denom, o.denom) * o.denom;
        return SignInt(num * (d / denom) + o.num * (d / o.denom), d);
    }
    SignInt operator-(const SignInt& o) const {
        long long d = denom / gcd(denom, o.denom) * o.denom;
        return SignInt(num * (d / denom) - o.num * (d / o.denom), d);
    }
    SignInt operator*(const SignInt& o) const {
        return SignInt(num * o.num, denom * o.denom);
    }
    SignInt operator/(const SignInt& o) const {
        return SignInt(num * o.denom, denom * o.num);
    }
    SignInt operator-() const { return SignInt(-num, denom); }
    bool is_zero() const { return num == 0; }
    std::string to_string() const {
        if (denom == 1) return std::to_string(num);
        return std::to_string(num) + "/" + std::to_string(denom);
    }
};

using PolyMatrix = std::vector<std::vector<SignInt>>;

PolyMatrix r_matrix_for_n(std::size_t n) {
    PolyMatrix M(n - 1, std::vector<SignInt>(n - 1, SignInt(0)));
    for (std::size_t i = 0; i + 1 < n - 1; ++i) {
        M[i][i] = SignInt(-1);
        M[i + 1][i] = SignInt(0, 1);  // X = 0 + 1*X
    }
    M[n - 2][n - 2] = SignInt(-1);
    return M;
}

std::vector<std::vector<SignInt>> later_values(std::size_t n, std::size_t r) {
    auto a = r_matrix_for_n(n);  // dummy to get size
    (void)a;
    // Construct the (n+r+1) x n matrix of polynomial coefficients
    // (indexed by (j, k) for j in 0..n+r, k in 0..n-1).
    std::vector<std::vector<SignInt>> a_v(n + r + 1,
                                          std::vector<SignInt>(n, SignInt(0)));
    a_v[0][0] = SignInt(1);
    for (std::size_t j = 1; j < n; ++j) a_v[j][j] = SignInt(1);
    for (std::size_t t = 0; t <= r; ++t) {
        SignInt sum(0);
        for (std::size_t j = 1; j < n; ++j) sum = sum + a_v[t + j][j];
        a_v[t + n][0] = a_v[t][0] - sum;
    }
    return a_v;
}

// Reconstruct the sequence from the (n-1) free parameters
std::vector<SignInt> reconstruct_sequence(std::size_t n, std::size_t L,
                                          const std::vector<SignInt>& free_params) {
    std::vector<SignInt> seq(n + L);
    seq[0] = SignInt(1);
    for (std::size_t j = 0; j < free_params.size(); ++j) seq[j + 1] = free_params[j];
    for (std::size_t t = 0; t < L; ++t) {
        SignInt sum(0);
        for (std::size_t j = 1; j < n; ++j) sum = sum + seq[t + j];
        seq[t + n] = seq[t] - sum;
    }
    return seq;
}

bool in_closed_unit_box(const SignInt& v) {
    return v == SignInt(-1) || v == SignInt(0) || v == SignInt(1)
        || (SignInt(-1) < v && v < SignInt(1));
}

bool is_covering(std::size_t n, std::size_t L,
                  const std::vector<SignInt>& seq) {
    for (std::size_t t = 0; t <= L; ++t) {
        bool covered = false;
        for (std::size_t i = 0; i < n; ++i) {
            const SignInt& v = seq[t + i];
            if (v == SignInt(1) || v == SignInt(-1)) { covered = true; break; }
        }
        if (!covered) return false;
    }
    return true;
}

long long count_distinct_abs(const std::vector<SignInt>& seq) {
    std::set<std::string> seen;
    for (const auto& v : seq) {
        SignInt a = v < SignInt(0) ? -v : v;
        seen.insert(a.to_string());
    }
    return static_cast<long long>(seen.size());
}

long long simplicity_score(const std::vector<SignInt>& seq,
                            const std::vector<SignInt>& free_params,
                            const std::vector<std::size_t>& indices,
                            const std::vector<long long>& cover_per_pin) {
    long long da = count_distinct_abs(seq);
    long long zr = 0;
    for (const auto& v : free_params) if (v == SignInt(0)) ++zr;
    long long dg = 0;
    std::set<long long> gaps;
    for (std::size_t k = 1; k < indices.size(); ++k) {
        gaps.insert(static_cast<long long>(indices[k]) -
                   static_cast<long long>(indices[k - 1]));
    }
    dg = static_cast<long long>(gaps.size());
    long long cover_sum = 0;
    for (auto c : cover_per_pin) cover_sum += c;
    return da * 1000 + dg * 100
           + (static_cast<long long>(free_params.size()) - zr) * 10
           + cover_sum;
}

std::vector<long long> cover_compactness(std::size_t n, std::size_t L,
                                          const std::vector<std::size_t>& indices) {
    std::vector<long long> out;
    for (std::size_t idx : indices) {
        long long c = 0;
        for (std::size_t t = 0; t <= L; ++t) {
            if (t <= idx && idx < t + n) ++c;
        }
        out.push_back(c);
    }
    return out;
}

struct Candidate {
    std::vector<std::size_t> indices;
    std::vector<long long> signs;
    std::vector<SignInt> free_params;
    std::vector<SignInt> sequence;
    long long distinct_abs_count = 0;
    long long zeros_in_solution = 0;
    long long cover_min = 0;
    long long cover_max = 0;
    long long distinct_gap_count = 0;
    long long simplicity_score = 0;
};

Candidate make_candidate(std::size_t n, std::size_t L,
                            const std::vector<std::size_t>& indices,
                            const std::vector<long long>& signs,
                            const std::vector<SignInt>& free_params,
                            const std::vector<SignInt>& sequence) {
    Candidate c;
    c.indices = indices;
    c.signs = signs;
    c.free_params = free_params;
    c.sequence = sequence;
    c.distinct_abs_count = count_distinct_abs(c.sequence);
    c.zeros_in_solution = 0;
    for (const auto& v : c.free_params) if (v == SignInt(0)) ++c.zeros_in_solution;
    c.distinct_gap_count = 0;
    std::set<long long> gaps;
    for (std::size_t k = 1; k < c.indices.size(); ++k) {
        gaps.insert(static_cast<long long>(c.indices[k]) -
                   static_cast<long long>(c.indices[k - 1]));
    }
    c.distinct_gap_count = static_cast<long long>(gaps.size());
    auto covers = cover_compactness(n, L, c.indices);
    if (!covers.empty()) {
        c.cover_min = *std::min_element(covers.begin(), covers.end());
        c.cover_max = *std::max_element(covers.begin(), covers.end());
    }
    c.simplicity_score = simplicity_score(c.sequence, c.free_params,
                                          c.indices, covers);
    return c;
}

Candidate find_simplest(std::size_t n, std::size_t L) {
    std::vector<std::size_t> candidates_idx;
    for (std::size_t j = 1; j < n + L; ++j) candidates_idx.push_back(j);
    std::vector<long long> sign_choices{-1, 1};
    std::vector<Candidate> valid;
    std::vector<std::size_t> subset(n - 1);
    std::vector<bool> choose(candidates_idx.size(), false);
    // Initial: FIRST n-1 entries true (largest in lex order);
    // step through all C(candidates_idx.size(), n-1) subsets via
    // std::prev_permutation (down to last n-1 true).
    for (std::size_t i = 0; i < n - 1; ++i) choose[i] = true;
    do {
        std::vector<std::size_t> indices;
        for (std::size_t i = 0; i < choose.size(); ++i)
            if (choose[i]) indices.push_back(candidates_idx[i]);
        std::printf("    iter choose=[");
        for (std::size_t i = 0; i < choose.size(); ++i)
            std::printf("%c", choose[i] ? 'T' : 'F');
        std::printf("] indices=[");
        for (std::size_t i = 0; i < indices.size(); ++i) {
            if (i) std::printf(",");
            std::printf("%zu", indices[i]);
        }
        std::printf("]\n");
        std::fflush(stdout);
        for (std::size_t s = 0; s < (1ull << (n - 1)); ++s) {
            std::vector<long long> signs(n - 1);
            for (std::size_t k = 0; k < n - 1; ++k)
                signs[k] = (s >> k) & 1 ? 1 : -1;
            // Build the (n-1) x (n-1) matrix A' of free-param
            // coefficients, and the (n-1)-vector rhs' of the constant
            // parts.
            auto a_v = later_values(n, L);
            std::vector<std::vector<SignInt>> A_p(n - 1,
                std::vector<SignInt>(n - 1, SignInt(0)));
            std::vector<SignInt> rhs_p(n - 1, SignInt(0));
            for (std::size_t k = 0; k < n - 1; ++k) {
                std::size_t row = indices[k];
                for (std::size_t j = 0; j < n - 1; ++j) {
                    A_p[k][j] = a_v[row][j + 1];
                }
                rhs_p[k] = SignInt(signs[k]) - a_v[row][0];
            }
            // Gaussian elimination on the augmented matrix
            std::vector<SignInt> sol(n - 1, SignInt(0));
            bool ok = true;
            for (std::size_t col = 0; col < n - 1; ++col) {
                // Find pivot
                std::size_t piv = n - 1;
                for (std::size_t i = col; i < n - 1; ++i) {
                    if (!A_p[i][col].is_zero()) { piv = i; break; }
                }
                if (piv == n - 1) { ok = false; break; }
                if (piv != col) {
                    std::swap(A_p[piv], A_p[col]);
                    std::swap(rhs_p[piv], rhs_p[col]);
                }
                SignInt piv_val = A_p[col][col];
                for (std::size_t i = col + 1; i < n - 1; ++i) {
                    SignInt factor = A_p[i][col] / piv_val;
                    for (std::size_t j = col; j < n - 1; ++j) {
                        A_p[i][j] = A_p[i][j] - factor * A_p[col][j];
                    }
                    rhs_p[i] = rhs_p[i] - factor * rhs_p[col];
                }
            }
            if (!ok) continue;
            // Back-substitution
            for (std::size_t i = n - 1; i > 0; --i) {
                std::size_t k = i - 1;
                SignInt s = rhs_p[k];
                for (std::size_t j = k + 1; j < n - 1; ++j) {
                    s = s - A_p[k][j] * sol[j];
                }
                if (A_p[k][k].is_zero()) { ok = false; break; }
                sol[k] = s / A_p[k][k];
            }
            if (!ok) continue;
            if (!in_closed_unit_box(sol[0]) ||
                (n >= 3 && !in_closed_unit_box(sol[1]))) {
                // Quick box check on the first two
            }
            bool all_box = true;
            for (const auto& v : sol) if (!in_closed_unit_box(v)) { all_box = false; break; }
            if (!all_box) {
                std::printf("    SKIP id=[");
                for (std::size_t i = 0; i < indices.size(); ++i) {
                    if (i) std::printf(",");
                    std::printf("%zu", indices[i]);
                }
                std::printf("] s=%zu (box)\n", s);
                continue;
            }
            auto seq = reconstruct_sequence(n, L, sol);
            if (!is_covering(n, L, seq)) {
                std::printf("    SKIP id=[");
                for (std::size_t i = 0; i < indices.size(); ++i) {
                    if (i) std::printf(",");
                    std::printf("%zu", indices[i]);
                }
                std::printf("] s=%zu (cover)\n", s);
                continue;
            }
            std::printf("    CANDIDATE id=[");
            for (std::size_t i = 0; i < indices.size(); ++i) {
                if (i) std::printf(",");
                std::printf("%zu", indices[i]);
            }
            std::printf("] signs=[");
            for (std::size_t i = 0; i < signs.size(); ++i) {
                if (i) std::printf(",");
                std::printf("%lld", (long long)signs[i]);
            }
            std::printf("] free=[");
            for (std::size_t i = 0; i < sol.size(); ++i) {
                if (i) std::printf(",");
                std::printf("%s", sol[i].to_string().c_str());
            }
            std::printf("] seq=[");
            for (std::size_t i = 0; i < seq.size(); ++i) {
                if (i) std::printf(",");
                std::printf("%s", seq[i].to_string().c_str());
            }
            std::printf("]\n");
            valid.push_back(make_candidate(n, L, indices, signs, sol, seq));
        }
    } while (std::prev_permutation(choose.begin(), choose.end()));
    if (valid.empty()) {
        std::printf("    no valid candidate at n=%zu L=%zu\n", n, L);
        return Candidate{};
    }
    for (const auto& v : valid) {
        std::printf("    candidate: idx=[");
        for (std::size_t i = 0; i < v.indices.size(); ++i) {
            if (i) std::printf(",");
            std::printf("%zu", v.indices[i]);
        }
        std::printf("] signs=[");
        for (std::size_t i = 0; i < v.signs.size(); ++i) {
            if (i) std::printf(",");
            std::printf("%lld", (long long)v.signs[i]);
        }
        std::printf("] score=%ld free_params=[", (long long)v.simplicity_score);
        for (std::size_t i = 0; i < v.free_params.size(); ++i) {
            if (i) std::printf(",");
            std::printf("%s", v.free_params[i].to_string().c_str());
        }
        std::printf("]\n");
    }
    return *std::min_element(valid.begin(), valid.end(),
        [](const Candidate& a, const Candidate& b) {
            return a.simplicity_score < b.simplicity_score;
        });
}

}  // namespace

std::optional<covering_witness_t> compute_simplest_covering_witness(
    std::size_t n, std::size_t L) {
    auto c = find_simplest(n, L);
    if (c.simplicity_score == 0 && c.indices.empty()) return std::nullopt;
    covering_witness_t w;
    w.n = static_cast<long long>(n);
    w.L = static_cast<long long>(L);
    w.indices.reserve(c.indices.size());
    for (auto i : c.indices) w.indices.push_back(static_cast<long long>(i));
    w.signs = c.signs;
    w.free_params.reserve(c.free_params.size());
    for (const auto& s : c.free_params) w.free_params.push_back(s.to_string());
    w.sequence.reserve(c.sequence.size());
    for (const auto& s : c.sequence) w.sequence.push_back(s.to_string());
    return w;
}

std::string check_box(const covering_witness_t& w) {
    auto parse = [](const std::string& s) -> long double {
        auto p = s.find('/');
        if (p == std::string::npos) return std::stold(s);
        return std::stold(s.substr(0, p)) / std::stold(s.substr(p + 1));
    };
    for (const auto& s : w.free_params) {
        long double v = parse(s);
        if (v < -1.0001 || v > 1.0001)
            return "free_param " + s + " not in [-1, 1]";
    }
    for (const auto& s : w.sequence) {
        long double v = parse(s);
        if (v < -1.0001 || v > 1.0001)
            return "sequence value " + s + " not in [-1, 1]";
    }
    return "";
}

std::string check_cover(const covering_witness_t& w) {
    auto parse = [](const std::string& s) -> long double {
        auto p = s.find('/');
        if (p == std::string::npos) return std::stold(s);
        return std::stold(s.substr(0, p)) / std::stold(s.substr(p + 1));
    };
    for (std::size_t t = 0; t <= static_cast<std::size_t>(w.L); ++t) {
        bool covered = false;
        for (std::size_t i = 0; i < static_cast<std::size_t>(w.n); ++i) {
            long double v = parse(w.sequence[t + i]);
            if (std::abs(v) > 0.9999) { covered = true; break; }
        }
        if (!covered) return "window t=" + std::to_string(t) + " not covered";
    }
    return "";
}

}  // namespace ravel
