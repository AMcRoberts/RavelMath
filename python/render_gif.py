import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.patches import Polygon
from matplotlib.lines import Line2D
import os
from PIL import Image

from spectre_gen import generate_tiling, COLOR_MAP

tiles = generate_tiling(iterations=4, crop_count=500)

xs = [p[0] for t in tiles for p in t["pts"]]
ys = [p[1] for t in tiles for p in t["pts"]]
minx, maxx = min(xs), max(xs)
miny, maxy = min(ys), max(ys)
padx = (maxx - minx) * 0.03
pady = (maxy - miny) * 0.03
minx -= padx; maxx += padx; miny -= pady; maxy += pady

W_IN, H_IN, DPI = 8, 8, 110
BG = "#FAFAF8"

captions = [
    "1/3 - colored tiles",
    "2/3 - marked colored tiles",
    "3/3 - just the marking",
]

os.makedirs("/home/claude/frames", exist_ok=True)
frame_paths = []

def new_ax():
    fig, ax = plt.subplots(figsize=(W_IN, H_IN), dpi=DPI)
    fig.patch.set_facecolor(BG)
    ax.set_facecolor(BG)
    ax.set_xlim(minx, maxx)
    ax.set_ylim(miny, maxy)
    ax.set_aspect("equal")
    ax.axis("off")
    fig.subplots_adjust(left=0, right=1, top=0.95, bottom=0)
    return fig, ax

def add_shapes(ax, key, fill_mode):
    for t in tiles:
        pts = t[key]
        if fill_mode == "color":
            fc = COLOR_MAP.get(t["label"], "#C8C8C8")
            ec = "#141414"
            lw = 0.6
        else:  # neutral
            fc = "#FFFFFF"
            ec = "#969696"
            lw = 0.6
        poly = Polygon(pts, closed=True, facecolor=fc, edgecolor=ec, linewidth=lw)
        ax.add_patch(poly)

def add_marks(ax):
    for t in tiles:
        line = Line2D([t["cx"], t["tip"][0]], [t["cy"], t["tip"][1]],
                       color="#D21E1E", linewidth=1.1, solid_capstyle="round")
        ax.add_line(line)

def save_frame(fig, idx, caption):
    fig.text(0.5, 0.015, caption, ha="center", va="bottom", fontsize=13, color="#444444",
              family="DejaVu Sans")
    path = f"/home/claude/frames/frame_{idx}.png"
    fig.savefig(path, facecolor=BG)
    plt.close(fig)
    frame_paths.append(path)

# Frame 1: colored spectre tiles, no marking
fig, ax = new_ax(); add_shapes(ax, "pts", "color"); save_frame(fig, 0, captions[0])

# Frame 2: colored spectre tiles + marking
fig, ax = new_ax(); add_shapes(ax, "pts", "color"); add_marks(ax); save_frame(fig, 1, captions[1])

# Frame 3: marking only
fig, ax = new_ax(); add_marks(ax); save_frame(fig, 2, captions[2])

# Assemble GIF: hold each frame, with a longer hold on the first and last.
images = [Image.open(p).convert("RGB") for p in frame_paths]
durations = [1400, 1400, 2200]

out_path = "spectre-transition.gif"
images[0].save(
    out_path,
    save_all=True,
    append_images=images[1:],
    duration=durations,
    loop=0,
)
print("wrote", out_path)
