"""
tsirelson_family2.py -- two-parameter family, clean separation of scales.

Family index (p, m):
  p = refinement level of the selection/settings window partitions
      (strides d1 = F_p, d2 = F_{p-1}: all partitions are unions of
       Fibonacci-sawtooth cells -- canonical, nothing hand-placed)
  m = return-vector level, separation L = F_m, delta_m = ||F_m alpha|| -> 0

Claim: S(p, m) -> 2*sqrt(2) as m -> inf at fixed large p, with the
residual plateau gap -> 0 as p -> inf (and N >> F_p).
Errors: |S - 2sqrt2| <= C1*delta_m + C2*(cross-scale discrepancy at p) + O(sqrt(F_p/N)).
Also: empirical measurement-dependence cost TV(rho(lambda|ab), uniform).
"""
import json, math
import numpy as np

ALPHA = (math.sqrt(5) - 1) / 2
TS = 2 * math.sqrt(2)
XA = {0: 0.0, 1: math.pi/2}
YB = {0: math.pi/4, 1: -math.pi/4}
FIB = [1,2,3,5,8,13,21,34,55,89,144,233,377,610,987,1597,2584,4181,6765,
       10946,17711,28657,46368,75025,121393,196418,317811]

def tri(d):
    dd = abs(math.remainder(d, 2*math.pi))
    return 1 - (2/math.pi)*dd

QUANTUM = {(a,b): math.cos(XA[a]-YB[b]) for a in (0,1) for b in (0,1)}

def run(N, L, d1, d2, targets):
    idx = np.arange(N, dtype=np.int64)
    theta = (idx * ALPHA) % 1.0
    th  = 2*math.pi*theta
    thB = 2*math.pi*(((idx + L) * ALPHA) % 1.0)
    Q = np.floor(4*((d2*theta) % 1.0)).astype(np.int8)
    r = (d1*theta) % 1.0
    E, tv, cnt = {}, {}, {}
    for q,(a,b) in enumerate([(0,0),(0,1),(1,0),(1,1)]):
        T = tri(XA[a]-YB[b]); C = targets[(a,b)]
        kap = (C-T)/(1-C*T)
        A = np.sign(np.cos(th - XA[a])); B = np.sign(np.cos(thB - YB[b]))
        AB = A*B
        mask = (Q==q)
        acc = mask & (r < (1+kap*AB)/(1+abs(kap)))
        n = int(acc.sum()); cnt[(a,b)] = n
        E[(a,b)] = float(AB[acc].mean()) if n else float('nan')
        # empirical TV of selected-lambda density vs uniform, on 512 bins
        h_acc,_ = np.histogram(theta[acc], bins=512, range=(0,1))
        h_all,_ = np.histogram(theta[mask], bins=512, range=(0,1))
        with np.errstate(divide='ignore', invalid='ignore'):
            rho = np.where(h_all>0, h_acc/h_acc.sum(), 0)
        tv[(a,b)] = 0.5*float(np.abs(rho - 1/512).sum())
    S = E[(0,0)]+E[(0,1)]+E[(1,0)]-E[(1,1)]
    return E, S, tv, cnt

def main():
    N = 1<<23
    print(f"N = {N}   Tsirelson = {TS:.6f}")
    rows = []
    for p in (13, 16, 19):
        d1, d2 = FIB[p], FIB[p-1]
        print(f"\n--- refinement level p={p} (strides F_p={d1}, F_p-1={d2}) ---")
        print(f"{'m':>3} {'L=F_m':>8} {'delta_m':>11} {'S':>9} {'2sqrt2-S':>10}")
        for m in (14, 16, 18, 20, 22, 24):
            L = FIB[m]
            delta = abs(L*ALPHA - round(L*ALPHA))
            E, S, tv, cnt = run(N, L, d1, d2, QUANTUM)
            print(f"{m:>3} {L:>8} {delta:>11.3e} {S:>9.5f} {TS-S:>+10.5f}")
            rows.append({"p":p,"m":m,"L":L,"delta":delta,"S":S,
                         "tv":{str(k):v for k,v in tv.items()}})
        print(f"   TV(rho(lambda|ab), uniform) at last row: "
              f"{[round(v,4) for v in tv.values()]}")
    with open("tsirelson_family2_results.json","w") as f:
        json.dump(rows,f,indent=1)
    print("\nsaved -> tsirelson_family2_results.json")

main()
