"""
null_audit.py -- adversarial audit of the W2 arithmetic claims.

Claims under audit:
 (C1) "rho_Q = 9-6sqrt2 is INTEGRAL (in Z[sqrt2]) -- special."
      Null: compute budgets rho(d) = (1-cos d)(1+T)/((1+cos d)(1-T)),
      T = 1-2d/pi, over ALL angles d with quadratic/rational cos.
      How often is the budget integral / low-denominator in its field?
 (C2) "the greedy silver expansion of rho_Q TERMINATES -- special."
      Theory check: 1/beta = 1+sqrt2 is a quadratic Pisot unit
      (x^2 = 2x+1), so Frougny-Solomyak property (F) applies: EVERY
      element of Z[sqrt2] in [0,1) has finite expansion. Termination
      is then equivalent to field membership = automatic trigonometry.
      Empirical: expand every a+b*sqrt2 in (0,1), |b|<=300; verify all
      terminate; get length distribution; locate rho_Q's length 4.
 (C3) "2sqrt2 sits isolated in the 150-point low-complexity menu."
      Null: nearest-neighbor spacing stats of the menu itself.
"""
import math, json, itertools
from fractions import Fraction as Fr

SQ2 = 2**0.5
rows = {}

# ---------------- (C1) angle-menu budgets ----------------
# numbers in Q(sqrt d): (r0, r1) with Fractions
def qmul(u, v, d): return (u[0]*v[0]+d*u[1]*v[1], u[0]*v[1]+u[1]*v[0])
def qdiv(u, v, d):
    den = v[0]*v[0]-d*v[1]*v[1]
    conj = (v[0], -v[1])
    num = qmul(u, conj, d)
    return (num[0]/den, num[1]/den)

ANGLES = [  # (name, dfield, cos as (r0,r1), T rational)
    ("pi/4",  2, (Fr(0), Fr(1,2)),  Fr(1,2)),
    ("3pi/4", 2, (Fr(0), Fr(-1,2)), Fr(-1,2)),
    ("pi/6",  3, (Fr(0), Fr(1,2)),  Fr(2,3)),
    ("5pi/6", 3, (Fr(0), Fr(-1,2)), Fr(-2,3)),
    ("pi/3",  1, (Fr(1,2), Fr(0)),  Fr(1,3)),
    ("2pi/3", 1, (Fr(-1,2), Fr(0)), Fr(-1,3)),
    ("pi/5",  5, (Fr(1,4), Fr(1,4)),  Fr(3,5)),
    ("2pi/5", 5, (Fr(-1,4), Fr(1,4)), Fr(1,5)),
    ("3pi/5", 5, (Fr(1,4), Fr(-1,4)), Fr(-1,5)),
    ("4pi/5", 5, (Fr(-1,4), Fr(-1,4)),Fr(-3,5)),
]
def min_denom(rho, d, cap=200):
    # minimal m: m*rho in ring of integers (Z[sqrt d] for d=2,3; Z[phi] for d=5)
    for m in range(1, cap+1):
        a, b = m*rho[0], m*rho[1]
        if d in (1,2,3):
            if a.denominator == 1 and b.denominator == 1:
                return m
        else:  # d=5: (a+b*sqrt5) in Z[phi] iff 2a,2b ints of equal parity
            A, B = 2*a, 2*b
            if A.denominator == 1 and B.denominator == 1 and (A.numerator-B.numerator) % 2 == 0:
                return m
    return None
print("(C1) budget arithmetic across the natural-angle menu:")
print(f"{'angle':>6} {'field':>7} {'rho':>28} {'min m (ring)':>12} {'integral?':>9}")
integral_count, total = 0, 0
for name, d, C, T in ANGLES:
    one = (Fr(1), Fr(0))
    num = qmul((one[0]-C[0], -C[1]), (one[0]+T, Fr(0)), d)
    den = qmul((one[0]+C[0], C[1]), (one[0]-T, Fr(0)), d)
    rho = qdiv(num, den, d)
    m = min_denom(rho, d)
    total += 1
    integ = (m == 1)
    integral_count += integ
    rs = f"{rho[0]}+{rho[1]}*sqrt{d}" if d != 1 else f"{rho[0]}"
    print(f"{name:>6} {('Q(sqrt%d)'%d if d!=1 else 'Q'):>7} {rs:>28} {m:>12} {str(integ):>9}")
print(f"    integral budgets: {integral_count}/{total}")
rows["C1_integral_fraction"] = f"{integral_count}/{total}"

# ---------------- (C2) termination + length null ----------------
def geq0(a, b):  # a + b*sqrt2 >= 0, integers
    if a >= 0 and b >= 0: return True
    if a < 0 and b < 0: return False
    if a >= 0: return a*a >= 2*b*b
    return 2*b*b > a*a
def greedy_len(a, b, maxsteps=60):
    # expand x = a+b*sqrt2 in powers of beta = -1+sqrt2; return #steps to 0
    ra, rb = a, b
    pa, pb = -1, 1                      # beta^1
    for i in range(1, maxsteps+1):
        d = 0
        for cand in (2, 1):
            ta, tb = ra - cand*pa, rb - cand*pb
            if geq0(ta, tb):
                d = cand; ra, rb = ta, tb; break
        if ra == 0 and rb == 0:
            return i
        pa, pb = (2*pb - pa), (pa - pb)   # beta^(i+1) = beta^i * (-1+sqrt2)
    return None
lens, byconj = [], {}
nonterm = 0
for b in range(-300, 301):
    lo = -b*SQ2
    a = math.floor(lo) + 1
    x = a + b*SQ2
    if not (0 < x < 1): continue
    L = greedy_len(a, b)
    if L is None:
        nonterm += 1; continue
    conj = abs(a - b*SQ2)
    lens.append((L, conj, a, b))
import statistics
Ls = [l for l,_,_,_ in lens]
print(f"\n(C2) silver expansions of ALL a+b*sqrt2 in (0,1), |b|<=300: "
      f"n={len(lens)}, non-terminating: {nonterm}")
print(f"    length distribution: min {min(Ls)}, median {statistics.median(Ls)}, "
      f"max {max(Ls)}")
# rho_Q: (9,-6), conjugate 9+6sqrt2 ~ 17.49
LQ = greedy_len(9, -6)
similar = [l for l, c, _, _ in lens if 8 <= c <= 35]
if similar:
    print(f"    rho_Q length = {LQ}; peers with conjugate size in [8,35]: "
          f"n={len(similar)}, median length {statistics.median(similar)}, "
          f"range [{min(similar)},{max(similar)}]")
else:
    print(f"    rho_Q length = {LQ}; no peers in conjugate range (check enumeration)")
# NOTE (post-run): C1 rows come in reciprocal pairs rho(pi-d) = 1/rho(d),
# which are the SAME physical selection; count independent families only.
rows["C2"] = {"n": len(lens), "nonterm": nonterm, "rhoQ_len": LQ,
              "peer_median": statistics.median(similar)}

# ---------------- (C3) menu spacing null ----------------
POWF = [(SQ2-1)**i for i in range(1, 7)]
menu = set()
for npos in (1, 2, 3):
    for pos in itertools.combinations(range(6), npos):
        for digs in itertools.product((1, 2), repeat=npos):
            r = sum(dd*POWF[p] for dd, p in zip(digs, pos))
            if 0 < r <= 1:
                menu.add(round(4*(3-r)/(3+r), 10))
menu = sorted(menu)
gaps = [menu[i+1]-menu[i] for i in range(len(menu)-1)]
TS = 2*SQ2
i = min(range(len(menu)), key=lambda j: abs(menu[j]-TS))
iso = min(menu[i]-menu[i-1] if i > 0 else 9, menu[i+1]-menu[i] if i < len(menu)-1 else 9)
print(f"\n(C3) menu spacing: n={len(menu)}, median gap {statistics.median(gaps):.4f}, "
      f"mean {statistics.mean(gaps):.4f}")
print(f"    isolation of the 2sqrt2 point: {iso:.4f} "
      f"(claimed 'isolated by ~0.01') -> percentile of its gap among all gaps: "
      f"{100*sum(g < iso for g in gaps)/len(gaps):.0f}%")
rows["C3"] = {"median_gap": statistics.median(gaps), "iso": iso}
json.dump(rows, open("null_audit_results.json","w"), indent=1)
print("saved -> null_audit_results.json")
