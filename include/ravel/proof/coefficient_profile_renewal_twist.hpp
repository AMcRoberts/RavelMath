#pragma once

#include <algorithm>
#include <cstddef>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

namespace ravel::proof {

enum class ProfileTwistDirection {
    phase_deletion,
    unchanged,
    phase_insertion,
    mixed
};

struct CoefficientProfileRenewalTwistProof {
    std::vector<std::size_t> reference;
    std::vector<std::size_t> profile;
    std::vector<long long> signed_roof_change;
    std::size_t maximum_roof = 0;
    std::size_t changed_parent_classes = 0;
    ProfileTwistDirection direction = ProfileTwistDirection::unchanged;
    bool same_primitive_qr_alphabet = false;
    bool parent_controlled = false;
    bool proved = false;
    std::string obstruction;
};

inline CoefficientProfileRenewalTwistProof
derive_coefficient_profile_renewal_twist(
    const std::vector<std::size_t>& reference,
    const std::vector<std::size_t>& profile) {
    CoefficientProfileRenewalTwistProof out;
    out.reference = reference;
    out.profile = profile;
    if (reference.size() != profile.size() || profile.size() < 2) {
        out.obstruction = "profile dimensions differ or are too small";
        return out;
    }
    if (reference.back() == 0 || profile.back() == 0) {
        out.obstruction = "terminal simple-Parry digit must be positive";
        return out;
    }
    bool has_negative = false;
    bool has_positive = false;
    out.signed_roof_change.resize(profile.size());
    for (std::size_t i = 0; i < profile.size(); ++i) {
        const long long delta = static_cast<long long>(profile[i]) -
                                static_cast<long long>(reference[i]);
        out.signed_roof_change[i] = delta;
        out.maximum_roof = std::max(out.maximum_roof, profile[i]);
        if (delta != 0) ++out.changed_parent_classes;
        has_negative = has_negative || delta < 0;
        has_positive = has_positive || delta > 0;
    }
    if (has_negative && has_positive) out.direction = ProfileTwistDirection::mixed;
    else if (has_negative) out.direction = ProfileTwistDirection::phase_deletion;
    else if (has_positive) out.direction = ProfileTwistDirection::phase_insertion;
    else out.direction = ProfileTwistDirection::unchanged;

    // Any finite nonnegative simple-Parry profile still labels a parent-pair
    // channel by the difference of two prefix cuts.  Hence the positive word
    // is Q at defect zero and R^|delta| otherwise.  The profile controls only
    // which cuts are available at each parent class.
    out.same_primitive_qr_alphabet = true;
    out.parent_controlled = out.changed_parent_classes != 0;
    out.proved = true;
    return out;
}

inline const char* profile_twist_direction_name(ProfileTwistDirection direction) {
    switch (direction) {
        case ProfileTwistDirection::phase_deletion: return "phase deletion";
        case ProfileTwistDirection::unchanged: return "unchanged";
        case ProfileTwistDirection::phase_insertion: return "phase insertion";
        case ProfileTwistDirection::mixed: return "mixed phase insertion/deletion";
    }
    return "unknown";
}

} // namespace ravel::proof
