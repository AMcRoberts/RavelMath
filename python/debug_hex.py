import pickle, sys, time
from spectre_hex import build_hex_groups, build_hex_graph, three_color_hex_graph, spectre_four_coloring, hex_polygons

with open("tiles_1000.pkl", "rb") as f:
    d = pickle.load(f)
tiles = d["tiles"]
adjacency = [set(a) for a in d["adjacency"]]

t0=time.time(); print("start", flush=True)
group_of, group_members = build_hex_groups(tiles, adjacency)
print("groups done", len(group_members), time.time()-t0, flush=True)

hex_adj = build_hex_graph(tiles, adjacency, group_of)
print("hex_adj done", len(hex_adj), time.time()-t0, flush=True)

hex_color = three_color_hex_graph(hex_adj)
print("hex_color done", len(hex_color), time.time()-t0, flush=True)

spectre_colors = spectre_four_coloring(tiles, group_of, hex_color)
print("spectre_colors done", time.time()-t0, flush=True)

polys, R = hex_polygons(tiles, group_members)
print("polys done", time.time()-t0, flush=True)
