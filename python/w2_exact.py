"""Exact-arithmetic redo of W2(B) + the low-complexity menu scan."""
import math, json
from fractions import Fraction

SQ2 = 2**0.5
TS = 2*SQ2
rows = {}

# Z[sqrt2] numbers as (a,b) = a + b*sqrt2, a,b rational (Fractions)
def add(u,v): return (u[0]+v[0], u[1]+v[1])
def sub(u,v): return (u[0]-v[0], u[1]-v[1])
def mul(u,v): return (u[0]*v[0]+2*u[1]*v[1], u[0]*v[1]+u[1]*v[0])
def geq0(u):
    a,b = u
    if a >= 0 and b >= 0: return True
    if a < 0 and b < 0: return False
    if a >= 0: return a*a >= 2*b*b     # b<0
    return 2*b*b >= a*a                # a<0, b>0... careful: a<0,b>0: pos iff 2b^2>a^2
def tof(u): return float(u[0]) + float(u[1])*SQ2

BETA = (Fraction(-1), Fraction(1))
RHO = (Fraction(9), Fraction(-6))

# exact greedy base-beta: digit d_i = max d in {0,1,2} with rem - d*beta^i >= 0
print("(B-exact) greedy base-beta expansion of rho_Q = 9-6sqrt2:")
rem = RHO
p = BETA
digits = []
for i in range(1, 9):
    d = 0
    for cand in (2,1,0):
        t = sub(rem, mul((Fraction(cand),Fraction(0)), p))
        if geq0(t):
            d = cand; rem = t; break
    digits.append(d)
    print(f"   position beta^{i}: digit {d}   remainder = "
          f"{rem[0]}+{rem[1]}*sqrt2 = {tof(rem):.3e}")
    if rem == (0,0):
        print(f"   TERMINATED EXACTLY at position {i}; word = {digits}")
        break
    p = mul(p, BETA)
rows["silver_digits"] = digits
rows["silver_terminates"] = rem == (Fraction(0),Fraction(0))

# golden: rho_Q not in Q(sqrt5) => cannot terminate; show err ~ alpha^k numerically
ALPHA = (5**0.5-1)/2
remf, pf, k, errs = 9-6*SQ2, ALPHA, 0, []
print("\n(B-exact) golden expansion (field-blocked; remainder ~ alpha^k forever):")
for i in range(1, 25):
    d = int(remf/pf + 1e-15)
    d = min(d, 1)   # golden Ostrowski digits 0/1 (Zeckendorf-like)
    remf -= d*pf; pf *= ALPHA
    if i in (4, 8, 12, 16, 20, 24):
        print(f"   after {i:>2} digits: remainder = {remf:.3e}  "
              f"(alpha^{i} = {ALPHA**i:.3e})")
rows["golden_rem_24"] = remf

# menu scan: all words with <=3 nonzero digits, positions 1..6, digits 1..2
print("\n(menu) achievable S from silver words with <=3 nonzero digits (pos<=6):")
import itertools
menu = set()
POWF = [(SQ2-1)**i for i in range(1,7)]
for npos in (1,2,3):
    for pos in itertools.combinations(range(6), npos):
        for digs in itertools.product((1,2), repeat=npos):
            rho = sum(d*POWF[p] for d,p in zip(digs,pos))
            if 0 < rho <= 1:
                menu.add(round(4*(3-rho)/(3+rho), 10))
menu = sorted(menu)
below = [s for s in menu if s < TS]
above = [s for s in menu if s > TS]
hit = [s for s in menu if abs(s-TS) < 1e-9]
print(f"   menu size = {len(menu)}; contains 2sqrt2 exactly: {len(hit)==1}")
print(f"   nearest below: {max(below):.6f}   2sqrt2 = {TS:.6f}   "
      f"nearest above: {min(above):.6f}")
print(f"   gap to neighbors: {TS-max(below):.4f} / {min(above)-TS:.4f}")
rows["menu_size"] = len(menu); rows["menu_hits_TS"] = len(hit) == 1
rows["menu_neighbors"] = [max(below), min(above)]
json.dump(rows, open("w2_exact_results.json","w"), indent=1)
print("saved -> w2_exact_results.json")
