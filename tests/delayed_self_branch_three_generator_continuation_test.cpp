#include <cassert>
#include <iostream>
#include "ravel/proof/delayed_self_branch_three_generator_continuation.hpp"
int main(){
 auto c=ravel::proof::derive_delayed_self_branch_three_generator_continuation(127);
 assert(c.proved);
 std::cout<<"delayed self-branch three-generator continuation PASS through D="<<c.checked_through_D<<"\n";
}
