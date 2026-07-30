// Exact finite sweep for the recurrent balanced-pair core of
// sigma_{a,1}.  Tests an explicit word-state parameterization and the
// resulting closed characteristic-polynomial family.

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "math/charpoly.hpp"
#include "ravel/balanced_pair.hpp"
#include "ravel/graph_divisor.hpp"

using namespace ravel;

namespace {

using Word = std::vector<std::int8_t>;
using Pair = std::pair<Word, Word>;
using Matrix = std::vector<std::vector<long long>>;

std::vector<std::vector<std::int8_t>> sigma_a1(std::size_t a) {
    std::vector<std::vector<std::int8_t>> sigma(3);
    sigma[0].assign(a, 0);
    sigma[0].push_back(1);
    sigma[0].push_back(2);
    sigma[1].assign(a, 0);
    sigma[1].push_back(2);
    sigma[2] = {0};
    return sigma;
}

Word concatenate(std::initializer_list<Word> pieces) {
    Word result;
    std::size_t size = 0;
    for (const auto& piece : pieces) size += piece.size();
    result.reserve(size);
    for (const auto& piece : pieces) {
        result.insert(result.end(), piece.begin(), piece.end());
    }
    return result;
}

Word zeros(std::size_t count) {
    return Word(count, 0);
}

std::vector<Pair> predicted_pairs(std::size_t a) {
    const Pair A{
        concatenate({{2}, zeros(a + 1), {1, 2}}),
        concatenate({{1, 2}, zeros(a), {2, 0}})};
    const Pair B{{1, 2, 0}, {0, 1, 2}};
    const Pair C{
        concatenate({{0, 1, 2}, zeros(a)}),
        concatenate({{2}, zeros(a + 1), {1}})};
    const Pair D{
        concatenate({{2}, zeros(a), {1}}),
        concatenate({{1, 2}, zeros(a)})};

    std::vector<Pair> representatives{A, B, C};
    if (a >= 2) representatives.push_back(D);
    std::vector<Pair> result;
    for (const auto& pair : representatives) {
        result.push_back(pair);
        result.push_back({pair.second, pair.first});
    }
    return result;
}

std::set<std::string> predicted_keys(std::size_t a) {
    std::set<std::string> result;
    for (const auto& pair : predicted_pairs(a)) {
        result.insert(pair_key(pair.first, pair.second));
    }
    return result;
}

Matrix canonical_transition(std::size_t a) {
    const auto states = predicted_pairs(a);
    std::map<std::string, std::size_t> index;
    for (std::size_t i = 0; i < states.size(); ++i) {
        index[pair_key(states[i].first, states[i].second)] = i;
    }
    const SubstitutionRule rule(sigma_a1(a));
    Matrix matrix(states.size(), std::vector<long long>(states.size(), 0));
    for (std::size_t source = 0; source < states.size(); ++source) {
        const auto image = sigma_pair(rule, states[source]);
        for (const auto& child :
             reduce_pair(image.first, image.second, 3)) {
            const auto found =
                index.find(pair_key(child.first, child.second));
            if (found != index.end()) {
                ++matrix[source][found->second];
            }
        }
    }
    return matrix;
}

std::pair<Matrix, Matrix> swap_quotients(const Matrix& matrix) {
    const std::size_t n = matrix.size() / 2;
    Matrix symmetric(n, std::vector<long long>(n, 0));
    Matrix antisymmetric(n, std::vector<long long>(n, 0));
    for (std::size_t source = 0; source < n; ++source) {
        for (std::size_t destination = 0; destination < n;
             ++destination) {
            symmetric[source][destination] =
                matrix[2 * source][2 * destination]
                + matrix[2 * source][2 * destination + 1];
            antisymmetric[source][destination] =
                matrix[2 * source][2 * destination]
                - matrix[2 * source][2 * destination + 1];
        }
    }
    return {std::move(symmetric), std::move(antisymmetric)};
}

struct Core {
    Matrix matrix;
    std::set<std::string> keys;
};

Core actual_core(std::size_t a) {
    const auto bp = balanced_pair_transition_graph(
        SubstitutionRule(sigma_a1(a)));
    if (!bp.terminated) return {};
    const auto graph = WeightedDigraph::from_dense(bp.matrix);
    const auto [core, original] = extract_dominant_recurrent_core(graph);
    Core result;
    result.matrix.assign(core.n, std::vector<long long>(core.n, 0));
    for (std::size_t source = 0; source < core.n; ++source) {
        for (const auto& [destination, weight] : core.out_adj[source]) {
            result.matrix[source][destination] += weight;
        }
        const auto& state = bp.states.at(original[source]);
        result.keys.insert(pair_key(state.left, state.right));
    }
    return result;
}

std::vector<long long> predicted_charpoly(std::size_t a) {
    // High-first coefficients of
    // q_a=x^6-(a^2+1)x^4-2x^3+a^2*x^2-2a*x+1.
    const long long aa = static_cast<long long>(a);
    std::vector<long long> q{
        1, 0, -(aa * aa + 1), -2, aa * aa, -2 * aa, 1};
    if (a >= 2) {
        q.push_back(0);
        q.push_back(0);
    }
    return q;
}

}  // namespace

int main() {
    constexpr std::size_t max_a = 64;
    bool ok = true;
    for (std::size_t a = 1; a <= max_a; ++a) {
        const auto core = actual_core(a);
        const auto expected_keys = predicted_keys(a);
        const auto actual_polynomial = charpoly_int(core.matrix);
        const auto expected_polynomial = predicted_charpoly(a);
        const auto canonical_polynomial =
            charpoly_int(canonical_transition(a));
        bool quotient_match = true;
        if (a >= 2) {
            const auto [symmetric, antisymmetric] =
                swap_quotients(canonical_transition(a));
            const long long aa = static_cast<long long>(a);
            quotient_match =
                charpoly_int(symmetric)
                    == std::vector<long long>(
                        {1, -(aa + 1), aa, -1, 0})
                && charpoly_int(antisymmetric)
                    == std::vector<long long>(
                        {1, aa + 1, aa, -1, 0});
        }
        const bool state_match = core.keys == expected_keys;
        const bool polynomial_match =
            actual_polynomial == expected_polynomial
            && canonical_polynomial == expected_polynomial
            && quotient_match;
        if (!state_match || !polynomial_match) {
            std::printf(
                "a=%zu FAILED: states %zu/%zu match=%d, "
                "charpoly match=%d\n",
                a, core.keys.size(), expected_keys.size(),
                state_match, polynomial_match);
            ok = false;
            break;
        }
    }
    std::printf(
        "sigma_{a,1}, 1<=a<=%zu: explicit recurrent word states and "
        "closed charpoly family %s\n",
        max_a, ok ? "MATCH EXACTLY" : "FAILED");
    for (std::size_t a : {std::size_t{2}, std::size_t{3}}) {
        std::printf("canonical transition a=%zu:\n", a);
        for (const auto& row : canonical_transition(a)) {
            std::printf("  ");
            for (long long value : row) std::printf("%lld ", value);
            std::printf("\n");
        }
    }
    for (std::size_t a : {
             std::size_t{2}, std::size_t{3}, std::size_t{4}}) {
        const auto [symmetric, antisymmetric] =
            swap_quotients(canonical_transition(a));
        std::printf("a=%zu quotient charpolys: +[", a);
        const auto plus = charpoly_int(symmetric);
        const auto minus = charpoly_int(antisymmetric);
        for (std::size_t i = 0; i < plus.size(); ++i) {
            std::printf("%s%lld", i ? "," : "", plus[i]);
        }
        std::printf("] -[");
        for (std::size_t i = 0; i < minus.size(); ++i) {
            std::printf("%s%lld", i ? "," : "", minus[i]);
        }
        std::printf("]\n");
    }
    for (std::size_t a : {std::size_t{2}, std::size_t{3}}) {
        const auto bp =
            balanced_pair_transition_graph(SubstitutionRule(sigma_a1(a)));
        const auto recurrent = actual_core(a).keys;
        std::printf("a=%zu transient noncoincidence pairs:", a);
        for (const auto& state : bp.states) {
            const auto key = pair_key(state.left, state.right);
            if (recurrent.count(key)) continue;
            std::printf(" (");
            for (auto x : state.left) std::printf("%d", static_cast<int>(x));
            std::printf("|");
            for (auto x : state.right) std::printf("%d", static_cast<int>(x));
            std::printf(")");
        }
        std::printf("\n");
        const SubstitutionRule rule(sigma_a1(a));
        const auto image = sigma_pair(rule, Pair{{0, 1}, {1, 0}});
        std::printf("a=%zu initial reduced image:", a);
        for (const auto& child : reduce_pair(image.first, image.second, 3)) {
            std::printf(" (");
            for (auto x : child.first) std::printf("%d", static_cast<int>(x));
            std::printf("|");
            for (auto x : child.second) std::printf("%d", static_cast<int>(x));
            std::printf(")");
        }
        std::printf("\n");
    }
    return ok ? 0 : 1;
}
