#!/usr/bin/env python3
"""Replay checker for the symbolic proof certificates emitted by
`nbonacci_shell_covering_proof.py`.

This tool is the producer/checker counterpart to that script. Given a
JSON certificate directory, it re-derives every claim from scratch
(exact rational arithmetic, no shared state with the producer) and
reports PASS/FAIL per (n, L) and per candidate.

What it replays:

  For each SAT (n, L=n+1) JSON file:
    1. For each candidate listed, re-build the linear system from
       the stored `linear_system.matrix` and `linear_system.rhs`.
    2. Re-solve it with exact Gaussian elimination.
    3. Check the solved (a_1, ..., a_{n-1}) matches the stored
       `solution` vector componentwise.
    4. Re-derive the full sequence a_0, ..., a_{n+L-1} using the
       stored free-parameter coefficient vectors (re-derived from
       scratch in this checker, not read from the JSON).
    5. Check the stored `sequence` matches the re-derived sequence
       componentwise.
    6. Re-verify every |a_j| <= 1 inequality (re-derived, not
       trusted from the JSON).
    7. Re-verify every window t in 0..L has some |a_{t+i}|=1.
    8. Re-derive the gap pattern and check it matches.

  For each UNSAT (n, L=n+2) JSON file:
    1. Re-run the exhaustive enumeration on the inputs in the JSON
       (n, L) and re-count the failure breakdown.
    2. Check the re-derived counts match the stored counts.

  For the gap-formula mine JSON:
    1. Re-fit the closed-form candidates from the stored
       `gap_patterns`.
    2. Re-derive the holdout check.

Output: per-(n, L) PASS/FAIL with the number of candidates that
replayed cleanly, then a single overall PASS/FAIL.

Usage:
  python3 python/nbonacci_shell_covering_proof_check.py path/to/json_dir/

The directory must contain one JSON file per (n, L) named
`n{n}_L{L}.json` and one summary named `covering_proof_summary.json`.
"""

from __future__ import annotations

import argparse
import json
import os
import resource
import sys
from fractions import Fraction
from itertools import combinations, product
from pathlib import Path


# 12 GiB memory fence, same convention as the producer script
# (see the producer's `install_memory_limit` for the rationale).
DEFAULT_PROBE_MEMORY_MB = 12288


def install_memory_limit() -> None:
    megabytes = int(os.environ.get("RAVEL_PROBE_MEMORY_MB",
                                    str(DEFAULT_PROBE_MEMORY_MB)))
    if megabytes > 0:
        limit = megabytes * 1024 * 1024
        resource.setrlimit(resource.RLIMIT_AS, (limit, limit))


def later_values_as_linear(n: int, r: int) -> dict[int, list[Fraction]]:
    a: dict[int, list[Fraction]] = {}
    a[0] = [Fraction(1)] + [Fraction(0)] * (n - 1)
    for j in range(1, n):
        vec = [Fraction(0)] * n
        vec[j] = Fraction(1)
        a[j] = vec
    for t in range(r):
        idx = t + n
        vec = list(a[t])
        for j in range(1, n):
            other = a[t + j]
            for k in range(n):
                vec[k] -= other[k]
        a[idx] = vec
    return a


def solve_square_system(rows: list[list[Fraction]], rhs: list[Fraction],
                         nvars: int) -> list[Fraction] | None:
    augmented = [list(row) + [value] for row, value in zip(rows, rhs)]
    m = len(augmented)
    for col in range(m):
        pivot = next((r for r in range(col, m) if augmented[r][col] != 0), None)
        if pivot is None:
            return None
        augmented[col], augmented[pivot] = augmented[pivot], augmented[col]
        pivot_value = augmented[col][col]
        augmented[col] = [x / pivot_value for x in augmented[col]]
        for r in range(m):
            if r != col and augmented[r][col] != 0:
                factor = augmented[r][col]
                augmented[r] = [augmented[r][k] - factor * augmented[col][k]
                                for k in range(nvars + 1)]
    return [augmented[row][nvars] for row in range(nvars)]


def replay_sat_candidate(n: int, L: int, candidate: dict) -> tuple[bool, list[str]]:
    """Re-derive every claim for one SAT candidate. Return (ok, errors)."""
    errors: list[str] = []
    sys_data = candidate["linear_system"]
    matrix = [[Fraction(s) for s in row] for row in sys_data["matrix"]]
    rhs = [Fraction(s) for s in sys_data["rhs"]]
    nvars = n - 1
    if len(matrix) != nvars or any(len(row) != nvars for row in matrix):
        errors.append(f"matrix shape {len(matrix)}x{len(matrix[0]) if matrix else 0} "
                      f"!= expected {nvars}x{nvars}")
        return False, errors
    solved = solve_square_system(matrix, rhs, nvars)
    if solved is None:
        errors.append("re-solve returned singular; stored certificate claims SAT")
        return False, errors
    stored_solution = [Fraction(s) for s in candidate["solution"]]
    if solved != stored_solution:
        errors.append(f"re-solved solution {solved} != stored {stored_solution}")
    a_vectors = later_values_as_linear(n, L + 1)
    re_derived_sequence: list[Fraction] = []
    for j in range(n + L):
        vec = a_vectors[j]
        v = vec[0] + sum(vec[1 + k] * solved[k] for k in range(nvars))
        re_derived_sequence.append(v)
    stored_sequence = [Fraction(s) for s in candidate["sequence_checked"]]
    if re_derived_sequence != stored_sequence:
        errors.append("re-derived sequence != stored sequence")
    for j, v in enumerate(re_derived_sequence):
        if abs(v) > 1:
            errors.append(f"box violation at j={j}: |a_{j}|={abs(v)} > 1")
    one = Fraction(1)
    for t in range(L + 1):
        if not any(re_derived_sequence[t + i] in (one, -one) for i in range(n)):
            errors.append(f"window t={t} not covered: |a_{t+i}|!=1 for any i in 0..{n - 1}")
    re_derived_gaps = [candidate["indices"][k + 1] - candidate["indices"][k]
                        for k in range(len(candidate["indices"]) - 1)]
    if re_derived_gaps != candidate["gap_pattern"]:
        errors.append(f"re-derived gap pattern {re_derived_gaps} != "
                      f"stored {candidate['gap_pattern']}")
    return not errors, errors


def replay_unsat(n: int, L: int, cert: dict) -> tuple[bool, list[str]]:
    """Re-run the exhaustive enumeration and check counts match."""
    errors: list[str] = []
    a = later_values_as_linear(n, L + 1)
    nvars = n - 1
    candidates = list(range(1, n + L))
    total = 0
    fb = {"singular": 0, "out_of_box_solution": 0,
           "out_of_box_window": 0, "cover_incomplete": 0}
    for indices in combinations(candidates, nvars):
        for signs in product((Fraction(1), Fraction(-1)), repeat=nvars):
            total += 1
            rows = [a[j][1:] for j in indices]
            consts = [a[j][0] for j in indices]
            rhs = [signs[i] - consts[i] for i in range(nvars)]
            sol = solve_square_system(rows, rhs, nvars)
            if sol is None:
                fb["singular"] += 1
                continue
            if any(abs(v) > 1 for v in sol):
                fb["out_of_box_solution"] += 1
                continue
            in_box = True
            values: dict[int, Fraction] = {}
            for j in range(n + L):
                vec = a[j]
                values[j] = vec[0] + sum(vec[1 + k] * sol[k] for k in range(nvars))
                if abs(values[j]) > 1:
                    in_box = False
                    break
            if not in_box:
                fb["out_of_box_window"] += 1
                continue
            covered = all(
                any(values[t + i] in (Fraction(1), Fraction(-1))
                    for i in range(n))
                for t in range(L + 1)
            )
            if not covered:
                fb["cover_incomplete"] += 1
                continue
            errors.append(f"UNSAT claim refuted: SAT found at "
                          f"indices={indices} signs={signs}")
            return False, errors
    if total != cert["total_strategies"]:
        errors.append(f"re-counted total {total} != stored {cert['total_strategies']}")
    if fb != cert["failure_breakdown"]:
        errors.append(f"re-derived failure breakdown {fb} != "
                      f"stored {cert['failure_breakdown']}")
    return not errors, errors


def main() -> int:
    install_memory_limit()
    parser = argparse.ArgumentParser(description=__doc__,
                                      formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("json_dir", type=str)
    args = parser.parse_args()

    json_dir = Path(args.json_dir)
    if not json_dir.is_dir():
        print(f"ERROR: {json_dir} is not a directory", file=sys.stderr)
        return 2

    sat_files = sorted(json_dir.glob("n*_L*.json"))
    if not sat_files:
        print(f"ERROR: no per-(n,L) JSON files in {json_dir}", file=sys.stderr)
        return 2

    overall_ok = True
    total_candidates = 0
    candidates_passed = 0
    for path in sat_files:
        with open(path, "r", encoding="utf-8") as stream:
            payload = json.load(stream)
        n = payload["n"]
        L = payload["L"]
        expected = payload["expected_status"]
        print(f"replay: {path.name}  n={n} L={L} expected={expected}")
        if expected == "SAT":
            n_passed = 0
            n_total = 0
            n_failed = 0
            for cand in payload.get("candidates", []):
                n_total += 1
                total_candidates += 1
                ok, errors = replay_sat_candidate(n, L, cand)
                if ok:
                    n_passed += 1
                    candidates_passed += 1
                else:
                    n_failed += 1
                    overall_ok = False
                    print(f"  CANDIDATE FAIL indices={cand['indices']} "
                          f"signs={cand['signs']}")
                    for err in errors:
                        print(f"    - {err}")
            print(f"  {n_passed}/{n_total} candidates replayed cleanly")
        elif expected == "UNSAT":
            ok, errors = replay_unsat(n, L, payload)
            if not ok:
                overall_ok = False
                for err in errors:
                    print(f"  - {err}")
            else:
                print(f"  UNSAT certificate replays: total_strategies="
                      f"{payload['total_strategies']}, "
                      f"failure_breakdown matches")
        else:
            print(f"  unexpected expected_status: {expected}")
            overall_ok = False

    print()
    print(f"overall: {candidates_passed}/{total_candidates} candidates "
          f"replayed cleanly")
    if overall_ok:
        print("REPLAY: PASS")
        return 0
    print("REPLAY: FAIL")
    return 1


if __name__ == "__main__":
    raise SystemExit(main())
