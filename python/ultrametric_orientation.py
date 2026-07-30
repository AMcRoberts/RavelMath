"""
ultrametric_orientation.py
------------------------------------------------------------------------
Follow-up to ultrametric_chsh.py, which showed S ~ 0 in every ultrametric
stratum WHEN THE READOUT IS THE RANDOM-SEEDED CA STATE. Diagnosis: the
substitution hierarchy only enters the CA through its wiring; the random
initial bits carry no lineage information, so conditioning the pair
ensemble on lineage cannot resurrect a correlation the observable never
contained.

Here the observable is instead the tiling's own deterministic structure:
the rotation class k(v) = round(theta/30) mod 12 -- with the two natural
"polarizer" settings already defined in check_natural_correlation.py:

    setting 0:  +1 if k in {0..5}            ("polarizer at 0 degrees")
    setting 1:  +1 if (k+3) mod 12 in {0..5}  ("polarizer at 90 degrees")

Nothing is injected, no coupling law, no bucket-to-target table: the
settings are two fixed functions applied identically to every tile, and
the ensemble is simply stratified by the substitution ultrametric u and
restricted to graph-spacelike pairs (distance > CA max_depth), exactly
as before. Because outcomes are again two fixed +-1 functions of the
frozen structure, |S| <= 2 must hold; the question is whether S detaches
from 0 -- i.e., whether the hierarchy is a real spacelike common cause
for THIS observable.

Also reported per stratum, as controls / decomposition:
  - E00 for the uniform (all-u pooled) spacelike ensemble, which is what
    check_natural_correlation.py effectively measured (expected ~ 0)
  - the label-pair mutual structure: fraction of pairs whose 30-degree
    relative rotation (k_A - k_B mod 12) equals its stratum's modal value,
    to show directly how rigidity decays (or doesn't) with u.
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
    else:
        return +1 if ((k + 3) % 12) < 6 else -1


def run(iterations=5, crop_count=1000, ca_seed=1):
    tiles = generate_tiling_with_lineage(iterations=iterations, crop_count=crop_count)
    adjacency, is_boundary = build_adjacency(tiles)
    n = len(tiles)

    # light-cone radius from the real CA run (structure-independent readout,
    # but "spacelike" is still defined by the actual dynamics as before)
    history, depth, max_depth, gate_type = run_ca(tiles, seed=ca_seed)
    lightcone = max_depth
    print(f"tiles: {n}   light-cone radius (max_depth) = {lightcone}")

    print("computing full graph-distance matrix ...")
    dist = all_pairs_distances(adjacency)

    kcls = [rotation_class(t) for t in tiles]

    strata = defaultdict(list)
    for a in range(n):
        for b in range(a + 1, n):
            u = ultrametric_level(tiles[a]["addr"], tiles[b]["addr"], iterations)
            strata[u].append((a, b, dist[a][b]))

    results = []
    print(f"\n{'u':>2} {'spacelike':>10} {'E00':>8} {'E01':>8} {'E10':>8} "
          f"{'E11':>8} {'S':>8} {'~1/sqrtN':>9} {'rigid%':>7}")

    pooled = []
    for u in sorted(strata):
        sl = [(a, b) for a, b, d in strata[u] if d > lightcone]
        pooled.extend(sl)
        row = {"u": u, "pairs_spacelike": len(sl)}
        if len(sl) >= 30:
            def corr(sa, sb):
                t = 0
                for a, b in sl:
                    t += orientation_outcome(kcls[a], sa) * orientation_outcome(kcls[b], sb)
                return t / len(sl)
            E00 = corr(0, 0)
            E01 = corr(0, 1)
            E10 = corr(1, 0)
            E11 = corr(1, 1)
            S = E00 + E01 + E10 - E11
            se = 1.0 / math.sqrt(len(sl))
            # rigidity: concentration of relative rotation within stratum
            rel = Counter((kcls[a] - kcls[b]) % 12 for a, b in sl)
            rigid = rel.most_common(1)[0][1] / len(sl)
            row.update({"E00": E00, "E01": E01, "E10": E10, "E11": E11,
                        "S": S, "naive_se": se, "rigidity": rigid,
                        "rel_rotation_hist": dict(rel)})
            print(f"{u:>2} {len(sl):>10} {E00:>+8.4f} {E01:>+8.4f} {E10:>+8.4f} "
                  f"{E11:>+8.4f} {S:>+8.4f} {se:>9.4f} {100*rigid:>6.1f}%")
        else:
            print(f"{u:>2} {len(sl):>10}   (too few spacelike pairs)")
        results.append(row)

    # control: the uniform pooled ensemble (what the old test measured)
    def pcorr(sa, sb):
        t = 0
        for a, b in pooled:
            t += orientation_outcome(kcls[a], sa) * orientation_outcome(kcls[b], sb)
        return t / len(pooled)
    E00 = pcorr(0, 0); E01 = pcorr(0, 1); E10 = pcorr(1, 0); E11 = pcorr(1, 1)
    Sp = E00 + E01 + E10 - E11
    print(f"\npooled uniform spacelike ensemble (old-style): "
          f"E00={E00:+.4f} E01={E01:+.4f} E10={E10:+.4f} E11={E11:+.4f} "
          f"S={Sp:+.4f}  (N={len(pooled)})")
    results.append({"u": "pooled", "E00": E00, "E01": E01, "E10": E10,
                    "E11": E11, "S": Sp, "pairs_spacelike": len(pooled)})

    with open("ultrametric_orientation_results.json", "w") as f:
        json.dump(results, f, indent=1)
    print("saved -> ultrametric_orientation_results.json")
    return results


if __name__ == "__main__":
    run()
