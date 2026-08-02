#!/usr/bin/env python3
"""Merge chamber rank constraints from several finite certificates.

Independent Bellman--Ford runs choose different canonical translations and
can acquire different longest-path offsets as a box grows.  This tool instead
identifies chambers by name, unions all weighted constraints, and searches for
one common integer potential.  A positive difference cycle is a sound
obstruction to that shared rank.
"""

from __future__ import annotations

import argparse
import json


def load(path: str) -> dict:
    with open(path, encoding="utf-8") as stream:
        data = json.load(stream)
    if data.get("kind") != "nbonacci-sign-chamber-rank-v1":
        raise SystemExit(f"{path}: wrong certificate kind")
    return data


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("certificates", nargs="+")
    parser.add_argument("--emit")
    args = parser.parse_args()
    records = [load(path) for path in args.certificates]
    for key in ("n", "mode", "modulus", "rank_base"):
        if len({record.get(key) for record in records}) != 1:
            raise SystemExit(f"metadata mismatch for {key}")

    names: dict[str, int] = {}
    edges: set[tuple[int, int, int]] = set()
    bounds = []
    sources = []
    for record, path in zip(records, args.certificates):
        bounds.append(record["bound"])
        sources.append(path)
        for name in record["chambers"]:
            names.setdefault(name, len(names))
        for source, destination, weight in record["weighted_edges"]:
            edges.add((names[record["chambers"][source]],
                       names[record["chambers"][destination]], weight))

    offsets = [0] * len(names)
    edge_list = sorted(edges)
    feasible = False
    for _ in range(len(names)):
        changed = False
        for source, destination, weight in edge_list:
            candidate = offsets[source] + weight
            if offsets[destination] < candidate:
                offsets[destination] = candidate
                changed = True
        if not changed:
            feasible = True
            break

    print(f"chamber merge: n={records[0]['n']} mode={records[0]['mode']} "
          f"bounds={bounds} chambers={len(names)} edges={len(edges)} "
          f"shared_rank={'PASS' if feasible else 'FAIL'}")
    if not feasible:
        return 1
    if args.emit:
        ordered = [name for name, _ in sorted(names.items(),
                                               key=lambda item: item[1])]
        output = {
            "kind": "nbonacci-sign-chamber-rank-merged-v1",
            "n": records[0]["n"], "mode": records[0]["mode"],
            "modulus": records[0]["modulus"],
            "rank_base": records[0]["rank_base"], "bounds": bounds,
            "sources": sources, "chambers": ordered,
            "weighted_edges": [list(edge) for edge in edge_list],
            "rank_certificate": {"feasible": True, "offsets": offsets},
        }
        with open(args.emit, "w", encoding="utf-8") as stream:
            json.dump(output, stream, indent=2, sort_keys=True)
            stream.write("\n")
        print(f"chamber merge: wrote {args.emit}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
