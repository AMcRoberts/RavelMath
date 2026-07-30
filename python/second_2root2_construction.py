"""
A SECOND, structurally unrelated construction hitting the same target
S = 2*sqrt(2), to make the "infinitude of fields at that value" claim
concrete rather than asserted.

Construction #1 (previous message) used:
    w(A,B) = (rotation_class(A) + rotation_class(B)) mod 4
  -- an ORIENTATION/geometric invariant of the substitution system.

Construction #2 here uses a totally different invariant of the same
kind of object -- pure CAUSAL/graph-metric structure, nothing
geometric at all:
    w(A,B) = graph_distance(A,B) mod 4
  -- i.e. how far apart the two tiles are in the adjacency graph
     (the same distance already used to certify "spacelike separated"),
     not their orientation, not their substitution label, not anything
     to do with angles.

Construction #3, for good measure, uses the SUBSTITUTION LABEL PAIR
(Gamma/Delta/Theta/.../Psi -- the 9 metatile types) hashed to 4 buckets
 -- a purely COMBINATORIAL/symbolic invariant, unrelated to both geometry
and graph metric.

If all three reproduce S = 2*sqrt(2) (up to finite-sample noise) despite
using unrelated features of the tiling to decide which pairs get which
setting, that demonstrates the claim directly: hitting a specific target
number is achieved by a large, heterogeneous family of laws, not a
single one -- because S never depended on WHICH invariant does the
bucketing, only on how the four resulting buckets are aligned with the
four CHSH terms and what target correlation is assigned to each.
"""
import math
import random
from collections import deque

from spectre_gen import generate_tiling
from spectre_ca import build_adjacency, compute_depth, run_ca


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


def build_pairs_with_dist(adjacency, valid_A, lightcone_radius, rng, n_pairs):
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
        d = dist_cache[a][b]
        if d > lightcone_radius:
            pairs.append((a, b, d))
    return pairs


def outcome(final_state, pair):
    if len(pair) == 2:
        bit = final_state[pair[0]] ^ final_state[pair[1]]
    elif len(pair) == 1:
        bit = final_state[pair[0]]
    else:
        bit = 0
    return 1 - 2 * bit


BUCKET_TO_SETTING = {0: (0, 0), 1: (0, 1), 2: (1, 0), 3: (1, 1)}
ALICE_ANGLE = {0: 0.0, 1: 90.0}
BOB_ANGLE = {0: 45.0, 1: -45.0}

def quantum_target_prob(w, rng):
    a, b = BUCKET_TO_SETTING[w]
    delta = math.radians(ALICE_ANGLE[a] - BOB_ANGLE[b])
    target_E = math.cos(delta)
    p_match = (1 + target_E) / 2.0
    return +1 if rng.random() < p_match else -1


def run_with_bucketing(bucket_fn, crop_count=1000, seed=7, n_pairs=20000, rng_seed=2024,
                        label="?"):
    tiles = generate_tiling(iterations=5, crop_count=crop_count)
    adjacency, is_boundary = build_adjacency(tiles)
    depth = compute_depth(adjacency, is_boundary)
    history, depth2, max_depth, gate_type = run_ca(tiles, seed=seed)
    final_state = history[-1]["state"]

    from spectre_ca import reference_neighbors
    setting0 = reference_neighbors(adjacency)
    setting1 = []
    for nbrs in adjacency:
        ordered = sorted(nbrs)
        setting1.append(ordered[-2:] if len(ordered) >= 2 else ordered)
    settings = [setting0, setting1]

    rng = random.Random(rng_seed)
    valid_A = [i for i in range(len(tiles)) if len(adjacency[i]) >= 2]
    pairs = build_pairs_with_dist(adjacency, valid_A, max_depth, rng, n_pairs)

    sums = {(0, 0): [], (0, 1): [], (1, 0): [], (1, 1): []}
    for A, B, dist in pairs:
        w = bucket_fn(tiles, gate_type, A, B, dist) % 4
        a, b = BUCKET_TO_SETTING[w]
        outcome_A = outcome(final_state, settings[a][A])
        r = quantum_target_prob(w, rng)
        outcome_B = outcome_A * r
        sums[(a, b)].append(outcome_A * outcome_B)

    E = {k: (sum(v) / len(v) if v else float("nan")) for k, v in sums.items()}
    S = E[(0, 0)] + E[(0, 1)] + E[(1, 0)] - E[(1, 1)]
    counts = {k: len(v) for k, v in sums.items()}
    print(f"[{label}] pairs={len(pairs)} max_depth={max_depth} counts={counts}")
    print(f"[{label}] E={ {k: round(v,4) for k,v in E.items()} }  S={S:+.5f}  (target 2*sqrt2={2*2**0.5:.5f})")
    return S


def rotation_class(tiles, i):
    return int(round(tiles[i]["theta"] / 30.0)) % 12

# Construction 2: pure graph-distance invariant, no geometry at all
def bucket_by_distance(tiles, gate_type, A, B, dist):
    return dist

# Construction 3: pure symbolic/combinatorial invariant (substitution labels)
LABEL_INDEX = {name: i for i, name in enumerate(
    ["Gamma","Gamma1","Gamma2","Delta","Theta","Lambda","Xi","Pi","Sigma","Phi","Psi"])}
def bucket_by_label(tiles, gate_type, A, B, dist):
    la = LABEL_INDEX.get(tiles[A]["label"], 0)
    lb = LABEL_INDEX.get(tiles[B]["label"], 0)
    return la * 11 + lb

# Construction 1 (from before), for direct side-by-side comparison
def bucket_by_orientation(tiles, gate_type, A, B, dist):
    return rotation_class(tiles, A) + rotation_class(tiles, B)

if __name__ == "__main__":
    print("=== Three unrelated bucketing invariants, same target 2*sqrt(2) ===\n")
    run_with_bucketing(bucket_by_orientation, label="orientation-class (geometric)")
    print()
    run_with_bucketing(bucket_by_distance, label="graph-distance mod 4 (causal/metric)")
    print()
    run_with_bucketing(bucket_by_label, label="substitution-label pair (combinatorial/symbolic)")
