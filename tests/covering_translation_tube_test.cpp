#include <cassert>
#include <iostream>
#include "ravel/proof/covering_translation_tube.hpp"
int main(){using namespace ravel::proof; auto c=certify_covering_translation_tube("n3.example",2,1,{{1,-2,1},{-2,1,1}},{{-1,-1,0},{-1,0,0}}); assert(c.dimensions_match); assert(c.translation_box_checked); assert(c.translation_cover_checked); assert(c.transported_box_checked); assert(c.first_return_transport); std::cout<<"covering translation tube PASS\n";}
