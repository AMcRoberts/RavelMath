#!/usr/bin/env python3
"""Diagnostic spectrum of the exact n+1-step carry block.

For A=M^{-1}, the block identity gives B=A^(n+1)=2A-I.  This probe checks
that identity numerically and reports the stable eigenvalue and the smallest
unstable modulus.  It is explanatory evidence for the escape mechanism, not a
replacement for an exact Pisot/root-location proof.
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


def carry_matrix(n: int) -> np.ndarray:
    matrix = np.zeros((n, n), dtype=float)
    matrix[0, :] = 1.0
    for column in range(n - 1):
        matrix[column + 1, column] = 1.0
    return matrix


def check(n: int) -> None:
    matrix = carry_matrix(n)
    inverse = np.linalg.inv(matrix)
    power = np.linalg.matrix_power(inverse, n + 1)
    identity_error = float(np.max(np.abs(power - (2.0 * inverse - np.eye(n)))))
    eigenvalues = np.linalg.eigvals(2.0 * inverse - np.eye(n))
    moduli = sorted(float(abs(value)) for value in eigenvalues)
    stable = moduli[0]
    unstable = moduli[1] if n > 1 else float("nan")
    print(
        f"block spectrum: n={n} identity_error={identity_error:.3e} "
        f"stable_modulus={stable:.9f} smallest_unstable={unstable:.9f} "
        f"unstable_count={sum(value > 1.0 + 1e-9 for value in moduli)}"
    )


def main() -> int:
    install_memory_limit()
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--min-n", type=int, default=3)
    parser.add_argument("--max-n", type=int, default=20)
    args = parser.parse_args()
    if args.min_n < 2 or args.max_n < args.min_n:
        parser.error("require 2 <= min-n <= max-n")
    for n in range(args.min_n, args.max_n + 1):
        check(n)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
