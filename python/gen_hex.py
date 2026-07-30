import pickle
from spectre_hex import convert_to_hex

with open("tiles_1000.pkl", "rb") as f:
    d = pickle.load(f)
tiles = d["tiles"]
adjacency = [set(a) for a in d["adjacency"]]

hexinfo = convert_to_hex(tiles, adjacency)
print("num hex groups:", len(hexinfo["group_members"]))
print("hex colors used:", set(hexinfo["hex_color"].values()))
print("hex radius:", hexinfo["hex_radius"])

with open("hexinfo_1000.pkl", "wb") as f:
    pickle.dump(hexinfo, f)
