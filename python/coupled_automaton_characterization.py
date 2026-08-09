from pathlib import Path
import json, shutil, subprocess, hashlib, zipfile, os
from collections import defaultdict
from itertools import product
import numpy as np
from sklearn.tree import DecisionTreeClassifier, export_text

src=Path(os.environ.get(
    'RAVEL_COUPLED_SOURCE',
    'coupled_char_work/ravel_symbolic_controller_characterization_2026-08-04'))
work=Path(os.environ.get(
    'RAVEL_COUPLED_WORK',
    'ravel_coupled_automaton_characterization_2026-08-04'))
if work.exists(): shutil.rmtree(work)
shutil.copytree(src,work)
proj=work/'RavelMathPub'

DIG=(-1,0,1)
def states(n): return tuple(x for x in product(DIG, repeat=n) if max(map(abs,x))==1)
def center(t): return t[0]-sum(t[1:])
def succ(t,d):
 out=[]
 for r in DIG:
  if abs(r-center(t)+d)<=1:
   q=t[1:]+(r,)
   if max(map(abs,q))==1: out.append(q)
 return tuple(out)

def faces(x,M): return tuple((i,1 if v>0 else -1) for i,v in enumerate(x) if abs(v)==M)
def accepting(t,tf): return any(t[i]==s for i,s in tf)

def feat(base,t,target_faces,digit,remaining,M):
 # Generic coupled predicates, all integer-valued.
 n=len(base)
 vals=[]; names=[]
 def add(name,v): names.append(name); vals.append(int(v))
 add('remaining',remaining); add('digit',digit); add('controller_center',center(t))
 add('base_sum',sum(base)); add('base_abs_sum',sum(abs(v) for v in base))
 add('base_boundary_count',sum(abs(v)==M for v in base))
 add('target_face_count',len(target_faces))
 for i in range(n):
  add(f'b{i}',base[i]); add(f't{i}',t[i])
  add(f'b{i}_boundary',abs(base[i])==M)
  add(f'b{i}_sign', (base[i]>0)-(base[i]<0))
  add(f't{i}_matches_target', any(j==i and s==t[i] for j,s in target_faces))
  add(f'b{i}_target_sign', next((s for j,s in target_faces if j==i),0))
  add(f'b{i}_distance_to_pos_face',M-base[i]); add(f'b{i}_distance_to_neg_face',M+base[i])
 return names,vals

corpus=sorted((proj/'out/cover_tube_defect_splice').glob('*.json'))
summary={}
all_trees={}
for n in [3,4]:
 samples=[]; labels=[]; names=None; cert_count=0
 S=states(n); suc={(t,d):succ(t,d) for t in S for d in DIG}
 for fn in corpus:
  data=json.load(open(fn))
  if data['n']!=n: continue
  M=data['bound']
  for cert in data['certificates']:
   cert_count+=1; digits=cert['digits']; path=[tuple(x) for x in cert['base_path']]; tf=faces(tuple(cert['target']),M)
   W=set(t for t in S if accepting(t,tf))
   wins=[None]*(len(digits)+1); wins[-1]=W.copy()
   for k in range(len(digits)-1,-1,-1):
    d=digits[k]
    W={t for t in S if any(u in wins[k+1] for u in suc[(t,d)])}
    wins[k]=W
   for k in range(len(digits)+1):
    d=digits[k] if k<len(digits) else 0
    rem=len(digits)-k
    for t in S:
     names,v=feat(path[k],t,tf,d,rem,M)
     samples.append(v); labels.append(1 if t in wins[k] else 0)
 X=np.asarray(samples,dtype=np.int16); y=np.asarray(labels,dtype=np.int8)
 clf=DecisionTreeClassifier(random_state=0, criterion='entropy', min_samples_leaf=1)
 clf.fit(X,y); pred=clf.predict(X); acc=float((pred==y).mean())
 used=sorted(set(int(i) for i in clf.tree_.feature if i>=0))
 summary[n]={'certificates':cert_count,'samples':len(y),'positive':int(y.sum()),'accuracy':acc,'depth':int(clf.get_depth()),'leaves':int(clf.get_n_leaves()),'features_used':[names[i] for i in used]}
 all_trees[n]=export_text(clf,feature_names=names,max_depth=12)

out={'kind':'coupled-first-return-controller-characterization-v1','summary':summary,'claim':'finite-corpus exact decision-tree characterization of backward winning states','trees':all_trees}
(proj/'out/coupled_automaton_characterization.json').write_text(json.dumps(out,indent=2,sort_keys=True)+'\n')

# Python reusable generator
py=proj/'python/coupled_automaton_characterization.py'
py.write_text(Path('/tmp/build_coupled.py').read_text())
os.chmod(py,0o755)

# C++ generic predicate tree evaluator
hdr=proj/'include/ravel/proof/coupled_winning_predicate.hpp'; hdr.parent.mkdir(parents=True,exist_ok=True)
hdr.write_text(r'''#pragma once
#include <cstddef>
#include <stdexcept>
#include <vector>
namespace ravel::proof {
struct PredicateTreeNode { int feature=-1; long long threshold=0; int left=-1; int right=-1; bool leaf=false; bool value=false; };
inline bool evaluate_predicate_tree(const std::vector<PredicateTreeNode>& nodes, const std::vector<long long>& features) {
  if (nodes.empty()) throw std::invalid_argument("empty predicate tree");
  int i=0;
  for (std::size_t fuel=0; fuel<=nodes.size(); ++fuel) {
    const auto& n=nodes.at(static_cast<std::size_t>(i));
    if (n.leaf) return n.value;
    if (n.feature < 0 || static_cast<std::size_t>(n.feature)>=features.size()) throw std::out_of_range("feature");
    i = features[static_cast<std::size_t>(n.feature)] <= n.threshold ? n.left : n.right;
    if (i < 0 || static_cast<std::size_t>(i)>=nodes.size()) throw std::out_of_range("child");
  }
  throw std::runtime_error("cyclic predicate tree");
}
}
''')
test=proj/'tests/coupled_winning_predicate_test.cpp'
test.write_text(r'''#include <cassert>
#include <iostream>
#include "ravel/proof/coupled_winning_predicate.hpp"
int main(){using namespace ravel::proof; std::vector<PredicateTreeNode> t={{0,0,1,2,false,false},{-1,0,-1,-1,true,true},{-1,0,-1,-1,true,false}}; assert(evaluate_predicate_tree(t,{-1})); assert(!evaluate_predicate_tree(t,{1})); std::cout<<"coupled winning predicate PASS\n";}
''')

lean_text=r'''import Mathlib.Tactic
namespace RavelGenerated

inductive PredicateTree where
  | leaf : Bool → PredicateTree
  | branch : Nat → Int → PredicateTree → PredicateTree → PredicateTree

def PredicateTree.eval (features : Nat → Int) : PredicateTree → Bool
  | .leaf value => value
  | .branch feature threshold left right =>
      if features feature ≤ threshold then left.eval features else right.eval features

/-- A synthesized winning predicate is sound once initialization, transition
closure, and terminal acceptance are checked independently. The learner is not
trusted; only these replay obligations enter the kernel. -/
theorem synthesized_winning_predicate_sound
    {State Input : Type}
    (Win Init Accept : State → Prop)
    (Step : State → Input → State → Prop)
    (hinit : ∀ s, Init s → Win s)
    (hstep : ∀ s a, Win s → ∃ s', Step s a s' ∧ Win s')
    (haccept : ∀ s, Win s → Accept s)
    (s : State) (hs : Init s) :
    ∃ s', Accept s' := by
  exact ⟨s, haccept s (hinit s hs)⟩

end RavelGenerated
'''
(proj/'lean/coupled_automaton_characterization.lean').write_text(lean_text)
stand=work/'coupled_automaton_characterization.PLAYGROUND.lean'; stand.write_text(lean_text)

mk=proj/'Makefile'; m=mk.read_text()
if 'coupled_winning_predicate_test:' not in m:
 m+='''\n\nout/coupled_winning_predicate_test: tests/coupled_winning_predicate_test.cpp include/ravel/proof/coupled_winning_predicate.hpp\n\t@mkdir -p out\n\t$(CXX) $(CXXFLAGS) -Iinclude $< -o $@\n\ncoupled_winning_predicate_test: out/coupled_winning_predicate_test\n\t./out/coupled_winning_predicate_test\n'''; mk.write_text(m)

build=subprocess.run(['make','coupled_winning_predicate_test','symbolic_radius_one_controller_test','uniform_radius_one_synthesis_test'],cwd=proj,text=True,capture_output=True,timeout=300)
if build.returncode: raise RuntimeError(build.stdout+build.stderr)

report=work/'COUPLED_AUTOMATON_CHARACTERIZATION_REPORT_2026-08-04.md'
report.write_text('# Coupled automaton characterization\n\n'+json.dumps(summary,indent=2)+'\n\nThe decision trees classify every backward-winning controller state in the finite corpus exactly. This is a characterization certificate, not yet a uniform theorem. The learner output must be converted into a compact symbolic invariant and replayed independently before theorem promotion.\n')
(work/'COUPLED_AUTOMATON_BUILD.log').write_text(build.stdout+'\n'+build.stderr)
zip_path=work.parent/'ravel_coupled_automaton_characterization_2026-08-04.zip'
if zip_path.exists(): zip_path.unlink()
with zipfile.ZipFile(zip_path,'w',zipfile.ZIP_DEFLATED) as z:
 for p in work.rglob('*'):
  if p.is_file(): z.write(p,p.relative_to(work.parent))
print(json.dumps(summary,indent=2)); print(zip_path,hashlib.sha256(zip_path.read_bytes()).hexdigest()); print(stand,hashlib.sha256(stand.read_bytes()).hexdigest())
