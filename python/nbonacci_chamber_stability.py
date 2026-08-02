#!/usr/bin/env python3
"""Compare two replayable chamber rank certificates.

This is deliberately a read-only post-processing step: the expensive finite
automaton enumeration is done by ``nbonacci_sign_chamber_probe.py`` once per
box.  Given a smaller and a larger certificate, report shared chambers,
whether the smaller edge constraints are respected by the larger offsets, and
whether the canonical Bellman--Ford offsets agree up to one global translation.
"""

from __future__ import annotations

import argparse
import json
from collections import Counter


def load(path: str) -> dict:
    with open(path, encoding="utf-8") as stream:
        data = json.load(stream)
    if data.get("kind") != "nbonacci-sign-chamber-rank-v1":
        raise SystemExit(f"{path}: wrong certificate kind")
    rank = data.get("rank_certificate", {})
    if not rank.get("feasible") or rank.get("offsets") is None:
        raise SystemExit(f"{path}: certificate is not feasible")
    if len(rank["offsets"]) != len(data["chambers"]):
        raise SystemExit(f"{path}: offset/chamber mismatch")
    return data


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("smaller")
    parser.add_argument("larger")
    args = parser.parse_args()
    small, large = load(args.smaller), load(args.larger)
    for key in ("n", "mode", "modulus", "rank_base"):
        if small.get(key) != large.get(key):
            raise SystemExit(f"metadata mismatch for {key}: "
                             f"{small.get(key)!r} != {large.get(key)!r}")

    s_names = small["chambers"]
    l_names = large["chambers"]
    s_index = {name: i for i, name in enumerate(s_names)}
    l_index = {name: i for i, name in enumerate(l_names)}
    shared = sorted(set(s_index) & set(l_index))
    s_offsets, l_offsets = (small["rank_certificate"]["offsets"],
                            large["rank_certificate"]["offsets"])

    # Offsets are defined only up to translation.  If all differences agree,
    # the larger finite box has retained the smaller certificate verbatim on
    # the shared chamber set.
    differences = [l_offsets[l_index[name]] - s_offsets[s_index[name]]
                   for name in shared]
    counts = Counter(differences)
    dominant = counts.most_common(1)[0] if counts else (None, 0)
    stable = bool(shared) and len(counts) == 1

    # Check every smaller weighted edge directly against the larger offsets.
    edge_failures = 0
    for source, destination, weight in small["weighted_edges"]:
        source_name, destination_name = s_names[source], s_names[destination]
        if source_name not in l_index or destination_name not in l_index:
            continue
        u, v = l_index[source_name], l_index[destination_name]
        if l_offsets[v] < l_offsets[u] + weight:
            edge_failures += 1

    print(f"chamber stability: n={small['n']} mode={small['mode']} "
          f"small_bound={small['bound']} large_bound={large['bound']} "
          f"shared={len(shared)}/{len(s_names)} "
          f"offset_translation={'PASS' if stable else 'DIFF'} "
          f"dominant_delta={dominant[0]} ({dominant[1]}) "
          f"small_edges_violated_by_large={edge_failures}")
    return 0 if edge_failures == 0 else 1


if __name__ == "__main__":
    raise SystemExit(main())
