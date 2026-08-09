import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.patches import Polygon
import pickle
from PIL import Image

from spectre_gen import COLOR_MAP

with open("tiles_1000.pkl", "rb") as f:
    d = pickle.load(f)
tiles = d["tiles"]

with open("hexinfo_1000.pkl", "rb") as f:
    hexinfo = pickle.load(f)

HEX_PALETTE = {0: "#F76C6C", 1: "#4AC6B7", 2: "#5B7FDB", 3: "#F4B942", 4: "#B983FF"}

xs = [p[0] for t in tiles for p in t["pts"]]
ys = [p[1] for t in tiles for p in t["pts"]]
minx, maxx = min(xs), max(xs)
miny, maxy = min(ys), max(ys)
padx = (maxx - minx) * 0.02
pady = (maxy - miny) * 0.02
minx -= padx; maxx += padx; miny -= pady; maxy += pady

W_IN, H_IN, DPI = 9, 9, 130
BG = "#FAFAF8"

def new_ax():
    fig, ax = plt.subplots(figsize=(W_IN, H_IN), dpi=DPI)
    fig.patch.set_facecolor(BG)
    ax.set_facecolor(BG)
    ax.set_xlim(minx, maxx)
    ax.set_ylim(miny, maxy)
    ax.set_aspect("equal")
    ax.axis("off")
    fig.subplots_adjust(left=0, right=1, top=0.94, bottom=0)
    return fig, ax

# ---- Frame A: spectre monotile tiling, 9-type colouring ----
fig, ax = new_ax()
for t in tiles:
    fc = COLOR_MAP.get(t["label"], "#C8C8C8")
    poly = Polygon(t["pts"], closed=True, facecolor=fc, edgecolor="#141414", linewidth=0.5)
    ax.add_patch(poly)
fig.text(0.5, 0.015, f"Spectre monotile aperiodic tiling  ·  {len(tiles)} tiles",
          ha="center", va="bottom", fontsize=13, color="#333333", family="DejaVu Sans")
fig.savefig("/home/claude/spectre_1000.png", facecolor=BG)
plt.close(fig)

# ---- Frame B: hex metatile tiling (Tatham's construction), 3(+boundary)-colouring ----
fig, ax = new_ax()
for g, poly6 in hexinfo["hex_polys"].items():
    c = hexinfo["hex_color"][g]
    fc = HEX_PALETTE.get(c, "#999999")
    poly = Polygon(poly6["pts"], closed=True, facecolor=fc, edgecolor="#141414", linewidth=0.6)
    ax.add_patch(poly)
fig.text(0.5, 0.015, f"Hexagonal metatile tiling (Gamma1+Gamma2 fused)  ·  {len(hexinfo['hex_polys'])} hexes",
          ha="center", va="bottom", fontsize=13, color="#333333", family="DejaVu Sans")
fig.savefig("/home/claude/hex_1000.png", facecolor=BG)
plt.close(fig)

# ---- 2-frame swap animation ----
imgs = [Image.open("/home/claude/spectre_1000.png").convert("RGB"),
        Image.open("/home/claude/hex_1000.png").convert("RGB")]
imgs[0].save(
    "spectre-hex-swap.gif",
    save_all=True, append_images=[imgs[1]], duration=[1000, 1000], loop=0,
)
print("done")
