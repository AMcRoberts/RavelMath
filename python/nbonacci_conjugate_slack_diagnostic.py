#!/usr/bin/env python3
"""Diagnose where the conjugate-height bound's slack actually lives.

`nbonacci_conjugate_height_bound.py` bounds |a_t| <= B(n) by combining each
root's individual periodicity bound R_k via a triangle-inequality sum
sum_k |lambda_k| * R_k. That sum is the exact maximum of the linear
functional a_t over the *box* {h : |h_k| <= R_k for every k}; it is not
obviously the exact maximum over the actual achievable set of h(x_t) values.

This script measures the gap directly against ground truth: it
independently re-derives the exact cyclic ternary state set (the same
object nbonacci_carry_cycle_probe.cpp computes) for each n, and reports
three numbers side by side:

  1. B_naive: the a priori bound (uses each root's formula R_k, no
     knowledge of the true cyclic states);
  2. B_using_true_per_root_max: substitutes each root's *actual* maximum
     |h_k| achieved on the real cyclic states (still combined via the same
     triangle-inequality sum) -- an oracle quantity, not a usable a priori
     bound, but diagnostic for whether the R_k formula itself is loose;
  3. true_max|a_t|: the ground truth (1, by construction of the ternary
     layer).

If (2) were close to (3), the remaining slack would be "R_k is a loose
formula, tighten it." It is not: substituting true per-root maxima only
partially closes the gap, and the residual gap *grows* with n. That rules
out per-root looseness as the dominant cause and points at the
triangle-inequality combination step itself -- see
docs/NBONACCI_CONJUGATE_HEIGHT_BOUND.md, "Mined diagnosis of the remaining
gap."
"""

from __future__ import annotations

import argparse
import itertools

import mpmath as mp


def forward_displacement(x: list[int], delta: int) -> list[int]:
    n = len(x)
    rhs = list(x)
    rhs[0] += delta
    result = [0] * n
    tail = rhs[0]
    for k in range(n - 1):
        result[k] = rhs[k + 1]
        tail -= rhs[k + 1]
    result[n - 1] = tail
    return result


def enumerate_cyclic_ternary(n: int) -> set[tuple[int, ...]]:
    states = list(itertools.product([-1, 0, 1], repeat=n))
    out_edges = {}
    for x in states:
        out_edges[x] = [
            tuple(forward_displacement(list(x), d))
            for d in (-1, 0, 1)
            if all(-1 <= v <= 1 for v in forward_displacement(list(x), d))
        ]
    alive = set(states)
    while True:
        indeg = {x: 0 for x in alive}
        for x in alive:
            for y in out_edges[x]:
                if y in alive:
                    indeg[y] += 1
        to_remove = {
            x
            for x in alive
            if indeg[x] == 0 or sum(1 for y in out_edges[x] if y in alive) == 0
        }
        if not to_remove:
            break
        alive -= to_remove
    return alive


def analyze(n: int):
    coeffs = [1] + [-1] * n
    roots = mp.polyroots(coeffs, maxsteps=200, extraprec=400)
    dominant = max(roots, key=lambda r: r.real)
    others = [r for r in roots if abs(r - dominant) > mp.mpf("1e-25")]
    allroots = [dominant] + others
    r_bound = [1 / (dominant.real - 1)] + [1 / (1 - abs(b)) for b in others]

    cyclic = enumerate_cyclic_ternary(n)

    matrix = mp.matrix(n, n)
    for k, beta_k in enumerate(allroots):
        running = mp.mpc(0)
        for j in range(n):
            matrix[k, j] = beta_k**j - running
            running += beta_k**j
    e0 = mp.matrix(n, 1)
    e0[0, 0] = 1
    lam = mp.lu_solve(matrix.T, e0)

    max_h = [mp.mpf(0)] * n
    for x in cyclic:
        for k in range(n):
            h = sum(matrix[k, j] * x[j] for j in range(n))
            max_h[k] = max(max_h[k], abs(h))

    bound_naive = sum(abs(lam[k, 0]) * r_bound[k] for k in range(n))
    bound_oracle = sum(abs(lam[k, 0]) * max_h[k] for k in range(n))
    true_max_a = max(abs(x[0]) for x in cyclic)

    return {
        "n": n,
        "cyclic_count": len(cyclic),
        "bound_naive": bound_naive.real,
        "bound_oracle": bound_oracle.real,
        "true_max_a": true_max_a,
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--n-min", type=int, default=3)
    parser.add_argument("--n-max", type=int, default=7)
    parser.add_argument("--precision", type=int, default=40)
    args = parser.parse_args()
    mp.mp.dps = args.precision

    for n in range(args.n_min, args.n_max + 1):
        result = analyze(n)
        print(
            f"n={result['n']} cyclic_count={result['cyclic_count']} "
            f"B_naive={mp.nstr(result['bound_naive'], 6)} "
            f"B_using_true_per_root_max={mp.nstr(result['bound_oracle'], 6)} "
            f"true_max|a_t|={result['true_max_a']}"
        )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
