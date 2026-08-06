#include <cassert>
#include <cmath>
#include <complex>
#include <iostream>
#include "ravel/shift_branch_substitution.hpp"
int main(){
  const auto r=ravel::quartic_next_pisot_rule();
  assert((r==std::vector<std::vector<std::int8_t>>{{1},{2},{3},{0,3}}));
  const auto c=ravel::quartic_next_pisot_characteristic_coefficients();
  assert((c==std::vector<long long>{-1,0,0,-1,1}));
  // Published/independently computed quartic Pisot root bracket.
  auto f=[](long double x){return x*x*x*x-x*x*x-1;};
  assert(f(1.38027L)<0 && f(1.38028L)>0);
  std::cout<<"quartic next Pisot candidate PASS\n";
}
