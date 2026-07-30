"""Closing checks: (i) effective hiddenness of the tilt model;
(ii) min-MI with FREE outcomes (Table-III outcome structure + optimized
probabilities; and random outcome-pattern search on 4..6 atoms);
(iii) refined-stride Fibonacci realization of Table III."""
import math, json, itertools
import numpy as np
from scipy.optimize import minimize
from fibmodel import QUANTUM, TRIANGLE, FIB, ALPHA, TS, XA, YB

SQ = math.sqrt(2); CTX = [(0,0),(0,1),(1,0),(1,1)]
rows = {}

def polsign(cell, center, n=8):
    th = 2*math.pi*(cell + 0.5)/n
    return 1.0 if math.cos(th - center) >= 0 else -1.0
Asig = {x: np.array([polsign(c, XA[x]) for c in range(8)]) for x in (0,1)}
Bsig = {y: np.array([polsign(c, YB[y]) for c in range(8)]) for y in (0,1)}

# (i) effective hiddenness of tilt model: distinct AB-sign profiles
profiles = set()
for c in range(8):
    profiles.add(tuple(Asig[x][c]*Bsig[y][c] for x,y in CTX))
print(f"(i) tilt model: distinct atom profiles = {len(profiles)} "
      f"-> effective H = {len(profiles)-1}")
rows["tilt_effective_H"] = len(profiles)-1

def MI_of(q):
    qbar = q.mean(axis=0); kl = 0.0
    for i in range(len(q)):
        nz = q[i] > 1e-14
        kl += (q[i][nz]*np.log(q[i][nz]/qbar[nz])).sum()
    return kl/4/math.log(2)

# (ii) min-MI with free outcomes: outcomes per atom are sign quadruples
# (A0,A1,B0,B1); AB profile per ctx = A_x*B_y. Search small atom counts.
def minMI_for(patterns):
    na = len(patterns)
    ABm = np.array([[pt[x]*pt[2+y] for pt in patterns] for x,y in CTX])
    def obj(qf):
        return MI_of(np.clip(qf.reshape(4,na),1e-14,None))
    cons = []
    for i in range(4):
        cons.append({"type":"eq","fun": lambda qf,i=i: qf.reshape(4,na)[i].sum()-1})
        cons.append({"type":"eq","fun": lambda qf,i=i:
                     (qf.reshape(4,na)[i]*ABm[i]).sum()-QUANTUM[CTX[i]]})
    best = None
    for trial in range(4):
        rng = np.random.default_rng(trial)
        q0 = rng.dirichlet(np.ones(na), size=4).flatten()
        r = minimize(obj, q0, constraints=cons, bounds=[(1e-14,1)]*(4*na),
                     method="SLSQP", options={"maxiter":400,"ftol":1e-12})
        if r.success and (best is None or r.fun < best):
            best = r.fun
    return best

signq = [s for s in itertools.product((1,-1), repeat=4)]
# Table III outcome set:
tab3 = [(1,-1,-1,1),(1,1,1,-1),(1,-1,1,1),(1,1,1,1)]
v = minMI_for(tab3)
print(f"(ii) min MI, Table-III outcomes, free probs: {v:.5f} bits "
      f"(tilt model: 0.04627)")
rows["minMI_tab3_outcomes"] = v
# random 5-atom outcome sets
rng = np.random.default_rng(0)
best5 = None
for _ in range(40):
    pats = [signq[i] for i in rng.choice(16, size=5, replace=False)]
    v = minMI_for(pats)
    if v is not None and (best5 is None or v < best5):
        best5 = v
print(f"     best over 40 random 5-atom outcome sets: {best5:.5f} bits")
rows["minMI_random5"] = best5

# (iii) refined strides for Table III realization
p = (SQ-1)/3
P = np.array([[0,p,p,p],[p,0,p,p],[p,p,0,p],[1-2*p,1-2*p,1-2*p,1-3*p]]).T
OUT = {0:(+1,-1,-1,+1),1:(+1,+1,+1,-1),2:(+1,-1,+1,+1),3:(+1,+1,+1,+1)}
N = 1 << 23
for d0i, d2i, d1i in ((12,15,18),(14,17,20),(16,19,22)):
    d0, d2, d1 = FIB[d0i], FIB[d2i], FIB[d1i]
    L = FIB[24]
    idx = np.arange(N, dtype=np.int64)
    theta = (idx*ALPHA) % 1.0
    thB = ((idx+L)*ALPHA) % 1.0
    lam  = np.floor(4*((d0*theta) % 1.0)).astype(np.int8)
    lamB = np.floor(4*((d0*thB)  % 1.0)).astype(np.int8)
    ctx  = np.floor(4*((d2*theta) % 1.0)).astype(np.int8)
    r    = (d1*theta) % 1.0
    OUTA = np.array([[OUT[l][x] for x in (0,1)] for l in range(4)], float)
    OUTB = np.array([[OUT[l][2+y] for y in (0,1)] for l in range(4)], float)
    E, q_meas = {}, np.zeros((4,4))
    for i,(x,y) in enumerate(CTX):
        acc = (ctx == i) & (r < P[i][lam]/P[i].max())
        E[(x,y)] = float((OUTA[lam[acc],x]*OUTB[lamB[acc],y]).mean())
        cnt = np.bincount(lam[acc], minlength=4).astype(float)
        q_meas[i] = cnt/cnt.sum()
    C = E[(0,0)]+E[(0,1)]+E[(1,0)]-E[(1,1)]
    M = max(np.abs(q_meas[i]-q_meas[j]).sum() for i in range(4) for j in range(i+1,4))
    print(f"(iii) strides F_{d0i},F_{d2i},F_{d1i}: C = {C:.5f} "
          f"(gap {TS-C:+.5f})  M = {M:.5f}  MI = {MI_of(q_meas):.5f}")
    rows[f"tab3_fib_{d0i}"] = {"C": C, "M": M}
json.dump(rows, open("slack_close_results.json","w"), indent=1)
print("saved -> slack_close_results.json")
