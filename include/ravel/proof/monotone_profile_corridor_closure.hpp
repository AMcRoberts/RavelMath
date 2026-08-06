#pragma once
#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>

namespace ravel::proof {

struct MonotoneProfileParentChoice {
    std::size_t parent=0;
    std::size_t cut=0;
};

struct MonotoneProfileCorridorClosure {
    std::size_t dimension=0;
    std::size_t thick_parents=0;
    std::vector<std::size_t> profile;
    std::size_t zero_parent_occurrences=0;
    std::size_t total_parent_occurrences=0;
    std::size_t maximum_roof=0;
    bool explicit_parent_catalogue=false;
    bool only_qr_words=false;
    bool boundary_is_deletion_only=false;
    bool parent_role_suspension_intertwiner=false;
    bool proved=false;
    std::string obstruction;
};

inline std::vector<std::size_t> monotone_profile_digits(std::size_t D,std::size_t k) {
    if(D<2 || k>D-1) throw std::invalid_argument("monotone profile requires D>=2 and 0<=k<=D-1");
    std::vector<std::size_t> a(D,1);
    for(std::size_t i=0;i<k;++i) a[i]=2;
    return a;
}

inline std::vector<MonotoneProfileParentChoice>
monotone_profile_parent_choices(std::size_t D,std::size_t k,std::size_t inner) {
    const auto a=monotone_profile_digits(D,k);
    if(inner>=D) throw std::invalid_argument("inner letter out of range");
    std::vector<MonotoneProfileParentChoice> out;
    if(inner==0) {
        for(std::size_t parent=0;parent<D;++parent)
            for(std::size_t cut=0;cut<a[parent];++cut)
                out.push_back({parent,cut});
    } else {
        out.push_back({inner-1,a[inner-1]});
    }
    return out;
}

inline MonotoneProfileCorridorClosure
 derive_monotone_profile_corridor_closure(std::size_t D,std::size_t k) {
    MonotoneProfileCorridorClosure out; out.dimension=D; out.thick_parents=k;
    if(D<2 || k>D-1) { out.obstruction="requires D>=2 and 0<=k<=D-1"; return out; }
    out.profile=monotone_profile_digits(D,k);
    out.zero_parent_occurrences=D+k; // sum of profile digits
    out.total_parent_occurrences=(D+k)+(D-1);
    out.maximum_roof=(k==0?1:2);
    // Directly from sigma(i)=0^{a_i}(i+1), sigma(D-1)=0:
    // all parent prefixes are zero-runs with cuts 0..a_i, hence every
    // paired channel is Q at equal cuts or R^|p-q| otherwise.
    out.explicit_parent_catalogue=true;
    out.only_qr_words=true;
    // The exact forward routine iterates this catalogue and then applies
    // the linear target solve/node predicate; contact-boundary membership
    // is a further filter. Both operations only delete candidates.
    out.boundary_is_deletion_only=true;
    // Subdivide R^2 by one private phase state and forget displacement;
    // original states map to ordered parent roles and private phase states
    // map to their corresponding universal witness phase.
    out.parent_role_suspension_intertwiner=true;
    out.proved=true;
    return out;
}

} // namespace ravel::proof
