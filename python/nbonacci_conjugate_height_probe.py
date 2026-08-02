#!/usr/bin/env python3
"""Numerically mine the conjugate-height bound for periodic carry words.

For the n-bonacci root beta, define c_0=1 and c_j=beta*c_{j-1}-1.  The
carry update satisfies beta*h(x')=h(x)+d.  Closing a periodic word bounds
each conjugate height; inverting the embedding matrix gives a finite bound
on every integer coefficient.  This is discovery output only: root-location
and interval errors must be replaced by certified algebra before theorem use.
"""

from __future__ import annotations

import argparse
import os
import resource

import numpy as np


def install_memory_limit() -> None:
    megabytes = int(os.environ.get("RAVEL_PROBE_MEMORY_MB", "10240"))
    if megabytes > 0:
        limit = megabytes * 1024 * 1024
        resource.setrlimit(resource.RLIMIT_AS, (limit, limit))


def roots(n: int) -> np.ndarray:
    # beta^n - beta^(n-1) - ... - beta - 1.
    coefficients = [1.0] + [-1.0] * n
    return np.roots(coefficients)


def covector(alpha: complex, n: int) -> np.ndarray:
    values = [1.0 + 0.0j]
    for _ in range(1, n):
        values.append(alpha * values[-1] - 1.0)
    return np.asarray(values, dtype=complex)


def bound(n: int) -> tuple[float, float, float, bool]:
    spectrum = roots(n)
    principal_index = int(np.argmax(np.abs(spectrum)))
    beta = spectrum[principal_index]
    embeddings = [covector(alpha, n) for alpha in spectrum]
    matrix = np.asarray(embeddings, dtype=complex)
    inverse = np.linalg.inv(matrix)
    heights = []
    for index, alpha in enumerate(spectrum):
        modulus = abs(alpha)
        if index == principal_index:
            heights.append(1.0 / max(abs(beta) - 1.0, 1e-15))
        else:
            heights.append(1.0 / max((1.0 - modulus) ** 2, 1e-15))
    heights = np.asarray(heights)
    coordinate_bounds = np.sum(np.abs(inverse) * heights[None, :], axis=1)
    return (float(np.real(beta)), float(np.max(coordinate_bounds)),
            float(np.max(np.abs(spectrum[np.arange(n) != principal_index]))),
            bool(np.all(np.abs(spectrum[np.arange(n) != principal_index]) < 1.0)))


def main() -> int:
    install_memory_limit()
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--min-n", type=int, default=2)
    parser.add_argument("--max-n", type=int, default=12)
    args = parser.parse_args()
    if args.min_n < 2 or args.max_n < args.min_n:
        parser.error("require 2 <= min-n <= max-n")
    for n in range(args.min_n, args.max_n + 1):
        beta, coefficient_bound, conjugate_radius, pisot = bound(n)
        print(f"conjugate height: n={n} beta={beta:.12g} "
              f"max_conjugate={conjugate_radius:.12g} "
              f"coefficient_bound={coefficient_bound:.12g} "
              f"pisot={'PASS' if pisot else 'FAIL'}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
