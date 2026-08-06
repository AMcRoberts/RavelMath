#include <iostream>
#include <stdexcept>
#include "ravel/proof/plastic_three_generator_grammar.hpp"
int main(){auto c=ravel::proof::derive_plastic_three_generator_grammar();if(!c.exactly_three_classes)throw std::runtime_error("not three classes");if(!c.signed_classes_globally_inequivalent)throw std::runtime_error("signed classes collapsed");if(c.involution_wrong_class)throw std::runtime_error("involution class flip failed");std::cout<<"plastic three-generator grammar PASS\n"<<"states="<<c.boundary_states<<" edges="<<c.edges<<" G0="<<c.neutral_edges<<" G+="<<c.positive_edges<<" G-="<<c.negative_edges<<" paired="<<c.involution_paired<<" unpaired="<<c.involution_unpaired<<"\n";}
