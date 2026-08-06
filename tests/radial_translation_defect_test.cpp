#include <cassert>
#include <iostream>
#include "ravel/proof/radial_translation_defect.hpp"
int main() {
  using namespace ravel::proof;
  IntegerMatrix B{{2,1},{0,1}};
  IntegerVector x{3,-2}, t{5,5}, f{1,-1};
  auto c=certify_translation_defect(B,x,t,f);
  assert(c.affine_transport_exact);
  assert(c.same_translation_defect == sub(mat_vec(B,t),t));
  std::cout << "radial translation defect machinery PASS\n";
}
