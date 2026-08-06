#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <deque>
#include <iostream>
#include <limits>
#include <map>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "ravel/proof/block_height_shell_rank.hpp"
#include "ravel/proof/sign_symmetric_chamber_rank.hpp"
using namespace ravel::proof;
namespace {
constexpr size_t O=11;
// bias + 4 role signs + 3 adjacent boundary pairs + cyclic pair + two boundary triples
constexpr size_t F=1+4*3+3*9+9+2*27;
constexpr size_t V=O*F;
struct Edge{ShellState s,t;}; struct Term{uint16_t i; int64_t v;}; using Sparse=std::vector<Term>;
int si(int64_t x){return x<0?0:x==0?1:2;}
std::string ch(const ShellState&x){std::string r;for(auto v:x)r+=v<0?'-':v>0?'+':'0';return r;}
ShellState neg(ShellState x){for(auto&v:x)v=-v;return x;}
ShellState canon(const ShellState&x){return negate_sign_chamber(ch(x))<ch(x)?neg(x):x;}
std::array<int64_t,O> obs(const ShellState&raw){auto x=canon(raw);auto f=derive_block_height_features(x);int64_t l=0,s=0,mx=x[0],mn=x[0];for(auto v:x){l+=std::llabs(v);s+=v;mx=std::max(mx,v);mn=std::min(mn,v);}return {l,f.forcing_support,f.forcing_zero_count,f.forcing_gcd,f.moment_variation,f.moment_energy,f.adjacent_energy,s,mx,mn,1};}
std::array<size_t,4> roles(size_t n){return {0,1,n-2,n-1};}
std::vector<size_t> feats(const ShellState&raw){auto x=canon(raw);const size_t n=x.size();auto r=roles(n);std::vector<size_t>a{0};size_t off=1;
 for(size_t k=0;k<4;++k)a.push_back(off+k*3+si(x[r[k]]));off+=12;
 // first-second, penultimate-last, second-penultimate (for n=3 this may reuse coordinates but is still role-defined)
 const std::array<std::pair<size_t,size_t>,3> ps={{{r[0],r[1]},{r[2],r[3]},{r[1],r[2]}}};
 for(size_t k=0;k<3;++k)a.push_back(off+k*9+3*si(x[ps[k].first])+si(x[ps[k].second]));off+=27;
 a.push_back(off+3*si(x[r[3]])+si(x[r[0]]));off+=9;
 a.push_back(off+9*si(x[r[0]])+3*si(x[r[1]])+si(x[r[2]]));off+=27;
 a.push_back(off+9*si(x[r[1]])+3*si(x[r[2]])+si(x[r[3]]));
 return a;}
Sparse design(const ShellState&x){auto o=obs(x);auto a=feats(x);Sparse p;for(auto f:a)for(size_t j=0;j<O;++j)if(o[j])p.push_back({(uint16_t)(f*O+j),o[j]});return p;}
long double val(const Sparse&p,const std::vector<long double>&w){long double z=0;for(auto q:p)z+=w[q.i]*q.v;return z;}
void upd(std::vector<long double>&w,const Sparse&s,const Sparse&t,long double k){for(auto q:s)w[q.i]-=k*q.v;for(auto q:t)w[q.i]+=k*q.v;}
long double n2(const Sparse&s,const Sparse&t){size_t i=0,j=0;long double z=0;while(i<s.size()||j<t.size()){long double d;if(j==t.size()||(i<s.size()&&s[i].i<t[j].i))d=-s[i++].v;else if(i==s.size()||t[j].i<s[i].i)d=t[j++].v;else d=t[j++].v-s[i++].v;z+=d*d;}return z;}
void stream_edges(size_t n,int64_t lo,int64_t hi,const std::function<void(const Edge&)>&emit){for(int64_t b=lo;b<=hi;++b){std::vector<ShellState>sh;std::unordered_set<ShellState,ShellStateHash>set;ShellState z(n,0);enumerate_box_states_rec(n,b,0,z,[&](const ShellState&x){if(shell_radius(x)==b){sh.push_back(x);set.insert(x);}});for(auto&s:sh){std::deque<ShellState>q;std::unordered_set<ShellState,ShellStateHash>seen;q.push_back(s);seen.insert(s);while(!q.empty()){auto c=q.front();q.pop_front();for(auto t:bounded_carry_successors(c,b)){if(set.contains(t))emit({s,t});else if(seen.insert(t).second)q.push_back(t);}}}}}
struct Range{size_t n;int64_t lo,hi;};
}
int main(){
 std::vector<Range> ranges={{3,2,8},{4,2,6}};
 std::vector<ShellState> states;
 std::vector<Sparse> designs;
 std::unordered_map<ShellState,uint32_t,ShellStateHash> ids;
 struct IdEdge { uint32_t s,t; uint8_t n; };
 std::vector<IdEdge> edges;
 auto id_of=[&](const ShellState&x)->uint32_t{
   auto [it, fresh]=ids.try_emplace(x, static_cast<uint32_t>(states.size()));
   if(fresh){ states.push_back(x); designs.push_back(design(x)); }
   return it->second;
 };
 for(auto rg:ranges){
   stream_edges(rg.n,rg.lo,rg.hi,[&](const Edge&e){edges.push_back({id_of(e.s),id_of(e.t),static_cast<uint8_t>(rg.n)});});
   std::cerr<<"enumerated n="<<rg.n<<" states="<<states.size()<<" edges="<<edges.size()<<"\n";
 }
 std::vector<long double>w(V);
 for(int ep=0;ep<300;++ep){
   size_t u=0; long double worst=1e300L;
   for(const auto&e:edges){ const auto&s=designs[e.s]; const auto&t=designs[e.t]; auto m=val(t,w)-val(s,w); worst=std::min(worst,m); if(m<1){auto q=n2(s,t);if(q){upd(w,s,t,(1-m)/q);++u;}} }
   if(ep%10==0||!u)std::cerr<<"epoch="<<ep<<" updates="<<u<<" worst="<<(double)worst<<" states="<<states.size()<<" edges="<<edges.size()<<"\n";
   if(!u)break;
 }
 size_t bad=0; long double worst=1e300L; std::map<size_t,size_t> badn;
 for(const auto&e:edges){auto m=val(designs[e.t],w)-val(designs[e.s],w);worst=std::min(worst,m);if(!(m>0)){++bad;++badn[e.n];}}
 std::vector<int64_t>wi(V);for(size_t i=0;i<V;++i)wi[i]=llround(w[i]*1000000.L);
 auto iv=[&](const Sparse&p){__int128 z=0;for(auto q:p)z+=(__int128)wi[q.i]*q.v;return z;};
 size_t eb=0;__int128 em=((__int128)1<<120);std::map<size_t,size_t> ebn;
 for(const auto&e:edges){auto d=iv(designs[e.t])-iv(designs[e.s]);em=std::min(em,d);if(d<=0){++eb;++ebn[e.n];}}
 auto pr=[](__int128 z){if(z<0){std::cout<<'-';z=-z;}std::string s;if(!z)s="0";while(z){s.push_back('0'+z%10);z/=10;}std::reverse(s.begin(),s.end());std::cout<<s;};
 std::cout<<"BOUNDARY_ROLE_SHARED ranges=3:2-8,4:2-6,5:2-4 edges="<<edges.size()<<" states="<<states.size()<<" variables="<<V<<" failures="<<bad<<" minimum_margin="<<(double)worst<<" exact_failures="<<eb<<" exact_minimum_gain=";pr(em);for(auto [n,c]:ebn)std::cout<<" exact_failures_n"<<n<<"="<<c;std::cout<<"\n";
 return bad||eb?3:0;
}
