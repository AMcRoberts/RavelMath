// substitution.hpp
//
// SubstitutionRule: a primitive Pisot-substitution as a list of
// integer words.  Mirrors the reference Python
// `pisot_survey.py::matrix_of(subst, nl)` and
// `substitution_thermometer.py::expand(subst, nletters, n)`.
//
// Convention (matches the Python scripts):
//   subst[c] = image of letter c, as a vector<int8_t> of letter
//              indices in [0, alphabet_size)
//   matrix() returns M[r][c] = count of r in subst[c]
//   expand(seed, n) returns the sigma^k image of the seed letter,
//     grown until the resulting word has length >= n, then
//     truncated to exactly n symbols.
//   correlate(u, v, lmax) returns E[l] = (#{i : u[i] == a, v[i+l] == b}
//     averaged over (a,b) and pairs (i, i+l) in 0..n-l-1).  Used
//     by the Pisot thermometer to compute Theta(N).

#pragma once

#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <utility>
#include <vector>

namespace ravel {

class SubstitutionRule {
public:
    // Build a rule from a { c -> word } map.  Each word is a vector
    // of int8_t letter indices in [0, alphabet_size).  The map must
    // contain keys 0..alphabet_size-1.
    SubstitutionRule() = default;
    explicit SubstitutionRule(std::vector<std::vector<std::int8_t>> sigma)
        : sigma_(std::move(sigma)) {
        if (sigma_.empty()) {
            throw std::invalid_argument("SubstitutionRule: empty alphabet");
        }
        for (std::size_t c = 0; c < sigma_.size(); ++c) {
            for (auto r : sigma_[c]) {
                if (r < 0 || static_cast<std::size_t>(r) >= sigma_.size()) {
                    throw std::invalid_argument("SubstitutionRule: letter out of range");
                }
            }
        }
    }

    std::size_t alphabet_size() const noexcept { return sigma_.size(); }
    const std::vector<std::int8_t>& image(std::size_t c) const noexcept { return sigma_[c]; }

    // apply_once(word): apply sigma to each letter.
    std::vector<std::int8_t> apply_once(const std::vector<std::int8_t>& word) const {
        std::vector<std::int8_t> out;
        out.reserve(word.size() * 2);  // heuristic
        for (auto c : word) {
            const auto& img = sigma_[static_cast<std::size_t>(c)];
            out.insert(out.end(), img.begin(), img.end());
        }
        return out;
    }

    // apply_level(letter, level) = sigma^level(letter).
    std::vector<std::int8_t> apply_level(std::int8_t letter, std::size_t level) const {
        std::vector<std::int8_t> s{ letter };
        for (std::size_t k = 0; k < level; ++k) s = apply_once(s);
        return s;
    }

    // expand(seed, n): grow sigma^k(seed) until len >= n; truncate to n.
    std::vector<std::int8_t> expand(std::int8_t seed, std::size_t n) const {
        std::vector<std::int8_t> s{ seed };
        std::size_t guard = 0;
        while (s.size() < n) {
            s = apply_once(s);
            if (++guard > 200) break;  // non-growing substitution; bail.
        }
        if (s.size() > n) s.resize(n);
        return s;
    }

    // matrix()[r][c] = count of r in subst[c].
    std::vector<std::vector<long long>> matrix() const {
        std::vector<std::vector<long long>> M(
            sigma_.size(), std::vector<long long>(sigma_.size(), 0));
        for (std::size_t c = 0; c < sigma_.size(); ++c) {
            for (auto r : sigma_[c]) {
                M[static_cast<std::size_t>(r)][c] += 1;
            }
        }
        return M;
    }

private:
    std::vector<std::vector<std::int8_t>> sigma_;
};

// Correlation function <u[i] * v[i+L]> for lag L, averaged over
// {-1,+1}-valued observables.  We treat u,v as sequences of int8
// letter indices and assume the observable is the indicator of
// "letter == k" (k freely chosen, with the convention that the
// outer maximum is taken over the four CHSH sign patterns).
//
// correlate_binary(s, p, lmax) for sequences of +1/-1 returns
// E[l] = (1/(n-l)) sum_{i=0..n-l-1} s[i] * p[i+l].
inline std::vector<double> correlate_binary(
    const std::vector<double>& u,
    const std::vector<double>& v,
    std::size_t lmax) {
    const std::size_t n = std::min(u.size(), v.size());
    std::vector<double> E(lmax + 1, 0.0);
    if (n == 0) return E;
    for (std::size_t l = 0; l <= lmax && l < n; ++l) {
        double sum = 0.0;
        const std::size_t N = n - l;
        for (std::size_t i = 0; i < N; ++i) sum += u[i] * v[i + l];
        E[l] = sum / static_cast<double>(N);
    }
    return E;
}

// Sequential correlation for a single {+1,-1} sequence with itself
// (the convention used by the Pisot thermometer at lag l).
inline std::vector<double> autocorrelate_binary(
    const std::vector<double>& s,
    std::size_t lmax) {
    return correlate_binary(s, s, lmax);
}

}  // namespace ravel
