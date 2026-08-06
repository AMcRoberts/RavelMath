#!/usr/bin/env python3
from __future__ import annotations
import argparse,json,math
from collections import defaultdict,deque
from fractions import Fraction
from itertools import product
from pathlib import Path
def step(s,d): return s[1:]+(s[0]-sum(s[1:])+d,)
def shell_states(n,M): return {x for x in product(range(-M,M+1),repeat=n) if max(map(abs,x))==M}
def first_returns(n,M):
 sh=shell_states(n,M); out=[]
 for src in sh:
  q=deque([(src,())]); seen={src}
  while q:
   cur,digs=q.popleft()
   for d in (-1,0,1):
    nxt=step(cur,d)
    if not all(-M<=v<=M for v in nxt): continue
    nd=digs+(d,)
    if nxt in sh: out.append((src,nxt,nd))
    elif nxt not in seen: seen.add(nxt); q.append((nxt,nd))
 return out
def lcm(a,b): return abs(a*b)//math.gcd(a,b)
def witnesses(path):
 p=json.loads(Path(path).read_text()); out=[]; seen=set()
 for c in p['candidates']:
  seq=tuple(Fraction(v) for v in c['sequence'])
  if seq in seen: continue
  seen.add(seq); den=1
  for v in seq: den=lcm(den,v.denominator)
  ints=tuple(int(v*den) for v in seq); out.append({'sequence':ints,'delta':max(map(abs,ints)),'indices':c['indices'],'signs':c['signs']})
 return out
def audit(n,M,wpath):
 edges=first_returns(n,M); ws=witnesses(wpath); horizon=n+1; explained=0; usage=defaultdict(int); examples=[]; un=[]; hist=defaultdict(int)
 for src,tgt,digs in edges:
  hist[len(digs)]+=1; found=None
  if len(digs)<=horizon:
   base=[src]; b=src
   for d in digs: b=step(b,d); base.append(b)
   for wi,w in enumerate(ws):
    for sg in (1,-1):
     seq=tuple(sg*v for v in w['sequence']); D=w['delta']
     for st in range(horizon-len(digs)+1):
      wins=[seq[st+j:st+j+n] for j in range(len(digs)+1)]
      if any(len(z)!=n or max(map(abs,z))!=D for z in wins): continue
      trans=[tuple(a+t for a,t in zip(x,z)) for x,z in zip(base,wins)]; R=M+D
      if max(map(abs,trans[0]))!=R or max(map(abs,trans[-1]))!=R: continue
      if any(max(map(abs,z))==R for z in trans[1:-1]): continue
      found={'witness_index':wi,'global_sign':sg,'start':st,'delta':D,'base_states':base,'translation_windows':wins,'transported':trans}; break
     if found: break
    if found: break
  if found:
   explained+=1; usage[found['witness_index']]+=1
   if len(examples)<12: examples.append({'source':src,'target':tgt,'digits':digs,**found})
  elif len(un)<12: un.append({'source':src,'target':tgt,'digits':digs,'path_length':len(digs)})
 return {'kind':'nbonacci-cover-tube-transport-audit-v1','n':n,'bound':M,'unique_witness_sequences':len(ws),'edges':len(edges),'explained':explained,'unexplained':len(edges)-explained,'coverage_ratio':explained/len(edges) if edges else 1.0,'path_length_histogram':dict(sorted(hist.items())),'witness_usage':dict(sorted(usage.items())),'examples':examples,'unexplained_examples':un}
def main():
 ap=argparse.ArgumentParser(); ap.add_argument('--n',type=int,required=True); ap.add_argument('--bound',type=int,required=True); ap.add_argument('--witness',required=True); ap.add_argument('--emit',required=True); a=ap.parse_args(); p=audit(a.n,a.bound,a.witness); Path(a.emit).write_text(json.dumps(p,indent=2,sort_keys=True)+'\n'); print(f"cover tube audit n={a.n} M={a.bound}: {p['explained']}/{p['edges']} ({p['coverage_ratio']:.3%}) explained")
if __name__=='__main__': main()
