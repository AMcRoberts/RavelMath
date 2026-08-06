#include <cassert>
#include <iostream>
#include "ravel/proof/path_injective_simulation.hpp"
using namespace ravel::proof;
int main(){
  // Fibonacci branching source embeds into a target with one extra edge.
  MultiAdjacency source{{0,1},{0}};
  MultiAdjacency target{{0,1,1},{0,1}};
  auto c=derive_path_injective_simulation(source,target);
  assert(c.total && c.spectral_radius_nonstrict);
  // A binary source cannot inject into a deterministic target.
  MultiAdjacency deterministic{{0}};
  assert(!derive_path_injective_simulation(source,deterministic).total);

  // Seven-phase renewal A^7=2I path-injects into the complete 2-state core.
  MultiAdjacency theta(7);
  theta[0]={4}; theta[1]={0}; theta[2]={3}; theta[3]={5};
  theta[4]={6}; theta[5]={1}; theta[6]={2,2};
  MultiAdjacency core{{0,1},{0,1}};
  auto renewal=derive_path_injective_simulation(theta,core);
  assert(renewal.total && renewal.spectral_radius_nonstrict);

  auto graded=compose_graded_path_injections({c,renewal});
  assert(graded.spectral_radius_bounded_by_grade_one);
  std::cout<<"path injective simulation PASS\n";
}
