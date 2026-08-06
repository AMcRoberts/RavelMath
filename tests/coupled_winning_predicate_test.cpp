#include <cassert>
#include <iostream>
#include "ravel/proof/coupled_winning_predicate.hpp"
int main(){using namespace ravel::proof; std::vector<PredicateTreeNode> t={{0,0,1,2,false,false},{-1,0,-1,-1,true,true},{-1,0,-1,-1,true,false}}; assert(evaluate_predicate_tree(t,{-1})); assert(!evaluate_predicate_tree(t,{1})); std::cout<<"coupled winning predicate PASS\n";}
