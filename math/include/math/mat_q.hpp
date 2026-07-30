// math/mat_q.hpp
//
// Matrix over Q (mpq_t entries).  General n×m with arbitrary n, m.
// All operations are exact (mpq_t arithmetic, no double precision).
//
// Operations:
//   - add, sub, neg, scalar mul
//   - matrix multiplication
//   - determinant (square, recursive cofactor expansion)
//   - inverse (square, via Gaussian elimination)
//
// Reference: SymPy Matrix with domain=QQ.
//
// All operations are GENERAL-n (no upper bound on matrix dimension).

#pragma once

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <utility>
#include <vector>

#include "math/bigint.hpp"

namespace mathlib {

// ===================================================================
// MatQ: n × m matrix over Q
// ===================================================================
//
// Stored as std::vector<std::vector<Rat>>.  Each row is a vector<Rat>.
// coeffs_[i][j] is the entry at row i, column j.
//
// Convention: rows and columns are 0-indexed.  size() gives the number
// of rows; each row has its own size() giving the number of columns.
// We require all rows to have the same length (a rectangular matrix).

class MatQ {
public:
    std::vector<std::vector<Rat>> entries_;

    MatQ() = default;
    MatQ(std::size_t n, std::size_t m) : entries_(n, std::vector<Rat>(m)) {
        for (auto& row : entries_)
            for (auto& c : row) set_si(c, 0, 1);
    }
    MatQ(std::initializer_list<std::initializer_list<long long>> rcs) {
        for (const auto& row : rcs) {
            std::vector<Rat> r;
            for (auto n : row) r.push_back(Rat(n, 1));
            entries_.push_back(std::move(r));
        }
    }
    MatQ(std::initializer_list<std::initializer_list<std::pair<long long,long long>>> rcs) {
        for (const auto& row : rcs) {
            std::vector<Rat> r;
            for (auto [n, d] : row) r.push_back(Rat(n, d));
            entries_.push_back(std::move(r));
        }
    }

    std::size_t n_rows() const { return entries_.size(); }
    std::size_t n_cols() const {
        return entries_.empty() ? 0 : entries_[0].size();
    }
    bool is_square() const { return n_rows() == n_cols(); }
    bool is_empty() const { return entries_.empty(); }

    Rat& at(std::size_t i, std::size_t j) {
        if (i >= n_rows() || j >= n_cols()) {
            throw std::out_of_range("MatQ::at");
        }
        return entries_[i][j];
    }
    const Rat& at(std::size_t i, std::size_t j) const {
        if (i >= n_rows() || j >= n_cols()) {
            throw std::out_of_range("MatQ::at");
        }
        return entries_[i][j];
    }

    bool operator==(const MatQ& o) const {
        if (n_rows() != o.n_rows() || n_cols() != o.n_cols()) return false;
        for (std::size_t i = 0; i < n_rows(); ++i) {
            for (std::size_t j = 0; j < n_cols(); ++j) {
                if (cmp(entries_[i][j], o.entries_[i][j]) != 0) return false;
            }
        }
        return true;
    }
    bool operator!=(const MatQ& o) const { return !(*this == o); }
};

// ===================================================================
// Constructors / display
// ===================================================================

inline MatQ zero_mat(std::size_t n, std::size_t m) {
    return MatQ(n, m);
}

inline MatQ identity_mat(std::size_t n) {
    MatQ I(n, n);
    for (std::size_t i = 0; i < n; ++i) {
        set_si(I.entries_[i][i], 1, 1);
    }
    return I;
}

inline MatQ scalar_mat(std::size_t n, std::size_t m, const Rat& s) {
    MatQ M(n, m);
    for (auto& row : M.entries_)
        for (auto& c : row) c = s;
    return M;
}

inline std::string str(const MatQ& M) {
    std::string out = "[";
    for (std::size_t i = 0; i < M.n_rows(); ++i) {
        if (i > 0) out += "; ";
        for (std::size_t j = 0; j < M.n_cols(); ++j) {
            if (j > 0) out += ", ";
            out += str(M.entries_[i][j]);
        }
    }
    out += "]";
    return out;
}

// ===================================================================
// Arithmetic
// ===================================================================

inline MatQ operator-(const MatQ& a) {
    MatQ r = a;
    for (auto& row : r.entries_)
        for (auto& c : row) neg(c);
    return r;
}

inline MatQ operator+(const MatQ& a, const MatQ& b) {
    if (a.n_rows() != b.n_rows() || a.n_cols() != b.n_cols()) {
        throw std::invalid_argument("MatQ +: dimension mismatch");
    }
    MatQ r(a.n_rows(), a.n_cols());
    for (std::size_t i = 0; i < a.n_rows(); ++i) {
        for (std::size_t j = 0; j < a.n_cols(); ++j) {
            add(r.entries_[i][j], a.entries_[i][j], b.entries_[i][j]);
        }
    }
    return r;
}

inline MatQ operator-(const MatQ& a, const MatQ& b) {
    if (a.n_rows() != b.n_rows() || a.n_cols() != b.n_cols()) {
        throw std::invalid_argument("MatQ -: dimension mismatch");
    }
    MatQ r(a.n_rows(), a.n_cols());
    for (std::size_t i = 0; i < a.n_rows(); ++i) {
        for (std::size_t j = 0; j < a.n_cols(); ++j) {
            sub(r.entries_[i][j], a.entries_[i][j], b.entries_[i][j]);
        }
    }
    return r;
}

inline MatQ operator*(const MatQ& a, const Rat& s) {
    MatQ r = a;
    for (auto& row : r.entries_)
        for (auto& c : row) mul(c, c, s);
    return r;
}

inline MatQ operator*(const Rat& s, const MatQ& a) { return a * s; }

inline MatQ operator*(const MatQ& a, const MatQ& b) {
    if (a.n_cols() != b.n_rows()) {
        throw std::invalid_argument("MatQ *: dimension mismatch");
    }
    MatQ r(a.n_rows(), b.n_cols());
    for (std::size_t i = 0; i < a.n_rows(); ++i) {
        for (std::size_t k = 0; k < b.n_cols(); ++k) {
            for (std::size_t j = 0; j < a.n_cols(); ++j) {
                Rat prod;
                mul(prod, a.entries_[i][j], b.entries_[j][k]);
                add(r.entries_[i][k], r.entries_[i][k], prod);
            }
        }
    }
    return r;
}

// ===================================================================
// Transpose
// ===================================================================

inline MatQ transpose(const MatQ& a) {
    MatQ r(a.n_cols(), a.n_rows());
    for (std::size_t i = 0; i < a.n_rows(); ++i) {
        for (std::size_t j = 0; j < a.n_cols(); ++j) {
            r.entries_[j][i] = a.entries_[i][j];
        }
    }
    return r;
}

// ===================================================================
// Determinant (square, recursive cofactor expansion along first row)
// ===================================================================

inline Rat determinant(const MatQ& A) {
    if (!A.is_square()) {
        throw std::invalid_argument("MatQ det: matrix is not square");
    }
    std::size_t n = A.n_rows();
    if (n == 0) {
        Rat one; set_si(one, 1, 1);
        return one;
    }
    if (n == 1) return A.entries_[0][0];

    // Base case 2x2: ad - bc
    if (n == 2) {
        Rat ad; mul(ad, A.entries_[0][0], A.entries_[1][1]);
        Rat bc; mul(bc, A.entries_[0][1], A.entries_[1][0]);
        Rat r; sub(r, ad, bc);
        return r;
    }

    // Cofactor expansion along first row.
    // For larger matrices, this is O(n!); a proper LU decomposition
    // would be O(n^3).  We keep the simple version for now; the
    // Q(β) layer in linalg_qbeta.hpp uses Gaussian elimination for
    // speed.
    Rat det; set_si(det, 0, 1);
    for (std::size_t j = 0; j < n; ++j) {
        // Build the (n-1)×(n-1) minor by skipping row 0 and column j.
        MatQ minor(n - 1, n - 1);
        for (std::size_t i = 1; i < n; ++i) {
            std::size_t mj = 0;
            for (std::size_t k = 0; k < n; ++k) {
                if (k == j) continue;
                minor.entries_[i - 1][mj++] = A.entries_[i][k];
            }
        }
        Rat cofactor; mul(cofactor, A.entries_[0][j], determinant(minor));
        if (j % 2 == 1) neg(cofactor);
        add(det, det, cofactor);
    }
    return det;
}

// ===================================================================
// Inverse (square, via Gaussian elimination with exact arithmetic)
// ===================================================================
//
// Returns the inverse.  Throws if A is singular.

inline MatQ inverse(const MatQ& A) {
    if (!A.is_square()) {
        throw std::invalid_argument("MatQ inverse: matrix is not square");
    }
    std::size_t n = A.n_rows();
    // Augment A with I.
    MatQ aug(n, 2 * n);
    for (std::size_t i = 0; i < n; ++i) {
        for (std::size_t j = 0; j < n; ++j) aug.entries_[i][j] = A.entries_[i][j];
        set_si(aug.entries_[i][n + i], 1, 1);
    }
    // Forward elimination with partial pivoting.
    for (std::size_t col = 0; col < n; ++col) {
        // Find first non-zero row in [col, n).
        std::size_t pivot = col;
        bool found = false;
        for (std::size_t row = col; row < n; ++row) {
            if (!is_zero(aug.entries_[row][col])) { pivot = row; found = true; break; }
        }
        if (!found) {
            throw std::runtime_error("MatQ inverse: matrix is singular");
        }
        if (pivot != col) {
            std::swap(aug.entries_[pivot], aug.entries_[col]);
        }
        for (std::size_t row = col + 1; row < n; ++row) {
            if (is_zero(aug.entries_[row][col])) continue;
            Rat factor;
            div(factor, aug.entries_[row][col], aug.entries_[col][col]);
            for (std::size_t k = col; k < 2 * n; ++k) {
                Rat prod;
                mul(prod, factor, aug.entries_[col][k]);
                sub(aug.entries_[row][k], aug.entries_[row][k], prod);
            }
        }
    }
    // Back-substitute (Gauss-Jordan: normalize each row and eliminate
    // the column entries above the diagonal in BOTH halves).
    for (long long row = static_cast<long long>(n) - 1; row >= 0; --row) {
        // Normalize row `row` so the diagonal entry is 1.
        for (std::size_t j = 0; j < 2 * n; ++j) {
            if (j == static_cast<std::size_t>(row)) continue;
            if (is_zero(aug.entries_[row][j])) continue;
            Rat q;
            div(q, aug.entries_[row][j], aug.entries_[row][row]);
            // We want aug[row][j] = 0.  We have it as some value; we'll
            // subtract q * aug[row][row] in the next step, but since
            // we're going to divide later, just track and divide at the
            // end.  For simplicity, do the full back-sub here.
        }
        for (std::size_t k = 0; k < static_cast<std::size_t>(row); ++k) {
            // Subtract aug[k][row] times this row from row k.
            Rat factor;
            div(factor, aug.entries_[k][row], aug.entries_[row][row]);
            for (std::size_t j = 0; j < 2 * n; ++j) {
                if (j == static_cast<std::size_t>(row)) {
                    // We want aug[k][row] = 0.  Subtract factor * aug[row][row] = factor * 1.
                    // Actually after we set aug[row][row] = 1 (below),
                    // we subtract factor * 1 = factor.
                    // For now, just set to 0.
                    set_si(aug.entries_[k][j], 0, 1);
                } else {
                    Rat prod;
                    mul(prod, factor, aug.entries_[row][j]);
                    sub(aug.entries_[k][j], aug.entries_[k][j], prod);
                }
            }
        }
    }
    // Now normalize each row by its diagonal.
    for (std::size_t row = 0; row < n; ++row) {
        if (is_zero(aug.entries_[row][row])) {
            throw std::runtime_error("MatQ inverse: matrix is singular (zero pivot)");
        }
        for (std::size_t j = n; j < 2 * n; ++j) {
            Rat q;
            div(q, aug.entries_[row][j], aug.entries_[row][row]);
            aug.entries_[row][j] = q;
        }
        // Set the diagonal of the left half to 1 (already done above? No.)
        set_si(aug.entries_[row][row], 1, 1);
    }
    // Extract the right half as the inverse.
    MatQ inv(n, n);
    for (std::size_t i = 0; i < n; ++i) {
        for (std::size_t j = 0; j < n; ++j) {
            inv.entries_[i][j] = aug.entries_[i][n + j];
        }
    }
    return inv;
}

}  // namespace mathlib
