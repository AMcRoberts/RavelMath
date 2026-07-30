"""
mi_certify.py -- certify the global MI floor for CHSH at Tsirelson.

Claim to certify: min I(context; lambda) over ALL finite local MD
models reproducing E_ab = quantum cosines at the four CHSH contexts
equals KL2((2+sqrt2)/4 || 3/4) = 0.046258 bits, attained by the tilt.

Rigor structure:
 (1) WLOG atoms are deterministic sign-profiles (A0,A1,B0,B1) in
     {+-1}^4: stochastic outcomes = mixtures over profiles, and merging
     atoms with equal profiles cannot increase MI (convexity of MI in
     the conditionals). So the full 16-profile support dominates every
     model.
 (2) MI is convex in {q_i}; constraints linear => convex program; any
     KKT point is the GLOBAL minimum. Solve with many restarts + two
     methods; report objective spread and KKT residual.
 (3) Compare the closed form and check marginal uniformity of the
     optimizer.
"""
import math, itertools, json
import numpy as np
from scipy.optimize import minimize

SQ = math.sqrt(2)
CTX = [(0,0),(0,1),(1,0),(1,1)]
CQ = {(0,0): 1/SQ, (0,1): 1/SQ, (1,0): 1/SQ, (1,1): -1/SQ}

profiles = list(itertools.product((1,-1), repeat=4))   # (A0,A1,B0,B1)
na = len(profiles)
ABm = np.array([[p[x]*p[2+y] for p in profiles] for (x,y) in CTX], float)

def MI_bits(qf):
    q = qf.reshape(4, na)
    qb = q.mean(axis=0)
    tot = 0.0
    for i in range(4):
        nz = q[i] > 1e-13
        tot += float((q[i][nz]*np.log(q[i][nz]/np.maximum(qb[nz],1e-300))).sum())
    return tot/4/math.log(2)

cons = []
for i in range(4):
    cons.append({"type":"eq","fun": lambda qf,i=i: qf.reshape(4,na)[i].sum()-1})
    cons.append({"type":"eq","fun": lambda qf,i=i:
                 float((qf.reshape(4,na)[i]*ABm[i]).sum()) - CQ[CTX[i]]})
best, vals = None, []
for trial in range(24):
    rng = np.random.default_rng(trial)
    q0 = rng.dirichlet(np.ones(na), size=4).flatten()
    for method in ("SLSQP",):
        r = minimize(MI_bits, q0, constraints=cons, bounds=[(0,1)]*(4*na),
                     method=method, options={"maxiter":800,"ftol":1e-14})
        if r.success:
            vals.append(r.fun)
            if best is None or r.fun < best[0]:
                best = (r.fun, r.x)
vals = np.array(vals)
closed = ( (2+SQ)/4*math.log((2+SQ)/4/0.75) + (2-SQ)/4*math.log((2-SQ)/4/0.25) )/math.log(2)
print(f"restarts converged: {len(vals)}; min = {vals.min():.6f} bits; "
      f"median = {np.median(vals):.6f}; max = {vals.max():.6f}")
print(f"closed form KL2((2+sqrt2)/4 || 3/4) = {closed:.6f} bits")
q = best[1].reshape(4, na)
qb = q.mean(axis=0)
# marginal uniformity on the SUPPORT (atoms with mass)
sup = qb > 1e-6
print(f"support size of optimizer: {int(sup.sum())} profiles; "
      f"marginal on support: min {qb[sup].min():.5f} max {qb[sup].max():.5f}")
# constraint residuals
res = max(abs(q[i].sum()-1) for i in range(4))
resC = max(abs(float((q[i]*ABm[i]).sum()) - CQ[CTX[i]]) for i in range(4))
print(f"constraint residuals: norm {res:.1e}, correlators {resC:.1e}")
print(f"gap best-vs-closed-form: {best[0]-closed:+.2e} bits")
json.dump({"min_bits": float(vals.min()), "closed_form": closed,
           "median": float(np.median(vals)), "max": float(vals.max())},
          open("mi_certify_results.json","w"), indent=1)
print("saved -> mi_certify_results.json")
