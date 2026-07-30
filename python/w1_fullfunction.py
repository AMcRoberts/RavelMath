"""
w1_fullfunction.py -- W1: the full correlation function E(x,y)=cos(x-y).

(1) MI budget: avg_d KL2((1+cos d)/2 || (1+T(d))/2), uniform d. FINITE?
(2) Marginal-uniformity: on symmetric setting grids, do tilts cancel?
    (If yes, KL-to-uniform = true MI.)
(3) Full-M of the lawful tilt model on an 8x8 setting grid; compare the
    CHSH floor 2(sqrt2-1)/3 = 0.27614.
(4) Joint min-M LP on a 4x4 grid (all 16 contexts at quantum targets,
    shared conditionals): is 0.27614 still jointly attainable?
(5) Pointwise-vs-aggregate on the Fibonacci chain (prediction P2):
    per-angle |E - cos| scatter (first order) vs optimal-CHSH-combo
    error (second order), at one refinement.
"""
import math, json
import numpy as np
from scipy.optimize import linprog
from fibmodel import FIB, ALPHA, TS

rows = {}
def T_tri(d):
    dd = abs(math.remainder(d, 2*math.pi))
    return 1 - (2/math.pi)*dd
def KL2(a, b):
    def t(p,q): return 0.0 if p==0 else p*math.log(p/q)
    return (t(a,b)+t(1-a,1-b))/math.log(2)

# ---- (1) MI budget over the full function
ds = np.linspace(0, 2*math.pi, 20001)[:-1]
vals = np.array([KL2((1+math.cos(d))/2, (1+T_tri(d))/2) for d in ds])
MI_full = float(vals.mean())
print(f"(1) MI budget, uniform d: {MI_full:.5f} bits "
      f"(CHSH-4 was 0.04627; Hall singlet figure 1/15 = {1/15:.5f})")
print(f"    max over d: {vals.max():.5f} at d = {ds[vals.argmax()]:.4f} "
      f"(pi/4 = {math.pi/4:.4f})")
rows["MI_full_uniform_d"] = MI_full
rows["MI_max_d"] = float(vals.max())

# ---- (2) marginal uniformity on a symmetric grid
ngrid = 16
thg = np.linspace(0, 1, 4096, endpoint=False)
def polsignv(th, c):  # theta in [0,1)
    return np.where(np.cos(2*math.pi*th - c) >= 0, 1.0, -1.0)
tilt_sum = np.zeros_like(thg)
cnt = 0
for xi in range(ngrid):
    for yi in range(ngrid):
        x, y = 2*math.pi*xi/ngrid, 2*math.pi*yi/ngrid + math.pi/ngrid
        d = x - y
        C, T = math.cos(d), T_tri(d)
        kap = (C-T)/(1-C*T)
        tilt_sum += kap*polsignv(thg,x)*polsignv(thg,y)
        cnt += 1
dev = np.abs(tilt_sum/cnt)
print(f"(2) marginal deviation from uniform on {ngrid}x{ngrid} grid: "
      f"max |avg tilt| = {dev.max():.2e}  (0 => KL-to-uniform = true MI)")
rows["marginal_max_dev"] = float(dev.max())

# ---- (3) full-M of the tilt model on 8x8 grid
nA = 8
ctxs = []
for xi in range(nA):
    for yi in range(nA):
        ctxs.append((2*math.pi*xi/nA, 2*math.pi*yi/nA + math.pi/nA))
dens = []
for (x,y) in ctxs:
    d = x-y; C, T = math.cos(d), T_tri(d)
    kap = (C-T)/(1-C*T)
    w = 1 + kap*polsignv(thg,x)*polsignv(thg,y)
    dens.append(w/w.sum())
dens = np.array(dens)
Mfull = 0.0
for i in range(len(dens)):
    dif = np.abs(dens[i+1:] - dens[i]).sum(axis=1)
    Mfull = max(Mfull, float(dif.max()))
print(f"(3) tilt-model full M (sup pairwise TV) on 8x8 grid: {Mfull:.5f} "
      f"(CHSH floor 0.27614)")
rows["tilt_full_M_8x8"] = Mfull

# ---- (4) joint min-M LP on 4x4 grid
xs = [0, math.pi/4, math.pi/2, 3*math.pi/4]
ys = [math.pi/8 + k*math.pi/4 for k in range(4)]
ctx4 = [(x,y) for x in xs for y in ys]
# atoms: circle split by all window boundaries (x±pi/2, y±pi/2)/2pi
bset = set()
for a in xs+ys:
    for s in (0.25, 0.75):
        bset.add(round((a/(2*math.pi)+s) % 1.0, 12))
bps = sorted(bset); bps.append(bps[0]+1.0)
mids = [ (bps[i]+bps[i+1])/2 % 1.0 for i in range(len(bps)-1)]
wts  = [ bps[i+1]-bps[i] for i in range(len(bps)-1)]
natoms = len(mids)
ABn = np.zeros((16, natoms))
for i,(x,y) in enumerate(ctx4):
    for l,m in enumerate(mids):
        ABn[i,l] = (1 if math.cos(2*math.pi*m-x)>=0 else -1) * \
                   (1 if math.cos(2*math.pi*m-y)>=0 else -1)
nq = 16*natoms
pairs = [(i,j) for i in range(16) for j in range(i+1,16)]
nu = len(pairs)*natoms
NV = nq + nu + 1
c = np.zeros(NV); c[-1] = 1
A_eq, b_eq = [], []
for i,(x,y) in enumerate(ctx4):
    r = np.zeros(NV); r[i*natoms:(i+1)*natoms] = 1
    A_eq.append(r); b_eq.append(1.0)
    r = np.zeros(NV); r[i*natoms:(i+1)*natoms] = ABn[i]
    A_eq.append(r); b_eq.append(math.cos(x-y))
A_ub, b_ub = [], []
for pi,(i,j) in enumerate(pairs):
    for l in range(natoms):
        for sgn in (1,-1):
            r = np.zeros(NV)
            r[i*natoms+l] = sgn; r[j*natoms+l] = -sgn; r[nq+pi*natoms+l] = -1
            A_ub.append(r); b_ub.append(0.0)
    r = np.zeros(NV); r[nq+pi*natoms:nq+(pi+1)*natoms] = 1; r[-1] = -1
    A_ub.append(r); b_ub.append(0.0)
res = linprog(c, A_ub=np.array(A_ub), b_ub=np.array(b_ub),
              A_eq=np.array(A_eq), b_eq=np.array(b_eq),
              bounds=[(0,None)]*(NV-1)+[(0,None)], method="highs")
print(f"(4) joint min-M over 16 contexts (4x4 grid, quantum targets): "
      f"M* = {res.x[-1]:.5f}  ({natoms} atoms)")
rows["joint_minM_4x4"] = float(res.x[-1])

# ---- (5) pointwise vs aggregate on the Fibonacci chain
N = 1 << 23
p = 16; d1, d2 = FIB[p], FIB[p-1]
L = FIB[22]
idx = np.arange(N, dtype=np.int64)
theta = (idx*ALPHA) % 1.0
thB = ((idx+L)*ALPHA) % 1.0
r = (d1*theta) % 1.0
octant = np.floor(8*((d2*theta) % 1.0)).astype(np.int8)
dlist = [(k+1)*math.pi/9 for k in range(8)]     # 8 relative angles, no CHSH tuning
Es, errs = [], []
for c8, d in enumerate(dlist):
    x, y = 0.0, -d
    C, T = math.cos(d), T_tri(d)
    kap = (C-T)/(1-C*T)
    A = np.sign(np.cos(2*math.pi*theta - x))
    B = np.sign(np.cos(2*math.pi*thB - y))
    AB = A*B
    acc = (octant == c8) & (r < (1+kap*AB)/(1+abs(kap)))
    E = float(AB[acc].mean())
    Es.append(E); errs.append(E - C)
    print(f"(5) d={d:.3f}: E = {E:+.5f}  cos d = {C:+.5f}  err = {E-C:+.2e}")
print(f"    pointwise |err|: mean {np.mean(np.abs(errs)):.2e}  "
      f"max {np.max(np.abs(errs)):.2e}")
rows["pointwise_errs"] = errs
json.dump(rows, open("w1_results.json","w"), indent=1)
print("saved -> w1_results.json")
