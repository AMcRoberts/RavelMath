"""
spectre_ca.py
------------------------------------------------------------------------
A reversible cellular automaton on the Spectre tiling's adjacency graph.

WHY THIS DESIGN
================
1. INFORMATION PRESERVATION ("quantumness").
   Each cell carries state s(v, t). The update is *second order*:

       s(v, t+1) = s(v, t-1) XOR F(neighbors of v at time t)

   This is the standard reversible-CA construction (same trick used by
   Fredkin/Margolus-style reversible automata and by the classical
   "billiard-ball" model): because XOR is its own inverse, and F is
   recomputable from state you still have, the map

       (s(v,t-1), s(v,t))  -->  (s(v,t), s(v,t+1))

   is a bijection. Nothing is thrown away -- given the last two frames of
   an (unfrozen) cell you can always run the rule backwards and recover
   every earlier frame. That's the concrete, non-mystical meaning of
   "quantumness" used here: discrete states, invertible transition.

2. "SUFFICIENTLY COMPLETE SET OF QUANTUM SWITCHES."
   F(neighbors) is built from two reference neighbors (n1, n2) as
   F = n1 AND n2. Folded into the XOR update this is exactly the
   Toffoli gate's core (two controls, one target) -- the Toffoli gate is
   a known *universal* gate for reversible classical computation, so a
   lattice of these local gates has, in principle, the switching
   vocabulary needed to realize arbitrary reversible circuits, not just
   this one fixed rule. Cells with only one graph-neighbor fall back to a
   CNOT (F = n1); fully isolated cells fall back to identity.

3. BOUNDARY-DRIVEN FREEZING ("causality").
   The tiling we ever have on screen is a *finite crop* of an infinite
   aperiodic tiling. A tile on the perimeter of that crop is missing a
   real neighbor that the infinite tiling would have supplied. Rather
   than invent data for it, we treat that tile as having already run out
   of valid information: it freezes immediately (depth 0).

   Every tile's "causal depth" is its graph distance (in adjacency hops)
   to the nearest such boundary tile -- i.e. a discrete erosion / distance
   transform over the tiling. A cell can only be safely updated for as
   many steps as its depth, because a light-cone of "the boundary doesn't
   know what's out there" propagates inward at one hop per step. Once
   t reaches a cell's depth, it freezes for good.

   The practical effect: the active (still-updating) region is the
   original patch eroded by one layer every step, so it visibly shrinks,
   converging on whichever cell(s) sit at the *maximum* depth -- the
   "peaks of causality," literally the tiling's discrete medial axis --
   before the whole patch is frozen.
------------------------------------------------------------------------
"""
import random
from collections import deque

def _edge_key(p, q, precision=3):
    a = (round(p[0], precision), round(p[1], precision))
    b = (round(q[0], precision), round(q[1], precision))
    return (a, b) if a <= b else (b, a)

def build_adjacency(tiles):
    """tiles: list of dicts with a 'pts' polygon (list of (x,y)).
    Returns: adjacency (list of sets of neighbor indices),
             is_boundary (list of bool, True if tile has an unmatched edge).
    """
    edge_map = {}
    for i, t in enumerate(tiles):
        pts = t["pts"]
        n = len(pts)
        for k in range(n):
            key = _edge_key(pts[k], pts[(k + 1) % n])
            edge_map.setdefault(key, []).append(i)

    adjacency = [set() for _ in tiles]
    is_boundary = [False] * len(tiles)
    for key, idxs in edge_map.items():
        if len(idxs) == 2:
            a, b = idxs
            if a != b:
                adjacency[a].add(b)
                adjacency[b].add(a)
        else:
            # edge touched by only one tile (or a degenerate >2 case from
            # float noise) -- treat as a perimeter edge of the crop.
            for i in idxs:
                is_boundary[i] = True
    return adjacency, is_boundary

def compute_depth(adjacency, is_boundary):
    """Multi-source BFS distance from the boundary set (depth 0)."""
    n = len(adjacency)
    depth = [-1] * n
    q = deque()
    for i in range(n):
        if is_boundary[i]:
            depth[i] = 0
            q.append(i)
    while q:
        u = q.popleft()
        for v in adjacency[u]:
            if depth[v] == -1:
                depth[v] = depth[u] + 1
                q.append(v)
    # any unreached node (shouldn't happen in a connected crop) -> treat as boundary
    for i in range(n):
        if depth[i] == -1:
            depth[i] = 0
    return depth

def reference_neighbors(adjacency):
    """Deterministic pick of up to 2 neighbors per node, used as the
    Toffoli gate's control inputs."""
    refs = []
    for nbrs in adjacency:
        ordered = sorted(nbrs)
        refs.append(ordered[:2])
    return refs

# Tiles carrying these labels run an explicit, unconditional NOT
# (s(t+1) = s(t-1) XOR 1) instead of the neighbor-dependent gate. Gamma1/
# Gamma2 are already the "special" tiles in the substitution system (the
# only ones that only ever appear as a locked Mystic pair), so it's a
# natural, *guaranteed* place to plant deliberate negation throughout the
# lattice, rather than leaving NOT to arise only by accident wherever a
# frozen neighbor happens to be sitting at 1.
NOT_LABELS = {"Gamma1", "Gamma2", "Gamma"}

def run_ca(tiles, seed=0):
    """Runs the reversible CA to completion (until every cell is frozen).
    Returns: history -- list of per-step snapshots, each a dict:
        { 'state': [...bits...], 'active': [...bool...], 'depth': [...] }
      history[0] is the initial condition (t=0, all cells still active
      except any with depth 0, which start frozen).
    Also returns depth, max_depth, and gate_type (per-cell label: "NOT",
    "TOFFOLI", "CNOT", or "ID") for inspection/rendering.
    """
    adjacency, is_boundary = build_adjacency(tiles)
    depth = compute_depth(adjacency, is_boundary)
    refs = reference_neighbors(adjacency)
    n = len(tiles)
    max_depth = max(depth)

    gate_type = []
    for i in range(n):
        if tiles[i].get("label") in NOT_LABELS:
            gate_type.append("NOT")
        elif len(refs[i]) == 2:
            gate_type.append("TOFFOLI")
        elif len(refs[i]) == 1:
            gate_type.append("CNOT")
        else:
            gate_type.append("ID")

    rng = random.Random(seed)
    s_prev = [rng.randint(0, 1) for _ in range(n)]
    s_curr = list(s_prev)  # start "at rest": no motion until the rule kicks in

    history = []
    active0 = [depth[i] > 0 for i in range(n)]
    history.append({"state": list(s_curr), "active": active0, "depth": depth})

    for t in range(0, max_depth):
        active = [depth[i] > t for i in range(n)]
        s_next = list(s_curr)
        for i in range(n):
            if not active[i]:
                continue
            gt = gate_type[i]
            if gt == "NOT":
                gate = 1                                 # unconditional flip
            elif gt == "TOFFOLI":
                r = refs[i]
                gate = s_curr[r[0]] & s_curr[r[1]]       # two-control AND
            elif gt == "CNOT":
                gate = s_curr[refs[i][0]]                 # single-control
            else:
                gate = 0                                  # identity
            s_next[i] = s_prev[i] ^ gate
        s_prev, s_curr = s_curr, s_next
        active_after = [depth[i] > t + 1 for i in range(n)]
        history.append({"state": list(s_curr), "active": active_after, "depth": depth})

    return history, depth, max_depth, gate_type
