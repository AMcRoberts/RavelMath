// A finite, reusable "forest" of all parent-role transitions for a canonical
// beta substitution.  This is deliberately independent of any one expansion
// tree: every pair of parent occurrences is recorded once, with its prefix
// defect (the cocycle label) and its source/target role.
#pragma once

#include <cstddef>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

#include "math/qbeta.hpp"
#include "math/sturm.hpp"
#include "ravel/canonical_beta_substitution.hpp"

namespace ravel::proof {

struct CanonicalParentOccurrence {
    long long parent_letter{};
    std::vector<long long> prefix;
};

struct CanonicalParentRoleEdge {
    std::size_t source_role{};
    std::size_t target_role{};
    long long defect{};
    long long left_letter{};
    long long right_letter{};
    long long left_parent{};
    long long right_parent{};
    std::vector<long long> left_prefix;
    std::vector<long long> right_prefix;
};

struct CanonicalParentRoleCatalogue {
    std::vector<long long> digits;
    std::vector<std::vector<long long>> substitution;
    std::vector<std::vector<CanonicalParentOccurrence>> parents;
    std::vector<CanonicalParentRoleEdge> edges;
    std::map<long long, std::size_t> edge_count_by_defect;
    std::set<std::vector<long long>> prefixes;
    std::set<long long> defects;
    std::size_t alphabet_size{};
    std::size_t role_count{};
    long long max_prefix_length{};
    bool proved{};
    std::string obstruction;

    std::size_t role(long long left, long long right) const {
        if (left < 0 || right < 0 || static_cast<std::size_t>(left) >= alphabet_size ||
            static_cast<std::size_t>(right) >= alphabet_size)
            throw std::out_of_range("CanonicalParentRoleCatalogue::role");
        return static_cast<std::size_t>(left) * alphabet_size + static_cast<std::size_t>(right);
    }
};

inline CanonicalParentRoleCatalogue derive_canonical_parent_role_catalogue(
    const mathlib::QBetaRing& R, const mathlib::RootInterval& beta_I) {
    using namespace mathlib;
    CanonicalParentRoleCatalogue out;
    auto ge = exact_greedy_beta_expansion_of_one(R, beta_I, 128);
    if (!ge.terminated && ge.period_len == 0) {
        out.obstruction = "greedy expansion did not terminate or cycle";
        return out;
    }
    out.digits = ge.digits;
    if (ge.terminated || ge.purely_periodic) {
        out.substitution = canonical_beta_substitution_from_digits(out.digits);
    } else {
        std::vector<long long> pre(out.digits.begin(), out.digits.begin() + ge.preperiod_len);
        std::vector<long long> period(out.digits.begin() + ge.preperiod_len, out.digits.end());
        out.substitution = canonical_beta_substitution_eventually_periodic(pre, period);
    }
    out.alphabet_size = out.substitution.size();
    out.role_count = out.alphabet_size * out.alphabet_size;
    out.parents.resize(out.alphabet_size);
    for (std::size_t parent = 0; parent < out.substitution.size(); ++parent) {
        const auto& image = out.substitution[parent];
        for (std::size_t k = 0; k < image.size(); ++k) {
            const auto inner = static_cast<std::size_t>(image[k]);
            if (inner >= out.alphabet_size) {
                out.obstruction = "substitution image contains an out-of-range letter";
                return out;
            }
            std::vector<long long> prefix(image.begin(), image.begin() + static_cast<std::ptrdiff_t>(k));
            out.parents[inner].push_back({static_cast<long long>(parent), prefix});
            out.prefixes.insert(prefix);
            if (static_cast<long long>(prefix.size()) > out.max_prefix_length)
                out.max_prefix_length = static_cast<long long>(prefix.size());
        }
    }
    for (long long left = 0; left < static_cast<long long>(out.alphabet_size); ++left)
        for (long long right = 0; right < static_cast<long long>(out.alphabet_size); ++right)
            for (const auto& lp : out.parents[static_cast<std::size_t>(left)])
                for (const auto& rp : out.parents[static_cast<std::size_t>(right)]) {
                    const long long defect = static_cast<long long>(rp.prefix.size()) -
                                             static_cast<long long>(lp.prefix.size());
                    out.edges.push_back({out.role(left, right), out.role(lp.parent_letter, rp.parent_letter),
                                         defect, left, right, lp.parent_letter, rp.parent_letter,
                                         lp.prefix, rp.prefix});
                    out.defects.insert(defect);
                    ++out.edge_count_by_defect[defect];
                }
    out.proved = true;
    return out;
}

}  // namespace ravel::proof
