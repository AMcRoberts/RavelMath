#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <deque>
#include <iostream>
#include <fstream>
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
constexpr size_t O=11,F=103,V=O*F; struct Edge{ShellState s,t;}; struct Term{uint16_t i; int64_t v;}; using Sparse=std::vector<Term>;
int si(int64_t x){return x<0?0:x==0?1:2;} std::string ch(const ShellState&x){std::string r;for(auto v:x)r+=v<0?'-':v>0?'+':'0';return r;} ShellState neg(ShellState x){for(auto&v:x)v=-v;return x;} ShellState canon(const ShellState&x){return negate_sign_chamber(ch(x))<ch(x)?neg(x):x;}
std::array<int64_t,O> obs(const ShellState&raw){auto x=canon(raw);auto f=derive_block_height_features(x);int64_t l=0,s=0,mx=x[0],mn=x[0];for(auto v:x){l+=std::llabs(v);s+=v;mx=std::max(mx,v);mn=std::min(mn,v);}return {l,f.forcing_support,f.forcing_zero_count,f.forcing_gcd,f.moment_variation,f.moment_energy,f.adjacent_energy,s,mx,mn,1};}
std::vector<size_t> feats(const ShellState&raw){auto x=canon(raw);std::vector<size_t>a{0};size_t off=1;for(size_t i=0;i<4;++i)a.push_back(off+i*3+si(x[i]));off+=12;for(size_t i=0;i<3;++i)a.push_back(off+i*9+3*si(x[i])+si(x[i+1]));off+=27;a.push_back(off+3*si(x[3])+si(x[0]));off+=9;a.push_back(off+9*si(x[0])+3*si(x[1])+si(x[2]));off+=27;a.push_back(off+9*si(x[1])+3*si(x[2])+si(x[3]));return a;}
Sparse design(const ShellState&x){auto o=obs(x);auto a=feats(x);Sparse p;for(auto f:a)for(size_t j=0;j<O;++j)if(o[j])p.push_back({(uint16_t)(f*O+j),o[j]});return p;}
long double val(const Sparse&p,const std::vector<long double>&w){long double z=0;for(auto q:p)z+=w[q.i]*q.v;return z;} void upd(std::vector<long double>&w,const Sparse&s,const Sparse&t,long double k){for(auto q:s)w[q.i]-=k*q.v;for(auto q:t)w[q.i]+=k*q.v;}
long double n2(const Sparse&s,const Sparse&t){size_t i=0,j=0;long double z=0;while(i<s.size()||j<t.size()){long double d;if(j==t.size()||(i<s.size()&&s[i].i<t[j].i))d=-s[i++].v;else if(i==s.size()||t[j].i<s[i].i)d=t[j++].v;else d=t[j++].v-s[i++].v;z+=d*d;}return z;}
std::vector<Edge> edges(){std::vector<Edge>e;for(int64_t b=2;b<=6;++b){std::vector<ShellState>sh;std::unordered_set<ShellState,ShellStateHash>set;ShellState z(4,0);enumerate_box_states_rec(4,b,0,z,[&](const ShellState&x){if(shell_radius(x)==b){sh.push_back(x);set.insert(x);}});for(auto&s:sh){std::deque<ShellState>q;std::unordered_set<ShellState,ShellStateHash>seen;q.push_back(s);seen.insert(s);while(!q.empty()){auto c=q.front();q.pop_front();for(auto t:bounded_carry_successors(c,b)){if(set.contains(t))e.push_back({s,t});else if(seen.insert(t).second)q.push_back(t);}}}}return e;}
}
int main(){auto es=edges();std::unordered_map<ShellState,Sparse,ShellStateHash>c;auto get=[&](const ShellState&x)->const Sparse&{auto[it,n]=c.try_emplace(x);if(n)it->second=design(x);return it->second;};std::vector<long double>w(V);for(int ep=0;ep<300;++ep){size_t u=0;long double worst=1e300L;for(auto&e:es){auto&s=get(e.s);auto&t=get(e.t);auto m=val(t,w)-val(s,w);worst=std::min(worst,m);if(m<1){auto q=n2(s,t);if(q){upd(w,s,t,(1-m)/q);++u;}}}if(ep%25==0||!u)std::cerr<<"epoch="<<ep<<" updates="<<u<<" worst="<<(double)worst<<"\n";if(!u)break;}size_t bad=0;long double worst=1e300L;for(auto&e:es){auto m=val(get(e.t),w)-val(get(e.s),w);worst=std::min(worst,m);bad+=!(m>0);}std::vector<int64_t>wi(V);for(size_t i=0;i<V;++i)wi[i]=llround(w[i]*1000000.L);auto iv=[&](const Sparse&p){__int128 z=0;for(auto q:p)z+=(__int128)wi[q.i]*q.v;return z;};size_t eb=0;__int128 em=((__int128)1<<120);for(auto&e:es){auto d=iv(get(e.t))-iv(get(e.s));em=std::min(em,d);eb+=d<=0;}auto pr=[](__int128 z){if(z<0){std::cout<<'-';z=-z;}std::string s;if(!z)s="0";while(z){s.push_back('0'+z%10);z/=10;}std::reverse(s.begin(),s.end());std::cout<<s;};std::cout<<"BOUNDARY_FLUX_SPARSE edges="<<es.size()<<" states="<<c.size()<<" variables="<<V<<" failures="<<bad<<" minimum_margin="<<(double)worst<<" exact_failures="<<eb<<" exact_minimum_gain=";pr(em);std::cout<<"\n"; std::ofstream wf("out/boundary_flux_weights.tsv"); wf<<"feature\tobservable\tweight\n"; for(size_t f=0;f<F;++f)for(size_t j=0;j<O;++j)if(wi[f*O+j])wf<<f<<"\t"<<j<<"\t"<<wi[f*O+j]<<"\n"; return bad||eb?3:0;}
