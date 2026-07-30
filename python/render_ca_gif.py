import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.patches import Polygon
import os
from PIL import Image

from spectre_gen import generate_tiling
from spectre_ca import run_ca

tiles = generate_tiling(iterations=4, crop_count=400)
history, depth, max_depth, gate_type = run_ca(tiles, seed=7)
from collections import Counter
print("tiles:", len(tiles), "max causal depth:", max_depth, "steps:", len(history))
print("gate mix:", dict(Counter(gate_type)))

xs = [p[0] for t in tiles for p in t["pts"]]
ys = [p[1] for t in tiles for p in t["pts"]]
minx, maxx = min(xs), max(xs)
miny, maxy = min(ys), max(ys)
padx = (maxx - minx) * 0.03
pady = (maxy - miny) * 0.03
minx -= padx; maxx += padx; miny -= pady; maxy += pady

W_IN, H_IN, DPI = 8, 8, 110
BG = "#111318"

ACTIVE_0 = "#FF6E6E"   # active cell, state 0
ACTIVE_1 = "#37D6C4"   # active cell, state 1
FROZEN_0 = "#2A2E38"   # frozen cell, state 0 (dark)
FROZEN_1 = "#565D6E"   # frozen cell, state 1 (lighter)
PEAK_EDGE = "#FFD54A"  # highlight ring on the deepest ("peak") tiles
NOT_EDGE = "#B983FF"   # highlight ring on tiles running the explicit NOT gate

os.makedirs("/home/claude/ca_frames", exist_ok=True)
frame_paths = []
max_d = max(depth)

for step, snap in enumerate(history):
    fig, ax = plt.subplots(figsize=(W_IN, H_IN), dpi=DPI)
    fig.patch.set_facecolor(BG)
    ax.set_facecolor(BG)
    ax.set_xlim(minx, maxx)
    ax.set_ylim(miny, maxy)
    ax.set_aspect("equal")
    ax.axis("off")
    fig.subplots_adjust(left=0, right=1, top=0.95, bottom=0)

    n_active = 0
    for i, t in enumerate(tiles):
        active = snap["active"][i]
        state = snap["state"][i]
        if active:
            n_active += 1
            fc = ACTIVE_1 if state else ACTIVE_0
            ec = "#0A0A0A"
            lw = 0.5
        else:
            fc = FROZEN_1 if state else FROZEN_0
            ec = "#000000"
            lw = 0.3
        is_peak = (depth[i] == max_d)
        poly = Polygon(t["pts"], closed=True, facecolor=fc, edgecolor=ec, linewidth=lw)
        ax.add_patch(poly)
        if gate_type[i] == "NOT":
            ring = Polygon(t["pts"], closed=True, facecolor="none",
                            edgecolor=NOT_EDGE, linewidth=1.1)
            ax.add_patch(ring)
        if is_peak:
            ring = Polygon(t["pts"], closed=True, facecolor="none",
                            edgecolor=PEAK_EDGE, linewidth=1.6)
            ax.add_patch(ring)

    n_not = sum(1 for g in gate_type if g == "NOT")
    caption = (f"step {step}/{len(history)-1}  ·  active: {n_active}  ·  depth 0-{max_d}  ·  "
               f"NOT-gate cells (violet ring): {n_not}")
    fig.text(0.5, 0.015, caption, ha="center", va="bottom", fontsize=11, color="#CCCCCC",
              family="DejaVu Sans")

    path = f"/home/claude/ca_frames/step_{step:03d}.png"
    fig.savefig(path, facecolor=BG)
    plt.close(fig)
    frame_paths.append(path)

images = [Image.open(p).convert("RGB") for p in frame_paths]
durations = [260] * len(images)
durations[0] = 900
durations[-1] = 2200

out_path = "/mnt/user-data/outputs/spectre-ca-freeze.gif"
images[0].save(
    out_path,
    save_all=True,
    append_images=images[1:],
    duration=durations,
    loop=0,
)
print("wrote", out_path)
