#include <cassert>
#include <iostream>
#include "ravel/proof/joint_pair_rank_one_dominance.hpp"
using namespace ravel::proof;
int main(){
  const std::vector<std::vector<long long>> qc{{1,0},{0,1}};
  const std::vector<std::vector<long long>> rc{{0,1},{1,0}};
  const std::vector<std::vector<long long>> qk{{1,0,0},{0,1,0},{0,0,2}};
  const std::vector<std::vector<long long>> rk{{0,1,0},{1,0,0},{0,0,1}};
  auto p=derive_joint_pair_rank_one_dominance(qc,rc,qk,rk,80,8);
  assert(p.proved && p.q_intertwiner && p.r_intertwiner);
  auto bad=qc; bad[0][0]=5;
  assert(!derive_joint_pair_rank_one_dominance(bad,rc,qk,rk,40,4).proved);
  std::cout<<"joint pair rank-one dominance PASS alpha="<<p.alpha<<" beta="<<p.beta<<"\n";
}
