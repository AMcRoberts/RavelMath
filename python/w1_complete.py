"""W1 completion: (1) MI budget integral; (4') joint min-M on a grid
CONTAINING the CHSH quadruple; (5') pointwise-vs-aggregate contrast
at m=8 with 8|-grid strides (prediction P2 demonstration)."""
import math, json
import numpy as np
from scipy.optimize import linprog
from fibmodel import FIB, ALPHA, TS

rows = {}
def T_tri(d):
    dd = abs(math.remainder(d, 2*math.pi))
    return 1 - (2/math.pi)*dd
def KL2(a, b):
    def t(p,q): return 0.0 if p<=0 else p*math.log(p/q)
    return (t(a,b)+t(1-a,1-b))/math.log(2)

# (1) MI budget for the full correlation function
ds = np.linspace(0, 2*math.pi, 20001)[:-1]
vals = np.array([KL2((1+math.cos(d))/2, (1+T_tri(d))/2) for d in ds])
print(f"(1) MI budget uniform-d: {vals.mean():.5f} bits  "
      f"max {vals.max():.5f} at d={ds[vals.argmax()]:.4f} (pi/4={math.pi/4:.4f}; "
      f"CHSH-4: 0.04627, Hall 1/15={1/15:.5f})")
rows["MI_full"] = float(vals.mean()); rows["MI_max"] = float(vals.max())

# (4') joint min-M, grid containing the CHSH quadruple:
xs = [0, math.pi/2, math.pi/4, 3*math.pi/4]
ys = [math.pi/4, -math.pi/4, 0, math.pi/2]
ctx = [(x,y) for x in xs for y in ys]
bset = set()
for a in set(xs+ys):
    for s in (0.25, 0.75):
        bset.add(round(((a/(2*math.pi))+s) % 1.0, 12))
bps = sorted(bset); bps.append(bps[0]+1.0)
mids = [(bps[i]+bps[i+1])/2 % 1.0 for i in range(len(bps)-1)]
na = len(mids)
ABn = np.zeros((len(ctx), na))
for i,(x,y) in enumerate(ctx):
    for l,m in enumerate(mids):
        ABn[i,l] = (1 if math.cos(2*math.pi*m-x)>=0 else -1)*\
                   (1 if math.cos(2*math.pi*m-y)>=0 else -1)
nc = len(ctx); nq = nc*na
pairs = [(i,j) for i in range(nc) for j in range(i+1,nc)]
NV = nq + len(pairs)*na + 1
c = np.zeros(NV); c[-1] = 1
A_eq, b_eq, A_ub, b_ub = [], [], [], []
for i,(x,y) in enumerate(ctx):
    r = np.zeros(NV); r[i*na:(i+1)*na] = 1; A_eq.append(r); b_eq.append(1.0)
    r = np.zeros(NV); r[i*na:(i+1)*na] = ABn[i]; A_eq.append(r); b_eq.append(math.cos(x-y))
for pi,(i,j) in enumerate(pairs):
    for l in range(na):
        for sgn in (1,-1):
            r = np.zeros(NV); r[i*na+l]=sgn; r[j*na+l]=-sgn; r[nq+pi*na+l]=-1
            A_ub.append(r); b_ub.append(0.0)
    r = np.zeros(NV); r[nq+pi*na:nq+(pi+1)*na]=1; r[-1]=-1
    A_ub.append(r); b_ub.append(0.0)
res = linprog(c, A_ub=np.array(A_ub), b_ub=np.array(b_ub),
              A_eq=np.array(A_eq), b_eq=np.array(b_eq),
              bounds=[(0,None)]*(NV-1)+[(0,None)], method="highs")
print(f"(4') joint min-M, 16 contexts INCLUDING CHSH quadruple: "
      f"M* = {res.x[-1]:.5f}  (floor 0.27614; equality => extra contexts free)")
rows["joint_minM_with_quadruple"] = float(res.x[-1])

# (5') pointwise (first-order) vs aggregate (second-order) contrast
N = 1 << 23
d1, d2 = FIB[22], FIB[16]           # both 8|, kills stride floor
m = 8; L = FIB[m]                    # delta ~ 8.1e-3, above floors
delta = abs(L*ALPHA - round(L*ALPHA))
idx = np.arange(N, dtype=np.int64)
theta = (idx*ALPHA) % 1.0
thB = ((idx+L)*ALPHA) % 1.0
r = (d1*theta) % 1.0
Q = np.floor(4*((d2*theta) % 1.0)).astype(np.int8)
XAo = {0:0.0, 1:math.pi/2}; YBo = {0:math.pi/4, 1:-math.pi/4}
E = {}
for q,(a,b) in enumerate([(0,0),(0,1),(1,0),(1,1)]):
    d = XAo[a]-YBo[b]; C, T = math.cos(d), T_tri(d)
    kap = (C-T)/(1-C*T)
    A = np.sign(np.cos(2*math.pi*theta - XAo[a]))
    B = np.sign(np.cos(2*math.pi*thB - YBo[b]))
    AB = A*B
    acc = (Q==q) & (r < (1+kap*AB)/(1+abs(kap)))
    E[(a,b)] = float(AB[acc].mean())
perr = [abs(E[k] - (1/math.sqrt(2) if k!=(1,1) else -1/math.sqrt(2)))
        for k in [(0,0),(0,1),(1,0),(1,1)]]
S = E[(0,0)]+E[(0,1)]+E[(1,0)]-E[(1,1)]
print(f"(5') m={m} (delta={delta:.2e}): pointwise |E-cos| = "
      f"{[f'{e:.2e}' for e in perr]}")
print(f"     aggregate |S-2sqrt2| = {abs(S-TS):.2e}   "
      f"contrast ratio = {np.mean(perr)/max(abs(S-TS),1e-12):.1f}x")
rows["P2_pointwise"] = perr; rows["P2_aggregate"] = abs(S-TS)
json.dump(rows, open("w1_complete_results.json","w"), indent=1)
print("saved -> w1_complete_results.json")
