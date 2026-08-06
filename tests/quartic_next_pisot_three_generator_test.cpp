#include <cassert>
#include <iostream>
#include "ravel/proof/quartic_next_pisot_three_generator.hpp"
int main(){auto c=ravel::proof::derive_quartic_next_pisot_three_generator();assert(c.proved);std::cout<<"quartic next Pisot three-generator PASS states="<<c.boundary_states<<" edges="<<c.edges<<" classes="<<c.class_edges[0]<<","<<c.class_edges[1]<<","<<c.class_edges[2]<<"\n";}
