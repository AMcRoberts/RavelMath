#include <cassert>
#include <iostream>
#include "ravel/proof/triangular_wave_terminal_shell.hpp"
using namespace ravel::proof;
int main(){
  std::size_t checked=0;
  for(std::int64_t M=1;M<=8;++M){
    const auto P=static_cast<std::size_t>(4*M);
    for(std::size_t q=1;q<=4;++q){
      const auto n=q*P+1;
      const auto c=derive_triangular_wave_terminal_shell(n,M);
      assert(c.refutes_uniform_shell_exclusion);
      assert(c.period==P);
      assert(c.cycle_spectral_radius_one);
      assert(c.cycle.size()==P);
      ++checked;
    }
  }
  const auto n9=derive_triangular_wave_terminal_shell(9,2);
  assert(n9.refutes_uniform_shell_exclusion);
  assert(n9.period==8);
  const auto radii9=triangular_wave_terminal_radii(9);
  assert((radii9==std::vector<std::int64_t>{1,2}));
  const auto rejected=derive_triangular_wave_terminal_shell(8,2);
  assert(!rejected.refutes_uniform_shell_exclusion);
  std::cout << "triangular wave terminal shell PASS families="<<checked<<"\n";
}
