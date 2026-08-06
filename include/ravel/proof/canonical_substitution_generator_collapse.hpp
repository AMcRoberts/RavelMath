// ravel/proof/canonical_substitution_generator_collapse.hpp
//
// Generalizes `beta_in_one_two_forces_qrs.hpp` beyond beta in (1,2).
// That earlier theorem showed: for 1 < beta < 2, every digit is 0 or 1,
// every parent-decomposition prefix is [] or [0], and the resulting
// three defect classes {0,+1,-1} are exactly Q/R/S with no possibility
// of a fourth.
//
// For beta >= 2, digits can exceed 1 (floor(beta) is the forced first
// digit), so prefixes can have length up to floor(beta), and the raw
// defect classification can have up to 2*floor(beta)+1 classes -- e.g.
// beta in [2,3) gives up to 5 raw classes {-2,-1,0,+1,+2} (checked on
// the silver ratio and x^2-2x-2, both of which realize all five).
//
// This header answers the question those two spot checks raised but
// didn't settle in general: do the |defect|>=2 raw classes ever survive
// as genuine NEW primitive generators, or are they always reducible --
// entrywise dominated by the corresponding power of the +-1 generator,
// hence expressible as words in Q/R/S rather than needing their own
// label? It checks this directly and exactly for whatever beta is
// supplied (any digit range), rather than assuming the pattern found at
// small examples continues; it does not itself prove the domination
// holds for every beta (that would require an inductive argument over
// arbitrary digit values, not yet written down) -- it is the checkable
// operation such a proof would need as its base case verifier, and the
// tool for finding a counterexample if the pattern ever breaks.

#pragma once

#include <cstddef>
#include <map>
#include <queue>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "math/qbeta.hpp"
#include "math/sturm.hpp"
#include "ravel/canonical_beta_substitution.hpp"

namespace ravel::proof {

struct GeneratorCollapseCertificate {
    std::vector<long long> digits;
    std::size_t alphabet_size{};
    std::size_t distinct_prefixes{};
    long long max_prefix_length{};       // = floor(beta), forced by the first digit
    std::size_t raw_defect_classes{};     // size of {-max,...,-1,0,1,...,max} actually realized
    std::size_t primitive_generator_count{}; // raw_defect_classes minus the dominated ones (always odd, >=1)
    std::map<long long,bool> dominated;   // defect value -> is it dominated by the +-1 generator's power (only for |defect|>=2)
    bool all_extremal_classes_dominated{};
    bool proved{};
    std::string obstruction;
};

inline GeneratorCollapseCertificate derive_canonical_substitution_generator_collapse(
    const mathlib::QBetaRing& R, const mathlib::RootInterval& beta_I) {
    using namespace mathlib;
    GeneratorCollapseCertificate out;

    auto ge = exact_greedy_beta_expansion_of_one(R, beta_I, 128);
    if (!ge.terminated && ge.period_len == 0) {
        out.obstruction = "greedy expansion neither terminated nor cycled within the step budget";
        return out;
    }
    out.digits = ge.digits;
    std::vector<std::vector<long long>> sigma;
    if (ge.terminated) {
        sigma = canonical_beta_substitution_from_digits(out.digits);
    } else if (ge.purely_periodic) {
        sigma = canonical_beta_substitution_from_digits(out.digits);
    } else {
        std::vector<long long> preperiod(out.digits.begin(), out.digits.begin() + (long long)ge.preperiod_len);
        std::vector<long long> period(out.digits.begin() + (long long)ge.preperiod_len, out.digits.end());
        sigma = canonical_beta_substitution_eventually_periodic(preperiod, period);
    }
    const std::size_t n = sigma.size();
    out.alphabet_size = n;

    std::vector<std::vector<std::pair<long long,std::vector<long long>>>> parents(n);
    std::set<std::vector<long long>> prefixes;
    long long max_prefix_len = 0;
    for (std::size_t c = 0; c < n; ++c) {
        const auto& img = sigma[c];
        for (std::size_t k = 0; k < img.size(); ++k) {
            auto inner = (std::size_t)img[k];
            std::vector<long long> pre(img.begin(), img.begin() + (long long)k);
            parents[inner].push_back({(long long)c, pre});
            prefixes.insert(pre);
            if ((long long)pre.size() > max_prefix_len) max_prefix_len = (long long)pre.size();
        }
    }
    out.distinct_prefixes = prefixes.size();
    out.max_prefix_length = max_prefix_len;

    auto role = [&](long long i, long long j) { return (std::size_t)(i * (long long)n + j); };
    std::map<long long, std::vector<std::vector<long long>>> G;
    for (long long d = -max_prefix_len; d <= max_prefix_len; ++d)
        G[d] = std::vector<std::vector<long long>>(n * n, std::vector<long long>(n * n, 0));

    std::set<long long> defects;
    for (long long i = 0; i < (long long)n; ++i) for (long long j = 0; j < (long long)n; ++j)
        for (const auto& [pc, pp] : parents[(std::size_t)i]) for (const auto& [qc, qp] : parents[(std::size_t)j]) {
            long long defect = (long long)qp.size() - (long long)pp.size();
            defects.insert(defect);
            G[defect][role(i, j)][role(pc, qc)]++;
        }
    out.raw_defect_classes = defects.size();

    // For each edge with |defect|>=2, ask the FULL question, not just
    // "does a pure chain of the same-sign unit generator dominate it":
    // is the specific (source role, target role) pair reachable via ANY
    // sequence of {-1,0,+1}-labeled edges summing to the same net
    // defect? This subsumes the earlier G[k] <= G[base]^k check (which
    // only tried the single pure-chain word) and additionally allows
    // detours through 0 and the opposite-sign generator. Validated
    // against known cases both ways (x^2-2x-2's known-reducible edges
    // are found reachable; x^3-2x^2-2's and x^3-2x^2-x+1's known-
    // irreducible witnesses remain unreachable under this stronger
    // test) before trusting it -- see the diary entry this accompanies.
    std::vector<std::vector<std::pair<std::size_t,long long>>> unit_edges(n * n);
    for (long long d = -max_prefix_len; d <= max_prefix_len; ++d) {
        if (d < -1 || d > 1) continue;
        for (std::size_t s = 0; s < n * n; ++s) for (std::size_t t = 0; t < n * n; ++t)
            if (G[d][s][t] > 0) unit_edges[s].push_back({t, d});
    }
    auto reachable_via_unit_steps = [&](std::size_t src, std::size_t tgt, long long target_net) {
        std::set<std::pair<std::size_t,long long>> visited;
        std::queue<std::pair<std::size_t,long long>> q;
        visited.insert({src, 0});
        q.push({src, 0});
        int steps = 0;
        const long long net_bound = 2 * max_prefix_len + 2;
        const int step_bound = 4 * static_cast<int>(n) + 4;
        while (!q.empty() && steps < step_bound) {
            std::size_t level_size = q.size();
            for (std::size_t k = 0; k < level_size; ++k) {
                auto [r, net] = q.front(); q.pop();
                for (auto& [tr, lbl] : unit_edges[r]) {
                    long long nnet = net + lbl;
                    if (nnet < -net_bound || nnet > net_bound) continue;
                    if (tr == tgt && nnet == target_net) return true;
                    if (!visited.count({tr, nnet})) { visited.insert({tr, nnet}); q.push({tr, nnet}); }
                }
            }
            ++steps;
        }
        return false;
    };

    out.all_extremal_classes_dominated = true;
    std::size_t dominated_count = 0;
    for (long long d : defects) {
        if (d == 0 || d == 1 || d == -1) continue; // primitive by definition (Q, R, S themselves)
        bool any_edge_irreducible = false;
        for (std::size_t s = 0; s < n * n; ++s) for (std::size_t t = 0; t < n * n; ++t) {
            if (G[d][s][t] == 0) continue;
            if (!reachable_via_unit_steps(s, t, d)) { any_edge_irreducible = true; break; }
        }
        bool dom = !any_edge_irreducible;
        out.dominated[d] = dom;
        if (dom) ++dominated_count; else out.all_extremal_classes_dominated = false;
    }
    out.primitive_generator_count = out.raw_defect_classes - dominated_count;
    out.proved = true;
    return out;
}

}  // namespace ravel::proof
