// Empirical construction of the collared return groupoid.
// Unlike the ordinary derived substitution, this uses the actual image of a
// long fixed-point prefix.  A collar records the letters immediately before
// and after a return interval; the probe reports the smallest collar radius
// for which phase transitions become single-valued.

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <functional>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "ravel/return_substitution.hpp"
#include "ravel/simplest_nonunit_pisot_substitution.hpp"
#include "ravel/substitution.hpp"

using Word = std::vector<std::int8_t>;
using Interval = std::pair<std::size_t, std::size_t>;

namespace {

Word orbit_prefix(const ravel::SubstitutionRule& rule, std::size_t minimum) {
    Word w{0};
    for (std::size_t round = 0; round < 64 && w.size() < minimum; ++round)
        w = rule.apply_once(w);
    return w;
}

std::string print_word(const Word& w) {
    std::string out;
    for (auto c : w) out.push_back(static_cast<char>('0' + c));
    return out;
}

std::vector<Interval> marker_intervals(const Word& w, std::int8_t marker) {
    std::vector<std::size_t> p;
    for (std::size_t i = 0; i < w.size(); ++i)
        if (w[i] == marker) p.push_back(i);
    std::vector<Interval> out;
    for (std::size_t i = 0; i + 1 < p.size(); ++i)
        out.push_back({p[i], p[i + 1]});
    return out;
}

struct PairHash {
    std::size_t operator()(const std::pair<std::size_t, std::size_t>& p) const {
        return p.first * 1000003u + p.second;
    }
};

std::vector<std::size_t> scc_sizes(
        const std::map<std::string, std::set<std::string>>& graph) {
    std::map<std::string, int> index, low;
    std::set<std::string> active;
    std::vector<std::string> stack;
    std::vector<std::size_t> sizes;
    int next = 0;
    std::function<void(const std::string&)> visit = [&](const std::string& v) {
        index[v] = low[v] = next++;
        stack.push_back(v);
        active.insert(v);
        auto it = graph.find(v);
        if (it != graph.end()) for (const auto& w : it->second) {
            if (!index.count(w)) {
                visit(w);
                low[v] = std::min(low[v], low[w]);
            } else if (active.count(w)) {
                low[v] = std::min(low[v], index[w]);
            }
        }
        if (low[v] != index[v]) return;
        std::size_t size = 0;
        while (true) {
            const std::string w = stack.back();
            stack.pop_back();
            active.erase(w);
            ++size;
            if (w == v) break;
        }
        sizes.push_back(size);
    };
    for (const auto& [v, edges] : graph)
        if (!index.count(v)) visit(v);
    return sizes;
}

ravel::SubstitutionRule power_rule(const ravel::SubstitutionRule& rule,
                                   std::size_t power) {
    std::vector<std::vector<std::int8_t>> images(rule.alphabet_size());
    for (std::size_t i = 0; i < images.size(); ++i) images[i] = {static_cast<std::int8_t>(i)};
    for (std::size_t k = 0; k < power; ++k) {
        for (std::size_t i = 0; i < images.size(); ++i)
            images[i] = rule.apply_once(images[i]);
    }
    return ravel::SubstitutionRule(images);
}

void report_marker_power(const char* name,
                         const ravel::SubstitutionRule& rule) {
    try {
        const auto induced = ravel::build_return_substitution(
            power_rule(rule, 3), 0, 1 << 18);
        const auto phase = ravel::build_return_phase_system(
            power_rule(rule, 3), 0, 1 << 18);
        std::map<std::string, std::set<std::string>> graph;
        for (std::size_t i = 0; i < phase.phase_images.size(); ++i) {
            const std::string source = std::to_string(i);
            for (std::size_t j : phase.phase_images[i])
                graph[source].insert(std::to_string(j));
        }
        for (const auto& [source, edges] : graph)
            for (const auto& edge : edges) graph.try_emplace(edge);
        const auto components = scc_sizes(graph);
        std::size_t largest = 0;
        for (std::size_t size : components) largest = std::max(largest, size);
        std::printf("  marker_power=3 return_words=%zu phase_states=%zu\n",
                    induced.words.size(), phase.states.size());
        std::printf("    return_phase_SCCs=%zu largest=%zu\n",
                    components.size(), largest);
    } catch (const std::exception& e) {
        std::printf("  marker_power=3 rejected: %s\n", e.what());
    }
}

void report(const char* name, const ravel::SubstitutionRule& rule,
            std::size_t max_collar = 2) {
    const Word source = orbit_prefix(rule, 200000);
    const Word image = rule.apply_once(source);
    const auto source_intervals = marker_intervals(source, 0);
    const auto image_intervals = marker_intervals(image, 0);
    std::map<Word, std::size_t> word_index;
    for (const auto& [a, b] : source_intervals)
        word_index.emplace(Word(source.begin() + static_cast<std::ptrdiff_t>(a),
                                source.begin() + static_cast<std::ptrdiff_t>(b)),
                           word_index.size());
    std::map<std::size_t, std::pair<std::size_t, std::size_t>> image_phase;
    std::vector<std::size_t> image_start(source.size() + 1, 0);
    for (std::size_t i = 0; i < source.size(); ++i)
        image_start[i + 1] = image_start[i] +
            rule.image(static_cast<std::size_t>(source[i])).size();
    std::printf("%s: orbit=%zu return_words=%zu\n", name, source.size(),
                word_index.size());
    for (std::size_t collar = 0; collar <= max_collar; ++collar) {
        std::map<std::string, std::set<std::string>> signatures;
        std::map<std::string, std::set<std::string>> graph;
        std::map<std::size_t, std::string> image_phase;
        for (const auto& [ia, ib] : image_intervals) {
            Word iw(image.begin() + static_cast<std::ptrdiff_t>(ia),
                    image.begin() + static_cast<std::ptrdiff_t>(ib));
            auto it = word_index.find(iw);
            if (it == word_index.end()) continue;
            const std::string left(
                image.begin() + static_cast<std::ptrdiff_t>(
                    ia - std::min(collar, ia)),
                image.begin() + static_cast<std::ptrdiff_t>(ia));
            const std::string right(
                image.begin() + static_cast<std::ptrdiff_t>(ib),
                image.begin() + static_cast<std::ptrdiff_t>(
                    std::min(image.size(), ib + collar)));
            for (std::size_t off = 0; ia + off < ib; ++off)
                image_phase[ia + off] = std::to_string(it->second) + ":" +
                    std::to_string(off) + ":" + left + ":" + right;
        }
        // Ignore a collar-dependent boundary margin: both source and image
        // contexts must be complete before a graph edge is trusted.
        const std::size_t margin = std::max<std::size_t>(4, collar + 2);
        for (std::size_t k = margin;
             k + margin < source_intervals.size(); ++k) {
            const auto [a, b] = source_intervals[k];
            Word rw(source.begin() + static_cast<std::ptrdiff_t>(a),
                    source.begin() + static_cast<std::ptrdiff_t>(b));
            const std::size_t rid = word_index.at(rw);
            const std::string left(
                source.begin() + static_cast<std::ptrdiff_t>(a - std::min(collar, a)),
                source.begin() + static_cast<std::ptrdiff_t>(a));
            const std::string right(
                source.begin() + static_cast<std::ptrdiff_t>(b),
                source.begin() + static_cast<std::ptrdiff_t>(
                    std::min(source.size(), b + collar)));
            for (std::size_t off = 0; off < rw.size(); ++off) {
                const std::size_t pos = a + off;
                const std::size_t begin = image_start[pos];
                const std::size_t end = image_start[pos + 1];
                std::string signature;
                bool complete = true;
                for (std::size_t q = begin; q < end; ++q) {
                    auto it = image_phase.find(q);
                    if (it == image_phase.end()) { complete = false; break; }
                    signature += it->second + ";";
                }
                if (!complete) continue;
                const std::string key = std::to_string(rid) + ":" +
                    std::to_string(off) + ":" + left + ":" + right;
                signatures[key].insert(signature);
                for (std::size_t q = begin; q < end; ++q)
                    graph[key].insert(image_phase.at(q));
            }
        }
        std::size_t conflicts = 0;
        for (const auto& [key, values] : signatures)
            if (values.size() > 1) ++conflicts;
        for (const auto& [key, edges] : graph)
            for (const auto& edge : edges) graph.try_emplace(edge);
        const auto components = scc_sizes(graph);
        std::size_t recurrent = 0, largest = 0;
        for (std::size_t size : components) {
            if (size > 1) { ++recurrent; largest = std::max(largest, size); }
        }
        std::printf("  collar=%zu states=%zu conflicts=%zu%s\n", collar,
                    signatures.size(), conflicts,
                    conflicts == 0 ? " (single-valued)" : "");
        if (conflicts == 0)
            std::printf("    SCCs=%zu recurrent_nontrivial=%zu largest=%zu\n",
                        components.size(), recurrent, largest);
    }
}

}  // namespace

int main() {
    for (int b = 1; b <= 5; ++b) {
        std::vector<std::vector<std::int8_t>> sigma(3);
        for (int i = 0; i < b; ++i) sigma[0].push_back(1);
        sigma[0].push_back(2);
        sigma[1] = {2};
        sigma[2] = {0};
        const std::string name = "sigma_{0," + std::to_string(b) + "}";
        const auto rule = ravel::SubstitutionRule(sigma);
        report(name.c_str(), rule, static_cast<std::size_t>(2 * b + 5));
        report_marker_power(name.c_str(), rule);
    }
    for (int a = 1; a <= 5; ++a) {
        std::vector<std::vector<std::int8_t>> sigma(3);
        for (int i = 0; i < a; ++i) sigma[0].push_back(0);
        sigma[0].push_back(1);
        sigma[0].push_back(2);
        for (int i = 0; i < a; ++i) sigma[1].push_back(0);
        sigma[1].push_back(2);
        sigma[2] = {0};
        const std::string name = "sigma_{" + std::to_string(a) + ",1}";
        report(name.c_str(), ravel::SubstitutionRule(sigma));
    }
    report("nonunit_q2", ravel::SubstitutionRule(
        ravel::simplest_nonunit_pisot_rule()));
    return 0;
}
