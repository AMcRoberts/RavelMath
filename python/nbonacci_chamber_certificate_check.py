#!/usr/bin/env python3
"""Replay an emitted n-bonacci chamber difference-constraints certificate."""

from __future__ import annotations

import argparse
import json


def sign_negation(name: str) -> str:
    """Apply the carry automaton's x -> -x symmetry to a chamber name."""
    fields = name.split("|", 1)
    signs = fields[0]
    flipped = "".join("+" if char == "-" else "-" if char == "+" else "0"
                      for char in signs)
    return flipped if len(fields) == 1 else flipped + "|" + fields[1]


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("certificate")
    args = parser.parse_args()
    with open(args.certificate, encoding="utf-8") as stream:
        data = json.load(stream)
    if data.get("kind") not in ("nbonacci-sign-chamber-rank-v1",
                                 "nbonacci-sign-chamber-rank-merged-v1"):
        raise SystemExit("wrong certificate kind")
    chambers = data["chambers"]
    edges = [tuple(edge) for edge in data["weighted_edges"]]
    offsets = data["rank_certificate"]["offsets"]
    if not data["rank_certificate"]["feasible"] or offsets is None:
        raise SystemExit("certificate records infeasible rank constraints")
    if len(offsets) != len(chambers):
        raise SystemExit("offset/chamber length mismatch")
    chamber_index = {name: index for index, name in enumerate(chambers)}
    symmetry_pairs = 0
    for name, index in chamber_index.items():
        partner = sign_negation(name)
        if partner not in chamber_index:
            raise SystemExit(f"missing sign-negation chamber for {name}")
        if offsets[chamber_index[partner]] != offsets[index]:
            raise SystemExit(f"sign-negation offset mismatch for {name}")
        symmetry_pairs += 1
    for source, destination, weight in edges:
        if not (0 <= source < len(chambers) and
                0 <= destination < len(chambers)):
            raise SystemExit("edge references unknown chamber")
        if offsets[destination] < offsets[source] + weight:
            raise SystemExit("difference constraint failed")

    # A positive cycle is exactly the obstruction to an integer rank offset.
    heights = [0] * len(chambers)
    for _ in range(len(chambers)):
        changed = False
        for source, destination, weight in edges:
            candidate = heights[source] + weight
            if heights[destination] < candidate:
                heights[destination] = candidate
                changed = True
        if not changed:
            bound = data.get("bound", data.get("bounds"))
            print(f"chamber certificate PASS: n={data['n']} bound={bound} "
                  f"chambers={len(chambers)} edges={len(edges)} "
                  f"sign_symmetry={symmetry_pairs}")
            return 0
    raise SystemExit("positive difference-constraint cycle found")


if __name__ == "__main__":
    raise SystemExit(main())
