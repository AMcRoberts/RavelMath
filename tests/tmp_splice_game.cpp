#include <filesystem>
#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <queue>
#include <functional>
#include "ravel/proof/covering_translation_tube.hpp"
#include "ravel/proof/first_return_joint_product.hpp"
using namespace ravel::proof;
struct E{IntegerState s,t;std::vector<int64_t>w;};
int main(){for(size_t n:{3ul,4ul}){
 std::vector<std::filesystem::path> fs={"out/cover_tube_defect_splice/n"+std::to_string(n)+"_M2.json"};if(n==3)fs.push_back("out/cover_tube_defect_splice/n3_M3.json");
 std::vector<FirstReturnWitness> ws;for(auto&f:fs){auto p=load_first_return_witnesses(f);ws.insert(ws.end(),p.begin(),p.end());}
 std::vector<E> es;std::set<IntegerState> vs;for(auto&w:ws){es.push_back({w.base_path.front(),w.target,w.digits});vs.insert(es.back().s);vs.insert(es.back().t);}std::vector<IntegerState> V(vs.begin(),vs.end());std::map<IntegerState,size_t> id;for(size_t i=0;i<V.size();++i)id[V[i]]=i;
 std::vector<std::vector<size_t>> adj(V.size()),radj(V.size());for(size_t i=0;i<es.size();++i){adj[id[es[i].s]].push_back(i);radj[id[es[i].t]].push_back(i);} // crude recurrent: vertices in SCC size>1 or self
 // Tarjan
 int idx=0;std::vector<int> ind(V.size(),-1),low(V.size());std::vector<char> on(V.size());std::vector<size_t> st;std::vector<int> comp(V.size(),-1);int cc=0;
 std::function<void(size_t)> dfs=[&](size_t v){ind[v]=low[v]=idx++;st.push_back(v);on[v]=1;for(auto ei:adj[v]){auto w=id[es[ei].t];if(ind[w]<0){dfs(w);low[v]=std::min(low[v],low[w]);}else if(on[w])low[v]=std::min(low[v],ind[w]);}if(low[v]==ind[v]){while(1){auto w=st.back();st.pop_back();on[w]=0;comp[w]=cc;if(w==v)break;}cc++;}};for(size_t v=0;v<V.size();++v)if(ind[v]<0)dfs(v);
 std::vector<int> csz(cc);for(auto c:comp)csz[c]++;std::vector<char> rec(V.size());for(size_t v=0;v<V.size();++v){if(csz[comp[v]]>1)rec[v]=1;else for(auto ei:adj[v])if(id[es[ei].t]==v)rec[v]=1;}
 auto states=enumerate_radius_one_states(n);auto plant=make_symbolic_controller_plant(states);
 std::vector<std::vector<char>> F(V.size(),std::vector<char>(states.size()));for(size_t v=0;v<V.size();++v)if(rec[v]){auto faces=detail::target_faces(V[v], n==3 && sup_norm(V[v])==3?3:2); // actually bound from norm
 auto b=sup_norm(V[v]);faces=detail::target_faces(V[v],b);for(size_t q=0;q<states.size();++q)for(auto[c,s]:faces)if(states[q][c]==s){F[v][q]=1;break;}}
 // cache relation per edge as bit target per source
 std::vector<std::vector<std::vector<size_t>>> rel(es.size(),std::vector<std::vector<size_t>>(states.size()));
 for(size_t ei=0;ei<es.size();++ei){for(size_t q=0;q<states.size();++q){std::set<size_t> cur{q};for(auto d:es[ei].w){std::set<size_t> nx;for(auto a:cur){auto it=plant.successors.find({a,d});if(it!=plant.successors.end())nx.insert(it->second.begin(),it->second.end());}cur.swap(nx);}rel[ei][q]={cur.begin(),cur.end()};}}
 bool ch=true;int rounds=0;while(ch){ch=false;rounds++;for(size_t v=0;v<V.size();++v)if(rec[v])for(size_t q=0;q<states.size();++q)if(F[v][q]){for(auto ei:adj[v]){auto w=id[es[ei].t];if(!rec[w]||comp[w]!=comp[v])continue;bool ok=false;for(auto t:rel[ei][q])if(F[w][t]){ok=true;break;}if(!ok){F[v][q]=0;ch=true;break;}}}}
 size_t rv=0,nonempty=0,minf=999999,maxf=0;for(size_t v=0;v<V.size();++v)if(rec[v]){rv++;size_t z=0;for(char x:F[v])z+=x;nonempty+=z>0;minf=std::min(minf,z);maxf=std::max(maxf,z);}std::cout<<"n="<<n<<" recurrent vertices="<<rv<<" nonempty="<<nonempty<<" minfiber="<<minf<<" max="<<maxf<<" rounds="<<rounds<<"\n";
}}
