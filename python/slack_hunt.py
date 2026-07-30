"""
slack_hunt.py -- is our tilt model optimal in Hall's M currency? No:
Hall floor at C=2sqrt2 is M=2(sqrt2-1)/3=0.27614. Measure our M, LP the
fixed-outcome class, then realize the saturating Kimura Table III model
canonically on the Fibonacci chain.
"""
import math, json
import numpy as np
from scipy.optimize import linprog, minimize
from fibmodel import QUANTUM, TRIANGLE, FIB, ALPHA, TS, XA, YB

SQ = math.sqrt(2)
CTX = [(0,0),(0,1),(1,0),(1,1)]
rows = {}

# ---------- 8-atom reduction: eighth cells, polarizer signs ----------
def polsign(cell, center):   # sign(cos(2pi theta - center)) at cell midpoint
    th = 2*math.pi*(cell + 0.5)/8
    return 1.0 if math.cos(th - center) >= 0 else -1.0
Asig = {x: np.array([polsign(c, XA[x]) for c in range(8)]) for x in (0,1)}
Bsig = {y: np.array([polsign(c, YB[y]) for c in range(8)]) for y in (0,1)}
ABp  = {i: Asig[x]*Bsig[y] for i,(x,y) in enumerate(CTX)}

# ---------- (a) tilt model on 8 atoms ----------
q_tilt = np.zeros((4,8))
for i,(x,y) in enumerate(CTX):
    T = TRIANGLE[(x,y)]; C = QUANTUM[(x,y)]
    kap = (C-T)/(1-C*T)
    w = 1 + kap*ABp[i]
    q_tilt[i] = w/ w.sum()
def M_of(q):
    return max(np.abs(q[i]-q[j]).sum() for i in range(4) for j in range(i+1,4))
def MI_of(q):     # avg KL to the marginal, bits
    qbar = q.mean(axis=0)
    kl = 0.0
    for i in range(4):
        nz = q[i] > 0
        kl += (q[i][nz]*np.log(q[i][nz]/qbar[nz])).sum()
    return kl/4/math.log(2)
print(f"(a) tilt model:      M = {M_of(q_tilt):.5f}   MI = {MI_of(q_tilt):.5f} bits")
print(f"    Hall floor at C=2sqrt2: M = {2*(SQ-1)/3:.5f}")
rows["tilt"] = {"M": M_of(q_tilt), "MI_bits": MI_of(q_tilt)}
# check correlators
for i,(x,y) in enumerate(CTX):
    assert abs((q_tilt[i]*ABp[i]).sum() - QUANTUM[(x,y)]) < 1e-9

# ---------- (b) LP: min M, fixed polarizer outcomes ----------
# vars: q[4*8]=32, u[6 pairs * 8]=48, M -> total 81
nq, pairs = 32, [(i,j) for i in range(4) for j in range(i+1,4)]
nu = len(pairs)*8
NV = nq + nu + 1
c = np.zeros(NV); c[-1] = 1.0
A_eq, b_eq = [], []
for i in range(4):                       # normalization
    r = np.zeros(NV); r[i*8:(i+1)*8] = 1; A_eq.append(r); b_eq.append(1.0)
for i,(x,y) in enumerate(CTX):           # correlators
    r = np.zeros(NV); r[i*8:(i+1)*8] = ABp[i]; A_eq.append(r); b_eq.append(QUANTUM[(x,y)])
A_ub, b_ub = [], []
for pi,(i,j) in enumerate(pairs):
    for l in range(8):
        for sgn in (+1,-1):              # u >= sgn*(q_i - q_j)
            r = np.zeros(NV)
            r[i*8+l] = sgn; r[j*8+l] = -sgn; r[nq+pi*8+l] = -1
            A_ub.append(r); b_ub.append(0.0)
    r = np.zeros(NV); r[nq+pi*8:nq+(pi+1)*8] = 1; r[-1] = -1
    A_ub.append(r); b_ub.append(0.0)     # sum_l u <= M
res = linprog(c, A_ub=np.array(A_ub), b_ub=np.array(b_ub),
              A_eq=np.array(A_eq), b_eq=np.array(b_eq),
              bounds=[(0,None)]*nq + [(0,None)]*nu + [(0,None)],
              method="highs")
q_lp = res.x[:32].reshape(4,8)
print(f"(b) LP (fixed polarizer outcomes): M* = {res.x[-1]:.5f}   "
      f"MI = {MI_of(q_lp):.5f} bits")
rows["lp_fixed_outcomes"] = {"M": float(res.x[-1]), "MI_bits": MI_of(q_lp)}

# ---------- (b2) convex: min MI, fixed polarizer outcomes ----------
def negMI_obj(qflat):
    q = qflat.reshape(4,8)
    return MI_of(np.clip(q,1e-12,None))
cons = []
for i in range(4):
    cons.append({"type":"eq","fun": (lambda qf,i=i: qf.reshape(4,8)[i].sum()-1)})
for i,(x,y) in enumerate(CTX):
    cons.append({"type":"eq","fun": (lambda qf,i=i,x=x,y=y:
                 (qf.reshape(4,8)[i]*ABp[i]).sum()-QUANTUM[(x,y)])})
r2 = minimize(negMI_obj, q_tilt.flatten(), constraints=cons,
              bounds=[(1e-12,1)]*32, method="SLSQP",
              options={"maxiter":500,"ftol":1e-12})
q_mi = r2.x.reshape(4,8)
print(f"(b2) min-MI (fixed outcomes): MI = {MI_of(q_mi):.5f} bits   "
      f"M = {M_of(q_mi):.5f}")
rows["minMI_fixed_outcomes"] = {"M": M_of(q_mi), "MI_bits": MI_of(q_mi)}

# ---------- (c) Kimura Table III realized on the Fibonacci chain ----------
p = (SQ-1)/3
P = np.array([[0,p,p,p],[p,0,p,p],[p,p,0,p],[1-2*p,1-2*p,1-2*p,1-3*p]]).T  # P[i][lam]
OUT = {0: (+1,-1,-1,+1), 1: (+1,+1,+1,-1), 2: (+1,-1,+1,+1), 3: (+1,+1,+1,+1)}
# analytic check
C_an = 0.0
for i,(x,y) in enumerate(CTX):
    s = 1 if i<3 else -1
    C_an += s*sum(P[i][l]*OUT[l][x]*OUT[l][2+y] for l in range(4))
M_an = max(np.abs(P[i]-P[j]).sum() for i in range(4) for j in range(i+1,4))
qk = P.copy()
print(f"(c) Table III analytic: C = {C_an:.5f}  M = {M_an:.5f}  "
      f"MI = {MI_of(qk):.5f} bits")
rows["tableIII_analytic"] = {"C": C_an, "M": M_an, "MI_bits": MI_of(qk)}

# realize deterministically: lambda-cell = quarter of {d0 theta};
# context = quarter of {d2 theta}; acceptance sawtooth {d1 theta} with
# weight P_i(lam)/max_lam P_i(lam)  -> conditional dist = P_i.  outcomes
# per (cell, setting) from OUT. All window functions of theta. L = F_22.
N = 1 << 23
d0, d2, d1 = FIB[12], FIB[15], FIB[18]
L = FIB[22]
idx = np.arange(N, dtype=np.int64)
theta = (idx*ALPHA) % 1.0
thB   = ((idx+L)*ALPHA) % 1.0
lam  = np.floor(4*((d0*theta) % 1.0)).astype(np.int8)
lamB = np.floor(4*((d0*thB) % 1.0)).astype(np.int8)
ctx  = np.floor(4*((d2*theta) % 1.0)).astype(np.int8)
r    = (d1*theta) % 1.0
OUTA = np.array([[OUT[l][x] for x in (0,1)] for l in range(4)], dtype=np.float64)
OUTB = np.array([[OUT[l][2+y] for y in (0,1)] for l in range(4)], dtype=np.float64)
E, q_meas = {}, np.zeros((4,4))
for i,(x,y) in enumerate(CTX):
    wmax = P[i].max()
    acc = (ctx == i) & (r < P[i][lam]/wmax)
    A = OUTA[lam[acc], x]
    B = OUTB[lamB[acc], y]
    E[(x,y)] = float((A*B).mean())
    cnt = np.bincount(lam[acc], minlength=4).astype(float)
    q_meas[i] = cnt/cnt.sum()
C_meas = E[(0,0)]+E[(0,1)]+E[(1,0)]-E[(1,1)]
M_meas = max(np.abs(q_meas[i]-q_meas[j]).sum() for i in range(4) for j in range(i+1,4))
print(f"    Fibonacci realization: C = {C_meas:.5f}  M = {M_meas:.5f}  "
      f"MI = {MI_of(q_meas):.5f} bits")
print(f"    correlators: {[round(E[k],5) for k in CTX]}")
rows["tableIII_fibonacci"] = {"C": C_meas, "M": M_meas, "MI_bits": MI_of(q_meas),
                              "E": {str(k): v for k,v in E.items()}}

json.dump(rows, open("slack_hunt_results.json","w"), indent=1)
print("\nsaved -> slack_hunt_results.json")
