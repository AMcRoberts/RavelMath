"""
beta_expansion_unimodularity_check.py -- Task 4 follow-up: Finding 6.5
flagged sigma_{0,2} (the only NON-unimodular candidate in that batch,
det=2) as the only one whose d_beta(1) didn't resolve to finite within
400 digits, and explicitly hypothesized the anomaly "may correlate
with unimodularity rather than with the AR-exact/AR-partial/non-AR
split" -- but that was one data point, not a checked hypothesis.

This script tests it directly: search small cubic monic integer
polynomials x^3 - p x^2 - q x - r for the Pisot property (all other
roots modulus < 1), split hits into unimodular (|r|=1) and
non-unimodular (|r|>=2), and run the SAME beta-expansion machinery
(python/beta_expansion_thread_a4.py's exact charpoly + 3000-dps
near-integer-snapped greedy expansion) on a sample of each group.

Reuses beta_expansion_thread_a4.py's core routines directly rather
than re-deriving them, to avoid re-introducing either of the two
precision bugs that file already found and fixed.
"""
import sys
import os
sys.path.insert(0, os.path.dirname(__file__))

import sympy as sp
import mpmath as mp
from beta_expansion_thread_a4 import (
    pisot_root, beta_expansion, find_period, mp as _mp,
)

mp.mp.dps = 3000
x = sp.symbols('x')


def is_pisot(p, q, r):
    """x^3 - p x^2 - q x - r: Pisot iff there is a dominant root that
    is REAL and > 1 (not just largest modulus -- a negative real root
    with the largest magnitude is not a Pisot number, Pisot numbers
    are real algebraic integers > 1 by definition), and every OTHER
    root has modulus < 1."""
    coeffs = [1, -p, -q, -r]
    mp_roots = mp.polyroots([mp.mpf(c) for c in coeffs], maxsteps=200, extraprec=200)
    by_mag = sorted(mp_roots, key=lambda z: abs(z), reverse=True)
    if len(by_mag) < 3:
        return False
    dominant = by_mag[0]
    if abs(mp.im(dominant)) > mp.mpf('1e-9'):
        return False
    if mp.re(dominant) <= 1 + mp.mpf('1e-9'):
        return False
    rest_mags = sorted((abs(z) for z in by_mag[1:]), reverse=True)
    return rest_mags[0] < 1 - mp.mpf('1e-9')


def search(max_pqr=6, want=6):
    unimodular, nonunimodular = [], []
    for p in range(0, max_pqr + 1):
        for q in range(-max_pqr, max_pqr + 1):
            for r in (-3, -2, -1, 1, 2, 3):
                if len(unimodular) >= want and len(nonunimodular) >= want:
                    return unimodular, nonunimodular
                try:
                    if not is_pisot(p, q, r):
                        continue
                except Exception:
                    continue
                entry = (p, q, r)
                if abs(r) == 1 and len(unimodular) < want:
                    unimodular.append(entry)
                elif abs(r) != 1 and len(nonunimodular) < want:
                    nonunimodular.append(entry)
    return unimodular, nonunimodular


def check_group(name, entries, n_digits=400):
    print(f"\n--- {name} ---")
    results = []
    for (p, q, r) in entries:
        cp = sp.expand((x**3 - p * x**2 - q * x - r))
        beta = pisot_root(cp)
        if beta is None:
            print(f"  x^3-{p}x^2-{q}x-{r}: no real dominant root found, skipping")
            continue
        digits, term = beta_expansion(beta, n_digits)
        preperiod, period = find_period(digits, min_tail=15)
        kind = (f"finite(len={term})" if term else
                (f"periodic(pp={preperiod},per={period})" if period else "UNRESOLVED"))
        print(f"  x^3-{p}x^2-{q}x-{r}  beta={mp.nstr(beta, 15)}  d_beta(1)={digits[:15]} -> {kind}")
        results.append(((p, q, r), kind))
    return results


def main():
    unimodular, nonunimodular = search()
    print(f"found {len(unimodular)} unimodular, {len(nonunimodular)} non-unimodular Pisot candidates")
    u_results = check_group("UNIMODULAR (|r|=1)", unimodular, n_digits=200)
    n_results = check_group("NON-UNIMODULAR (|r|>=2)", nonunimodular, n_digits=400)

    print("\n=== SUMMARY ===")
    u_finite = sum(1 for _, k in u_results if k.startswith("finite"))
    n_finite = sum(1 for _, k in n_results if k.startswith("finite"))
    print(f"unimodular: {u_finite}/{len(u_results)} finite")
    print(f"non-unimodular: {n_finite}/{len(n_results)} finite")


if __name__ == "__main__":
    main()
