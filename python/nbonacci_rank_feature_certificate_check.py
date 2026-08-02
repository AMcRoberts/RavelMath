#!/usr/bin/env python3
"""Replay an integer feature-rank certificate without floating arithmetic."""

from __future__ import annotations

import argparse
import json

from nbonacci_rank_feature_search import features, parse


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("certificate")
    args = parser.parse_args()
    with open(args.certificate, encoding="utf-8") as stream:
        data = json.load(stream)
    if data.get("kind") != "nbonacci-rank-feature-v1":
        raise SystemExit("wrong certificate kind")
    names = data["chambers"]
    parsed = [parse(name) for name in names]
    sectors = {signs: index for index, signs in enumerate(
        sorted({item[0] for item in parsed}))}
    gap_count = len(parsed[0][1]) if parsed else 0
    vectors = [features(name, data["family"], sectors, gap_count)
               for name in names]
    coefficients = data["coefficients"]
    if any(len(vector) != len(coefficients) for vector in vectors):
        raise SystemExit("feature/coefficient length mismatch")
    for source, destination, weight in data["weighted_edges"]:
        left = sum(a * b for a, b in zip(vectors[source], coefficients))
        right = sum(a * b for a, b in zip(vectors[destination], coefficients))
        if right - left < weight:
            raise SystemExit(
                f"rank inequality failed: {source}->{destination} weight={weight}"
            )
    print(f"feature rank certificate PASS: family={data['family']} "
          f"chambers={len(names)} edges={len(data['weighted_edges'])}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
