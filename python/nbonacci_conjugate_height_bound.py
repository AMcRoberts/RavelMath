#!/usr/bin/env python3
"""High-precision conjugate-height certificate for the periodic carry-bound
lemma (see docs/NBONACCI_ARITHMETIC_DOMINANCE_PROGRAM.md).

Claim under test: let a : Z -> Z be periodic with
d_t = -a_t + a_{t+1} + ... + a_{t+n} in {-1,0,1} for every t. Then |a_t| is
bounded by a constant B(n) depending only on n, not on the period.

Mechanism (see docs/NBONACCI_ARITHMETIC_DOMINANCE_PROGRAM.md, "Algebraic-
height route"): for each root beta_k of x^n - x^{n-1} - ... - x - 1 = 0
(k=1 the dominant real root, k=2..n the conjugates, all with |beta_k| < 1
since the n-bonacci constant is Pisot), the height functional
h_k(x) = sum_j c_j^{(k)} x_j, c_j^{(k)} = beta_k^j - (1+beta_k+...+beta_k^{j-1}),
satisfies beta_k * h_k(x') = h_k(x) + delta whenever x' is the carry update
of x by digit delta (forward_displacement in
include/ravel/nbonacci_margin_invariant.hpp). This script:

1. verifies that identity directly against forward_displacement on random
   integer states, for every n in the tested range;
2. computes each root beta_k to high precision (mpmath, configurable
   working precision) and confirms the Pisot property numerically;
3. derives the periodicity bound R_k on |h_k(a_t)| for every t and every
   period (dominant root: exact telescoping max-attained argument, tight;
   conjugate roots: a uniform-in-period backward-telescoping bound, not
   claimed tight);
4. inverts the change of basis (Cramer's rule via matrix solve) to bound
   |a_t| <= B(n) = sum_k |lambda_k| R_k, lambda = (C^T)^{-1} e_0.

This is a high-precision numeric certificate (order-of-magnitude decimal
digits set by --precision), not a machine-checked exact/interval-arithmetic
proof. Promoting it to exact algebraic (Sturm-isolated or Lean-checked)
rigor is the remaining honest gap; see the doc for how this compares to the
exhaustive small-box checks in nbonacci_carry_cycle_probe.
"""

from __future__ import annotations

import argparse
import random

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


def verify_identity(n: int, beta, trials: int, seed: int) -> bool:
    c = []
    running = mp.mpc(0)
    for j in range(n):
        c.append(beta**j - running)
        running += beta**j
    rng = random.Random(seed)
    tol = mp.mpf(10) ** (-(mp.mp.dps - 10))
    for _ in range(trials):
        x = [rng.randint(-5, 5) for _ in range(n)]
        delta = rng.choice([-1, 0, 1])
        xp = forward_displacement(x, delta)
        h_x = sum(c[j] * x[j] for j in range(n))
        h_xp = sum(c[j] * xp[j] for j in range(n))
        if abs(beta * h_xp - (h_x + delta)) > tol:
            return False
    return True


def nbonacci_roots(n: int):
    coeffs = [1] + [-1] * n  # x^n - x^{n-1} - ... - x - 1, highest degree first
    return mp.polyroots(coeffs, maxsteps=200, extraprec=10 * mp.mp.dps)


def conjugate_height_bound(n: int, identity_trials: int, seed: int):
    roots = nbonacci_roots(n)
    dominant = max(roots, key=lambda r: r.real)
    others = [r for r in roots if abs(r - dominant) > mp.mpf(10) ** (-(mp.mp.dps // 2))]
    if len(others) != n - 1:
        raise RuntimeError(
            f"n={n}: expected {n - 1} non-dominant roots, found {len(others)}"
        )
    pisot = all(abs(r) < 1 for r in others)

    ordered = [dominant] + others
    identity_ok = verify_identity(n, dominant, identity_trials, seed)

    matrix = mp.matrix(n, n)
    for k in range(n):
        beta_k = ordered[k]
        running = mp.mpc(0)
        for j in range(n):
            matrix[k, j] = beta_k**j - running
            running += beta_k**j

    bounds = [mp.mpf(0)] * n
    bounds[0] = 1 / (dominant.real - 1)
    for k in range(1, n):
        bounds[k] = 1 / (1 - abs(ordered[k])) ** 2

    e0 = mp.matrix(n, 1)
    e0[0, 0] = 1
    lam = mp.lu_solve(matrix.T, e0)
    bound_a = sum(abs(lam[k, 0]) * bounds[k] for k in range(n))

    return {
        "n": n,
        "beta": dominant.real,
        "max_other_modulus": max(abs(r) for r in others),
        "pisot": pisot,
        "identity_ok": identity_ok,
        "dominant_bound": bounds[0],
        "bound_on_a_t": bound_a.real,
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--n-min", type=int, default=2)
    parser.add_argument("--n-max", type=int, default=12)
    parser.add_argument("--precision", type=int, default=60,
                         help="mpmath decimal digits of working precision")
    parser.add_argument("--identity-trials", type=int, default=500)
    parser.add_argument("--seed", type=int, default=1)
    args = parser.parse_args()
    mp.mp.dps = args.precision

    failures = 0
    for n in range(args.n_min, args.n_max + 1):
        result = conjugate_height_bound(n, args.identity_trials, args.seed)
        ok = result["pisot"] and result["identity_ok"]
        failures += 0 if ok else 1
        print(
            f"n={result['n']:2d} beta={mp.nstr(result['beta'], 15)} "
            f"max|other root|={mp.nstr(result['max_other_modulus'], 15)} "
            f"pisot={'PASS' if result['pisot'] else 'FAIL'} "
            f"identity={'PASS' if result['identity_ok'] else 'FAIL'} "
            f"dominant_bound={mp.nstr(result['dominant_bound'], 10)} "
            f"bound_on_a_t={mp.nstr(result['bound_on_a_t'], 10)}"
        )
    print(f"conjugate height bound: n={args.n_min}..{args.n_max} "
          f"precision={args.precision} failures={failures}")
    return 0 if failures == 0 else 1


if __name__ == "__main__":
    raise SystemExit(main())
