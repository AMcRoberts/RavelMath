"""
spectre_gen_lineage.py
------------------------------------------------------------------------
Identical tiling to spectre_gen.generate_tiling (same geometry, same
substitution rules, same crop), with ONE change: the recursive draw()
now records, for every base tile, its full substitution ancestor path

    addr = (i0, i1, ..., i_{L-1} [, g])

where i_k is the child slot chosen at inflation level k (root first)
and g is the extra Gamma1/Gamma2 split index when the base tile is a
mystic pair. This path is normally discarded by the flattening step in
spectre_gen -- nothing about the tiling is altered by keeping it.

The natural ULTRAMETRIC on tiles is:

    u(A, B) = L - common_prefix_length(addr_A, addr_B)

i.e. the inflation level of the least common ancestor supertile
(u = 0 would mean same tile; u = 1 means siblings inside one level-1
supertile; u = L means the two tiles only meet at the root).
This satisfies the strong triangle inequality u(A,C) <= max(u(A,B),
u(B,C)) by construction -- it's the standard p-adic-style metric on the
leaves of a substitution tree, the concrete analogue of the state-space
ultrametric in Palmer's invariant-set argument.
------------------------------------------------------------------------
"""
import math

from spectre_gen import (
    S3, SPECTRE_POINTS, CHEVRON_POINTS, IDENTITY,
    mul, trot, ttrans, transTo, transPt,
    TILE_NAMES, SUPER_RULES,
)


class LTile:
    def __init__(self, label):
        self.label = label
        self.quad = [SPECTRE_POINTS[3], SPECTRE_POINTS[5], SPECTRE_POINTS[7], SPECTRE_POINTS[11]]

    def draw(self, collector, T=IDENTITY, path=()):
        collector.append((self.label, T, path))


class LMetaTile:
    def __init__(self, geometries, quad):
        self.geometries = geometries
        self.quad = quad

    def draw(self, collector, T=IDENTITY, path=()):
        for i, (shape, shapeT) in enumerate(self.geometries):
            shape.draw(collector, mul(T, shapeT), path + (i,))


def build_spectre_base():
    base = {}
    for l in TILE_NAMES:
        if l != "Gamma":
            base[l] = LTile(l)
    mystic = LMetaTile(
        [
            (LTile("Gamma1"), IDENTITY),
            (LTile("Gamma2"), mul(ttrans(*SPECTRE_POINTS[8]), trot(math.pi / 6)))
        ],
        [SPECTRE_POINTS[3], SPECTRE_POINTS[5], SPECTRE_POINTS[7], SPECTRE_POINTS[11]]
    )
    base["Gamma"] = mystic
    return base


def build_supertiles(tile_system):
    quad = tile_system["Delta"].quad
    R = (-1, 0, 0, 0, 1, 0)
    rules = [(60, 3, 1), (0, 2, 0), (60, 3, 1), (60, 3, 1), (0, 2, 0), (60, 3, 1), (-120, 3, 3)]
    transformations = [IDENTITY]
    total_angle = 0
    rotation = IDENTITY
    transformed_quad = list(quad)
    for angle, frm, to in rules:
        if angle != 0:
            total_angle += angle
            rotation = trot(math.radians(total_angle))
            transformed_quad = [transPt(rotation, p) for p in quad]
        ttt = transTo(transformed_quad[to], transPt(transformations[-1], quad[frm]))
        transformations.append(mul(ttt, rotation))
    transformations = [mul(R, t) for t in transformations]

    super_quad = [
        transPt(transformations[6], quad[2]),
        transPt(transformations[5], quad[1]),
        transPt(transformations[3], quad[2]),
        transPt(transformations[0], quad[1])
    ]
    result = {}
    for label, subs in SUPER_RULES.items():
        geoms = []
        for i, s in enumerate(subs):
            if s:
                geoms.append((tile_system[s], transformations[i]))
        result[label] = LMetaTile(geoms, super_quad)
    return result


def generate_tiling_with_lineage(iterations=4, crop_count=500):
    """Same output as spectre_gen.generate_tiling plus tile['addr'].

    addr is root-first: addr[0] = which child of the top-level Delta
    supertile, ..., addr[-1] = deepest slot (or the Gamma1/Gamma2 split).
    """
    shapes = build_spectre_base()
    for _ in range(iterations):
        shapes = build_supertiles(shapes)
    collector = []
    shapes["Delta"].draw(collector, IDENTITY, ())

    tiles = []
    for label, T, path in collector:
        pts = [transPt(T, p) for p in SPECTRE_POINTS]
        chev = [transPt(T, p) for p in CHEVRON_POINTS]
        cx = sum(p[0] for p in chev) / len(chev)
        cy = sum(p[1] for p in chev) / len(chev)
        tip = chev[3]
        theta = math.degrees(math.atan2(T[3], T[0])) % 360.0
        tiles.append({"label": label, "pts": pts, "chev": chev, "cx": cx, "cy": cy,
                      "tip": tip, "theta": theta, "addr": path})

    gcx = sum(t["cx"] for t in tiles) / len(tiles)
    gcy = sum(t["cy"] for t in tiles) / len(tiles)
    for t in tiles:
        t["d"] = math.hypot(t["cx"] - gcx, t["cy"] - gcy)
    tiles.sort(key=lambda t: t["d"])
    return tiles[:min(crop_count, len(tiles))]


def ultrametric_level(addr_a, addr_b, iterations):
    """Inflation level of the least common ancestor supertile.

    Only the first `iterations` slots are substitution levels (a trailing
    Gamma1/Gamma2 index, if present, is a sub-tile split, not a level).
    Returns u in {0, ..., iterations}; u = iterations means the pair only
    shares the root.
    """
    a = addr_a[:iterations]
    b = addr_b[:iterations]
    c = 0
    for x, y in zip(a, b):
        if x != y:
            break
        c += 1
    return iterations - c


if __name__ == "__main__":
    # sanity check: geometry must be bit-identical to the original generator
    from spectre_gen import generate_tiling
    for it, crop in [(4, 700), (5, 1000)]:
        old = generate_tiling(iterations=it, crop_count=crop)
        new = generate_tiling_with_lineage(iterations=it, crop_count=crop)
        assert len(old) == len(new)
        for o, n in zip(old, new):
            assert o["label"] == n["label"]
            assert all(abs(po[0] - pn[0]) < 1e-12 and abs(po[1] - pn[1]) < 1e-12
                       for po, pn in zip(o["pts"], n["pts"]))
        depths = set(len(t["addr"]) for t in new)
        print(f"iterations={it} crop={crop}: geometry identical to original; "
              f"addr lengths present: {sorted(depths)}")
        # show a couple of addresses
        for t in new[:3]:
            print("   ", t["label"], t["addr"])
