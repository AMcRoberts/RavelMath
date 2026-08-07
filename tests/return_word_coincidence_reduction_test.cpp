// Locks in Finding 40: for sigma_{0,1} (this project's own hardest
// documented coincidence case -- non-AR, worst-case depth 13), the
// return-word-induced substitution (built via this project's
// pre-existing, literature-validated return_substitution.hpp, applied
// to sigma^3 since no single letter of sigma_{0,1} generates its own
// fixed point directly) resolves EVERY pair within depth 2 -- a
// dramatic reduction, verified exactly against a from-scratch
// coincidence search on both substitutions.

#include <cassert>
#include <iostream>
#include <vector>

#include "ravel/return_substitution.hpp"
#include "ravel/substitution.hpp"

using namespace ravel;

namespace {

using Word = std::vector<long long>;

long long worst_case_depth(const std::vector<Word>& sigma, long long max_depth) {
    std::size_t n = sigma.size();
    long long worst = 0;
    for (std::size_t i = 0; i < n; ++i) {
        for (std::size_t j = i + 1; j < n; ++j) {
            Word w1 = {static_cast<long long>(i)}, w2 = {static_cast<long long>(j)};
            long long depth = -1;
            for (long long k = 1; k <= max_depth; ++k) {
                Word nw1, nw2;
                for (auto l : w1) nw1.insert(nw1.end(), sigma[static_cast<std::size_t>(l)].begin(), sigma[static_cast<std::size_t>(l)].end());
                for (auto l : w2) nw2.insert(nw2.end(), sigma[static_cast<std::size_t>(l)].begin(), sigma[static_cast<std::size_t>(l)].end());
                w1 = nw1; w2 = nw2;
                Word total1(n, 0), total2(n, 0);
                for (auto l : w1) total1[static_cast<std::size_t>(l)]++;
                for (auto l : w2) total2[static_cast<std::size_t>(l)]++;
                std::vector<std::vector<Word>> pfx1(n), sfx1(n);
                Word running(n, 0);
                for (std::size_t p = 0; p < w1.size(); ++p) {
                    long long a = w1[p];
                    pfx1[static_cast<std::size_t>(a)].push_back(running);
                    Word suf(n);
                    for (std::size_t kk = 0; kk < n; ++kk) suf[kk] = total1[kk] - running[kk] - (static_cast<long long>(kk) == a ? 1 : 0);
                    sfx1[static_cast<std::size_t>(a)].push_back(suf);
                    running[static_cast<std::size_t>(a)]++;
                }
                running.assign(n, 0);
                bool hit = false;
                for (std::size_t p = 0; p < w2.size() && !hit; ++p) {
                    long long a = w2[p];
                    for (auto& pv : pfx1[static_cast<std::size_t>(a)]) if (pv == running) { hit = true; break; }
                    if (!hit) {
                        Word suf(n);
                        for (std::size_t kk = 0; kk < n; ++kk) suf[kk] = total2[kk] - running[kk] - (static_cast<long long>(kk) == a ? 1 : 0);
                        for (auto& sv : sfx1[static_cast<std::size_t>(a)]) if (sv == suf) { hit = true; break; }
                    }
                    running[static_cast<std::size_t>(a)]++;
                }
                if (hit) { depth = k; break; }
            }
            assert(depth > 0);
            if (depth > worst) worst = depth;
        }
    }
    return worst;
}

}  // namespace

int main() {
    // Original sigma_{0,1}: worst-case depth 13 (established earlier
    // this session).
    std::vector<Word> sigma01 = {{1, 2}, {2}, {0}};
    long long original_depth = worst_case_depth(sigma01, 20);
    std::cout << "sigma_{0,1} original worst-case depth: " << original_depth << "\n";
    assert(original_depth == 13);

    // sigma^3 (needed since no letter of sigma_{0,1} generates its own
    // fixed point directly: sigma(0)=[1,2], sigma(1)=[2], sigma(2)=[0]
    // -- none starts with its own index; sigma^3(0)=[0,1,2] does).
    SubstitutionRule sigma3({{0, 1, 2}, {1, 2}, {2, 0}});
    auto rs = build_return_substitution(sigma3, 0);
    assert(rs.words.size() == 5);

    std::vector<Word> induced(rs.derived_images.size());
    for (std::size_t i = 0; i < rs.derived_images.size(); ++i)
        for (auto c : rs.derived_images[i]) induced[i].push_back(static_cast<long long>(c));

    long long induced_depth = worst_case_depth(induced, 20);
    std::cout << "return-word-induced substitution (5 letters) worst-case depth: " << induced_depth << "\n";
    assert(induced_depth == 2);

    std::cout << "return_word_coincidence_reduction: 13 -> 2, confirmed exactly.\n";
    return 0;
}
