from spectre_chsh import run_chsh
import json

results = []
for crop in [700, 1000]:
    for seed in [1, 7, 42]:
        r = run_chsh(crop_count=crop, seed=seed, n_pairs=4000, rng_seed=1234)
        r["crop_count"] = crop
        r["seed"] = seed
        results.append(r)
        print(f"crop={crop} seed={seed}: max_depth={r['max_depth']} pairs={r['n_pairs']} "
              f"S={r['S']:+.4f} |S|={abs(r['S']):.4f}")

with open("chsh_results.json", "w") as f:
    json.dump(results, f, indent=2)
