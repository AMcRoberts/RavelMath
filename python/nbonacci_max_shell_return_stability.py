#!/usr/bin/env python3
"""Scan exact first-return shell ranks over a bounded range of shells."""

from __future__ import annotations

import argparse
import os

from nbonacci_max_shell_return_probe import first_return_graph, dag_rank, install_memory_limit


def main() -> int:
    install_memory_limit()
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--n", type=int, required=True)
    parser.add_argument("--min-bound", type=int, default=2)
    parser.add_argument("--max-bound", type=int, required=True)
    args = parser.parse_args()
    if args.n < 2 or args.min_bound < 1 or args.max_bound < args.min_bound:
        parser.error("require n>=2 and 1<=min-bound<=max-bound")
    for bound in range(args.min_bound, args.max_bound + 1):
        shell, relation = first_return_graph(args.n, bound)
        acyclic, height, _ = dag_rank(shell, relation)
        print(f"shell stability: n={args.n} bound={bound} shell={len(shell)} "
              f"edges={sum(map(len, relation.values()))} "
              f"acyclic={'PASS' if acyclic else 'FAIL'} height={height}")
        if not acyclic:
            return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
