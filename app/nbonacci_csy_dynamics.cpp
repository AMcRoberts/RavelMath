// nbonacci_csy_dynamics.cpp
//
// "Complete classification and dynamics" of the CSY state count
// for the n-bonacci Pisot numeration.  For each n in [n_min, n_max]
// and each max_prefix in a list, build the CSY finite zero-
// expansion automaton and record the state count, raw node count,
// transition count, and whether the search was truncated.
//
// The dynamics question: how does the state count grow with
// max_prefix_length, and at what max_prefix does it saturate (if
// at all)?  Saturation here means the state count no longer grows
// between successive max_prefix values, which is the direct finite
// test of CSY Theorem 3's "Pisot F system ⇒ finite automaton" claim
// for the n-bonacci numeration.
//
// The classification question: per n, what's the saturated state
// count, and is it bounded by the homogeneous-shell survival bound
// n+1 (which the C++ covering witness enumerator has verified for
// n=2..8)?
//
// Usage: ./out/nbonacci_csy_dynamics
//   --n-min=N --n-max=M
//   --max-prefixes=4,6,8,10,12
//   --bound-bits=B
//   --out=PATH

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "adelic/csy_carry_automaton.hpp"
#include "adelic/csy_finite_carry_automaton.hpp"

using namespace adelic;

namespace {

PisotPoly nbonacci_poly(std::size_t n) {
    return PisotPoly::fromCoefficients(std::vector<long long>(n, 1));
}

std::vector<long long> nbonacci_alphabet() { return {0, 1}; }

std::vector<std::size_t> parse_size_list(const std::string& s) {
    std::vector<std::size_t> out;
    std::stringstream ss(s);
    std::string tok;
    while (std::getline(ss, tok, ',')) out.push_back(std::stoull(tok));
    return out;
}

}  // namespace

int main(int argc, char** argv) {
    std::size_t n_min = 2, n_max = 8;
    std::vector<std::size_t> max_prefixes = {4, 6, 8, 10, 12};
    std::size_t bound_bits = 12;
    std::string out_path = "out/nbonacci_csy_dynamics.json";
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        auto eq = a.find('=');
        if (eq == std::string::npos) continue;
        std::string k = a.substr(0, eq), v = a.substr(eq + 1);
        if (k == "--n-min") n_min = std::stoull(v);
        else if (k == "--n-max") n_max = std::stoull(v);
        else if (k == "--max-prefixes") max_prefixes = parse_size_list(v);
        else if (k == "--bound-bits") bound_bits = std::stoull(v);
        else if (k == "--out") out_path = v;
    }
    std::printf("nbonacci_csy_dynamics: n in [%zu, %zu], "
                "max_prefix in [", n_min, n_max);
    for (std::size_t mp : max_prefixes) std::printf("%zu,", mp);
    std::printf("], bound_bits=%zu\n", bound_bits);
    // For each n, scan max_prefixes; record the full data.
    // Schema: results[n] = { max_prefix -> (states, raw_nodes,
    //                                     transitions, truncated) }
    std::map<std::size_t,
             std::map<std::size_t,
                      std::tuple<std::size_t, std::size_t,
                                 std::size_t, bool>>> results;
    for (std::size_t n = n_min; n <= n_max; ++n) {
        std::printf("\nn=%zu  ", n);
        PisotPoly P = nbonacci_poly(n);
        std::vector<long long> B = nbonacci_alphabet();
        std::printf("(PisotPoly=x^%zu - x^%zu - ... - x - 1)\n", n, n - 1);
        for (std::size_t mp : max_prefixes) {
            CSYZeroAutomaton aut;
            aut.build(P, B, mp, bound_bits);
            results[n][mp] = std::make_tuple(aut.state_count(),
                                             aut.raw_node_count(),
                                             aut.transition_count(),
                                             aut.truncated);
            const auto& [s, r, t, tr] = results[n][mp];
            std::printf("  max_prefix=%2zu  states=%4zu  raw=%8zu  "
                        "trans=%5zu  truncated=%d\n",
                        mp, s, r, t, tr ? 1 : 0);
        }
    }
    // Saturation analysis: per n, find the largest max_prefix at
    // which state_count equals the next-larger max_prefix's
    // state_count (i.e., has saturated).  If never, mark UNSAT.
    std::printf("\n=== CSY state-count saturation analysis ===\n");
    std::printf("n | states(per max_prefix)                 | "
                "saturated?  saturated_at_mp  n+1  states<=n+1?\n");
    std::printf("--+------------------------------------------+"
                "-----------+------------------+----+--------------\n");
    for (std::size_t n = n_min; n <= n_max; ++n) {
        std::printf("  %zu | ", n);
        for (std::size_t mp : max_prefixes) {
            std::printf("%zu ", std::get<0>(results[n][mp]));
        }
        // Find the smallest mp such that the state count doesn't
        // grow in the next step (if any).
        bool saturated = false;
        std::size_t saturated_mp = 0;
        for (std::size_t i = 0; i + 1 < max_prefixes.size(); ++i) {
            std::size_t s_now = std::get<0>(results[n][max_prefixes[i]]);
            std::size_t s_next = std::get<0>(results[n][max_prefixes[i + 1]]);
            if (s_now == s_next) {
                saturated = true;
                saturated_mp = max_prefixes[i];
                break;
            }
        }
        std::printf("| %s      | %zu                  | %zu  ",
                    (saturated ? "YES" : "no "),
                    (saturated ? saturated_mp : 0),
                    n + 1);
        if (saturated) {
            std::size_t sat = std::get<0>(results[n][saturated_mp]);
            std::printf("| %s\n", (sat <= n + 1 ? "YES" : "no"));
        } else {
            std::printf("| (n/a)\n");
        }
    }
    // JSON output
    {
        std::ofstream f(out_path);
        if (!f) {
            std::fprintf(stderr, "Failed to write %s\n", out_path.c_str());
        } else {
            f << "{";
            f << "\"n_range\":[" << n_min << "," << n_max << "],";
            f << "\"max_prefixes\":[";
            for (std::size_t i = 0; i < max_prefixes.size(); ++i) {
                if (i) f << ",";
                f << max_prefixes[i];
            }
            f << "],";
            f << "\"bound_bits\":" << bound_bits << ",";
            f << "\"results\":{";
            bool first_n = true;
            for (std::size_t n = n_min; n <= n_max; ++n) {
                if (!first_n) f << ",";
                first_n = false;
                f << "\"" << n << "\":{";
                bool first_mp = true;
                for (std::size_t mp : max_prefixes) {
                    if (!first_mp) f << ",";
                    first_mp = false;
                    const auto& [s, r, t, tr] = results[n][mp];
                    f << "\"" << mp << "\":{"
                      << "\"states\":" << s
                      << ",\"raw_nodes\":" << r
                      << ",\"transitions\":" << t
                      << ",\"truncated\":" << (tr ? "true" : "false")
                      << "}";
                }
                f << "}";
            }
            f << "}}";
            std::printf("\nwrote %s\n", out_path.c_str());
        }
    }
    return 0;
}
