// Small structural probe for the non-AR seam hypotheses.
// It reports the induced return substitution and its phase lift, so that
// finite seam complexity is separated from later contact/valuation fibres.

#include <cstdio>
#include <set>
#include <string>
#include <vector>

#include "ravel/return_substitution.hpp"
#include "ravel/simplest_nonunit_pisot_substitution.hpp"

using ravel::ReturnPhaseSystem;
using ravel::SubstitutionRule;

namespace {

std::string word(const std::vector<std::int8_t>& w) {
    std::string out;
    for (auto c : w) out.push_back(static_cast<char>('0' + c));
    return out;
}

std::vector<std::size_t> phase_scc_sizes(const ReturnPhaseSystem& s) {
    const std::size_t n = s.states.size();
    std::vector<int> index(n, -1), low(n, 0), stack;
    std::vector<bool> on_stack(n, false);
    std::vector<std::size_t> sizes;
    int next = 0;
    auto visit = [&](auto&& self, std::size_t v) -> void {
        index[v] = low[v] = next++;
        stack.push_back(static_cast<int>(v));
        on_stack[v] = true;
        for (std::size_t w : s.phase_images[v]) {
            if (index[w] < 0) {
                self(self, w);
                low[v] = std::min(low[v], low[w]);
            } else if (on_stack[w]) {
                low[v] = std::min(low[v], index[w]);
            }
        }
        if (low[v] != index[v]) return;
        std::size_t size = 0;
        for (;;) {
            const std::size_t w = static_cast<std::size_t>(stack.back());
            stack.pop_back();
            on_stack[w] = false;
            ++size;
            if (w == v) break;
        }
        sizes.push_back(size);
    };
    for (std::size_t v = 0; v < n; ++v)
        if (index[v] < 0) visit(visit, v);
    return sizes;
}

void report(const char* name, const SubstitutionRule& rule) {
    ReturnPhaseSystem phase;
    try {
        phase = ravel::build_return_phase_system(rule, 0);
    } catch (const std::exception& e) {
        std::printf("%s: return-phase construction rejected: %s\n", name,
                    e.what());
        return;
    }
    std::printf("%s: return_words=%zu phase_states=%zu\n", name,
                phase.induced.words.size(), phase.states.size());
    for (std::size_t i = 0; i < phase.induced.words.size(); ++i) {
        std::printf("  r%zu=%s ->", i, word(phase.induced.words[i]).c_str());
        for (std::size_t j : phase.induced.derived_images[i])
            std::printf(" r%zu", j);
        std::printf("\n");
    }
    const auto sccs = phase_scc_sizes(phase);
    std::printf("  phase_sccs=%zu sizes=", sccs.size());
    for (std::size_t i = 0; i < sccs.size(); ++i)
        std::printf("%s%zu", i ? "," : "", sccs[i]);
    std::printf("\n");
}

}  // namespace

int main() {
    // sigma_{0,1}: 0->12, 1->2, 2->0.
    report("sigma_{0,1}", SubstitutionRule({{1, 2}, {2}, {0}}));
    report("nonunit_q2", SubstitutionRule(
        ravel::simplest_nonunit_pisot_rule()));
    return 0;
}
