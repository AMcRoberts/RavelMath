import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.patches import Polygon
import pickle
from PIL import Image
from spectre_hex_ca import run_hex_ca

with open("hexinfo_1000.pkl", "rb") as f:
    hexinfo = pickle.load(f)
hex_polys = hexinfo["hex_polys"]
hex_adj = hexinfo["hex_color"]
base_color = hexinfo["hex_color"]

history, depth, max_depth, gate_type = run_hex_ca(hexinfo["hex_adj"], base_color, seed=3)
print("steps:", len(history), "max_depth:", max_depth)

PALETTE = {0: "#F76C6C", 1: "#4AC6B7", 2: "#5B7FDB", 3: "#F4B942", 4: "#B983FF"}

allpts = [p for poly in hex_polys.values() for p in poly["pts"]]
xs = [p[0] for p in allpts]; ys = [p[1] for p in allpts]
minx, maxx = min(xs), max(xs); miny, maxy = min(ys), max(ys)
padx = (maxx-minx)*0.02; pady = (maxy-miny)*0.02
minx -= padx; maxx += padx; miny -= pady; maxy += pady

W_IN, H_IN, DPI = 9, 9, 130
BG = "#FAFAF8"

frame_paths = []
for step, snap in enumerate(history):
    fig, ax = plt.subplots(figsize=(W_IN, H_IN), dpi=DPI)
    fig.patch.set_facecolor(BG); ax.set_facecolor(BG)
    ax.set_xlim(minx, maxx); ax.set_ylim(miny, maxy)
    ax.set_aspect("equal"); ax.axis("off")
    fig.subplots_adjust(left=0, right=1, top=0.94, bottom=0)
    n_switched = 0
    for g, poly in hex_polys.items():
        c = snap["display_color"][g]
        if c != base_color[g] and base_color[g] in (0,1,2):
            n_switched += 1
        fc = PALETTE.get(c, "#999999")
        ax.add_patch(Polygon(poly["pts"], closed=True, facecolor=fc, edgecolor="#141414", linewidth=0.6))
    fig.text(0.5, 0.015,
              f"hex recolour CA · step {step}/{len(history)-1} · switched this frame: {n_switched}/{len(hex_polys)}",
              ha="center", va="bottom", fontsize=12, color="#333333", family="DejaVu Sans")
    path = f"/home/claude/hexca_step_{step}.png"
    fig.savefig(path, facecolor=BG)
    plt.close(fig)
    frame_paths.append(path)

imgs = [Image.open(p).convert("RGB") for p in frame_paths]
durations = [1200]*len(imgs)
imgs[0].save("/mnt/user-data/outputs/spectre-hex-recolor.gif",
             save_all=True, append_images=imgs[1:], duration=durations, loop=0)
print("wrote gif with", len(imgs), "frames")
