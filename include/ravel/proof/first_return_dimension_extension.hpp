#pragma once

#include <algorithm>
#include <cstddef>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

#include "ravel/proof/first_return_joint_role_quotient.hpp"

namespace ravel::proof {

struct DimensionExtensionProof {
    std::size_t lower_dimension = 0;
    std::size_t upper_dimension = 0;
    std::size_t lower_classes = 0;
    std::size_t upper_classes = 0;
    std::size_t transported_classes = 0;
    std::size_t new_boundary_classes = 0;
    bool old_role_transport_total = false;
    bool transition_transport = false;
    bool finite_new_boundary_schema = false;
    bool proved = false;
    std::string obstruction;
};

namespace detail {

inline std::vector<std::int64_t> extension_key(
    const JointFirstReturnState& state, std::size_t dimension) {
    auto profile = role_profile(state, dimension);
    std::vector<std::int64_t> key;
    key.reserve(profile.normalized_plant.size() + 5);
    for (const auto x : profile.normalized_plant) key.push_back(x);
    key.push_back(static_cast<std::int64_t>(profile.normalized_faces.size()));
    key.push_back(static_cast<std::int64_t>(profile.remaining_phase));
    return key;
}

} // namespace detail

inline DimensionExtensionProof synthesize_dimension_extension(
    const FirstReturnJointProduct& lower_product,
    const JointRoleQuotientProof& lower,
    const FirstReturnJointProduct& upper_product,
    const JointRoleQuotientProof& upper) {
    DimensionExtensionProof p;
    p.lower_dimension = lower.dimension;
    p.upper_dimension = upper.dimension;
    p.lower_classes = lower.refined_classes;
    p.upper_classes = upper.refined_classes;
    if (upper.dimension != lower.dimension + 1) {
        p.obstruction = "dimension extension requires adjacent dimensions";
        return p;
    }
    if (!lower.exact_on_reachable_product || !upper.exact_on_reachable_product) {
        p.obstruction = "joint quotient is not exact in both dimensions";
        return p;
    }

    // This stage deliberately checks a concrete shadow candidate rather than
    // declaring it a theorem: an upper role transports a lower role when
    // deleting the final normalized coordinate preserves the lower role data.
    std::map<std::vector<std::int64_t>, std::set<std::size_t>> lower_by_key;
    for (const auto& state : lower_product.reachable)
        lower_by_key[detail::extension_key(state, lower.dimension)]
            .insert(lower.class_of.at(state));

    std::set<std::size_t> transported_upper;
    std::set<std::size_t> boundary_upper;
    for (const auto& state : upper_product.reachable) {
        auto profile = detail::role_profile(state, upper.dimension);
        if (profile.normalized_plant.empty()) continue;
        profile.normalized_plant.pop_back();
        std::vector<std::int64_t> key;
        for (const auto x : profile.normalized_plant) key.push_back(x);
        key.push_back(static_cast<std::int64_t>(profile.normalized_faces.size()));
        key.push_back(static_cast<std::int64_t>(state.remaining % (lower.dimension + 1)));
        const auto q = upper.class_of.at(state);
        const auto it = lower_by_key.find(key);
        if (it != lower_by_key.end() && it->second.size() == 1)
            transported_upper.insert(q);
        else
            boundary_upper.insert(q);
    }
    p.transported_classes = transported_upper.size();
    p.new_boundary_classes = boundary_upper.size();
    p.old_role_transport_total = transported_upper.size() >= lower.refined_classes;

    // A genuine proof needs symbolic transition equations and a dimension-free
    // classification of boundary classes.  Report the exact remaining seam.
    p.transition_transport = false;
    p.finite_new_boundary_schema = false;
    p.proved = false;
    std::ostringstream reason;
    reason << "shadow candidate found " << p.transported_classes
           << " transported upper classes and " << p.new_boundary_classes
           << " boundary classes, but no symbolic n->n+1 transition theorem "
              "or dimension-free boundary schema has been derived";
    p.obstruction = reason.str();
    return p;
}

inline std::string render_dimension_extension_report(
    const DimensionExtensionProof& p) {
    std::ostringstream o;
    o << "FIRST_RETURN_DIMENSION_EXTENSION\n";
    o << "dimensions=" << p.lower_dimension << "->" << p.upper_dimension << "\n";
    o << "lower_classes=" << p.lower_classes << "\n";
    o << "upper_classes=" << p.upper_classes << "\n";
    o << "transported_classes=" << p.transported_classes << "\n";
    o << "new_boundary_classes=" << p.new_boundary_classes << "\n";
    o << "old_role_transport_total=" << (p.old_role_transport_total ? "true" : "false") << "\n";
    o << "transition_transport=" << (p.transition_transport ? "true" : "false") << "\n";
    o << "finite_new_boundary_schema=" << (p.finite_new_boundary_schema ? "true" : "false") << "\n";
    o << "proved=" << (p.proved ? "true" : "false") << "\n";
    if (!p.obstruction.empty()) o << "obstruction=" << p.obstruction << "\n";
    return o.str();
}

} // namespace ravel::proof
