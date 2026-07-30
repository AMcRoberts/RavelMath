import numpy as np

IDENTITY = [1, 0, 0, 0, 1, 0]
TILE_NAMES = ["Gamma", "Delta", "Theta", "Lambda", "Xi", "Pi", "Sigma", "Phi", "Psi"]

class pt:
    def __init__(self, x, y):
        self.x = x
        self.y = y

SPECTRE_POINTS = [
    pt(0, 0),
    pt(1.0, 0.0),
    pt(1.5, -np.sqrt(3)/2),
    pt(1.5+np.sqrt(3)/2, 0.5-np.sqrt(3)/2),
    pt(1.5+np.sqrt(3)/2, 1.5-np.sqrt(3)/2),
    pt(2.5+np.sqrt(3)/2, 1.5-np.sqrt(3)/2),
    pt(3+np.sqrt(3)/2, 1.5),
    pt(3.0, 2.0),
    pt(3-np.sqrt(3)/2, 1.5),
    pt(2.5-np.sqrt(3)/2, 1.5+np.sqrt(3)/2),
    pt(1.5-np.sqrt(3)/2, 1.5+np.sqrt(3)/2),
    pt(0.5-np.sqrt(3)/2, 1.5+np.sqrt(3)/2),
    pt(-np.sqrt(3)/2, 1.5),
    pt(0.0, 1.0)
]

def mul(A, B):
    return [
        A[0]*B[0] + A[1]*B[3],
        A[0]*B[1] + A[1]*B[4],
        A[0]*B[2] + A[1]*B[5] + A[2],
        A[3]*B[0] + A[4]*B[3],
        A[3]*B[1] + A[4]*B[4],
        A[3]*B[2] + A[4]*B[5] + A[5]
    ]

def trot(ang):
    c = np.cos(ang); s = np.sin(ang)
    return [c, -s, 0, s, c, 0]

def ttrans(tx, ty):
    return [1, 0, tx, 0, 1, ty]

def transTo(p, q):
    return ttrans(q.x - p.x, q.y - p.y)

def transPt(M, P):
    return pt(M[0]*P.x + M[1]*P.y + M[2], M[3]*P.x + M[4]*P.y + M[5])

OUTPUT_POLYS = []

def drawPolygon(T, label):
    pts = [transPt(T, p) for p in SPECTRE_POINTS]
    OUTPUT_POLYS.append((pts, label))

class Tile:
    def __init__(self, pts, label):
        self.quad = [pts[3], pts[5], pts[7], pts[11]]
        self.label = label
    def draw(self, T=IDENTITY):
        drawPolygon(T, self.label)

class MetaTile:
    def __init__(self, geometries=[], quad=[]):
        self.geometries = geometries
        self.quad = quad
    def draw(self, T=IDENTITY):
        for shape, shape_T in self.geometries:
            shape.draw(mul(T, shape_T))

def buildSpectreBase():
    cluster = {label: Tile(SPECTRE_POINTS, label) for label in TILE_NAMES if label != "Gamma"}
    mystic = MetaTile(
        [
            [Tile(SPECTRE_POINTS, "Gamma1"), IDENTITY],
            [Tile(SPECTRE_POINTS, "Gamma2"), mul(ttrans(SPECTRE_POINTS[8].x, SPECTRE_POINTS[8].y), trot(np.pi/6))]
        ],
        [SPECTRE_POINTS[3], SPECTRE_POINTS[5], SPECTRE_POINTS[7], SPECTRE_POINTS[11]]
    )
    cluster["Gamma"] = mystic
    return cluster

def buildSupertiles(tileSystem):
    quad = tileSystem["Delta"].quad
    R = [-1, 0, 0, 0, 1, 0]
    transformation_rules = [
        [60, 3, 1], [0, 2, 0], [60, 3, 1], [60, 3, 1],
        [0, 2, 0], [60, 3, 1], [-120, 3, 3]
    ]
    transformations = [IDENTITY]
    total_angle = 0
    rotation = IDENTITY
    transformed_quad = list(quad)
    for _angle, _from, _to in transformation_rules:
        if _angle != 0:
            total_angle += _angle
            rotation = trot(np.deg2rad(total_angle))
            transformed_quad = [transPt(rotation, qp) for qp in quad]
        ttt = transTo(transformed_quad[_to], transPt(transformations[-1], quad[_from]))
        transformations.append(mul(ttt, rotation))
    transformations = [mul(R, t) for t in transformations]

    super_rules = {
        "Gamma": ["Pi", "Delta", None, "Theta", "Sigma", "Xi", "Phi", "Gamma"],
        "Delta": ["Xi", "Delta", "Xi", "Phi", "Sigma", "Pi", "Phi", "Gamma"],
        "Theta": ["Psi", "Delta", "Pi", "Phi", "Sigma", "Pi", "Phi", "Gamma"],
        "Lambda": ["Psi", "Delta", "Xi", "Phi", "Sigma", "Pi", "Phi", "Gamma"],
        "Xi": ["Psi", "Delta", "Pi", "Phi", "Sigma", "Psi", "Phi", "Gamma"],
        "Pi": ["Psi", "Delta", "Xi", "Phi", "Sigma", "Psi", "Phi", "Gamma"],
        "Sigma": ["Xi", "Delta", "Xi", "Phi", "Sigma", "Pi", "Lambda", "Gamma"],
        "Phi": ["Psi", "Delta", "Psi", "Phi", "Sigma", "Pi", "Phi", "Gamma"],
        "Psi": ["Psi", "Delta", "Psi", "Phi", "Sigma", "Psi", "Phi", "Gamma"]
    }
    super_quad = [
        transPt(transformations[6], quad[2]),
        transPt(transformations[5], quad[1]),
        transPt(transformations[3], quad[2]),
        transPt(transformations[0], quad[1])
    ]
    return {
        label: MetaTile(
            [[tileSystem[sub], tr] for sub, tr in zip(subs, transformations) if sub],
            super_quad
        ) for label, subs in super_rules.items()
    }

N_ITERATIONS = 3
shapes = buildSpectreBase()
for _ in range(N_ITERATIONS):
    shapes = buildSupertiles(shapes)

shapes["Delta"].draw()

print("num tiles:", len(OUTPUT_POLYS))

xs = [p.x for pts, _ in OUTPUT_POLYS for p in pts]
ys = [p.y for pts, _ in OUTPUT_POLYS for p in pts]
print("bbox", min(xs), max(xs), min(ys), max(ys))

import json
data = []
for pts, label in OUTPUT_POLYS:
    data.append({"pts": [[round(p.x, 3), round(p.y, 3)] for p in pts], "label": label})

with open("/home/claude/tiles.json", "w") as f:
    json.dump(data, f)
