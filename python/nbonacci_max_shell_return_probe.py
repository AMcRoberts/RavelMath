#!/usr/bin/env python3
"""Exact shell-return DAG probe for periodic carry escape.

Fix a coefficient box ``[-M,M]^n`` and its outer shell
``max(abs(a_i)) == M``.  Build the first-return relation on the shell:
an edge ``u -> v`` exists when a legal carry path starts at ``u``, visits no
other shell state, and first returns at ``v``.  A periodic orbit attaining
the global maximum M would induce a directed cycle in this return graph.
Consequently an acyclic return graph is a finite, exact certificate that no
periodic orbit in this box has maximum M.  This is stronger than checking
only edges that stay on the shell and is useful for mining a parametric
shell rank.
"""

from __future__ import annotations

import argparse
import os
import resource
from collections import defaultdict, deque
from itertools import product


def install_memory_limit() -> None:
    megabytes = int(os.environ.get("RAVEL_PROBE_MEMORY_MB", "10240"))
    if megabytes > 0:
        limit = megabytes * 1024 * 1024
        resource.setrlimit(resource.RLIMIT_AS, (limit, limit))


def transitions(state: tuple[int, ...], bound: int):
    tail = state[0] - sum(state[1:])
    for digit in (-1, 0, 1):
        target = state[1:] + (tail + digit,)
        if all(-bound <= value <= bound for value in target):
            yield target


def first_return_graph(n: int, bound: int):
    states = list(product(range(-bound, bound + 1), repeat=n))
    shell = {state for state in states if max(map(abs, state)) == bound}
    relation: dict[tuple[int, ...], set[tuple[int, ...]]] = defaultdict(set)
    for source in shell:
        queue = deque([source])
        seen = {source}
        while queue:
            current = queue.popleft()
            for target in transitions(current, bound):
                if target in shell:
                    relation[source].add(target)
                elif target not in seen:
                    seen.add(target)
                    queue.append(target)
    return shell, relation


def dag_rank(nodes, edges):
    indegree = {node: 0 for node in nodes}
    for source, targets in edges.items():
        for target in targets:
            indegree[target] += 1
    queue = deque(node for node in nodes if indegree[node] == 0)
    rank = {node: 0 for node in nodes}
    visited = 0
    while queue:
        source = queue.popleft()
        visited += 1
        for target in edges.get(source, ()):
            rank[target] = max(rank[target], rank[source] + 1)
            indegree[target] -= 1
            if indegree[target] == 0:
                queue.append(target)
    return visited == len(nodes), max(rank.values(), default=0)


def main() -> int:
    install_memory_limit()
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--n", type=int, required=True)
    parser.add_argument("--bound", type=int, required=True)
    args = parser.parse_args()
    if args.n < 2 or args.bound < 1:
        parser.error("require n>=2 and bound>=1")
    shell, relation = first_return_graph(args.n, args.bound)
    acyclic, height = dag_rank(shell, relation)
    print(
        f"max-shell return probe: n={args.n} bound={args.bound} "
        f"shell={len(shell)} return_edges={sum(map(len, relation.values()))} "
        f"acyclic={'PASS' if acyclic else 'FAIL'} rank_height={height}"
    )
    return 0 if acyclic else 1


if __name__ == "__main__":
    raise SystemExit(main())
