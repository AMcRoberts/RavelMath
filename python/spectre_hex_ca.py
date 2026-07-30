"""
spectre_hex_ca.py
------------------------------------------------------------------------
A reversible cellular automaton on the HEX-TILE graph (i.e. every Spectre
pair Gamma1+Gamma2 fused into one hexagon, per spectre_hex.py / Tatham's
construction) that gives each of the 3 principal colour groups (from the
hex tiling's proper 3-colouring) a local, deterministic *reason* to switch
to another colour -- and does it in a way whose time-evolution is
provably invertible.

THE RULE
========
Each hex node v carries:
  - a fixed base colour  c0(v) in {0,1,2,3,4}   (from spectre_hex.three_color_hex_graph;
    almost all nodes are 0/1/2, a few boundary nodes spill into 3/4 -- see
    spectre_hex.py's docstring on why a finite crop needs that escape hatch)
  - a single reversible bit  r(v,t), evolved by the *same* second-order
    XOR/AND (Toffoli-style) rule as spectre_ca.py, but now over the coarser
    hex adjacency graph:

        r(v, t+1) = r(v, t-1)  XOR  [ r(n1(v), t) AND r(n2(v), t) ]

    where n1(v), n2(v) are two fixed reference neighbours of v in the hex
    graph (deterministically chosen: the two lowest-indexed neighbours).
    This is exactly the AND-then-XOR ("Toffoli core") construction the
    brief asked for: the *product* (AND) of two neighbours decides whether
    v flips, and flipping is done with XOR, whose defining property
    (X XOR X = 0, i.e. it is its own inverse) is what makes the whole
    time-step a bijection. Given r(v,t-1) and r(v,t) you can always
    recover r(v,t+1) forwards, or r(v,t-1) backwards:
        r(v,t-1) = r(v,t+1) XOR [ r(n1,t) AND r(n2,t) ]
    -- nothing is thrown away, so the recolouring history is exactly as
    time-reversible as spectre_ca.py's freeze rule was.

  - a DISPLAYED colour   c(v,t) = ( c0(v) + r(v,t) ) mod 3

    i.e. "switch to the next colour in the cycle" whenever the bit is 1,
    "stay put" whenever it's 0. Because the cycle is mod 3, this gives
    each of the 3 principal colour groups a well-defined, rule-driven
    reason to hand its tiles over to the *next* group (0->1->2->0) --
    never a 4th colour, so the "principal" colour groups stay exactly 3
    -- while the underlying r(v,t) bit remains the fully reversible XOR/AND
    automaton described above. (Boundary nodes sitting at colour index 3
    or 4, from the crop's colouring defects, are left displaying their
    escape-hatch colour unchanged -- they were never one of the 3
    principal groups to begin with.)

CAUSALITY / FREEZING
=====================
As in spectre_ca.py, a hex node on the edge of the crop is missing real
neighbours the infinite tiling would supply, so it's treated as already
out of information and frozen at depth 0. Every node's causal depth is
its hex-graph BFS distance to the nearest such boundary node, and it
stops updating once t reaches that depth -- the same discrete light-cone
/ erosion structure as before, just measured in hex-hops instead of
spectre-hops.
------------------------------------------------------------------------
"""
import random
from collections import deque

def hex_boundary(hex_adj, degree_full=6):
    """A hex node is a 'boundary' node of the finite crop if its degree in
    the hex graph is less than the interior degree (6, since 8 of the 9
    Spectre metatile types collapse to hexes and the fused Gamma tile is
    also topologically hexagonal -- every interior node of an infinite
    tiling has exactly 6 hex-neighbours)."""
    return {v: (len(hex_adj.get(v, ())) < degree_full) for v in hex_adj}

def compute_hex_depth(hex_adj):
    is_boundary = hex_boundary(hex_adj)
    depth = {v: -1 for v in hex_adj}
    q = deque()
    for v, b in is_boundary.items():
        if b:
            depth[v] = 0
            q.append(v)
    while q:
        u = q.popleft()
        for w in hex_adj.get(u, ()):
            if depth[w] == -1:
                depth[w] = depth[u] + 1
                q.append(w)
    for v in depth:
        if depth[v] == -1:
            depth[v] = 0
    return depth, is_boundary

def reference_neighbors(hex_adj):
    refs = {}
    for v, nbrs in hex_adj.items():
        refs[v] = sorted(nbrs)[:2]
    return refs

def run_hex_ca(hex_adj, base_color, seed=0):
    """Runs the reversible hex-graph CA to completion.
    base_color: dict group_id -> colour index (0,1,2,3,4,...) from
      spectre_hex.three_color_hex_graph.
    Returns: history (list of {'r':dict, 'display_color':dict, 'active':dict}),
             depth, max_depth, gate_type (per-node).
    """
    nodes = list(hex_adj.keys())
    depth, is_boundary = compute_hex_depth(hex_adj)
    refs = reference_neighbors(hex_adj)
    max_depth = max(depth.values()) if depth else 0

    gate_type = {}
    for v in nodes:
        if len(refs[v]) == 2:
            gate_type[v] = "TOFFOLI"
        elif len(refs[v]) == 1:
            gate_type[v] = "CNOT"
        else:
            gate_type[v] = "ID"

    rng = random.Random(seed)
    r_prev = {v: rng.randint(0, 1) for v in nodes}
    r_curr = dict(r_prev)

    def display(v, r):
        c0 = base_color[v]
        return ((c0 + r) % 3) if c0 in (0, 1, 2) else c0  # leave boundary-defect colours alone

    history = []
    active0 = {v: depth[v] > 0 for v in nodes}
    history.append({
        "r": dict(r_curr),
        "display_color": {v: display(v, r_curr[v]) for v in nodes},
        "active": active0,
    })

    for t in range(0, max_depth):
        active = {v: depth[v] > t for v in nodes}
        r_next = dict(r_curr)
        for v in nodes:
            if not active[v]:
                continue
            gt = gate_type[v]
            if gt == "TOFFOLI":
                a, b = refs[v]
                gate = r_curr[a] & r_curr[b]
            elif gt == "CNOT":
                gate = r_curr[refs[v][0]]
            else:
                gate = 0
            r_next[v] = r_prev[v] ^ gate
        r_prev, r_curr = r_curr, r_next
        active_after = {v: depth[v] > t + 1 for v in nodes}
        history.append({
            "r": dict(r_curr),
            "display_color": {v: display(v, r_curr[v]) for v in nodes},
            "active": active_after,
        })

    return history, depth, max_depth, gate_type

if __name__ == "__main__":
    import pickle
    with open("hexinfo_1000.pkl", "rb") as f:
        hexinfo = pickle.load(f)
    hex_adj = hexinfo["hex_adj"]
    base_color = hexinfo["hex_color"]
    history, depth, max_depth, gate_type = run_hex_ca(hex_adj, base_color, seed=3)
    print("hex nodes:", len(hex_adj), " max causal depth:", max_depth, " steps:", len(history))
    from collections import Counter
    print("gate mix:", dict(Counter(gate_type.values())))
    c0 = Counter(history[0]["display_color"].values())
    cF = Counter(history[-1]["display_color"].values())
    print("colour distribution before:", dict(c0))
    print("colour distribution after: ", dict(cF))
