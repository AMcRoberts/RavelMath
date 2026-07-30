"""THE COMPUTATIONAL SHOT: plastic (K~479) and smallest-Pisot (K~64) at
N=2^24, LMAX=2^19. Bandwidth model prediction: gap(L) ~ K * ceiling(L).
Track gap, ceiling, ratio at dyadic L. Confirmation = gap breaks
downward tracking the ceiling; refutation = gap flat while ceiling
falls (ratio blows up)."""
import math, json
import numpy as np

def expand(subst, n):
    s = np.array([0], dtype=np.int8)
    while len(s) < n:
        s = np.concatenate([np.array(subst[c], dtype=np.int8) for c in s])
    return s[:n]

def matrix(subst, nl):
    M = np.zeros((nl, nl))
    for c, w in subst.items():
        for r in w: M[r][c] += 1
    return M

def pf_right(M):
    ev, V = np.linalg.eig(M)
    i = np.argmax(ev.real)
    x = np.abs(V[:, i].real)
    return x/x.sum()

def ceiling_running(f, lmax, lmin=33):
    Ls = np.arange(lmax+1, dtype=np.float64)
    worst = np.zeros(lmax+1)
    for m in f[:-1]:
        x = (Ls*m) % 1.0
        worst = np.maximum(worst, np.minimum(x, 1-x))
    worst[:lmin] = np.inf
    return np.minimum.accumulate(worst)

SIGNS = [(1,1,1,-1),(1,1,-1,1),(1,-1,1,1),(-1,1,1,1),
         (-1,-1,-1,1),(-1,-1,1,-1),(-1,1,-1,-1),(1,-1,-1,-1)]

def run_system(subst, nl, N, LMAX):
    s = expand(subst, N)
    A0 = (1.0 - 2.0*(s == 0))[:-1].astype(np.float32)
    A1 = (1.0 - 2.0*((s == 0) ^ (np.roll(s, -1) == 0)))[:-1].astype(np.float32)
    del s
    n = len(A0); size = 1
    while size < n + LMAX + 1: size <<= 1
    counts = (n - np.arange(LMAX+1)).astype(np.float64)
    fA = {0: np.fft.rfft(A0, size), 1: np.fft.rfft(A1, size)}
    E = {}
    for a in (0,1):
        for b in (0,1):
            cc = np.fft.irfft(np.conj(fA[a])*fA[b], size)
            E[(a,b)] = (cc[:LMAX+1]/counts).astype(np.float64)
            del cc
    del fA, A0, A1
    st = np.stack([E[(0,0)],E[(0,1)],E[(1,0)],E[(1,1)]])
    sm = np.full(LMAX+1, -np.inf)
    for g in SIGNS:
        np.maximum(sm, g[0]*st[0]+g[1]*st[1]+g[2]*st[2]+g[3]*st[3], out=sm)
    sm[:33] = -np.inf
    return np.maximum.accumulate(sm)

SYS = {
 "plastic":        ({0:[1], 1:[2], 2:[0,1]}, 3, 478.9),
 "smallest_pisot": ({0:[0,1], 1:[2], 2:[0]}, 3, 64.0),
}
N, LMAX = 1 << 24, 1 << 19
out = {}
for name, (subst, nl, K) in SYS.items():
    f = pf_right(matrix(subst, nl))
    ceil = ceiling_running(f, LMAX)
    run = run_system(subst, nl, N, LMAX)
    print(f"\n{name} (K fitted at 2^16 = {K}):")
    print(f"{'L':>8} {'gap':>10} {'ceiling':>10} {'K*ceiling':>10} {'ratio':>8}")
    rowl = []
    for e in range(13, 20):
        L = 1 << e
        gap = 2 - run[L]; c = ceil[L]
        rowl.append({"L": L, "gap": gap, "ceil": float(c), "ratio": gap/c})
        print(f"{L:>8} {gap:>10.4e} {c:>10.3e} {K*c:>10.4f} {gap/c:>8.1f}")
    out[name] = rowl
json.dump(out, open("big_shot_results.json","w"), indent=1)
print("\nsaved -> big_shot_results.json")
