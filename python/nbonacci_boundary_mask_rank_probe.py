#!/usr/bin/env python3
"""Check whether outer-shell rank is determined by signed boundary mask."""

from __future__ import annotations

import argparse
from collections import defaultdict

from nbonacci_max_shell_return_probe import dag_rank, first_return_graph, install_memory_limit


def boundary_mask(state: tuple[int, ...], bound: int):
    return tuple((i, 1 if value > 0 else -1)
                 for i, value in enumerate(state) if abs(value) == bound)


def main() -> int:
    install_memory_limit()
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--n", type=int, required=True)
    parser.add_argument("--bound", type=int, required=True)
    args = parser.parse_args()
    shell, relation = first_return_graph(args.n, args.bound)
    acyclic, height, rank = dag_rank(shell, relation)
    groups = defaultdict(list)
    for state, value in rank.items():
        groups[boundary_mask(state, args.bound)].append(value)
    varying = [values for values in groups.values() if len(set(values)) > 1]
    span = max((max(values) - min(values) for values in varying), default=0)
    print(f"boundary-mask rank: n={args.n} bound={args.bound} "
          f"acyclic={'PASS' if acyclic else 'FAIL'} height={height} "
          f"masks={len(groups)} varying_masks={len(varying)} max_span={span}")
    return 0 if acyclic else 1


if __name__ == "__main__":
    raise SystemExit(main())
