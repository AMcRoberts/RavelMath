// nbonacci_csy_state_count.cpp
//
// C++ probe: for the n-bonacci Pisot numeration (PisotPoly with
// coefficients c = {1, 1, ..., 1}, the n-bonacci polynomial
// x^n - x^{n-1} - ... - x - 1), build the Carton-Sudbery-Yassawi
// finite zero-expansion automaton and report the state count.
// This is the *direct* test of whether the n-bonacci numeration
// satisfies CSY's "Condition F" (preserves zeros, i.e. the
// zero-expansion language is regular): a finite state count is
// CSY Theorem 3's actual condition.  The Frougny-Solomyak
// Condition F for β-numerations is the p-adic / adelic
// generalization of CSY's preserves-zeros, and the CSY
// construction reduces to the F check at every node (see
// include/adelic/coincidence_and_property_f.hpp and
// include/adelic/csy_carry_automaton.hpp for the existing
// cross-validation).
//
// For Pisot F systems, the state count is the pumping length N
// (CSY Lemma 43): if [g0^n]_U = 0 for some n >= N, it is zero for
// infinitely many such n.  This bounds the carry propagation in
// the CSY addition.  Compare the state count to the survival-depth
// bound n+1 from the homogeneous-shell witness: if the state
// count <= n+1, the CSY automaton is *strictly* more efficient
// than the homogeneous-shell covering witness at bounding the
// carry; if the state count >= n+1 but the n+1 SAT witness is
// already verified (n=2..8), the CSY and homogeneous-shell bounds
// are consistent; if the state count is infinite at some n, the
// n-bonacci fails CSY Theorem 3 (i.e. the n-bonacci does NOT
// satisfy Condition F / preserves zeros), which is a real
// mathematical negative result worth reporting.
//
// Usage: ./out/nbonacci_csy_state_count [--n-min=N] [--n-max=M]
//                                       [--max-prefix=L] [--bound-bits=B]
//                                       [--out=PATH]
//
// The defaults: n in [2, 8], max_prefix_length=8, bound_bits=14.

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "adelic/csy_carry_automaton.hpp"
#include "adelic/csy_finite_carry_automaton.hpp"

using namespace adelic;

namespace {

PisotPoly nbonacci_poly(std::size_t n) {
    // x^n - x^{n-1} - ... - x - 1, i.e. c_0 = c_1 = ... = c_{n-1} = 1.
    // In PisotPoly's high-to-low storage, that's {1, 1, ..., 1}
    // with n copies of 1.
    return PisotPoly::fromCoefficients(std::vector<long long>(n, 1));
}

// Standard digit alphabet for a Pisot β-numeration with β < 2:
//   {0, 1, ..., ⌈β⌉ - 1} where ⌈β⌉ is the smallest integer > β.
// For the n-bonacci β (Pisot root of x^n - x^{n-1} - ... - x - 1),
// β is in (1, 2) for all n >= 2, so ⌈β⌉ = 2 and the digit
// alphabet is {0, 1}.
std::vector<long long> nbonacci_alphabet() { return {0, 1}; }

}  // namespace

int main(int argc, char** argv) {
    std::size_t n_min = 2, n_max = 8;
    std::size_t max_prefix_length = 8;
    std::size_t bound_bits = 14;
    std::string out_path;
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        auto eq = a.find('=');
        if (eq == std::string::npos) continue;
        std::string k = a.substr(0, eq), v = a.substr(eq + 1);
        if (k == "--n-min") n_min = std::stoull(v);
        else if (k == "--n-max") n_max = std::stoull(v);
        else if (k == "--max-prefix") max_prefix_length = std::stoull(v);
        else if (k == "--bound-bits") bound_bits = std::stoull(v);
        else if (k == "--out") out_path = v;
    }
    std::printf("nbonacci_csy_state_count: n in [%zu, %zu], "
                "max_prefix=%zu, bound_bits=%zu\n",
                n_min, n_max, max_prefix_length, bound_bits);
    std::printf("(This probes CSY Theorem 3 on the n-bonacci PisotPoly.\n"
                " A finite state count means the n-bonacci numeration\n"
                " satisfies CSY's 'preserves zeros' (a.k.a. Frougny-Solomyak\n"
                " Condition F) at the tested bound.)\n\n");
    std::map<std::size_t, std::map<std::string, std::size_t>> results;
    for (std::size_t n = n_min; n <= n_max; ++n) {
        PisotPoly P = nbonacci_poly(n);
        std::vector<long long> B = nbonacci_alphabet();
        std::printf("n=%zu  PisotPoly=(x^%zu - x^%zu - ... - x - 1), "
                    "B={0,1}\n", n, n, n - 1);
        CSYZeroAutomaton aut;
        aut.build(P, B, max_prefix_length, bound_bits);
        std::printf("  raw_nodes=%zu  states=%zu  transitions=%zu  "
                    "max_visited_position=%zu  truncated=%d\n",
                    aut.raw_node_count(),
                    aut.state_count(),
                    aut.transition_count(),
                    aut.max_visited_position,
                    aut.truncated ? 1 : 0);
        results[n]["states"] = aut.state_count();
        results[n]["transitions"] = aut.transition_count();
        results[n]["max_visited_position"] = aut.max_visited_position;
        results[n]["raw_nodes"] = aut.raw_node_count();
        results[n]["truncated"] = aut.truncated ? 1 : 0;
    }
    // Compare to the n+1 survival bound
    std::printf("\n=== CSY state count vs n+1 survival-depth bound ===\n");
    std::printf("n | CSY states | n+1 bound | states <= n+1?\n");
    std::printf("--+------------+-----------+----------------\n");
    for (std::size_t n = n_min; n <= n_max; ++n) {
        std::size_t s = results[n]["states"];
        std::size_t bound = n + 1;
        std::printf("  %zu | %zu | %zu | %s\n", n, s, bound,
                    (s <= bound ? "YES" : "no"));
    }
    // JSON output
    if (!out_path.empty()) {
        std::ofstream f(out_path);
        if (!f) {
            std::fprintf(stderr, "Failed to write %s\n", out_path.c_str());
        } else {
            f << "{\"n_range\":[" << n_min << "," << n_max << "],"
              << "\"max_prefix\":" << max_prefix_length
              << ",\"bound_bits\":" << bound_bits << ",\"results\":{";
            bool first = true;
            for (std::size_t n = n_min; n <= n_max; ++n) {
                if (!first) f << ",";
                first = false;
                f << "\"" << n << "\":{"
                  << "\"states\":" << results[n]["states"] << ","
                  << "\"transitions\":" << results[n]["transitions"] << ","
                  << "\"raw_nodes\":" << results[n]["raw_nodes"] << ","
                  << "\"max_visited_position\":" << results[n]["max_visited_position"] << ","
                  << "\"truncated\":" << results[n]["truncated"]
                  << "}";
            }
            f << "}}";
            std::printf("\nwrote %s\n", out_path.c_str());
        }
    }
    return 0;
}
