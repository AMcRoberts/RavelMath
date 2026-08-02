#!/usr/bin/env python3
"""Search small linear feature families for a uniform chamber rank.

The input is an emitted chamber certificate.  Its weighted edge ``w`` means
that a chamber correction h must satisfy h(v)-h(u) >= w.  We ask whether
``h = c dot features(chamber)`` satisfies all inequalities.  This is a
discovery/negative-result tool; successful floating coefficients require exact
rational reconstruction before becoming a proof object.
"""

from __future__ import annotations

import argparse
import json

import numpy as np
from scipy.optimize import linprog


def load(path: str) -> dict:
    with open(path, encoding="utf-8") as stream:
        data = json.load(stream)
    if data.get("kind") not in ("nbonacci-sign-chamber-rank-v1",
                                 "nbonacci-sign-chamber-rank-merged-v1"):
        raise SystemExit("wrong certificate kind")
    return data


def parse(name: str) -> tuple[str, list[int], int, int]:
    fields = name.split("|")
    signs = fields[0]
    gaps = [int(value) for value in fields[1].split(",")]
    minimum_residue = int(fields[2]) if len(fields) >= 3 else 0
    quotient = int(fields[3]) if len(fields) >= 4 else 0
    return signs, gaps, minimum_residue, quotient


def features(name: str, family: str, sectors: dict[str, int] | None = None,
             gap_count: int | None = None) -> list[float]:
    signs, gaps, residue, quotient = parse(name)
    sign_values = [1.0 if c == "+" else -1.0 if c == "-" else 0.0
                   for c in signs]
    result: list[float] = []
    if family in ("sector-gaps", "sector-gaps-scale"):
        if sectors is None or gap_count is None:
            raise ValueError("sector-gaps requires sector metadata")
        width = gap_count + (2 if family == "sector-gaps-scale" else 1)
        result = [0.0] * (len(sectors) * width)
        base = sectors[signs] * width
        result[base] = 1.0
        for index, value in enumerate(gaps):
            result[base + 1 + index] = float(value)
        if family == "sector-gaps-scale":
            result[base + 1 + gap_count] = float(quotient)
        return result
    if family in ("sign", "quadratic", "rich"):
        result += sign_values
    if family in ("gaps", "quadratic", "rich"):
        result += [float(value) for value in gaps]
    if family in ("quadratic", "rich"):
        result += [float(value * value) for value in gaps]
    if family == "rich":
        result += [float(gaps[i] - gaps[j])
                   for i in range(len(gaps)) for j in range(i)]
        result += [1.0 if value % 2 else 0.0 for value in gaps]
        result += [float(residue), float(quotient)]
    return result


def search(data: dict, family: str):
    names = data["chambers"]
    parsed = [parse(name) for name in names]
    sectors = {signs: index for index, signs in enumerate(
        sorted({item[0] for item in parsed}))}
    gap_count = len(parsed[0][1]) if parsed else 0
    vectors = np.asarray([features(name, family, sectors, gap_count)
                          for name in names])
    rows, rhs = [], []
    for source, destination, weight in data["weighted_edges"]:
        rows.append(-(vectors[destination] - vectors[source]))
        rhs.append(-float(weight))
    # A zero objective asks only for feasibility.  Coefficients are free;
    # fix the irrelevant additive constant by omitting an intercept.
    result = linprog(np.zeros(vectors.shape[1]), A_ub=np.asarray(rows),
                     b_ub=np.asarray(rhs), bounds=[(None, None)] * vectors.shape[1],
                     method="highs")
    if not result.success:
        return result, None, 0.0, vectors
    residual = vectors @ result.x
    slack = np.asarray([residual[d] - residual[s] - w
                        for s, d, w in data["weighted_edges"]])
    return result, result.x, float(slack.min(initial=0.0)), vectors


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("certificate")
    parser.add_argument("--family", choices=("sign", "gaps", "quadratic", "rich", "sector-gaps", "sector-gaps-scale"),
                        action="append")
    parser.add_argument("--emit", help="write a rounded integer coefficient certificate")
    args = parser.parse_args()
    data = load(args.certificate)
    families = args.family or ["sign", "gaps", "quadratic", "rich", "sector-gaps", "sector-gaps-scale"]
    for family in families:
        result, coefficients, minimum_slack, vectors = search(data, family)
        if coefficients is None:
            print(f"feature search: family={family} INFEASIBLE "
                  f"status={result.status}")
            continue
        print(f"feature search: family={family} FEASIBLE "
              f"features={len(coefficients)} min_slack={minimum_slack:.3g}")
        print("  coefficients=" + " ".join(f"{value:.8g}" for value in coefficients))
        rounded = np.rint(coefficients).astype(np.int64)
        integer_slack = np.asarray([
            np.dot(vectors[destination] - vectors[source], rounded) - weight
            for source, destination, weight in data["weighted_edges"]
        ])
        exact = bool(np.all(integer_slack >= 0))
        print(f"  rounded_integer_replay={'PASS' if exact else 'FAIL'}")
        if args.emit and len(families) == 1:
            output = {
                "kind": "nbonacci-rank-feature-v1",
                "source": args.certificate, "family": family,
                "chambers": data["chambers"],
                "weighted_edges": data["weighted_edges"],
                "coefficients": rounded.tolist(),
                "rounded_integer_replay": exact,
            }
            with open(args.emit, "w", encoding="utf-8") as stream:
                json.dump(output, stream, indent=2, sort_keys=True)
                stream.write("\n")
            print(f"  wrote={args.emit}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
