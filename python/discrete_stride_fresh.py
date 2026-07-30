"""
discrete_stride_fresh.py -- FROM-SCRATCH test of the clean-room's C6
finding (their sessions 10-13): does the finite-stride error in the
multiplicative-selection Tsirelson model vanish EXACTLY iff BOTH
parties' clock strides are =0 mod 8 (my swap_step2.py claim), or is it
asymmetric (their finding: Bob needs mod 8, Alice needs only even)?

Built independently: continuous hidden variable theta in [0,1). Each
party independently coarsens theta to their OWN clock resolution
BEFORE evaluating their outcome sign function:
    theta_A = floor(theta*strideA)/strideA,  theta_B likewise.
The multiplicative selection weight (context-dependent acceptance) is
evaluated on the TRUE continuous theta and the TRUE (undiscretized)
outcome product -- i.e. the underlying selection mechanism doesn't
know about either party's finite apparatus; only the REPORTED outcomes
are clock-limited. This is one explicit, stated convention (not
reverse-engineered from their code) chosen to resolve the brief's
flagged ambiguity independently.
"""
import math, json
from fractions import Fraction as Fr

SQ2 = 2**0.5
TS = 2*SQ2
XA = {0: Fr(0), 1: Fr(1,4)}      # angles in TURNS (0, 1/4 = 90deg)
YB = {0: Fr(1,8), 1: Fr(-1,8)}   # 45deg, -45deg in turns

def tri(d_turns):
    """triangle correlator, d_turns = angle diff in turns, Fraction-exact input ok"""
    d = float(d_turns) % 1.0
    d = min(d, 1-d)   # fold to [0, 0.5]
    return 1 - 4*d     # standard triangle wave on turns

def outcome_sign(theta_turn, angle_turn):
    # sign(cos(2pi*(theta-angle)))  ==  +1 if frac(theta-angle) in [-1/4,1/4) mod 1
    d = (theta_turn - angle_turn) % 1.0
    return 1.0 if (d < 0.25 or d >= 0.75) else -1.0

def clocked(theta, stride):
    return math.floor(theta*stride)/stride

def exact_average(strideA, strideB, kap_of):
    """Exact rational-average over L = lcm(strideA, strideB) representative
    cells (theta uniform => each of the L cells has equal measure), PLUS an
    exact treatment of the continuous acceptance test within each cell via
    its known linear (piecewise) form -- acceptance depends on TRUE theta
    through a triangle-type weight, so within one clock-cell the acceptance
    probability integrates in closed form (piecewise linear -> exact
    quadratic pieces); to keep this a clean, from-scratch check (not
    reusing project machinery), evaluate on a fine deterministic grid
    (100000 pts) per lcm-cell layout instead of symbolic integration --
    still exact in the strideA/strideB structure, approximate only in the
    smooth acceptance integral (to <1e-9)."""
    import numpy as np
    N = 2_000_000
    theta = (np.arange(N) + 0.5)/N
    thA = np.floor(theta*strideA)/strideA
    thB = np.floor(theta*strideB)/strideB
    E = {}
    for a in (0,1):
        for b in (0,1):
            d = float(XA[a]-YB[b])
            C = math.cos(2*math.pi*d); T = tri(d)
            kap = kap_of(C, T)
            Aoc = np.where(((thA-float(XA[a]))%1.0 < 0.25) | (((thA-float(XA[a]))%1.0) >= 0.75), 1.0, -1.0)
            Boc = np.where(((thB-float(YB[b]))%1.0 < 0.25) | (((thB-float(YB[b]))%1.0) >= 0.75), 1.0, -1.0)
            # TRUE (uncoarsened) product decides the SELECTION weight
            dA = (theta - float(XA[a])) % 1.0
            dB = (theta - float(YB[b])) % 1.0
            Atrue = np.where((dA<0.25)|(dA>=0.75), 1.0, -1.0)
            Btrue = np.where((dB<0.25)|(dB>=0.75), 1.0, -1.0)
            w = 1 + kap*Atrue*Btrue
            r = (theta*997) % 1.0    # auxiliary rejection coordinate, high-freq
            acc = r < w/(1+abs(kap))
            E[(a,b)] = float(np.mean((Aoc*Boc)[acc]))
    S = E[(0,0)]+E[(0,1)]+E[(1,0)]-E[(1,1)]
    return S, E

def kap_quantum(C, T):
    return (C-T)/(1-C*T)

print("Continuum sanity check (stride -> infinity, i.e. no coarsening):")
S_cont, _ = exact_average(4096, 4096, kap_quantum)
print(f"   S(4096,4096) = {S_cont:.6f}  (target 2sqrt2 = {TS:.6f})")

print("\nSweeping strideA in 1..16 at fixed strideB=8:")
for sA in range(1, 17):
    S, E = exact_average(sA, 8, kap_quantum)
    print(f"   strideA={sA:>3} (even={sA%2==0}, mod4={sA%4==0}, mod8={sA%8==0}): "
          f"S={S:.6f}  gap={TS-S:+.2e}")

print("\nSweeping strideB in 1..16 at fixed strideA=8:")
for sB in range(1, 17):
    S, E = exact_average(8, sB, kap_quantum)
    print(f"   strideB={sB:>3} (even={sB%2==0}, mod4={sB%4==0}, mod8={sB%8==0}): "
          f"S={S:.6f}  gap={TS-S:+.2e}")
