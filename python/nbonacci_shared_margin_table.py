#!/usr/bin/env python3
"""Find rational shell margins with the shared digit-selector SMT model."""

from __future__ import annotations

import argparse

from nbonacci_shared_interval_smt import solve


def safe(n: int, denominator: int, timeout_ms: int) -> str:
    status, _ = solve(n, n + 2, f"1/{denominator}", timeout_ms)
    return str(status)


def find_margin(n: int, max_denominator: int, timeout_ms: int):
    initial = safe(n, max_denominator, timeout_ms)
    if initial != "unsat":
        return None, "inconclusive" if initial == "unknown" else "no-safe-bound"
    lo, hi = 2, max_denominator
    while lo < hi:
        mid = (lo + hi) // 2
        status = safe(n, mid, timeout_ms)
        if status == "unknown":
            return None, "inconclusive"
        if status == "unsat":
            hi = mid
        else:
            lo = mid + 1
    threshold = lo
    coarse = safe(n, threshold - 1, timeout_ms) if threshold > 2 else "sat"
    if coarse == "unknown":
        return None, "inconclusive"
    return threshold, coarse


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--n", type=int, required=True)
    parser.add_argument("--n-max", type=int)
    parser.add_argument("--max-denominator", type=int, default=100)
    parser.add_argument("--timeout-ms", type=int, default=0)
    args = parser.parse_args()
    if args.n < 2 or (args.n_max is not None and args.n_max < args.n):
        parser.error("require n>=2 and n-max>=n")
    dimensions = range(args.n, args.n_max + 1) if args.n_max else (args.n,)
    for n in dimensions:
        threshold, coarse = find_margin(n, args.max_denominator,
                                         args.timeout_ms)
        if threshold is None:
            print(f"shared-margin: n={n} status={coarse} "
                  f"max-denominator={args.max_denominator}")
        else:
            print(f"shared-margin: n={n} k={threshold} "
                  f"coarser={coarse.upper()}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
