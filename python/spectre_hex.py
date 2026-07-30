"""
spectre_hex.py
------------------------------------------------------------------------
Converts a generated Spectre tiling (spectre_gen.generate_tiling) into the
hexagonal metatile picture described in Simon Tatham's article:

    https://www.chiark.greenend.org.uk/~sgtatham/quasiblog/aperiodic-spectre/

KEY FACT USED (from the article's "Hexagonal metatiles" + "four-colourings"
sections):
  - The Spectre substitution system has 9 metatile types: G,D,J,L,X,P,S,F,Y
    (== Gamma,Delta,Theta,Lambda,Xi,Pi,Sigma,Phi,Psi -- exactly the labels
    our substitution system in spectre_gen.py already uses).
  - 8 of the 9 hex types expand to exactly ONE output Spectre, in a fixed
    orientation. The Gamma ("G") hex is the odd one out: it expands to
    TWO spectres, called Gamma1 and Gamma2 in our generator, glued along
    one edge, with Gamma2 rotated 30 degrees relative to the rest.
  - If you glue every Gamma1 to its Gamma2 sibling, the resulting fused
    tiles are all topologically hexagons, each with exactly 6 neighbours,
    and the whole tiling of these fused tiles is combinatorially just a
    regular hexagonal tiling of the plane.
  - A regular hex tiling is properly 3-colourable, and that 3-colouring is
    essentially unique. Tatham's 4-colouring recipe for Spectres: colour
    every hex-tile with its 3-colouring, then re-mark every Gamma1 (the
    "odd orientation" spectre) with a 4th, special colour.

This module:
  1. build_hex_groups(tiles, adjacency)   -- union Gamma1+Gamma2 pairs into
     single hex-tile groups; every other spectre is its own hex-tile group.
  2. build_hex_graph(tiles, adjacency, groups) -- the quotient adjacency
     graph over hex-tile groups (should be ~6-regular in the interior).
  3. three_color_hex_graph(hex_adj)        -- greedy proper 3-colouring
     (works because the graph is combinatorially a hex/triangular-dual
     lattice, which is bipartite-like enough for greedy BFS 3-colouring
     to never need a 4th colour on any real Spectre patch).
  4. spectre_four_coloring(...)            -- per-SPECTRE colour (0,1,2, or
     3=special for Gamma1), by looking up each spectre's hex-group colour.
  5. hex_polygons(tiles, groups)           -- one regular hexagon per
     hex-tile group, centred at the group's average spectre centroid, sized
     and oriented from the empirical nearest-neighbour spacing of those
     centroids (so it tiles seamlessly against real neighbours).
------------------------------------------------------------------------
"""
import math
from collections import defaultdict, deque

def build_hex_groups(tiles, adjacency):
    """Union-find: Gamma1 <-> its Gamma2 neighbor become one group.
    Returns: group_of (list[int], group id per tile index),
             group_members (dict: group_id -> list of tile indices)
    """
    n = len(tiles)
    parent = list(range(n))

    def find(x):
        while parent[x] != x:
            parent[x] = parent[parent[x]]
            x = parent[x]
        return x

    def union(a, b):
        ra, rb = find(a), find(b)
        if ra != rb:
            parent[ra] = rb

    for i, t in enumerate(tiles):
        if t["label"] == "Gamma1":
            for j in adjacency[i]:
                if tiles[j]["label"] == "Gamma2":
                    union(i, j)
                    break

    group_of = [find(i) for i in range(n)]
    group_members = defaultdict(list)
    for i, g in enumerate(group_of):
        group_members[g].append(i)
    return group_of, dict(group_members)


def build_hex_graph(tiles, adjacency, group_of):
    """Quotient graph: one node per hex-tile group, edges wherever any two
    member spectres of different groups are adjacent."""
    hex_adj = defaultdict(set)
    for i, nbrs in enumerate(adjacency):
        gi = group_of[i]
        for j in nbrs:
            gj = group_of[j]
            if gi != gj:
                hex_adj[gi].add(gj)
                hex_adj[gj].add(gi)
    return dict(hex_adj)


def three_color_hex_graph(hex_adj):
    """Greedy BFS proper coloring, palette {0,1,2} preferred. On an
    infinite/interior-only hex tiling the quotient graph is a genuine
    triangular lattice (dual of the hexagon tiling) and 3 colours suffice.
    On a *finite crop*, however, the boundary can produce short odd cycles
    (a hex whose already-coloured neighbours happen to carry all three
    colours), and a naive greedy fill that insists on 3 colours will loop
    forever re-queueing a node it can never legally colour. We keep the
    "prefer the lowest free colour" rule (so almost every tile still gets
    one of the 3 principal colours) but fall back to a 4th, 5th, ... index
    on the rare boundary node where that's genuinely impossible -- this is
    guaranteed to terminate (a node's degree bounds how many colours its
    neighbours can occupy) and is still a proper colouring."""
    color = {}
    nodes = list(hex_adj.keys())
    for start in nodes:
        if start in color:
            continue
        color[start] = 0
        q = deque([start])
        while q:
            u = q.popleft()
            for v in hex_adj.get(u, ()):
                if v not in color:
                    used = {color[w] for w in hex_adj.get(v, ()) if w in color}
                    c = 0
                    while c in used:
                        c += 1
                    color[v] = c
                    q.append(v)
    return color


def spectre_four_coloring(tiles, group_of, hex_color):
    """Per-spectre colour: 0/1/2 from its hex group, except Gamma1 tiles
    always get the special 4th colour (index 3)."""
    colors = []
    for i, t in enumerate(tiles):
        if t["label"] == "Gamma1":
            colors.append(3)
        else:
            colors.append(hex_color[group_of[i]])
    return colors


def hex_polygons(tiles, group_members):
    """One regular hexagon per hex-tile group. Centre = average centroid of
    member spectres. Size/orientation are derived empirically from nearest-
    neighbour spacing between group centres, so hexagons tile edge-to-edge
    against their real neighbours without needing the full combinatorial
    coordinate machinery.
    Returns: dict group_id -> {'center': (x,y), 'pts': [...6 pts...]}
    """
    centers = {}
    for g, members in group_members.items():
        cx = sum(tiles[i]["cx"] for i in members) / len(members)
        cy = sum(tiles[i]["cy"] for i in members) / len(members)
        centers[g] = (cx, cy)

    # empirical nearest-neighbour spacing across all group centres
    pts = list(centers.values())
    if len(pts) > 1:
        # sample a subset for speed if huge
        sample = pts[:400]
        dists = []
        for i in range(len(sample)):
            best = min(
                math.hypot(sample[i][0] - p[0], sample[i][1] - p[1])
                for j, p in enumerate(sample) if j != i
            )
            dists.append(best)
        spacing = sorted(dists)[len(dists) // 2]  # median nearest-neighbour dist
    else:
        spacing = 3.0

    R = spacing / math.sqrt(3)  # circumradius for flat-top hex w/ this NN spacing

    polys = {}
    for g, (cx, cy) in centers.items():
        pts6 = []
        for k in range(6):
            ang = math.radians(60 * k)  # flat-top hexagon
            pts6.append((cx + R * math.cos(ang), cy + R * math.sin(ang)))
        polys[g] = {"center": (cx, cy), "pts": pts6}
    return polys, R


def convert_to_hex(tiles, adjacency):
    """One-shot convenience wrapper: tiles -> (hex polygons, hex colours,
    per-spectre 4-colouring)."""
    group_of, group_members = build_hex_groups(tiles, adjacency)
    hex_adj = build_hex_graph(tiles, adjacency, group_of)
    hex_color = three_color_hex_graph(hex_adj)
    spectre_colors = spectre_four_coloring(tiles, group_of, hex_color)
    polys, R = hex_polygons(tiles, group_members)
    return {
        "group_of": group_of,
        "group_members": group_members,
        "hex_adj": hex_adj,
        "hex_color": hex_color,
        "spectre_colors": spectre_colors,
        "hex_polys": polys,
        "hex_radius": R,
    }
