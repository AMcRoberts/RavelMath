// Parameter-uniform prefix-role grammar for sigma_{a,1}.
//
// This is the word-level part of the Class-II tail argument.  It enumerates
// occurrences in the three substitution images and checks the exact parent /
// prefix-length channels seen by the Property-(F) backward graph.
#pragma once

#include <cstddef>
#include <map>
#include <vector>

namespace adelic {

struct PropertyFClassIIPrefixRoleGrammar {
    std::size_t parameter_a = 0;
    std::size_t zero_prefix_count = 0;
    std::size_t zero_parent_channels = 0;
    std::size_t one_parent_channels = 0;
    std::size_t two_parent_channels = 0;
    std::map<long long, std::vector<std::size_t>> zero_parents;
    std::map<long long, std::vector<std::size_t>> one_parents;
    std::map<long long, std::vector<std::size_t>> two_parents;
    bool zero_family_exact = false;
    bool one_forced_exact = false;
    bool two_family_exact = false;
    bool valid = false;
};

inline std::map<long long, std::vector<std::size_t>>
property_f_class_ii_prefix_roles_for_target(
        std::size_t a, long long target) {
    std::map<long long, std::vector<std::size_t>> roles;
    const std::vector<std::vector<long long>> images{
        [&] {
            std::vector<long long> image(a, 0);
            image.push_back(1);
            image.push_back(2);
            return image;
        }(),
        [&] {
            std::vector<long long> image(a, 0);
            image.push_back(2);
            return image;
        }(),
        {0}};
    for (std::size_t parent = 0; parent < images.size(); ++parent)
        for (std::size_t position = 0; position < images[parent].size(); ++position)
            if (images[parent][position] == target)
                roles[static_cast<long long>(parent)].push_back(position);
    return roles;
}

inline PropertyFClassIIPrefixRoleGrammar
derive_property_f_class_ii_prefix_role_grammar(std::size_t a) {
    PropertyFClassIIPrefixRoleGrammar out;
    out.parameter_a = a;
    out.zero_prefix_count = a;
    out.zero_parents =
        property_f_class_ii_prefix_roles_for_target(a, 0);
    out.one_parents =
        property_f_class_ii_prefix_roles_for_target(a, 1);
    out.two_parents =
        property_f_class_ii_prefix_roles_for_target(a, 2);
    out.zero_parent_channels = out.zero_parents.size();
    out.one_parent_channels = out.one_parents.size();
    out.two_parent_channels = out.two_parents.size();

    bool zero = out.zero_parents.size() == 3;
    for (long long parent = 0; parent <= 1; ++parent) {
        const auto it = out.zero_parents.find(parent);
        zero &= it != out.zero_parents.end();
        if (it == out.zero_parents.end()) continue;
        std::vector<std::size_t> expected;
        for (std::size_t j = 0; j < a; ++j) expected.push_back(j);
        zero &= it->second == expected;
    }
    zero &= out.zero_parents.at(2) == std::vector<std::size_t>{0};
    out.zero_family_exact = zero;

    out.one_forced_exact = out.one_parents.size() == 1 &&
        out.one_parents.at(0) == std::vector<std::size_t>{a};

    out.two_family_exact = out.two_parents.size() == 2 &&
        out.two_parents.at(0) == std::vector<std::size_t>{a + 1} &&
        out.two_parents.at(1) == std::vector<std::size_t>{a};
    out.valid = out.zero_family_exact && out.one_forced_exact &&
                out.two_family_exact;
    return out;
}

}  // namespace adelic
