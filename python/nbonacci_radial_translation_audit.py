#!/usr/bin/env python3
"""Audit the naive radial-translation closure of maximum-shell first returns.

For a nonzero-coordinate state x and period p=n+1, define
  T_p(x)_i = x_i + p*sign(x_i).
This preserves signs, exact absolute gaps, and min-absolute-value modulo p.
The audit asks whether every first-return edge x->y at shell M transports to
T_p(x)->T_p(y) at shell M+p. Counterexamples disprove naive phase transport.
"""
from __future__ import annotations
import argparse, json
from nbonacci_max_shell_return_probe import first_return_graph, install_memory_limit

def radial_translate(x, period):
    if any(v == 0 for v in x):
        return None
    return tuple(v + period if v > 0 else v - period for v in x)

def main():
    install_memory_limit()
    ap=argparse.ArgumentParser()
    ap.add_argument('--n',type=int,required=True)
    ap.add_argument('--bound',type=int,required=True)
    ap.add_argument('--emit',required=True)
    args=ap.parse_args()
    p=args.n+1
    shell, relation=first_return_graph(args.n,args.bound)
    shell2, relation2=first_return_graph(args.n,args.bound+p)
    checked=transported=failed=0
    witnesses=[]
    for x, ys in relation.items():
        tx=radial_translate(x,p)
        if tx is None: continue
        for y in ys:
            ty=radial_translate(y,p)
            if ty is None: continue
            checked += 1
            if ty in relation2.get(tx,set()):
                transported += 1
            else:
                failed += 1
                if len(witnesses)<32:
                    witnesses.append({'source':list(x),'target':list(y),
                                      'translated_source':list(tx),
                                      'translated_target':list(ty)})
    payload={'kind':'nbonacci-naive-radial-translation-audit-v1',
             'n':args.n,'bound':args.bound,'period':p,
             'checked_edges':checked,'transported_edges':transported,
             'failed_edges':failed,'closure_holds':failed==0,
             'counterexamples':witnesses}
    with open(args.emit,'w',encoding='utf-8') as f:
        json.dump(payload,f,indent=2,sort_keys=True); f.write('\n')
    print(f"radial translation audit: n={args.n} M={args.bound} "
          f"transported={transported}/{checked} failed={failed} "
          f"closure={'PASS' if failed==0 else 'FAIL'}")
    return 0
if __name__=='__main__': raise SystemExit(main())
