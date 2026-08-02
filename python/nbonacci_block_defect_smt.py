#!/usr/bin/env python3
"""Symbolically replay the exact (n+1)-block defect identity.

For a carry sequence
  a[t+n] = a[t] - sum(a[t+1:t+n]) + d[t],
eliminating the window sum gives
  a[t+n+1] - 2*a[t+1] + a[t] = d[t+1] - d[t].

This probe asks exact rational linear arithmetic to find a counterexample for
each t.  It is a small independent guard on the algebra used by the shell
automata; it does not assert the missing exterior-escape theorem.
"""

from __future__ import annotations

import argparse
import os
import resource

import z3


def install_memory_limit() -> None:
    megabytes = int(os.environ.get("RAVEL_PROBE_MEMORY_MB", "10240"))
    if megabytes > 0:
        limit = megabytes * 1024 * 1024
        resource.setrlimit(resource.RLIMIT_AS, (limit, limit))


def check(n: int, t: int):
    # We need a[t] through a[t+n+1], plus the two digit variables.
    a = [z3.Real(f"a_{i}") for i in range(t + n + 2)]
    d = [z3.Real(f"d_{i}") for i in range(t + 2)]
    solver = z3.Solver()
    for k in range(t + 2):
        solver.add(
            a[k + n] == a[k] - sum(a[k + j] for j in range(1, n)) + d[k]
        )
    solver.add(a[t + n + 1] - 2 * a[t + 1] + a[t] != d[t + 1] - d[t])
    return solver.check()


def main() -> int:
    install_memory_limit()
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--n", type=int, required=True)
    parser.add_argument("--max-n", action="store_true")
    args = parser.parse_args()
    if args.n < 2:
        parser.error("require n>=2")
    dimensions = range(2, args.n + 1) if args.max_n else (args.n,)
    for n in dimensions:
        statuses = [str(check(n, t)) for t in range(n)]
        verdict = "PASS" if all(value == "unsat" for value in statuses) else "FAIL"
        print(f"block-defect: n={n} statuses={','.join(statuses)} {verdict}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
