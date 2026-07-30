"""
Checks whether the substitution hierarchy's own, un-forced structure --
orientation classes, and separately, real CA final-state bits under two
different NATURAL neighbor-pair readouts -- shows any genuine CHSH
violation for genuinely spacelike-separated tile pairs, with NO injected
r(w) coupling anywhere. Also verifies, numerically, the elementary
algebraic identity that bounds any such "two fixed functions of a common
frozen state" construction to |S| <= 2 regardless of which natural
feature is used.
"""
import random
from collections import deque
from spectre_gen import generate_tiling
from spectre_ca import build_adjacency, compute_depth, run_ca

def bfs_distances(adjacency, source):
    n = len(adjacency)
    dist = [-1]*n
    dist[source] = 0
    q = deque([source])
    while q:
        u = q.popleft()
        for v in adjacency[u]:
            if dist[v] == -1:
                dist[v] = dist[u]+1
                q.append(v)
    return dist

def build_pairs(adjacency, valid_A, lightcone_radius, rng, n_pairs):
    pairs = []
    dist_cache = {}
    attempts, target = 0, n_pairs
    while len(pairs) < target and attempts < target*50:
        attempts += 1
        a = rng.choice(valid_A)
        if a not in dist_cache:
            dist_cache[a] = bfs_distances(adjacency, a)
        b = rng.choice(valid_A)
        if b == a: continue
        if dist_cache[a][b] > lightcone_radius:
            pairs.append((a,b))
    return pairs

def rotation_class(tiles, i):
    return int(round(tiles[i]["theta"]/30.0)) % 12

def orientation_outcome(tiles, i, setting):
    k = rotation_class(tiles, i)
    if setting == 0:
        return +1 if k < 6 else -1                 # "polarizer" at 0 deg
    else:
        return +1 if ((k+3) % 12) < 6 else -1       # "polarizer" rotated 90 deg

def ca_neighbor_outcome(final_state, adjacency, i, setting):
    ordered = sorted(adjacency[i])
    pair = ordered[:2] if setting == 0 else ordered[-2:]
    if len(pair) == 2:
        bit = final_state[pair[0]] ^ final_state[pair[1]]
    elif len(pair) == 1:
        bit = final_state[pair[0]]
    else:
        bit = 0
    return 1 - 2*bit

def chsh_from_outcomes(pairs, outcome_fn_A_setting0, outcome_fn_A_setting1,
                        outcome_fn_B_setting0, outcome_fn_B_setting1):
    sums = {(0,0):[], (0,1):[], (1,0):[], (1,1):[]}
    for A,B in pairs:
        oa0, oa1 = outcome_fn_A_setting0(A), outcome_fn_A_setting1(A)
        ob0, ob1 = outcome_fn_B_setting0(B), outcome_fn_B_setting1(B)
        sums[(0,0)].append(oa0*ob0)
        sums[(0,1)].append(oa0*ob1)
        sums[(1,0)].append(oa1*ob0)
        sums[(1,1)].append(oa1*ob1)
    E = {k: sum(v)/len(v) for k,v in sums.items()}
    S = E[(0,0)]+E[(0,1)]+E[(1,0)]-E[(1,1)]
    return E, S

def run_check(crop_count=1000, seed=7, n_pairs=6000, rng_seed=99):
    tiles = generate_tiling(iterations=5, crop_count=crop_count)
    adjacency, is_boundary = build_adjacency(tiles)
    depth = compute_depth(adjacency, is_boundary)
    history, depth2, max_depth, gate_type = run_ca(tiles, seed=seed)
    final_state = history[-1]["state"]

    rng = random.Random(rng_seed)
    valid = [i for i in range(len(tiles)) if len(adjacency[i]) >= 2]
    pairs = build_pairs(adjacency, valid, max_depth, rng, n_pairs)

    # Check 1: natural orientation-class "polarizer" readout, no CA involved at all
    E1, S1 = chsh_from_outcomes(
        pairs,
        lambda i: orientation_outcome(tiles, i, 0),
        lambda i: orientation_outcome(tiles, i, 1),
        lambda i: orientation_outcome(tiles, i, 0),
        lambda i: orientation_outcome(tiles, i, 1),
    )

    # Check 2: real CA final-state bits, natural neighbor-pair settings (same as spectre_chsh.py)
    E2, S2 = chsh_from_outcomes(
        pairs,
        lambda i: ca_neighbor_outcome(final_state, adjacency, i, 0),
        lambda i: ca_neighbor_outcome(final_state, adjacency, i, 1),
        lambda i: ca_neighbor_outcome(final_state, adjacency, i, 0),
        lambda i: ca_neighbor_outcome(final_state, adjacency, i, 1),
    )

    return dict(n_pairs=len(pairs), max_depth=max_depth,
                orientation=(E1,S1), ca_dynamics=(E2,S2))

if __name__ == "__main__":
    print("Sanity check: algebraic identity A0(B0+B1)+A1(B0-B1) in {-2,+2} for ANY +-1 assignment")
    import itertools
    bad = 0
    for A0,A1,B0,B1 in itertools.product([1,-1], repeat=4):
        val = A0*B0 + A0*B1 + A1*B0 - A1*B1
        if abs(val) > 2:
            bad += 1
    print("violations found across all 16 sign assignments:", bad, "(expected 0)\n")

    print("=== Natural, UN-forced correlation checks on real tiling/CA data ===")
    for crop, seed in [(700,1),(700,7),(1000,1),(1000,42)]:
        r = run_check(crop_count=crop, seed=seed)
        (E1,S1), (E2,S2) = r["orientation"], r["ca_dynamics"]
        print(f"crop={crop} seed={seed}  pairs={r['n_pairs']} max_depth={r['max_depth']}")
        print(f"   orientation-class 'polarizer' readout:  E={ {k:round(v,4) for k,v in E1.items()} }  S={S1:+.4f}")
        print(f"   real CA final-state neighbor readout:   E={ {k:round(v,4) for k,v in E2.items()} }  S={S2:+.4f}")
