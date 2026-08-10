// Empirical construction of the collared return groupoid.
// Unlike the ordinary derived substitution, this uses the actual image of a
// long fixed-point prefix.  A collar records the letters immediately before
// and after a return interval; the probe reports the smallest collar radius
// for which phase transitions become single-valued.

#include <cstddef>
#include <cstdint>
#include <cstdio>
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
    for (const auto& [a, b] : image_intervals) {
        Word w(image.begin() + static_cast<std::ptrdiff_t>(a),
               image.begin() + static_cast<std::ptrdiff_t>(b));
        auto it = word_index.find(w);
        if (it == word_index.end()) continue;
        for (std::size_t off = 0; a + off < b; ++off)
            image_phase[a + off] = {it->second, off};
    }
    std::vector<std::size_t> image_start(source.size() + 1, 0);
    for (std::size_t i = 0; i < source.size(); ++i)
        image_start[i + 1] = image_start[i] +
            rule.image(static_cast<std::size_t>(source[i])).size();
    std::printf("%s: orbit=%zu return_words=%zu\n", name, source.size(),
                word_index.size());
    for (std::size_t collar = 0; collar <= max_collar; ++collar) {
        std::map<std::string, std::set<std::string>> transitions;
        // Ignore a few boundary intervals: their image context is incomplete.
        for (std::size_t k = 4; k + 4 < source_intervals.size(); ++k) {
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
                    signature += std::to_string(it->second.first) + ":" +
                                 std::to_string(it->second.second) + ";";
                }
                if (!complete) continue;
                const std::string key = std::to_string(rid) + ":" +
                    std::to_string(off) + ":" + left + ":" + right;
                transitions[key].insert(signature);
            }
        }
        std::size_t conflicts = 0;
        for (const auto& [key, signatures] : transitions)
            if (signatures.size() > 1) ++conflicts;
        std::printf("  collar=%zu states=%zu conflicts=%zu%s\n", collar,
                    transitions.size(), conflicts,
                    conflicts == 0 ? " (single-valued)" : "");
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
        report(name.c_str(), ravel::SubstitutionRule(sigma), b);
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
