#!/usr/bin/env python3
"""Scan exact rational shell margins across dimensions.

For each dimension n, the default obstruction length is n+2 transitions.  The
probe tests all canonical digit words at ``0 <= q < 1/k`` and reports the
largest tested interval that is uniformly UNSAT.  It is intentionally a thin
driver over ``nbonacci_word_interval_smt`` so every result uses the same exact
QF_LRA encoding.
"""

from __future__ import annotations

import argparse

from nbonacci_word_interval_smt import canonical_words, solve_word


def uniformly_unsat(n: int, length: int, denominator: int,
                    timeout_ms: int) -> tuple[bool, int, int]:
    words = list(canonical_words(length))
    unsat = 0
    for word in words:
        status, _ = solve_word(n, word, f"1/{denominator}", timeout_ms)
        if str(status) == "unsat":
            unsat += 1
        else:
            return False, unsat, len(words)
    return True, unsat, len(words)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--n", type=int, required=True)
    parser.add_argument("--n-max", type=int)
    parser.add_argument("--max-denominator", type=int, default=20)
    parser.add_argument("--timeout-ms", type=int, default=0)
    args = parser.parse_args()
    if args.n < 2 or (args.n_max is not None and args.n_max < args.n):
        parser.error("require n>=2 and n-max>=n")
    dimensions = range(args.n, args.n_max + 1) if args.n_max else (args.n,)
    for n in dimensions:
        length = n + 2
        first_sat = None
        safe = None
        for denominator in range(2, args.max_denominator + 1):
            ok, count, total = uniformly_unsat(
                n, length, denominator, args.timeout_ms)
            print(f"margin-scan: n={n} length={length} k={denominator} "
                  f"unsat={count}/{total} uniform={'YES' if ok else 'NO'}")
            if ok:
                if safe is None or denominator < safe:
                    safe = denominator
            elif first_sat is None:
                first_sat = denominator
        print(f"margin-summary: n={n} safe_denominator={safe} "
              f"first_nonuniform={first_sat}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
