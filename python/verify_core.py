"""
verify_core.py -- independent verification of the central quantitative
result everything rests on, by exact identification with a PUBLISHED
analytic model.

Claim: our tilt model at the CHSH angles IS Hall-Branciard PRA 102,
052228 (2020) Table I (their proven-optimal retrocausal model), exactly.
If so, every downstream number (MI = 0.046274 bits, correlators, S)
inherits their analytic proof -- the strongest possible independent
check, no shared code with our chain implementations.

HB Table I at S = 2sqrt2:  p := (4-S)/8 = (2-sqrt2)/4.
Per context, conditional over 4 atoms: one atom gets p, three get
(1-p)/3.  MI = 2 - h(p) - (1-p) log2 3.

Our tilt: per context, conditional over the 4 outcome-profile atoms
(each of window measure 1/4): the unique AB=-1 profile gets
(1-k)/(4(1+kT)), the three AB=+1 profiles get (1+k)/(4(1+kT)),
with k = (C-T)/(1-CT), C = 1/sqrt2, T = 1/2.

Identity to verify exactly:
   (1-k)/(4(1+kT)) == p   and   (1+k)/(4(1+kT)) == (1-p)/3.
"""
import math
from fractions import Fraction

SQ = math.sqrt(2)

# exact arithmetic in Q(sqrt2): (a, b) = a + b*sqrt2, Fractions
def mul(u, v): return (u[0]*v[0] + 2*u[1]*v[1], u[0]*v[1] + u[1]*v[0])
def div(u, v):
    den = v[0]*v[0] - 2*v[1]*v[1]
    n = mul(u, (v[0], -v[1]))
    return (n[0]/den, n[1]/den)
def add(u, v): return (u[0]+v[0], u[1]+v[1])
F = Fraction
ONE = (F(1), F(0))
C = (F(0), F(1,2))                 # 1/sqrt2 = sqrt2/2
T = (F(1,2), F(0))
k = div(add(C, (-T[0], F(0))), add(ONE, (mul(C, T)[0]*-1, -mul(C, T)[1])))
# k = (C - T)/(1 - C T)
num = add(C, (-F(1,2), F(0)))
den = add(ONE, (-mul(C, T)[0], -mul(C, T)[1]))
k = div(num, den)
onepk  = add(ONE, k)
onemk  = add(ONE, (-k[0], -k[1]))
onepkT = add(ONE, mul(k, T))
q_special = div(onemk, (4*onepkT[0], 4*onepkT[1]))
q_other   = div(onepk, (4*onepkT[0], 4*onepkT[1]))

p_HB = ( (F(2), F(-1)) )           # (2 - sqrt2)/4 -> represent as (2,-1)/4
p_HB = (F(1,2), F(-1,4))           # 2/4 - (1/4) sqrt2
one_minus_p_over3 = ((F(1)-p_HB[0])/3, (-p_HB[1])/3)

print("exact identification with Hall-Branciard Table I (S = 2sqrt2):")
print(f"  tilt special atom  = {q_special[0]} + {q_special[1]}*sqrt2")
print(f"  HB p               = {p_HB[0]} + {p_HB[1]}*sqrt2   "
      f"MATCH: {q_special == p_HB}")
print(f"  tilt other atoms   = {q_other[0]} + {q_other[1]}*sqrt2")
print(f"  HB (1-p)/3         = {one_minus_p_over3[0]} + {one_minus_p_over3[1]}*sqrt2   "
      f"MATCH: {q_other == one_minus_p_over3}")

# numeric MI from HB closed form vs our certified value
p = (2 - SQ)/4
h = -p*math.log2(p) - (1-p)*math.log2(1-p)
MI_HB = 2 - h - (1-p)*math.log2(3)
print(f"  HB closed-form MI  = {MI_HB:.6f} bits   (our certified: 0.046274)")

# independent A2 re-check: pure-measure model, fresh finite differences
def T_tri(d):
    dd = abs(math.remainder(d, 2*math.pi))
    return 1 - (2/math.pi)*dd
XA = {0:0.0, 1:math.pi/2}; YB = {0:math.pi/4, 1:-math.pi/4}
def S_of_delta(dlt):
    S = 0.0
    for sgn, (a, b) in zip((1,1,1,-1), [(0,0),(0,1),(1,0),(1,1)]):
        d0 = XA[a]-YB[b]
        Cq = math.cos(d0); T0 = T_tri(d0)
        kap = (Cq-T0)/(1-Cq*T0)
        Td = T_tri(d0 - 2*math.pi*dlt)
        S += sgn*(Td+kap)/(1+kap*Td)
    return S
S0 = S_of_delta(0.0)
for dlt in (1e-3, 3e-3, 1e-2):
    A2 = (S_of_delta(dlt) + S_of_delta(-dlt) - 2*S0)/dlt**2
    print(f"  A2 (pure measure, fresh FD, delta={dlt:.0e}): {A2:+.4f}  "
          f"(closed form -11.7821)")
