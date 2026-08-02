// Round-by-round birth mechanism check for the recurrent-SCC
// exhaustion birth-round claim (2026-08-02), generalizing
// app/class_ii_neighbor2_round2_birth_mechanism_check.cpp from the
// single round-1-to-2 step to every round of the corona iteration.
//
// Built on the trusted `algorithm2_trace` (corona.hpp) directly,
// seeded from the closed-form parameter-free
// class_ii_neighbor2_signed_contact_set() -- verified separately
// (verify_trace_matches_manual, scratch) that algorithm2_trace
// reproduces the round-2 check's manual one-step computation exactly
// at a=7 before trusting this generalization, and that the seed is
// already mirror-closed so algorithm2_trace's internal
// build_signed_contact_set call is a no-op on it. This bypasses the
// expensive D_cont/backward_closure front-end entirely (the seed is
// already proven equal to that derivation's result), which is why
// this reaches full round-by-round exactness cheaply where the
// original exhaustion-check tools could not.
//
// Expected rank-per-round mapping, derived from
// class_ii_neighbor_recurrent_component_states's own trusted dispatch
// for neighbor 2:
//   round 1: rank=a (dominant core)
//   round 2: rank=a-1 (regular shell) AND rank=a-2 (special shell #1)
//   round p, 3<=p<=a-1: rank=a-p (regular shell, parameter=a-p+1)
//   round p=a: rank=0 (special shell #0)
//
// Result: every catalogued recurrent rank's birth round matches this
// mapping exactly (containment: the rank's full catalogued state set
// is present among that round's new arrivals) at every tested `a` from
// 7 through 30 -- zero exceptions, checked round by round, not just
// the aggregate birth_round number
// class_ii_neighbor2_round_stratified_transient_check already reports.
// Round 2 also contains other, uncatalogued transient states not
// covered by this check (see the round-2 check's own header for the
// partial characterization of those); this check only asserts that
// the catalogued recurrent ranks are NOT missing, not that nothing
// else is present.
//
// Cost grows with `a` (same pattern observed throughout this session's
// automated checks -- not O(1) per value), so extend the swept range
// deliberately rather than assuming it stays cheap.
#include <cstdio>
#include <set>
#include <string>
#include "ravel/class_ii_neighbor_family.hpp"
#include "ravel/corona.hpp"
#include "ravel/pisot_substitution_properties.hpp"
#include "ravel/spectral.hpp"
#include "ravel/substitution.hpp"
#include "ravel/substitution_neighborhood.hpp"
using namespace ravel;
namespace {
using Matrix = std::vector<std::vector<long long>>;
FiniteSubstitution class_ii(std::size_t a) {
    FiniteSubstitution result(3);
    result[0].assign(a, 0); result[0].push_back(1); result[0].push_back(2);
    result[1].assign(a, 0); result[1].push_back(2);
    result[2] = {0};
    return result;
}
Matrix incidence(const FiniteSubstitution& s) {
    Matrix m(3, std::vector<long long>(3, 0));
    for (std::size_t im = 0; im < 3; ++im) for (auto l : s[im]) ++m[(std::size_t)l][im];
    return m;
}

std::set<SNode<3>> expected_for_round(long long a, long long p) {
    std::set<SNode<3>> out;
    if (p == 1) {
        auto core = class_ii_neighbor_dominant_core_states(2);
        out.insert(core.begin(), core.end());
    } else if (p == 2) {
        auto s1 = class_ii_neighbor_regular_shell_states(2, a, a - 1);
        auto s2 = class_ii_neighbor_special_shell_states(2, a, 1);
        out.insert(s1.begin(), s1.end());
        out.insert(s2.begin(), s2.end());
    } else if (p == a) {
        auto s0 = class_ii_neighbor_special_shell_states(2, a, 0);
        out.insert(s0.begin(), s0.end());
    } else if (p >= 3 && p <= a - 1) {
        auto s = class_ii_neighbor_regular_shell_states(2, a, a - p + 1);
        out.insert(s.begin(), s.end());
    }
    return out;
}

bool check_one(long long a, std::string& detail) {
    const auto center = class_ii((std::size_t)a);
    const auto spectral = classify_matrix_spectral(incidence(center));
    const auto neighbors = adjacent_swap_neighbors(center);
    SubstitutionRule rule(neighbors[2].substitution);
    const auto subst = make_substitution<3>(rule, spectral.beta);
    const auto seed = class_ii_neighbor2_signed_contact_set();

    const auto trace = algorithm2_trace<3>(subst, seed,
        CoronaConnectorPolicy::fixed_signed_contact,
        static_cast<int>(a) + 4);
    if (!trace.converged) {
        detail = "did not converge within a+4 rounds";
        return false;
    }

    bool ok = true;
    std::string problems;
    std::set<SNode<3>> prev;  // round p-1's survivor set
    for (std::size_t idx = 0; idx < trace.layers.size(); ++idx) {
        const long long p = static_cast<long long>(idx) + 1;
        const auto& cur = trace.layers[idx].nodes;
        std::set<SNode<3>> new_arrivals;
        for (const auto& n : cur) if (!prev.count(n)) new_arrivals.insert(n);

        const auto expected = expected_for_round(a, p);
        std::size_t missing = 0;
        for (const auto& n : expected) if (!new_arrivals.count(n)) ++missing;
        if (missing > 0) {
            ok = false;
            problems += " round=" + std::to_string(p)
                     + " expected=" + std::to_string(expected.size())
                     + " missing=" + std::to_string(missing);
        }
        prev = cur;
    }
    detail = problems.empty() ? "clean" : problems;
    return ok;
}
}  // namespace
int main(int argc, char** argv) {
    long long a_min = argc > 1 ? std::atoll(argv[1]) : 7;
    long long a_max = argc > 2 ? std::atoll(argv[2]) : 20;
    long long fails = 0, total = 0;
    for (long long a = a_min; a <= a_max; ++a) {
        std::string detail;
        bool ok = check_one(a, detail);
        ++total;
        if (!ok) { std::printf("a=%lld FAIL:%s\n", a, detail.c_str()); ++fails; }
    }
    std::printf("Swept a=%lld..%lld (%lld values): %lld failures\n", a_min, a_max, total, fails);
    if (fails == 0) std::printf("FULL ROUND-BY-ROUND BIRTH MECHANISM CONFIRMED, every round.\n");
    return 0;
}
