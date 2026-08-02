#!/usr/bin/env python3
"""Exact linear-algebra reproduction of the homogeneous-shell survival-depth
threshold (docs/NBONACCI_CODE_MECHANISM.md, "the sharpest available
statement of the remaining gap") -- no Z3, no floating point.

Fix a_0=1 (WLOG by sign symmetry and the unsat-core finding that a_0 is
always pinned -- see nbonacci_homogeneous_shell_unsat_core.py). The
remaining n-1 values a_1,...,a_{n-1} are free parameters; every later value
a_j (j>=n) is an explicit linear function of them via the homogeneous
recurrence a_t = a_{t+1}+...+a_{t+n}. This script searches directly, with
exact rational (Fraction) arithmetic, for an assignment of a_1,...,a_{n-1}
that:

  1. pins some subset of scalar indices to exactly +-1 (using all n-1
     degrees of freedom, i.e. a fully-determined linear system -- this is
     the search strategy, not a proven-necessary restriction: it is the
     strategy that reproduces the known exact answer for the n tested so
     far, not yet shown to be exhaustive over all strategies);
  2. keeps every scalar value a_0,...,a_{n+L-1} inside [-1,1];
  3. covers every window t=0,...,L (some coordinate of window t equals
     exactly +-1).

For every n=2,3,4,5 tested, this reproduces the exact known threshold
(SAT at L=n+1, UNSAT at L=n+2) via a completely different, non-SMT method
(direct combinatorial search over which scalar indices to pin, solved by
exact Gaussian elimination) -- an independent cross-check of
nbonacci_homogeneous_shell_smt.py's Z3 result, not a re-run of it.

This is discovery/verification machinery, not yet the general-n proof: it
finds *a* satisfying assignment (or proves none exists among the K=n-1
fully-determined pinning strategies searched) for a specific n, it does not
derive a formula for the pin pattern as a function of n. The printed pin
indices are informative but not yet shown canonical -- like the Z3
witnesses, multiple valid assignments can exist.
"""

from __future__ import annotations

import argparse
from fractions import Fraction
from itertools import combinations, product


def later_values_as_linear(n: int, r: int) -> dict[int, list[Fraction]]:
    """a_j for j=0..n+r-1, each as a coefficient vector [const, c_1,...,c_{n-1}]
    over the free parameters a_1,...,a_{n-1} (a_0=1 folded into const)."""
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
    """Exact Gaussian elimination for a square (nvars x nvars) system.
    Returns None if singular."""
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


def try_length(n: int, L: int):
    """Search for a covering assignment of windows t=0..L. Returns
    (found, pin_indices, signs, solution) or (False, None, None, None)."""
    a = later_values_as_linear(n, L + 1)
    nvars = n - 1
    candidates = list(range(1, n + L))
    for indices in combinations(candidates, nvars):
        for signs in product((Fraction(1), Fraction(-1)), repeat=nvars):
            rows = [a[j][1:] for j in indices]
            consts = [a[j][0] for j in indices]
            rhs = [signs[i] - consts[i] for i in range(nvars)]
            solution = solve_square_system(rows, rhs, nvars)
            if solution is None or any(abs(v) > 1 for v in solution):
                continue
            values = {}
            in_box = True
            for j in range(n + L):
                vec = a[j]
                values[j] = vec[0] + sum(vec[1 + k] * solution[k] for k in range(nvars))
                if abs(values[j]) > 1:
                    in_box = False
                    break
            if not in_box:
                continue
            covered = all(
                any(values[t + i] in (Fraction(1), Fraction(-1)) for i in range(n))
                for t in range(L + 1)
            )
            if covered:
                return True, indices, signs, solution
    return False, None, None, None


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__,
                                      formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--n-min", type=int, default=2)
    parser.add_argument("--n-max", type=int, default=5)
    args = parser.parse_args()

    failures = 0
    for n in range(args.n_min, args.n_max + 1):
        expected_max_L = n + 1
        for L in (expected_max_L, expected_max_L + 1):
            found, indices, signs, solution = try_length(n, L)
            expect_sat = L == expected_max_L
            status = "SAT" if found else "UNSAT(within searched strategies)"
            ok = found == expect_sat
            failures += 0 if ok else 1
            detail = f" pins={indices} signs={[str(s) for s in signs]}" if found else ""
            print(f"n={n} L={L}: {status} expected={'SAT' if expect_sat else 'UNSAT'} "
                  f"match={'YES' if ok else 'NO'}{detail}")
    print(f"nbonacci_shell_covering_search: {failures} mismatches against the known "
          f"Z3-derived n+1 threshold")
    return 0 if failures == 0 else 1


if __name__ == "__main__":
    raise SystemExit(main())
