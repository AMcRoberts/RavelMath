import math

S3 = math.sqrt(3)

def P(x, y):
    return (x, y)

SPECTRE_POINTS = [
    P(0,0), P(1,0), P(1.5,-S3/2),
    P(1.5+S3/2, 0.5-S3/2), P(1.5+S3/2, 1.5-S3/2),
    P(2.5+S3/2, 1.5-S3/2), P(3+S3/2, 1.5), P(3,2),
    P(3-S3/2,1.5), P(2.5-S3/2,1.5+S3/2), P(1.5-S3/2,1.5+S3/2),
    P(0.5-S3/2,1.5+S3/2), P(-S3/2,1.5), P(0,1)
]

CHEVRON_POINTS = [
    P(0,0), P(S3/2,0.5), P(S3/2,1.5), P(0,2), P(-S3/2,1.5), P(0,1)
]

IDENTITY = (1,0,0,0,1,0)

def mul(A, B):
    return (
        A[0]*B[0]+A[1]*B[3], A[0]*B[1]+A[1]*B[4], A[0]*B[2]+A[1]*B[5]+A[2],
        A[3]*B[0]+A[4]*B[3], A[3]*B[1]+A[4]*B[4], A[3]*B[2]+A[4]*B[5]+A[5]
    )

def trot(a):
    c, s = math.cos(a), math.sin(a)
    return (c,-s,0, s,c,0)

def ttrans(tx, ty):
    return (1,0,tx, 0,1,ty)

def transTo(p, q):
    return ttrans(q[0]-p[0], q[1]-p[1])

def transPt(M, p):
    return (M[0]*p[0]+M[1]*p[1]+M[2], M[3]*p[0]+M[4]*p[1]+M[5])

TILE_NAMES = ["Gamma","Delta","Theta","Lambda","Xi","Pi","Sigma","Phi","Psi"]

class Tile:
    def __init__(self, label):
        self.label = label
        self.quad = [SPECTRE_POINTS[3], SPECTRE_POINTS[5], SPECTRE_POINTS[7], SPECTRE_POINTS[11]]
    def draw(self, collector, T=IDENTITY):
        collector.append((self.label, T))

class MetaTile:
    def __init__(self, geometries, quad):
        self.geometries = geometries
        self.quad = quad
    def draw(self, collector, T=IDENTITY):
        for shape, shapeT in self.geometries:
            shape.draw(collector, mul(T, shapeT))

def build_spectre_base():
    base = {}
    for l in TILE_NAMES:
        if l != "Gamma":
            base[l] = Tile(l)
    mystic = MetaTile(
        [
            (Tile("Gamma1"), IDENTITY),
            (Tile("Gamma2"), mul(ttrans(*SPECTRE_POINTS[8]), trot(math.pi/6)))
        ],
        [SPECTRE_POINTS[3], SPECTRE_POINTS[5], SPECTRE_POINTS[7], SPECTRE_POINTS[11]]
    )
    base["Gamma"] = mystic
    return base

SUPER_RULES = {
    "Gamma":  ["Pi","Delta",None,"Theta","Sigma","Xi","Phi","Gamma"],
    "Delta":  ["Xi","Delta","Xi","Phi","Sigma","Pi","Phi","Gamma"],
    "Theta":  ["Psi","Delta","Pi","Phi","Sigma","Pi","Phi","Gamma"],
    "Lambda": ["Psi","Delta","Xi","Phi","Sigma","Pi","Phi","Gamma"],
    "Xi":     ["Psi","Delta","Pi","Phi","Sigma","Psi","Phi","Gamma"],
    "Pi":     ["Psi","Delta","Xi","Phi","Sigma","Psi","Phi","Gamma"],
    "Sigma":  ["Xi","Delta","Xi","Phi","Sigma","Pi","Lambda","Gamma"],
    "Phi":    ["Psi","Delta","Psi","Phi","Sigma","Pi","Phi","Gamma"],
    "Psi":    ["Psi","Delta","Psi","Phi","Sigma","Psi","Phi","Gamma"]
}

def build_supertiles(tile_system):
    quad = tile_system["Delta"].quad
    R = (-1,0,0, 0,1,0)
    rules = [(60,3,1),(0,2,0),(60,3,1),(60,3,1),(0,2,0),(60,3,1),(-120,3,3)]
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
        result[label] = MetaTile(geoms, super_quad)
    return result

COLOR_MAP = {
    "Gamma": "#FFFFFF", "Gamma1": "#FFFFFF", "Gamma2": "#FFFFFF",
    "Delta": "#DCDCDC", "Theta": "#FFBFBF", "Lambda": "#FFA07A",
    "Xi": "#FFF200", "Pi": "#87CEFA", "Sigma": "#F5F5DC",
    "Phi": "#00FF00", "Psi": "#00FFFF"
}

def generate_tiling(iterations=4, crop_count=500):
    shapes = build_spectre_base()
    for _ in range(iterations):
        shapes = build_supertiles(shapes)
    collector = []
    shapes["Delta"].draw(collector, IDENTITY)

    tiles = []
    for label, T in collector:
        pts = [transPt(T, p) for p in SPECTRE_POINTS]
        chev = [transPt(T, p) for p in CHEVRON_POINTS]
        cx = sum(p[0] for p in chev) / len(chev)
        cy = sum(p[1] for p in chev) / len(chev)
        tip = chev[3]
        theta = math.degrees(math.atan2(T[3], T[0])) % 360.0
        tiles.append({"label": label, "pts": pts, "chev": chev, "cx": cx, "cy": cy,
                      "tip": tip, "theta": theta})

    gcx = sum(t["cx"] for t in tiles) / len(tiles)
    gcy = sum(t["cy"] for t in tiles) / len(tiles)
    for t in tiles:
        t["d"] = math.hypot(t["cx"] - gcx, t["cy"] - gcy)
    tiles.sort(key=lambda t: t["d"])
    return tiles[:min(crop_count, len(tiles))]
