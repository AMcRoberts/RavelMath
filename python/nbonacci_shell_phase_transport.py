#!/usr/bin/env python3
"""Merge exact maximum-shell first-return constraints across shell radii.

For a state x, the phase is:
  signs(x), exact gaps |x_i|-min_j|x_j|, min_j|x_j| mod (n+1).

The rank model is
  R(x) = sum_i |x_i| + h(phase(x)).

For every exact first-return edge x -> y, strict increase requires
  h(phase(y)) >= h(phase(x)) + sum|x| - sum|y| + 1.

Merging those inequalities across several shell radii tests whether one phase
operator transports across the finite family instead of fitting each shell DAG
separately.
"""

from __future__ import annotations

import argparse
import json
from collections import defaultdict

from nbonacci_max_shell_return_probe import (
    first_return_graph,
    install_memory_limit,
)


def phase(state: tuple[int, ...], n: int) -> str:
    signs = "".join("+" if v > 0 else "-" if v < 0 else "0" for v in state)
    mags = tuple(abs(v) for v in state)
    minimum = min(mags)
    gaps = tuple(v - minimum for v in mags)
    return f"{signs}|{','.join(map(str, gaps))}|{minimum % (n + 1)}"


def solve(phases: list[str], edges: list[tuple[int, int, int]]):
    height = [0] * len(phases)
    for _ in range(len(phases)):
        changed = False
        for u, v, w in edges:
            candidate = height[u] + w
            if height[v] < candidate:
                height[v] = candidate
                changed = True
        if not changed:
            return height
    return None


def main() -> int:
    install_memory_limit()
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--n", type=int, required=True)
    parser.add_argument("--min-bound", type=int, default=2)
    parser.add_argument("--max-bound", type=int, required=True)
    parser.add_argument("--emit", required=True)
    args = parser.parse_args()

    phase_ids: dict[str, int] = {}
    constraints: dict[tuple[int, int], int] = {}
    instances = []
    raw_edges = 0

    def pid(name: str) -> int:
        if name not in phase_ids:
            phase_ids[name] = len(phase_ids)
        return phase_ids[name]

    for bound in range(args.min_bound, args.max_bound + 1):
        shell, relation = first_return_graph(args.n, bound)
        instances.append({
            "n": args.n,
            "bound": bound,
            "shell_states": len(shell),
            "first_return_edges": sum(map(len, relation.values())),
        })
        for source, targets in relation.items():
            ps = pid(phase(source, args.n))
            ls = sum(map(abs, source))
            for target in targets:
                pt = pid(phase(target, args.n))
                lt = sum(map(abs, target))
                weight = ls - lt + 1
                constraints[(ps, pt)] = max(constraints.get((ps, pt), weight), weight)
                raw_edges += 1

    names = [None] * len(phase_ids)
    for name, idx in phase_ids.items():
        names[idx] = name
    edges = [(u, v, w) for (u, v), w in sorted(constraints.items())]
    offsets = solve(names, edges)
    checked = offsets is not None and all(
        offsets[v] >= offsets[u] + w for u, v, w in edges
    )

    payload = {
        "kind": "nbonacci-shell-phase-transport-v1",
        "n": args.n,
        "bounds": [args.min_bound, args.max_bound],
        "rank": "sum_abs(state) + phase_offset",
        "phase_model": "signs|exact_gaps|min_abs_mod_(n+1)",
        "instances": instances,
        "phases": names,
        "constraints": [
            {"source": u, "target": v, "required_gain": w}
            for u, v, w in edges
        ],
        "raw_first_return_edges": raw_edges,
        "offsets": offsets,
        "feasible": offsets is not None,
        "checked": checked,
    }
    with open(args.emit, "w", encoding="utf-8") as stream:
        json.dump(payload, stream, indent=2, sort_keys=True)
        stream.write("\n")

    print(
        f"shell phase transport: n={args.n} bounds={args.min_bound}..{args.max_bound} "
        f"phases={len(names)} constraints={len(edges)} raw_edges={raw_edges} "
        f"transport={'PASS' if checked else 'FAIL'}"
    )
    return 0 if checked else 1


if __name__ == "__main__":
    raise SystemExit(main())
