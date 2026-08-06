#include <cstdint>
#include <deque>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>
#include "ravel/proof/block_height_shell_rank.hpp"
using namespace ravel::proof;

static std::vector<std::int64_t> feat(const ShellState& x) {
  auto f=derive_block_height_features(x);
  std::int64_t sum=0, signed_sum=0, maxv=0, minv=0;
  for(auto v:x){sum+=std::llabs(v); signed_sum+=v; maxv=std::max(maxv,v); minv=std::min(minv,v);}
  return {sum,f.forcing_support,f.forcing_zero_count,f.forcing_gcd,
          f.moment_variation,f.moment_energy,f.adjacent_energy,signed_sum,maxv,minv};
}
int main(int argc,char**argv){
 if(argc!=5){std::cerr<<"usage: n lo hi csv\n";return 2;}
 size_t n=std::stoul(argv[1]); long long lo=std::stoll(argv[2]),hi=std::stoll(argv[3]);
 std::ofstream out(argv[4]); out<<"d_level,d_bfsupport,d_bfzero,d_bfgcd,d_mvariation,d_menergy,d_adjenergy,d_sum,d_max,d_min\n";
 size_t edges=0;
 for(long long b=lo;b<=hi;++b){
  std::vector<ShellState> shell; std::unordered_set<ShellState,ShellStateHash> ss; ShellState st(n,0);
  enumerate_box_states_rec(n,b,0,st,[&](auto const&x){if(shell_radius(x)==b){shell.push_back(x);ss.insert(x);}});
  for(auto const&s:shell){std::deque<ShellState>q;std::unordered_set<ShellState,ShellStateHash>seen;q.push_back(s);seen.insert(s);auto fs=feat(s);
   while(!q.empty()){auto c=std::move(q.front());q.pop_front();for(auto t:bounded_carry_successors(c,b)){if(ss.contains(t)){auto ft=feat(t);for(size_t i=0;i<fs.size();++i){if(i)out<<',';out<<(ft[i]-fs[i]);}out<<'\n';++edges;}else if(seen.insert(t).second)q.push_back(std::move(t));}}
  }
 }
 std::cerr<<"edges="<<edges<<"\n";
}
