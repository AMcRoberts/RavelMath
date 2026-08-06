#pragma once
#include <algorithm>
#include <cstddef>
#include <tuple>
#include <vector>
#include "ravel/proof/face_relative_sign_grammar.hpp"
namespace ravel::proof {
struct CappedSignRun { int sign=0; std::size_t length=0; auto operator<=>(const CappedSignRun&) const=default; };
struct ParametricFaceSignRole {
 std::size_t target_offset=0;
 std::size_t distance_to_target_from_end=0;
 std::size_t support=0;
 std::vector<CappedSignRun> runs;
 auto operator<=>(const ParametricFaceSignRole&) const=default;
};
inline std::size_t cap_positive(std::size_t x,std::size_t cap){return std::min(x,cap);}
inline ParametricFaceSignRole derive_parametric_face_sign_role(
 const std::vector<long long>& coordinates,std::size_t active_face,std::size_t target_face,
 std::size_t cap=4){
 auto g=derive_face_relative_sign_grammar(coordinates,active_face,target_face,true);
 ParametricFaceSignRole r;
 r.target_offset=cap_positive(g.target_face_offset,cap);
 const auto backward=(g.dimension+active_face-target_face)%g.dimension;
 r.distance_to_target_from_end=cap_positive(backward,cap);
 r.support=cap_positive(g.support,cap);
 for(auto x:g.runs)r.runs.push_back({x.sign,cap_positive(x.length,cap)});
 // Merge adjacent capped runs if canonicalization created equal neighbors.
 std::vector<CappedSignRun> merged;
 for(auto x:r.runs){if(!merged.empty()&&merged.back().sign==x.sign)
   merged.back().length=cap_positive(merged.back().length+x.length,cap);
  else merged.push_back(x);}
 r.runs=std::move(merged);return r;
}
}
