#include <cassert>
#include <iostream>
#include "ravel/proof/shell_two_interior_factorization.hpp"
using namespace ravel::proof;
int main(){
  std::size_t total=0;
  for(std::size_t n=2;n<=7;++n){
    const auto c=derive_shell_two_interior_factorization(n);
    assert(c.valid);
    total += c.factored_first_return.size();
    std::cout << "n="<<n<<" interior="<<c.interior_states.size()
              <<" boundary="<<c.factored_first_return.size()
              <<" rank="<<([&]{std::int64_t m=0; for(const auto& kv:c.rank.rank)m=std::max(m,kv.second); return m;}())<<"\n";
  }
  std::cout << "shell two interior factorization PASS total="<<total<<"\n";
}
