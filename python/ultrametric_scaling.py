"""
ultrametric_scaling.py
------------------------------------------------------------------------
Convergence check for the nonzero stratified S found by
ultrametric_orientation.py (S(u=4) = -0.084 exactly, on the 1000-tile
iterations=5 crop).

Question: structural constant of the substitution system, or finite-crop
accident? We recompute S(u) and rigidity(u) on larger crops / deeper
substitution (iterations=5 crop=1000 baseline; iterations=6 with crops
2000 and 3000), for the same fixed natural observables. If S(u) at fixed
u roughly stabilizes as the crop grows, it's structure; if it wanders
toward 0 with N, it was a boundary/finite-size effect.

Also reported: rigidity for ALL pairs (not just spacelike) at every u,
including u=1,2 where no spacelike pairs exist -- to expose the full
rigidity gradient of the hierarchy.
------------------------------------------------------------------------
"""
import json
import math
from collections import deque, defaultdict, Counter

from spectre_gen_lineage import generate_tiling_with_lineage, ultrametric_level
from spectre_ca import build_adjacency, run_ca


def all_pairs_distances(adjacency):
    n = len(adjacency)
    dist = [[-1] * n for _ in range(n)]
    for s in range(n):
        d = dist[s]
        d[s] = 0
        q = deque([s])
        while q:
            u = q.popleft()
            for v in adjacency[u]:
                if d[v] == -1:
                    d[v] = d[u] + 1
                    q.append(v)
    return dist


def rotation_class(tile):
    return int(round(tile["theta"] / 30.0)) % 12


def orientation_outcome(k, setting):
    if setting == 0:
        return +1 if k < 6 else -1
    return +1 if ((k + 3) % 12) < 6 else -1


def analyze(iterations, crop_count, ca_seed=1):
    tiles = generate_tiling_with_lineage(iterations=iterations, crop_count=crop_count)
    adjacency, is_boundary = build_adjacency(tiles)
    n = len(tiles)
    history, depth, max_depth, gate_type = run_ca(tiles, seed=ca_seed)
    lightcone = max_depth
    dist = all_pairs_distances(adjacency)
    kcls = [rotation_class(t) for t in tiles]

    strata_all = defaultdict(list)
    strata_sl = defaultdict(list)
    for a in range(n):
        da = dist[a]
        ta = tiles[a]["addr"]
        for b in range(a + 1, n):
            u = ultrametric_level(ta, tiles[b]["addr"], iterations)
            strata_all[u].append((a, b))
            if da[b] > lightcone:
                strata_sl[u].append((a, b))

    print(f"\n##### iterations={iterations}  crop={n}  lightcone={lightcone} #####")
    print(f"{'u':>2} {'all pairs':>10} {'spacelike':>10} {'rigidAll%':>9} "
          f"{'E00':>8} {'E01':>8} {'E10':>8} {'E11':>8} {'S':>8}")
    out = []
    for u in sorted(strata_all):
        allp = strata_all[u]
        sl = strata_sl.get(u, [])
        rel = Counter((kcls[a] - kcls[b]) % 12 for a, b in allp)
        rigid_all = rel.most_common(1)[0][1] / len(allp)
        row = {"iterations": iterations, "crop": n, "u": u,
               "pairs_all": len(allp), "pairs_spacelike": len(sl),
               "rigidity_all": rigid_all}
        if len(sl) >= 100:
            def corr(sa, sb):
                t = 0
                for a, b in sl:
                    t += orientation_outcome(kcls[a], sa) * orientation_outcome(kcls[b], sb)
                return t / len(sl)
            E00 = corr(0, 0); E01 = corr(0, 1); E10 = corr(1, 0); E11 = corr(1, 1)
            S = E00 + E01 + E10 - E11
            row.update({"E00": E00, "E01": E01, "E10": E10, "E11": E11, "S": S})
            print(f"{u:>2} {len(allp):>10} {len(sl):>10} {100*rigid_all:>8.1f}% "
                  f"{E00:>+8.4f} {E01:>+8.4f} {E10:>+8.4f} {E11:>+8.4f} {S:>+8.4f}")
        else:
            print(f"{u:>2} {len(allp):>10} {len(sl):>10} {100*rigid_all:>8.1f}%   "
                  f"(too few spacelike pairs for S)")
        out.append(row)
    return out


if __name__ == "__main__":
    all_rows = []
    for it, crop in [(5, 1000), (6, 2000), (6, 3000)]:
        all_rows.extend(analyze(it, crop))
    with open("ultrametric_scaling_results.json", "w") as f:
        json.dump(all_rows, f, indent=1)
    print("\nsaved -> ultrametric_scaling_results.json")
