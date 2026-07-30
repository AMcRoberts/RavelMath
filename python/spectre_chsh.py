"""
spectre_chsh.py
------------------------------------------------------------------------
A CHSH test run *on* the spectre CA substrate, to check empirically
whether the superdeterministic structure argued for in conversation
shows up as a measurable Bell-inequality violation.

SETUP
  - "Hidden variable" lambda = the entire deterministic generator: the
    substitution history (which fixes the tiling + adjacency graph) plus
    the single global RNG seed used to initialize the CA. Nothing else
    is random.
  - "Setting" for a tile v = which pair of its graph-neighbors gets read
    out. Setting 0 = the *same* pair the CA itself uses as gate inputs
    (sorted(adjacency[v])[:2]). Setting 1 = a different, equally
    deterministic pair (sorted(adjacency[v])[-2:]) -- a different "local
    observable" on the same tile.
  - "Outcome" for (v, setting) = XOR of the two chosen neighbors' FINAL
    frozen CA states, mapped to +-1. This is read off the real history
    produced by run_ca -- no numbers are invented for this test.
  - "Spacelike separated" pair (A,B): graph distance(A,B) > max_depth,
    i.e. strictly farther apart than any signal could have propagated in
    the CA's own dynamics (one hop per step, run for max_depth steps).
    No influence could have passed between A and B during the run.

We then compute the four correlators E(a,b) for a,b in {0,1} over many
sampled spacelike pairs, and the CHSH quantity
    S = E(0,0) + E(0,1) + E(1,0) - E(1,1)
comparing |S| against:
    2       classical/local-hidden-variable bound
    2*sqrt(2) ~ 2.828   Tsirelson bound (max for genuine quantum correlations)
    4       algebraic maximum (achievable by an unconstrained/"conspiratorial"
            hidden-variable model with no independence assumption at all)
------------------------------------------------------------------------
"""
import random
from collections import deque

from spectre_gen import generate_tiling
from spectre_ca import build_adjacency, compute_depth, reference_neighbors, run_ca

def bfs_distances(adjacency, source):
    n = len(adjacency)
    dist = [-1] * n
    dist[source] = 0
    q = deque([source])
    while q:
        u = q.popleft()
        for v in adjacency[u]:
            if dist[v] == -1:
                dist[v] = dist[u] + 1
                q.append(v)
    return dist

def settings_for(adjacency):
    """Two deterministic 'measurement settings' per tile: two different
    neighbor pairs to read out. This reuses setting 0 = the CA's own
    gate wiring; setting 1 = the other end of the same sorted list."""
    setting0 = reference_neighbors(adjacency)  # sorted(nbrs)[:2]
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
    return 1 - 2 * bit  # 0 -> +1, 1 -> -1

def run_chsh(crop_count=700, seed=7, n_pairs=4000, rng_seed=1234):
    tiles = generate_tiling(iterations=4, crop_count=crop_count)
    adjacency, is_boundary = build_adjacency(tiles)
    depth = compute_depth(adjacency, is_boundary)
    history, depth2, max_depth, gate_type = run_ca(tiles, seed=seed)
    final_state = history[-1]["state"]

    setting0, setting1 = settings_for(adjacency)
    n = len(tiles)

    rng = random.Random(rng_seed)
    lightcone_radius = max_depth
    valid_A = [i for i in range(n) if len(adjacency[i]) >= 2]

    pairs = []
    dist_cache = {}
    attempts, target = 0, n_pairs
    while len(pairs) < target and attempts < target * 50:
        attempts += 1
        a = rng.choice(valid_A)
        if a not in dist_cache:
            dist_cache[a] = bfs_distances(adjacency, a)
        b = rng.choice(valid_A)
        if b == a:
            continue
        if dist_cache[a][b] > lightcone_radius:
            pairs.append((a, b))

    if not pairs:
        raise RuntimeError("No spacelike-separated pairs found -- patch too small "
                            "relative to its own causal depth; increase crop_count.")

    def correlator(sA, sB):
        total = 0.0
        for a, b in pairs:
            oa = outcome(final_state, sA[a])
            ob = outcome(final_state, sB[b])
            total += oa * ob
        return total / len(pairs)

    E00 = correlator(setting0, setting0)
    E01 = correlator(setting0, setting1)
    E10 = correlator(setting1, setting0)
    E11 = correlator(setting1, setting1)
    S = E00 + E01 + E10 - E11

    return {
        "n_tiles": n, "max_depth": max_depth, "n_pairs": len(pairs),
        "min_pair_distance_used": lightcone_radius + 1,
        "E00": E00, "E01": E01, "E10": E10, "E11": E11, "S": S,
    }

if __name__ == "__main__":
    result = run_chsh()
    print(f"tiles: {result['n_tiles']}   CA light-cone radius (max_depth): {result['max_depth']}")
    print(f"spacelike pairs sampled: {result['n_pairs']}  (graph distance > {result['min_pair_distance_used']-1})")
    print(f"E(0,0) = {result['E00']:+.4f}")
    print(f"E(0,1) = {result['E01']:+.4f}")
    print(f"E(1,0) = {result['E10']:+.4f}")
    print(f"E(1,1) = {result['E11']:+.4f}")
    print(f"S = E(0,0)+E(0,1)+E(1,0)-E(1,1) = {result['S']:+.4f}   |S| = {abs(result['S']):.4f}")
    print(f"classical bound: 2.0000   Tsirelson bound: {2*2**0.5:.4f}   algebraic max: 4.0000")
