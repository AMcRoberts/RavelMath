import pickle, time
from spectre_hex import build_hex_groups, build_hex_graph

with open("tiles_1000.pkl", "rb") as f:
    d = pickle.load(f)
tiles = d["tiles"]
adjacency = [set(a) for a in d["adjacency"]]

group_of, group_members = build_hex_groups(tiles, adjacency)
hex_adj = build_hex_graph(tiles, adjacency, group_of)

# manual BFS coloring with step counting and time limit
from collections import deque
color = {}
nodes = list(hex_adj.keys())
print("n nodes", len(nodes), "n edges approx", sum(len(v) for v in hex_adj.values())//2)
steps = 0
t0 = time.time()
for start in nodes:
    if start in color:
        continue
    color[start] = 0
    q = deque([start])
    while q:
        u = q.popleft()
        for v in hex_adj.get(u, ()):
            steps += 1
            if steps % 200000 == 0:
                print("steps", steps, "colored", len(color), "elapsed", time.time()-t0, flush=True)
            if steps > 5_000_000:
                print("TOO MANY STEPS - likely infinite loop / huge degree issue")
                raise SystemExit(1)
            if v not in color:
                used = {color[w] for w in hex_adj.get(v, ()) if w in color}
                for c in (0,1,2):
                    if c not in used:
                        color[v] = c
                        break
                q.append(v)
print("done", len(color), time.time()-t0)
