// thread_a4_extension_graph_probe.cpp
//
// Thread A4: locate the language branching that distinguishes the
// non-AR sigma_{0,1} orbit from AR-exact Tribonacci and the AR-partial
// sigma_{a,1} family. For every factor u of length n occurring away
// from the sampled orbit's endpoints, collect
//
//   L(u) = {a : au occurs}, R(u) = {b : ub occurs},
//   E(u) = {(a,b) : aub occurs}.
//
// Arnoux-Rauzy languages have one left-special and one right-special
// factor at every length, each with full alphabet valence. Multiple
// special factors expose the precise local branching hidden by the
// aggregate complexity count p(n).
//
// Observed result (2026-08-01, orbit prefixes 5.2e5--9.3e5 symbols):
// the direct extension-graph pass checks factors through length 64;
// a suffix-automaton pass on a separately enlarged orbit checks
// sigma_{0,1} through length 500000 (orbit=35676949 symbols, a ~71x
// margin over the checked length -- widened from an initial 60000/
// 4983377-symbol pass after that pass's own margin, ~83x, still left
// room for doubt following the smaller 525456-symbol prefix's spurious
// n=55406 boundary artifact recorded below).
// It has p(n)=5n-5 for every checked n>=4, with five left-special and
// three right-special factors at every checked n>=4. By contrast every
// AR-complexity control has
// p(n)=2n+1: Tribonacci and sigma_2 realize this with one ternary
// special factor on each side, sigma_{a,1} with two binary left-
// special factors and one ternary right-special factor, and sigma_1
// with the left/right orientation reversed. This is a finite exact
// language computation, not yet an all-n theorem or a proved
// correspondence with stepped-hyperplane seam types.  Klouda--
// Pelantova (2009), preserved under refs/FullText and inventoried in
// refs/FULLTEXT_MANIFEST.tsv, supplies relevant special-factor/branch
// machinery for the next proof step; it does not itself prove this
// substitution's displayed formula.

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "ravel/substitution.hpp"

using ravel::SubstitutionRule;

namespace {

using Word = std::vector<std::int8_t>;
using Sigma = std::vector<Word>;

Sigma sigma_ab(int a, int b) {
    Sigma sigma(3);
    for (int i = 0; i < a; ++i) sigma[0].push_back(0);
    for (int i = 0; i < b; ++i) sigma[0].push_back(1);
    sigma[0].push_back(2);
    for (int i = 0; i < a; ++i) sigma[1].push_back(0);
    sigma[1].push_back(2);
    sigma[2] = {0};
    return sigma;
}

Sigma tribonacci() {
    return {{0, 1}, {0, 2}, {0}};
}

Sigma n_bonacci(std::size_t n) {
    Sigma sigma(n);
    for (std::size_t i = 0; i + 1 < n; ++i)
        sigma[i] = {0, static_cast<std::int8_t>(i + 1)};
    sigma[n - 1] = {0};
    return sigma;
}

Sigma sigma1_3l() { return {{0, 0, 1}, {0, 2}, {0}}; }
Sigma sigma2_3l() { return {{0, 0, 1}, {0, 0, 2}, {0}}; }

Word orbit_prefix(const SubstitutionRule& rule, std::size_t minimum) {
    Word orbit{0};
    for (std::size_t round = 0; round < 128 && orbit.size() < minimum; ++round) {
        orbit = rule.apply_once(orbit);
    }
    return orbit;
}

struct Extensions {
    std::set<std::int8_t> left;
    std::set<std::int8_t> right;
    std::set<std::pair<std::int8_t, std::int8_t>> both;
};

// A suffix automaton groups factors with identical end-position sets.
// Every state represents exactly one factor at each length in
// (len(link(state)), len(state)], all sharing the same right-extension
// set. Range addition over those intervals therefore gives p(n) and
// the number of right-special factors for every n in linear time.
// Running the same construction on the reversed orbit gives the
// left-special counts.
class SuffixAutomaton {
public:
    struct State {
        std::array<int, 3> next{{-1, -1, -1}};
        int link = -1;
        std::size_t len = 0;
    };

    explicit SuffixAutomaton(const Word& word) {
        states_.reserve(2 * word.size());
        states_.push_back(State{});
        int last = 0;
        for (std::int8_t raw : word) {
            const int letter = static_cast<int>(raw);
            const int current = static_cast<int>(states_.size());
            states_.push_back(State{});
            states_[current].len = states_[last].len + 1;
            int p = last;
            while (p >= 0 && states_[p].next[letter] < 0) {
                states_[p].next[letter] = current;
                p = states_[p].link;
            }
            if (p < 0) {
                states_[current].link = 0;
            } else {
                const int q = states_[p].next[letter];
                if (states_[p].len + 1 == states_[q].len) {
                    states_[current].link = q;
                } else {
                    const int clone = static_cast<int>(states_.size());
                    states_.push_back(states_[q]);
                    states_[clone].len = states_[p].len + 1;
                    while (p >= 0 && states_[p].next[letter] == q) {
                        states_[p].next[letter] = clone;
                        p = states_[p].link;
                    }
                    states_[q].link = clone;
                    states_[current].link = clone;
                }
            }
            last = current;
        }
    }

    std::vector<long long> interval_counts(std::size_t max_n,
                                           bool special_only) const {
        std::vector<long long> delta(max_n + 2, 0);
        for (std::size_t i = 1; i < states_.size(); ++i) {
            if (special_only) {
                std::size_t degree = 0;
                for (int destination : states_[i].next) {
                    degree += destination >= 0;
                }
                if (degree < 2) continue;
            }
            std::size_t low = states_[static_cast<std::size_t>(states_[i].link)].len + 1;
            std::size_t high = std::min(states_[i].len, max_n);
            if (low > high) continue;
            ++delta[low];
            --delta[high + 1];
        }
        std::vector<long long> counts(max_n + 1, 0);
        long long active = 0;
        for (std::size_t n = 1; n <= max_n; ++n) {
            active += delta[n];
            counts[n] = active;
        }
        return counts;
    }

private:
    std::vector<State> states_;
};

void report_fast_stability(const Word& orbit) {
    const std::size_t max_n = std::min<std::size_t>(500000, orbit.size() / 8);
    const SuffixAutomaton forward(orbit);
    const auto complexity = forward.interval_counts(max_n, false);
    const auto right_special = forward.interval_counts(max_n, true);
    Word reversed(orbit.rbegin(), orbit.rend());
    const SuffixAutomaton backward(reversed);
    const auto left_special = backward.interval_counts(max_n, true);

    std::size_t first_deviation = 0;
    for (std::size_t n = 4; n <= max_n; ++n) {
        if (complexity[n] != static_cast<long long>(5 * n - 5)
            || left_special[n] != 5 || right_special[n] != 3) {
            first_deviation = n;
            break;
        }
    }
    if (first_deviation == 0) {
        std::printf("  FAST suffix-automaton check (orbit=%zu): "
                    "p(n)=5n-5, LS=5, RS=3 for every 4<=n<=%zu\n",
                    orbit.size(), max_n);
    } else {
        std::printf("  FAST suffix-automaton first deviation at n=%zu: "
                    "p=%lld LS=%lld RS=%lld\n", first_deviation,
                    complexity[first_deviation], left_special[first_deviation],
                    right_special[first_deviation]);
    }
}

std::string digits(const Word& word) {
    std::string out;
    out.reserve(word.size());
    for (std::int8_t letter : word) {
        out.push_back(static_cast<char>('0' + letter));
    }
    return out;
}

std::string letters(const std::set<std::int8_t>& values) {
    std::string out;
    for (std::int8_t value : values) {
        out.push_back(static_cast<char>('0' + value));
    }
    return out;
}

void report(const char* name, const Sigma& sigma, std::size_t max_n = 20,
            bool show_special_factors = false) {
    const SubstitutionRule rule(sigma);
    const Word orbit = orbit_prefix(rule, 1U << 19);
    std::printf("=== %s (orbit=%zu) ===\n", name, orbit.size());
    std::printf(" n  p(n)  left-special  right-special  bispecial  max-L  max-R  sum bilateral\n");
    for (std::size_t n = 1; n <= max_n; ++n) {
        std::map<Word, Extensions> factors;
        for (std::size_t i = 1; i + n < orbit.size(); ++i) {
            Word factor(orbit.begin() + static_cast<std::ptrdiff_t>(i),
                        orbit.begin() + static_cast<std::ptrdiff_t>(i + n));
            auto& ext = factors[factor];
            ext.left.insert(orbit[i - 1]);
            ext.right.insert(orbit[i + n]);
            ext.both.insert({orbit[i - 1], orbit[i + n]});
        }
        std::size_t left_special = 0;
        std::size_t right_special = 0;
        std::size_t bispecial = 0;
        std::size_t max_left = 0;
        std::size_t max_right = 0;
        long long bilateral_sum = 0;
        for (const auto& [factor, ext] : factors) {
            (void)factor;
            const bool ls = ext.left.size() >= 2;
            const bool rs = ext.right.size() >= 2;
            left_special += ls;
            right_special += rs;
            bispecial += ls && rs;
            max_left = std::max(max_left, ext.left.size());
            max_right = std::max(max_right, ext.right.size());
            bilateral_sum += static_cast<long long>(ext.both.size())
                - static_cast<long long>(ext.left.size())
                - static_cast<long long>(ext.right.size()) + 1;
        }
        std::printf("%2zu  %4zu  %12zu  %13zu  %9zu  %5zu  %5zu  %+13lld\n",
                    n, factors.size(), left_special, right_special,
                    bispecial, max_left, max_right, bilateral_sum);
        if (show_special_factors && n <= 20) {
            std::printf("    L:");
            for (const auto& [factor, ext] : factors) {
                if (ext.left.size() >= 2) {
                    std::printf(" %s[%s]", digits(factor).c_str(),
                                letters(ext.left).c_str());
                }
            }
            std::printf("\n    R:");
            for (const auto& [factor, ext] : factors) {
                if (ext.right.size() >= 2) {
                    std::printf(" %s[%s]", digits(factor).c_str(),
                                letters(ext.right).c_str());
                }
            }
            std::printf("\n");
            if (n <= 3) {
                std::printf("    ALL:");
                for (const auto& [factor, ext] : factors) {
                    std::printf(" %s[L%s,R%s]", digits(factor).c_str(),
                                letters(ext.left).c_str(),
                                letters(ext.right).c_str());
                }
                std::printf("\n");
            }
        }
    }
    // The direct n<=64 pass needs only the base orbit. The long-range
    // special-factor count needs a much larger boundary margin: on the base
    // 525456-symbol prefix the left-special count spuriously drops at n=55406,
    // while a large enough prefix restores all five branches far past that --
    // checked to n=500000 on a 35676949-symbol prefix (2026-08-01, first
    // cadence-loop cycle).
    if (show_special_factors) report_fast_stability(orbit_prefix(rule, 1U << 25));
    std::printf("\n");
}

}  // namespace

int main() {
    report("Tribonacci (AR-exact)", tribonacci());
    report("Tetrabonacci (AR-exact, 4-letter)", n_bonacci(4));
    report("Pentanacci (AR-exact, 5-letter)", n_bonacci(5));
    report("Hexanacci (AR-exact, 6-letter)", n_bonacci(6));
    report("sigma_{1,1} (AR-partial)", sigma_ab(1, 1));
    report("sigma_{2,1} (AR-partial)", sigma_ab(2, 1));
    report("sigma_1 (AR-partial)", sigma1_3l());
    report("sigma_2 (AR-partial)", sigma2_3l());
    report("sigma_{0,1} (non-AR)", sigma_ab(0, 1), 64, true);
    return 0;
}
