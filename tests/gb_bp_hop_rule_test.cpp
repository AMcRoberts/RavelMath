// gb_bp_hop_rule_test.cpp
//
// Regression test for include/ravel/gb_bp_hop_rule.hpp: the
// hop-distance rule discovered while cross-checking BP-core's
// candidate bijection against G_B's real dominant recurrent core.
// Turns the informal "COMBINED RULE: X/Y edges match" printout from
// app/gb_bp_crosscheck.cpp's exploratory session into an actual
// asserted regression check -- if this rule ever stops matching
// exactly (e.g. a future change to the corona/contact-boundary
// construction shifts G_B's node identities), this test fails loudly
// instead of the discrepancy sitting unnoticed in a printout no one
// is re-reading.
//
// See docs/RESEARCH_STATUS.md "The G_B cross-check and the hop-distance
// dichotomy" for the full derivation this rule comes from.

#include "ravel/gb_bp_hop_rule.hpp"

#include <cstdio>
#include <vector>

using namespace ravel::gb_bp_hop;

static int total_tests = 0;
static int failed = 0;

#define CHECK(expr, label) do {                                       \
    ++total_tests;                                                    \
    if (!(expr)) { std::printf("  [FAIL] %s\n", label); ++failed; }    \
    else         { std::printf("  [ok]   %s\n", label); }              \
} while (0)

int main() {
    std::printf("gb_bp_hop_rule self-tests:\n");

    // n=3
    {
        auto rep = verify_hop_rule<3>(3);
        std::printf("  n=3: %zu/%zu states confirmed, %zu/%zu rule-edges match\n",
                    rep.confirmed_states, rep.total_states, rep.edges_matched, rep.edges_checked);
        CHECK(rep.fully_confirmed(), "n=3: every BP-core recurrent state confirmed");
        CHECK(rep.rule_fully_matches(), "n=3: hop-distance rule matches every confirmed-confirmed edge");
    }
    // n=4
    {
        auto rep = verify_hop_rule<4>(4);
        std::printf("  n=4: %zu/%zu states confirmed, %zu/%zu rule-edges match\n",
                    rep.confirmed_states, rep.total_states, rep.edges_matched, rep.edges_checked);
        CHECK(rep.fully_confirmed(), "n=4: every BP-core recurrent state confirmed");
        CHECK(rep.rule_fully_matches(), "n=4: hop-distance rule matches every confirmed-confirmed edge");
    }
    // n=5 -- as of this writing, iterative propagation confirms 38/40
    // states (the 2 stragglers sit exactly at the {0,n-2} pair's own
    // (i,j) identity, per docs/RESEARCH_STATUS.md); the rule itself still
    // must match 100% of whatever IS confirmed.
    {
        auto rep = verify_hop_rule<5>(5);
        std::printf("  n=5: %zu/%zu states confirmed, %zu/%zu rule-edges match\n",
                    rep.confirmed_states, rep.total_states, rep.edges_matched, rep.edges_checked);
        CHECK(rep.confirmed_states >= 38, "n=5: at least 38/40 states confirmed (known stragglers aside)");
        CHECK(rep.rule_fully_matches(), "n=5: hop-distance rule matches every confirmed-confirmed edge");
    }
    // n=6 -- similarly, 64/70 confirmed as of this writing.
    {
        auto rep = verify_hop_rule<6>(6);
        std::printf("  n=6: %zu/%zu states confirmed, %zu/%zu rule-edges match\n",
                    rep.confirmed_states, rep.total_states, rep.edges_matched, rep.edges_checked);
        CHECK(rep.confirmed_states >= 64, "n=6: at least 64/70 states confirmed (known stragglers aside)");
        CHECK(rep.rule_fully_matches(), "n=6: hop-distance rule matches every confirmed-confirmed edge");
    }

    std::printf("\n%d tests run, %d failed.\n", total_tests, failed);
    return failed == 0 ? 0 : 1;
}
