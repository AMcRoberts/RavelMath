"""
w2_field.py -- W2: is the quantum selection strength DISTINGUISHED on
some substrate? Claim under test: the quantum acceptance ratio
rho_Q = (1-C)(1+T)/[(1+C)(1-T)] at (T=1/2, C=1/sqrt2) equals
9 - 6*sqrt2 = beta + beta^3 + beta^4, beta = sqrt2 - 1 (silver mean):
a THREE-DIGIT word in the silver substrate's own number system, while
on the golden substrate (field Q(sqrt5)) it is not finitely
representable at all (Q(sqrt2) ^ Q(sqrt5) = Q; rho_Q irrational).
Deliverables:
 (A) exact Z[sqrt2] verification of the identity;
 (B) digit-budget table: |S - 2sqrt2| for k-digit greedy expansions of
     rho_Q, base beta (silver) vs base alpha (golden);
 (C) end-to-end run on an actual SILVER chain (theta_k = {k*beta},
     Pell-number strides/separations) with the exact field constant.
"""
import math, json
import numpy as np

SQ = math.sqrt(2)
BETA = SQ - 1
ALPHA_G = (math.sqrt(5)-1)/2
TS = 2*SQ
PELL = [1,2,5,12,29,70,169,408,985,2378,5741,13860,33461,80782,195025,470832]
FIBL = [1,2,3,5,8,13,21,34,55,89,144,233,377,610,987,1597,2584,4181,6765,
        10946,17711,28657,46368]
rows = {}

# ---- (A) exact algebra in Z[sqrt2]: numbers as (a, b) = a + b*sqrt2
def mul2(u, v): return (u[0]*v[0]+2*u[1]*v[1], u[0]*v[1]+u[1]*v[0])
b1 = (-1, 1)                       # beta
b2 = mul2(b1, b1)                  # beta^2
b3 = mul2(b2, b1)
b4 = mul2(b2, b2)
ssum = (b1[0]+b3[0]+b4[0], b1[1]+b3[1]+b4[1])
print(f"(A) beta+beta^3+beta^4 = {ssum[0]} + {ssum[1]}*sqrt2  "
      f"(claim: 9 - 6*sqrt2)  ->  {'EXACT MATCH' if ssum==(9,-6) else 'MISMATCH'}")
print(f"    also beta^2 = {b2[0]}+{b2[1]}*sqrt2, and 3*beta^2 = "
      f"({3*b2[0]},{3*b2[1]}) -> same number: {(3*b2[0],3*b2[1])==(9,-6)}")
rows["identity_exact"] = ssum == (9, -6)
RHO_Q = 9 - 6*SQ

# ---- (B) digit-budget: greedy base-b expansions, S error via S=4(3-rho)/(3+rho)
def greedy_digits(x, base, k):
    out, rem = [], x
    p = base
    for _ in range(k):
        d = int(rem // p)
        out.append(d); rem -= d*p; p *= base
    return out, x - rem, rem
def S_of(rho): return 4*(3-rho)/(3+rho)
print("\n(B) k-digit truncations of rho_Q: |S - 2sqrt2|")
print(f"{'k':>3} {'silver err':>12} {'golden err':>12}")
tab = []
for k in (1,2,3,4,5,6,8,10,12):
    _, sapprox, srem = greedy_digits(RHO_Q, BETA, k)
    _, gapprox, grem = greedy_digits(RHO_Q, ALPHA_G, k)
    es = abs(S_of(sapprox := sapprox if False else sapprox) - TS) if False else abs(S_of(sapprox)-TS)
    es = abs(S_of(sapprox)-TS); eg = abs(S_of(gapprox)-TS)
    tab.append({"k":k,"silver":es,"golden":eg})
    print(f"{k:>3} {es:>12.3e} {eg:>12.3e}")
rows["digit_table"] = tab

# ---- (C) end-to-end silver-chain realization
def T_tri(d):
    dd = abs(math.remainder(d, 2*math.pi))
    return 1 - (2/math.pi)*dd
N = 1 << 23
d1, d2 = PELL[11], PELL[8]        # 13860, 985
m = 13; L = PELL[m]
delta = abs(L*BETA - round(L*BETA))
idx = np.arange(N, dtype=np.int64)
theta = (idx*BETA) % 1.0
thB = ((idx+L)*BETA) % 1.0
r = (d1*theta) % 1.0
Q = np.floor(4*((d2*theta) % 1.0)).astype(np.int8)
XAo = {0:0.0, 1:math.pi/2}; YBo = {0:math.pi/4, 1:-math.pi/4}
E = {}
for q,(a,b) in enumerate([(0,0),(0,1),(1,0),(1,1)]):
    d = XAo[a]-YBo[b]; C, T = math.cos(d), T_tri(d)
    kap = (C-T)/(1-C*T)
    # sanity: acceptance ratio (1-kap)/(1+kap) must equal RHO_Q (sign-adjusted)
    ratio = (1-abs(kap))/(1+abs(kap))
    assert abs(ratio - RHO_Q) < 1e-12, ratio
    A = np.sign(np.cos(2*math.pi*theta - XAo[a]))
    B = np.sign(np.cos(2*math.pi*thB - YBo[b]))
    AB = A*B
    acc = (Q==q) & (r < (1+kap*AB)/(1+abs(kap)))
    E[(a,b)] = float(AB[acc].mean())
S = E[(0,0)]+E[(0,1)]+E[(1,0)]-E[(1,1)]
print(f"\n(C) SILVER chain (beta = sqrt2-1, Pell strides {d2},{d1}, L=P_{m}={L},"
      f" delta={delta:.2e}):")
print(f"    acceptance ratio used = 9-6sqrt2 = {RHO_Q:.12f} (exact field constant)")
print(f"    S = {S:.5f}   |S-2sqrt2| = {abs(S-TS):.2e}")
rows["silver_run"] = {"S": S, "err": abs(S-TS), "L": L, "delta": delta}
json.dump(rows, open("w2_field_results.json","w"), indent=1)
print("saved -> w2_field_results.json")
