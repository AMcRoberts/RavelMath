"""
ultrametric_chsh.py
------------------------------------------------------------------------
The test the earlier runs never performed.

spectre_chsh.py certified "spacelike" with GRAPH distance and then
sampled pairs UNIFORMLY over the crop -- i.e. it judged both separation
and measure with the same (Euclidean-like) yardstick, and got S ~ 0.

Here the pair ensemble is instead stratified by the tiling's own
substitution ULTRAMETRIC:

    u(A,B) = inflation level of the least common ancestor supertile

restricted throughout to pairs that are GRAPH-SPACELIKE for the real CA
run (graph_distance > max_depth, exactly the old criterion). A pair that
is spacelike-but-ultrametric-close shares a recent common generative
cause (a small supertile stamped out by one substitution event) while
being outside each other's CA light cone.

NOTHING is injected: settings and outcomes are literally the ones from
spectre_chsh.py (setting0 = sorted(nbrs)[:2], setting1 = sorted(nbrs)[-2:],
outcome = XOR of final frozen CA bits -> +-1). Since outcomes remain two
fixed functions of the common frozen state, |S| <= 2 algebraically; the
live question is whether S (and the raw correlators) detach from 0 when
the ensemble is conditioned on the hierarchy instead of on uniform
position.

All pairs are enumerated EXHAUSTIVELY per stratum (full BFS distance
matrix), so there is no pair-sampling noise; the only stochasticity left
is the CA seed, which we scan.
------------------------------------------------------------------------
"""
import json
import math
from collections import deque, defaultdict

from spectre_gen_lineage import generate_tiling_with_lineage, ultrametric_level
from spectre_ca import build_adjacency, compute_depth, reference_neighbors, run_ca


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


def settings_for(adjacency):
    setting0 = reference_neighbors(adjacency)          # sorted(nbrs)[:2]
    setting1 = []
    for nbrs in adjacency:
        ordered = sorted(nbrs)
        setting1.append(ordered[-2:] if len(ordered) >= 2 else ordered)
    return setting0, setting1


def outcome(final_state, pair):
    if len(pair) == 2:
        bit = final_state[pair[0]] ^ final_state[pair[1]]
    elif len(pair) == 1:
        bit = final_state[pair[0]]
    else:
        bit = 0
    return 1 - 2 * bit


def run(iterations=5, crop_count=1000, ca_seeds=(1, 7, 42)):
    tiles = generate_tiling_with_lineage(iterations=iterations, crop_count=crop_count)
    adjacency, is_boundary = build_adjacency(tiles)
    n = len(tiles)
    print(f"tiles: {n}   (iterations={iterations})")

    print("computing full graph-distance matrix ...")
    dist = all_pairs_distances(adjacency)

    setting0, setting1 = settings_for(adjacency)
    valid = [i for i in range(n) if len(adjacency[i]) >= 2
             and set(setting0[i]) != set(setting1[i])]
    validset = set(valid)
    print(f"tiles with two distinct natural settings: {len(valid)}")

    # bucket every unordered valid pair by ultrametric level, keep graph dist
    strata = defaultdict(list)   # u -> list of (a, b, graphdist)
    for ia in range(len(valid)):
        a = valid[ia]
        for ib in range(ia + 1, len(valid)):
            b = valid[ib]
            u = ultrametric_level(tiles[a]["addr"], tiles[b]["addr"], iterations)
            strata[u].append((a, b, dist[a][b]))

    results = []
    for seed in ca_seeds:
        history, depth, max_depth, gate_type = run_ca(tiles, seed=seed)
        final_state = history[-1]["state"]
        lightcone = max_depth
        print(f"\n=== CA seed {seed}   light-cone radius (max_depth) = {max_depth} ===")
        header = (f"{'u':>2} {'pairs(all)':>10} {'spacelike':>10} "
                  f"{'E00':>8} {'E01':>8} {'E10':>8} {'E11':>8} {'S':>8} {'~1/sqrtN':>9}")
        print(header)
        for u in sorted(strata):
            sl = [(a, b) for a, b, d in strata[u] if d > lightcone]
            row = {"seed": seed, "u": u,
                   "pairs_all": len(strata[u]), "pairs_spacelike": len(sl)}
            if len(sl) >= 30:
                def corr(sA, sB):
                    t = 0
                    for a, b in sl:
                        t += outcome(final_state, sA[a]) * outcome(final_state, sB[b])
                    return t / len(sl)
                E00 = corr(setting0, setting0)
                E01 = corr(setting0, setting1)
                E10 = corr(setting1, setting0)
                E11 = corr(setting1, setting1)
                S = E00 + E01 + E10 - E11
                se = 1.0 / math.sqrt(len(sl))
                row.update({"E00": E00, "E01": E01, "E10": E10, "E11": E11,
                            "S": S, "naive_se": se})
                print(f"{u:>2} {len(strata[u]):>10} {len(sl):>10} "
                      f"{E00:>+8.4f} {E01:>+8.4f} {E10:>+8.4f} {E11:>+8.4f} "
                      f"{S:>+8.4f} {se:>9.4f}")
            else:
                print(f"{u:>2} {len(strata[u]):>10} {len(sl):>10}   "
                      f"(too few spacelike pairs)")
            results.append(row)

        # pooled "ultrametric-close" (smallest u with enough spacelike pairs)
        # vs "ultrametric-far" (largest u) comparison at matched spacelikeness
    with open("ultrametric_chsh_results.json", "w") as f:
        json.dump(results, f, indent=1)
    print("\nsaved -> ultrametric_chsh_results.json")
    return results


if __name__ == "__main__":
    run()
