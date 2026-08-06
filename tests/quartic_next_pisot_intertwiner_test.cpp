#include <cassert>
#include <iostream>
#include "ravel/proof/quartic_next_pisot_intertwiner.hpp"
int main(){auto c=ravel::proof::derive_quartic_next_pisot_intertwiner();assert(c.proved);std::cout<<"quartic next Pisot intertwiner PASS states="<<c.boundary_states<<" boundary_edges="<<c.boundary_edges<<" universal_edges="<<c.universal_edges<<"\n";}
