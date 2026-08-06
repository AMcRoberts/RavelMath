#include <iostream>
#include <stdexcept>
#include "ravel/plastic_number_substitution.hpp"
int main(){
  const auto M=ravel::plastic_number_incidence_matrix();
  const long long det = M[0][0]*(M[1][1]*M[2][2]-M[1][2]*M[2][1])
                      - M[0][1]*(M[1][0]*M[2][2]-M[1][2]*M[2][0])
                      + M[0][2]*(M[1][0]*M[2][1]-M[1][1]*M[2][0]);
  if(det!=1) throw std::runtime_error("plastic matrix is not unimodular");
  const auto p=ravel::plastic_number_characteristic_coefficients();
  if(p!=std::vector<long long>({-1,-1,0,1})) throw std::runtime_error("x^3-x-1 mismatch");
  const auto s=ravel::plastic_number_rule();
  if(s!=std::vector<std::vector<std::int8_t>>({{1},{2},{0,1}})) throw std::runtime_error("substitution mismatch");
  std::cout<<"plastic number candidate PASS\n";
}
