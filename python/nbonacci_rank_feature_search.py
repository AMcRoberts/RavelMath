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
from scipy import sparse
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


def sector_label(signs: str, gaps: list[int], family: str, minimum_residue: int = 0) -> str:
    if family in ("sector-order-gaps", "sector-order-gaps-scale", "sector-order-quadratic-scale", "sector-order-state-affine", "sector-order-residue-affine"):
        levels = {value: index for index, value in enumerate(sorted(set(gaps)))}
        label = signs + "#" + ",".join(str(levels[value]) for value in gaps)
        if family == "sector-order-residue-affine":
            return label + "#" + str(minimum_residue)
        return label
    if family in ("sector-gaps-mask", "mask-gaps"):
        mask = "".join("1" if value == 0 else "0" for value in gaps)
        return (signs + "#" + mask) if family == "sector-gaps-mask" else mask
    if family == "order-gaps":
        levels = {value: index for index, value in enumerate(sorted(set(gaps)))}
        return ",".join(str(levels[value]) for value in gaps)
    if family == "order-sign-interaction":
        levels = {value: index for index, value in enumerate(sorted(set(gaps)))}
        return ",".join(str(levels[value]) for value in gaps)
    return signs


def features(name: str, family: str, sectors: dict[str, int] | None = None,
             gap_count: int | None = None) -> list[float]:
    signs, gaps, residue, quotient = parse(name)
    sign_values = [1.0 if c == "+" else -1.0 if c == "-" else 0.0
                   for c in signs]
    result: list[float] = []
    if family in ("sector-gaps", "sector-gaps-scale", "sector-gaps-mask", "mask-gaps", "order-gaps", "sector-order-gaps", "sector-order-gaps-scale", "sector-order-state-affine", "sector-order-residue-affine"):
        if sectors is None or gap_count is None:
            raise ValueError("sector-gaps requires sector metadata")
        width = gap_count + (2 if family == "sector-gaps-scale" else 1)
        if family == "sector-order-gaps":
            width = gap_count + 1
        elif family == "sector-order-gaps-scale":
            width = gap_count + 2
        elif family == "sector-order-state-affine":
            width = gap_count + 3
        elif family == "sector-order-residue-affine":
            width = gap_count + 2
        elif family == "sector-order-gaps-scale":
            width = gap_count + 2
        result = [0.0] * (len(sectors) * width)
        base = sectors[sector_label(signs, gaps, family, residue)] * width
        result[base] = 1.0
        for index, value in enumerate(gaps):
            result[base + 1 + index] = float(value)
        if family in ("sector-gaps-scale", "sector-order-gaps-scale"):
            result[base + 1 + gap_count] = float(quotient)
        elif family == "sector-order-state-affine":
            result[base + 1 + gap_count] = float(residue)
            result[base + 2 + gap_count] = float(quotient)
        elif family == "sector-order-residue-affine":
            result[base + 1 + gap_count] = float(quotient)
        return result
    if family == "sign-interaction":
        result = sign_values + [float(value) for value in gaps]
        result += [sign * gap for sign in sign_values for gap in gaps]
        return result
    if family == "order-sign-interaction":
        levels = {value: index for index, value in enumerate(sorted(set(gaps)))}
        order = ",".join(str(levels[value]) for value in gaps)
        order_values = [order == key for key in sectors]
        result = sign_values + [float(value) for value in gaps]
        result += [float(value) for value in order_values]
        result += [sign * gap for sign in sign_values for gap in gaps]
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
    sectors = {key: index for index, key in enumerate(sorted(
        {sector_label(item[0], item[1], family, item[2]) for item in parsed}))}
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


def feature_count(data: dict, family: str) -> int:
    parsed = [parse(name) for name in data["chambers"]]
    if not parsed:
        return 0
    sectors = {sector_label(item[0], item[1], family, item[2]) for item in parsed}
    width = len(parsed[0][1]) + (2 if family in ("sector-gaps-scale", "sector-order-gaps-scale") else 1)
    if family == "sector-order-quadratic-scale":
        q = len(parsed[0][1]) + 1
        width = 1 + q + q * (q + 1) // 2
    elif family == "sector-order-state-affine":
        width = len(parsed[0][1]) + 3
    elif family == "sector-order-residue-affine":
        width = len(parsed[0][1]) + 2
    if family in ("sector-gaps", "sector-gaps-scale", "sector-gaps-mask",
                  "mask-gaps", "order-gaps", "sector-order-gaps", "sector-order-gaps-scale",
                  "sector-order-quadratic-scale", "sector-order-state-affine",
                  "sector-order-residue-affine"):
        return len(sectors) * width
    if family == "sign-interaction":
        n = len(parsed[0][1])
        return 2 * n + n * n
    if family == "order-sign-interaction":
        n = len(parsed[0][1])
        return len(sectors) + 2 * n + n * n
    return len(features(data["chambers"][0], family))


def sparse_sector_search(data: dict, family: str):
    """Build only the nonzero support of sector-affine features.

    A chamber has one intercept and a handful of gap/scale coordinates, so
    the edge matrix is sparse even when the number of local sectors is large.
    """
    names = data["chambers"]
    parsed = [parse(name) for name in names]
    labels = [sector_label(item[0], item[1], family, item[2]) for item in parsed]
    sectors = {key: index for index, key in enumerate(sorted(set(labels)))}
    gap_count = len(parsed[0][1]) if parsed else 0
    width = gap_count + (2 if family in ("sector-gaps-scale", "sector-order-gaps-scale") else 1)
    if family == "sector-order-gaps":
        width = gap_count + 1
    elif family == "sector-order-gaps-scale":
        width = gap_count + 2
    elif family == "sector-order-quadratic-scale":
        q = gap_count + 1
        width = 1 + q + q * (q + 1) // 2
    elif family == "sector-order-state-affine":
        width = gap_count + 3
    elif family == "sector-order-residue-affine":
        width = gap_count + 2

    def terms(index):
        _, gaps, residue, quotient = parsed[index]
        gap_values = [float(value) for value in gaps]
        values = gap_values + [float(residue), float(quotient)]
        if family == "sector-order-quadratic-scale":
            poly = gap_values + [float(quotient)]
            return [1.0] + poly + [poly[i] * poly[j]
                                    for i in range(len(poly))
                                    for j in range(i, len(poly))]
        if family == "sector-order-state-affine":
            return [1.0] + values
        if family == "sector-order-residue-affine":
            return [1.0] + gap_values + [float(quotient)]
        return [1.0] + gap_values + ([float(quotient)] if family in
                                      ("sector-gaps-scale", "sector-order-gaps-scale") else [])
    rows, cols, vals = [], [], []
    # A_ub row is feature(source) - feature(destination).
    for row, (source, destination, weight) in enumerate(data["weighted_edges"]):
        sbase = sectors[labels[source]] * width
        dbase = sectors[labels[destination]] * width
        for offset, value in enumerate(terms(source)):
            if value:
                rows.append(row); cols.append(sbase + offset); vals.append(value)
        for offset, value in enumerate(terms(destination)):
            if value:
                rows.append(row); cols.append(dbase + offset); vals.append(-value)
    matrix = sparse.coo_matrix((vals, (rows, cols)),
                               shape=(len(data["weighted_edges"]), len(sectors) * width)).tocsr()
    rhs = -np.asarray([float(edge[2]) for edge in data["weighted_edges"]])
    result = linprog(np.zeros(matrix.shape[1]), A_ub=matrix, b_ub=rhs,
                     bounds=[(None, None)] * matrix.shape[1], method="highs")
    if not result.success:
        return result, None, 0.0, sectors, width
    # For A x <= rhs, the desired rank slack is rhs - A x.
    slack = rhs - matrix @ result.x
    return result, result.x, float(slack.min(initial=0.0)), sectors, width


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("certificate")
    parser.add_argument("--family", choices=("sign", "gaps", "quadratic", "rich", "sector-gaps", "sector-gaps-scale", "sector-gaps-mask", "mask-gaps", "order-gaps", "sector-order-gaps", "sector-order-gaps-scale", "sector-order-quadratic-scale", "sector-order-state-affine", "sector-order-residue-affine", "sign-interaction", "order-sign-interaction"),
                        action="append")
    parser.add_argument("--emit", help="write a rounded integer coefficient certificate")
    parser.add_argument("--max-features", type=int, default=5000,
                        help="skip LP families larger than this (default: 5000)")
    args = parser.parse_args()
    data = load(args.certificate)
    families = args.family or ["sign", "gaps", "quadratic", "rich", "sector-gaps", "sector-gaps-scale", "sector-gaps-mask", "mask-gaps", "order-gaps", "sector-order-gaps", "sector-order-gaps-scale", "sector-order-quadratic-scale", "sector-order-state-affine", "sector-order-residue-affine", "sign-interaction", "order-sign-interaction"]
    for family in families:
        count = feature_count(data, family)
        if count > args.max_features:
            print(f"feature search: family={family} INCONCLUSIVE "
                  f"features={count} > cap={args.max_features}")
            continue
        if family in ("sector-order-gaps", "sector-order-gaps-scale", "sector-order-quadratic-scale", "sector-order-state-affine", "sector-order-residue-affine"):
            result, coefficients, minimum_slack, sectors, width = sparse_sector_search(data, family)
            vectors = None
        else:
            result, coefficients, minimum_slack, vectors = search(data, family)
        if coefficients is None:
            print(f"feature search: family={family} INFEASIBLE "
                  f"status={result.status}")
            continue
        print(f"feature search: family={family} FEASIBLE "
              f"features={len(coefficients)} min_slack={minimum_slack:.3g}")
        if len(coefficients) <= 500:
            print("  coefficients=" + " ".join(f"{value:.8g}" for value in coefficients))
        else:
            print("  coefficient_summary="
                  f"nonzero={np.count_nonzero(np.abs(coefficients) > 1e-10)} "
                  f"min={coefficients.min():.6g} max={coefficients.max():.6g}")
        rounded = np.rint(coefficients).astype(np.int64)
        if family in ("sector-order-gaps", "sector-order-gaps-scale", "sector-order-quadratic-scale", "sector-order-state-affine", "sector-order-residue-affine"):
            parsed = [parse(name) for name in data["chambers"]]
            labels = [sector_label(item[0], item[1], family, item[2]) for item in parsed]
            integer_slack = []
            for source, destination, weight in data["weighted_edges"]:
                def rank(index):
                    _, gaps, _, _ = parsed[index]
                    base = sectors[labels[index]] * width
                    values = [int(value) for value in gaps] + [int(parsed[index][2]), int(parsed[index][3])]
                    if family == "sector-order-quadratic-scale":
                        terms = [1] + values + [values[i] * values[j]
                                                for i in range(len(values))
                                                for j in range(i, len(values))]
                    elif family == "sector-order-state-affine":
                        terms = [1] + values
                    elif family == "sector-order-residue-affine":
                        terms = [1] + values[:-2] + [values[-1]]
                    else:
                        terms = [1] + values[:-1] + ([values[-1]] if family ==
                                                      "sector-order-gaps-scale" else [])
                    return int(sum(rounded[base + i] * term
                                   for i, term in enumerate(terms)))
                integer_slack.append(rank(destination) - rank(source) - weight)
            integer_slack = np.asarray(integer_slack)
        else:
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
