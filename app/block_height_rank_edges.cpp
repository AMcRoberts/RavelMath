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
 auto f=derive_block_height_features(x); std::int64_t level=0,sum=0,maxv=x[0],minv=x[0];
 for(auto v:x){level+=std::llabs(v);sum+=v;maxv=std::max(maxv,v);minv=std::min(minv,v);}
 return {level,f.forcing_support,f.forcing_zero_count,f.forcing_gcd,f.moment_variation,f.moment_energy,f.adjacent_energy,sum,maxv,minv};
}
static std::string chamber(const ShellState&x){std::string s;for(auto v:x)s.push_back(v<0?'-':v>0?'+':'0');return s;}
int main(int argc,char**argv){if(argc!=5)return 2;size_t n=std::stoul(argv[1]);long long lo=std::stoll(argv[2]),hi=std::stoll(argv[3]);std::ofstream out(argv[4]);out<<"source_chamber,target_chamber";for(int side=0;side<2;++side)for(auto name:{"level","bfsupport","bfzero","bfgcd","mvariation","menergy","adjenergy","sum","max","min"})out<<','<<(side?'t':'s')<<'_'<<name;out<<'\n';size_t edges=0;
for(long long b=lo;b<=hi;++b){std::vector<ShellState>shell;std::unordered_set<ShellState,ShellStateHash>ss;ShellState st(n,0);enumerate_box_states_rec(n,b,0,st,[&](auto const&x){if(shell_radius(x)==b){shell.push_back(x);ss.insert(x);}});for(auto const&s:shell){std::deque<ShellState>q;std::unordered_set<ShellState,ShellStateHash>seen;q.push_back(s);seen.insert(s);auto fs=feat(s);while(!q.empty()){auto c=std::move(q.front());q.pop_front();for(auto t:bounded_carry_successors(c,b)){if(ss.contains(t)){auto ft=feat(t);out<<chamber(s)<<','<<chamber(t);for(auto v:fs)out<<','<<v;for(auto v:ft)out<<','<<v;out<<'\n';++edges;}else if(seen.insert(t).second)q.push_back(std::move(t));}}}}
std::cerr<<"edges="<<edges<<"\n";}
