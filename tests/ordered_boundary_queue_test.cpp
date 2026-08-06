#include <cstdint>
#include <iostream>
#include <vector>
#include "ravel/proof/ordered_boundary_queue.hpp"
using namespace ravel::proof;
int main(){std::size_t checks=0;for(std::size_t n=3;n<=10;++n){ShellState x(n);std::function<bool(std::size_t)> rec=[&](std::size_t i){if(i==n){for(auto a:{-1LL,0LL,1LL})for(std::size_t d=1;d<=n;++d){auto u=derive_boundary_queue_update(x,a,d);if(!u.exact)return false;std::vector<std::int64_t> lw(d),rw(d);for(std::size_t k=0;k<d;++k){lw[k]=std::int64_t(2*k+1);rw[k]=std::int64_t(3*k-2);}auto y=ordered_boundary_queue(shift_append(x,a),d);auto actual=weighted_boundary_value(y,lw,rw)-weighted_boundary_value(u.before,lw,rw);auto formula=weighted_boundary_delta_formula(u.before,a,u.left_entering,lw,rw);if(actual!=formula)return false;++checks;}return true;}for(auto v:{-1LL,0LL,1LL}){x[i]=v;if(!rec(i+1))return false;}return true;};if(!rec(0)){std::cerr<<"FAIL n="<<n<<"\n";return 2;}}std::cout<<"ordered boundary queue PASS checks="<<checks<<"\n";}
