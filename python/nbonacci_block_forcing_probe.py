#!/usr/bin/env python3
"""Exact finite certificate for the n+1-block digit forcing bound.

For digits d_0,...,d_n in {-1,0,1}, the block forcing vector is

  (-e_0 + 2 e_last)d_0
    + sum_{r=1}^{n-1}(e_{r-1}-e_r)d_r + e_last*d_n.

This probe enumerates every digit word in a deliberately small, explicit
range and records the sharp coordinate and L1 bounds.  It is evidence for the
forcing sublemma; the universal inequality still belongs in Lean.
"""

from __future__ import annotations

import argparse
import itertools
import os
import resource


def install_memory_limit() -> None:
    megabytes = int(os.environ.get("RAVEL_PROBE_MEMORY_MB", "10240"))
    if megabytes > 0:
        limit = megabytes * 1024 * 1024
        resource.setrlimit(resource.RLIMIT_AS, (limit, limit))


def forcing(n: int, digits: tuple[int, ...]) -> tuple[int, ...]:
    result = [0] * n
    for r, digit in enumerate(digits):
        if r == 0:
            result[0] -= digit
            result[-1] += 2 * digit
        elif r < n:
            result[r - 1] += digit
            result[r] -= digit
        else:
            result[-1] += digit
    return tuple(result)


def sharp_l1_bound(n: int) -> tuple[int, tuple[int, ...], tuple[int, ...]]:
    maximum = -1
    witness_digits: tuple[int, ...] = ()
    witness_forcing: tuple[int, ...] = ()
    for digits in itertools.product((-1, 0, 1), repeat=n + 1):
        vector = forcing(n, digits)
        norm = sum(abs(value) for value in vector)
        if norm > maximum:
            maximum = norm
            witness_digits = digits
            witness_forcing = vector
    return maximum, witness_digits, witness_forcing


def check(n: int) -> None:
    maximum, digits, vector = sharp_l1_bound(n)
    expected = 2 * n + (2 if n % 2 == 0 else 0)
    if maximum != expected:
        raise AssertionError(
            f"n={n}: observed L1 bound {maximum}, expected {expected}"
        )
    coordinate_bound = max(abs(value) for value in vector)
    print(
        f"block forcing: n={n} words={3 ** (n + 1)} "
        f"max_L1={maximum} max_coordinate={coordinate_bound} "
        f"witness={digits} forcing={vector}"
    )


def main() -> int:
    install_memory_limit()
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--min-n", type=int, default=2)
    parser.add_argument("--max-n", type=int, default=10)
    args = parser.parse_args()
    if args.min_n < 2 or args.max_n < args.min_n:
        parser.error("require 2 <= min-n <= max-n")
    for n in range(args.min_n, args.max_n + 1):
        check(n)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
