#pragma once
#include <cstdint>
#include <vector>
namespace ravel {
inline std::vector<std::vector<std::int8_t>> plastic_number_rule(){ return {{1},{2},{0,1}}; }
inline std::vector<std::vector<long long>> plastic_number_incidence_matrix(){ return {{0,0,1},{1,0,1},{0,1,0}}; }
inline std::vector<long long> plastic_number_characteristic_coefficients(){ return {-1,-1,0,1}; }
}
