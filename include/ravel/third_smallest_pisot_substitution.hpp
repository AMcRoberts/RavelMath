#pragma once
#include <cstdint>
#include <vector>
namespace ravel {
inline std::vector<std::vector<std::int8_t>> third_smallest_pisot_beta_rule(){
 return {{0,1},{2},{3},{0,4},{5},{6},{0}};
}
inline std::vector<long long> third_smallest_pisot_minpoly_coefficients(){return {-1,0,1,-1,-1,1};}
inline std::vector<long long> third_smallest_pisot_parry_polynomial_coefficients(){return {-1,0,0,-1,0,0,-1,1};}
}
