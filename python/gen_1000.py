import json
from spectre_gen import generate_tiling
from spectre_ca import build_adjacency

tiles = generate_tiling(iterations=5, crop_count=1000)
adjacency, is_boundary = build_adjacency(tiles)
print("tiles:", len(tiles))
print("boundary tiles:", sum(is_boundary))

# stash for reuse
import pickle
with open("tiles_1000.pkl", "wb") as f:
    pickle.dump({"tiles": tiles, "adjacency": [list(a) for a in adjacency], "is_boundary": is_boundary}, f)
