#!/usr/bin/env python3
"""Quotient the finite carry automaton by sign/magnitude chambers.

This is a discovery probe for the escape proof.  It permits all signed digits,
prunes states that are not on directed cycles, and then asks how much of the
transient graph survives after forgetting exact magnitudes.  `sign` records
only {- , 0, +}; `ordered` also records the weak ordering of absolute values.
Nontrivial quotient SCCs are a precise signal that the chamber description is
too coarse for a ranking certificate.
"""

from __future__ import annotations

import argparse
from collections import defaultdict, deque
import os
import resource


def install_memory_limit() -> None:
    """Cap virtual memory; use 0 to disable for a deliberate large run."""
    megabytes = int(os.environ.get("RAVEL_PROBE_MEMORY_MB", "1024"))
    if megabytes > 0:
        limit = megabytes * 1024 * 1024
        resource.setrlimit(resource.RLIMIT_AS, (limit, limit))


def encode(x: tuple[int, ...], base: int, bound: int) -> int:
    code = 0
    place = 1
    for value in x:
        code += (value + bound) * place
        place *= base
    return code


def decode(code: int, n: int, base: int, bound: int) -> tuple[int, ...]:
    values = []
    for _ in range(n):
        values.append(code % base - bound)
        code //= base
    return tuple(values)


def chamber(x: tuple[int, ...], mode: str, modulus: int = 3) -> str:
    signs = "".join("+" if v > 0 else "-" if v < 0 else "0" for v in x)
    if mode == "sign":
        return signs
    magnitudes = tuple(abs(v) for v in x)
    if mode == "ordered":
        levels = {value: rank for rank, value in enumerate(sorted(set(magnitudes)))}
        return signs + "|" + ",".join(str(levels[value]) for value in magnitudes)
    minimum = min(magnitudes)
    if mode == "gapcap":
        gaps = tuple(min(2, value - minimum) for value in magnitudes)
        return signs + "|" + ",".join(map(str, gaps))
    if mode == "gaps":
        gaps = tuple(value - minimum for value in magnitudes)
        return signs + "|" + ",".join(map(str, gaps))
    if mode in ("gaps-parity", "gaps-mod3", "gaps-mod"):
        gaps = tuple(value - minimum for value in magnitudes)
        local_modulus = 2 if mode == "gaps-parity" else modulus
        return (signs + "|" + ",".join(map(str, gaps)) + "|"
                + str(minimum % local_modulus))
    if mode == "parity":
        levels = {value: rank for rank, value in enumerate(sorted(set(magnitudes)))}
        parity = tuple(value % 2 for value in magnitudes)
        return (signs + "|" + ",".join(str(levels[value]) for value in magnitudes)
                + "|" + ",".join(map(str, parity)))
    raise ValueError(f"unknown chamber mode: {mode}")


def quotient_scc_sizes(edges: dict[str, set[str]]) -> list[int]:
    nodes = sorted(set(edges) | {v for values in edges.values() for v in values})
    index = {node: k for k, node in enumerate(nodes)}
    graph = [[index[v] for v in edges.get(node, set())] for node in nodes]
    reverse = [[] for _ in nodes]
    for source, destinations in enumerate(graph):
        for destination in destinations:
            reverse[destination].append(source)
    seen = [False] * len(nodes)
    order: list[int] = []
    for root in range(len(nodes)):
        if seen[root]:
            continue
        stack = [(root, False)]
        while stack:
            vertex, exiting = stack.pop()
            if exiting:
                order.append(vertex)
                continue
            if seen[vertex]:
                continue
            seen[vertex] = True
            stack.append((vertex, True))
            for destination in graph[vertex]:
                if not seen[destination]:
                    stack.append((destination, False))
    component = [-1] * len(nodes)
    sizes: list[int] = []
    for root in reversed(order):
        if component[root] >= 0:
            continue
        component[root] = len(sizes)
        size = 0
        stack = [root]
        while stack:
            vertex = stack.pop()
            size += 1
            for source in reverse[vertex]:
                if component[source] < 0:
                    component[source] = component[root]
                    stack.append(source)
        sizes.append(size)
    return sorted((size for size in sizes if size > 1), reverse=True)


def run(n: int, bound: int, mode: str, modulus: int) -> int:
    base = 2 * bound + 1
    states = base**n
    outgoing = [[] for _ in range(states)]
    incoming = [[] for _ in range(states)]
    for code in range(states):
        x = decode(code, n, base, bound)
        tail = x[0] - sum(x[1:])
        for digit in (-1, 0, 1):
            target = x[1:] + (tail + digit,)
            if -bound <= target[-1] <= bound:
                destination = encode(target, base, bound)
                outgoing[code].append(destination)
                incoming[destination].append(code)
    indegree = [len(values) for values in incoming]
    outdegree = [len(values) for values in outgoing]
    queue = deque(k for k in range(states) if not indegree[k] or not outdegree[k])
    removed = [False] * states
    while queue:
        source = queue.popleft()
        if removed[source]:
            continue
        removed[source] = True
        for destination in outgoing[source]:
            if not removed[destination]:
                indegree[destination] -= 1
                if not indegree[destination]:
                    queue.append(destination)
        for predecessor in incoming[source]:
            if not removed[predecessor]:
                outdegree[predecessor] -= 1
                if not outdegree[predecessor]:
                    queue.append(predecessor)

    edges: dict[str, set[str]] = defaultdict(set)
    for source in range(states):
        if not removed[source]:
            continue
        source_chamber = chamber(decode(source, n, base, bound), mode, modulus)
        for destination in outgoing[source]:
            if removed[destination]:
                edges[source_chamber].add(
                    chamber(decode(destination, n, base, bound), mode, modulus)
                )
    self_loops = sum(node in values for node, values in edges.items())
    nontrivial = quotient_scc_sizes(edges)
    print(
        f"sign chamber probe: n={n} bound={bound} mode={mode} "
        f"chambers={len(edges)} self_loops={self_loops} "
        f"nontrivial_SCCs={nontrivial[:12]}"
    )
    return 0


def main() -> int:
    install_memory_limit()
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--n", type=int, required=True)
    parser.add_argument("--bound", type=int, required=True)
    parser.add_argument(
        "--mode", choices=("sign", "ordered", "gapcap", "gaps", "gaps-parity",
                           "gaps-mod3", "gaps-mod", "parity"),
        default="ordered",
    )
    parser.add_argument("--modulus", type=int, default=3)
    args = parser.parse_args()
    if args.n < 2 or args.bound < 1 or args.modulus < 2:
        parser.error("require n>=2, bound>=1, and modulus>=2")
    return run(args.n, args.bound, args.mode, args.modulus)


if __name__ == "__main__":
    raise SystemExit(main())
