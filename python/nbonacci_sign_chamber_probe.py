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
import json
import os
import resource


def install_memory_limit() -> None:
    """Cap virtual memory; use 0 to disable for a deliberate large run."""
    megabytes = int(os.environ.get("RAVEL_PROBE_MEMORY_MB", "10240"))
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
    if mode in ("gaps-parity", "gaps-mod3", "gaps-mod", "gaps-residue",
                "gaps-mod-scale"):
        gaps = tuple(value - minimum for value in magnitudes)
        local_modulus = 2 if mode == "gaps-parity" else modulus
        if mode == "gaps-residue":
            gaps = tuple(value % local_modulus for value in gaps)
        result = (signs + "|" + ",".join(map(str, gaps)) + "|"
                  + str(minimum % local_modulus))
        if mode == "gaps-mod-scale":
            result += "|" + str(minimum // local_modulus)
        return result
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


def affine_min_rank_feasible(
    weighted_edges: set[tuple[int, int, int]], chamber_count: int,
) -> bool | None:
    """Check R=min_abs+h(chamber) increasing on every quotient edge.

    The inequalities are h(v) >= h(u) + (min_u-min_v) + 1.  Bellman-Ford
    relaxation detects a positive cycle.  None means the bounded iteration
    budget was insufficient, rather than a mathematical failure.
    """
    edges = list(weighted_edges)
    graph = [[] for _ in range(chamber_count)]
    reverse = [[] for _ in range(chamber_count)]
    for source, destination, weight in edges:
        graph[source].append((destination, weight))
        reverse[destination].append((source, weight))

    # Kosaraju, iteratively, so large gap boxes do not consume Python stack.
    seen = [False] * chamber_count
    order: list[int] = []
    for root in range(chamber_count):
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
            for destination, _ in graph[vertex]:
                if not seen[destination]:
                    stack.append((destination, False))
    component = [-1] * chamber_count
    components: list[list[int]] = []
    for root in reversed(order):
        if component[root] >= 0:
            continue
        component[root] = len(components)
        vertices = [root]
        stack = [root]
        while stack:
            vertex = stack.pop()
            for source, _ in reverse[vertex]:
                if component[source] < 0:
                    component[source] = component[root]
                    vertices.append(source)
                    stack.append(source)
        components.append(vertices)

    iteration_cap = int(os.environ.get("RAVEL_RANK_ITERATIONS", "200"))
    if iteration_cap < 1:
        return None
    for vertices in components:
        if len(vertices) == 1:
            vertex = vertices[0]
            if any(destination == vertex and weight > 0
                   for destination, weight in graph[vertex]):
                return False
            continue
        local = set(vertices)
        heights = {vertex: 0 for vertex in vertices}
        internal = [
            (source, destination, weight)
            for source in vertices
            for destination, weight in graph[source]
            if destination in local
        ]
        for _ in range(min(len(vertices), iteration_cap)):
            changed = False
            for source, destination, weight in internal:
                if heights[destination] < heights[source] + weight:
                    heights[destination] = heights[source] + weight
                    changed = True
            if not changed:
                break
        else:
            return None if len(vertices) > iteration_cap else False
        if changed:
            return False
    return True


def affine_rank_offsets(
    weighted_edges: set[tuple[int, int, int]], chamber_count: int,
) -> tuple[bool, list[int] | None]:
    """Produce an integer difference-constraints certificate when feasible.

    Heights satisfy h[v] >= h[u] + weight(u,v).  Bellman-Ford relaxation from
    zero detects a positive cycle after chamber_count rounds and otherwise
    returns replayable integer offsets.
    """
    heights = [0] * chamber_count
    edges = sorted(weighted_edges)
    for _ in range(chamber_count):
        changed = False
        for source, destination, weight in edges:
            candidate = heights[source] + weight
            if heights[destination] < candidate:
                heights[destination] = candidate
                changed = True
        if not changed:
            return True, heights
    return False, None


def run(n: int, bound: int, mode: str, modulus: int,
        rank_base: str | None, emit_certificate: str | None) -> int:
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

    edges: dict[int, set[int]] = defaultdict(set)
    weighted_edges: set[tuple[int, int, int]] = set()
    chamber_ids: dict[str, int] = {}

    def chamber_id(name: str) -> int:
        if name not in chamber_ids:
            chamber_ids[name] = len(chamber_ids)
        return chamber_ids[name]
    for source in range(states):
        if not removed[source]:
            continue
        source_state = decode(source, n, base, bound)
        source_chamber = chamber_id(chamber(source_state, mode, modulus))
        source_magnitudes = tuple(abs(value) for value in source_state)
        if rank_base == "min":
            source_level = min(source_magnitudes)
        elif rank_base == "max":
            source_level = max(source_magnitudes)
        elif rank_base == "sum":
            source_level = sum(source_magnitudes)
        elif rank_base == "quotient":
            source_level = sum(value // modulus for value in source_magnitudes)
        elif rank_base == "sum-quotient":
            scale = int(os.environ.get("RAVEL_QUOTIENT_SCALE", "1"))
            source_level = sum(source_magnitudes) + scale * sum(
                value // modulus for value in source_magnitudes
            )
        elif rank_base == "facet":
            if n != 3:
                raise ValueError("facet rank base currently requires n=3")
            source_level = max(
                abs(value) for value in (
                    *source_state,
                    source_state[0] + source_state[1],
                    source_state[1] + source_state[2],
                    sum(source_state),
                )
            )
        elif rank_base == "facet-sum":
            if n != 3:
                raise ValueError("facet-sum rank base currently requires n=3")
            facet = max(abs(value) for value in (
                *source_state, source_state[0] + source_state[1],
                source_state[1] + source_state[2], sum(source_state)))
            source_level = facet + sum(source_magnitudes)
        else:
            source_level = 0
        for destination in outgoing[source]:
            if removed[destination]:
                destination_state = decode(destination, n, base, bound)
                destination_chamber = chamber_id(
                    chamber(destination_state, mode, modulus)
                )
                edges[source_chamber].add(destination_chamber)
                destination_magnitudes = tuple(abs(value) for value in destination_state)
                if rank_base == "min":
                    destination_level = min(destination_magnitudes)
                elif rank_base == "max":
                    destination_level = max(destination_magnitudes)
                elif rank_base == "sum":
                    destination_level = sum(destination_magnitudes)
                elif rank_base == "quotient":
                    destination_level = sum(value // modulus for value in destination_magnitudes)
                elif rank_base == "sum-quotient":
                    scale = int(os.environ.get("RAVEL_QUOTIENT_SCALE", "1"))
                    destination_level = sum(destination_magnitudes) + scale * sum(
                        value // modulus for value in destination_magnitudes
                    )
                elif rank_base == "facet":
                    if n != 3:
                        raise ValueError("facet rank base currently requires n=3")
                    destination_level = max(
                        abs(value) for value in (
                            *destination_state,
                            destination_state[0] + destination_state[1],
                            destination_state[1] + destination_state[2],
                            sum(destination_state),
                        )
                    )
                elif rank_base == "facet-sum":
                    if n != 3:
                        raise ValueError("facet-sum rank base currently requires n=3")
                    facet = max(abs(value) for value in (
                        *destination_state, destination_state[0] + destination_state[1],
                        destination_state[1] + destination_state[2],
                        sum(destination_state)))
                    destination_level = facet + sum(destination_magnitudes)
                else:
                    destination_level = 0
                weighted_edges.add(
                    (source_chamber, destination_chamber,
                     source_level - destination_level + 1)
                )
    self_loops = sum(node in values for node, values in edges.items())
    nontrivial = quotient_scc_sizes(edges)
    rank_text = ""
    if rank_base is not None:
        feasible = affine_min_rank_feasible(weighted_edges, len(chamber_ids))
        rank_text = (f" affine_{rank_base}_rank="
                     f"{'PASS' if feasible is True else 'FAIL' if feasible is False else 'INCONCLUSIVE'}")
    if emit_certificate is not None:
        certificate_ok, offsets = affine_rank_offsets(
            weighted_edges, len(chamber_ids)
        ) if rank_base is not None else (False, None)
        certificate = {
            "kind": "nbonacci-sign-chamber-rank-v1",
            "n": n,
            "bound": bound,
            "mode": mode,
            "modulus": modulus,
            "rank_base": rank_base,
            "chambers": [name for name, _ in sorted(
                chamber_ids.items(), key=lambda item: item[1]
            )],
            "weighted_edges": [list(edge) for edge in sorted(weighted_edges)],
            "rank_certificate": {
                "feasible": certificate_ok,
                "offsets": offsets,
                "checked_edges": bool(certificate_ok and offsets is not None and
                    all(offsets[destination] >= offsets[source] + weight
                        for source, destination, weight in weighted_edges)),
            },
        }
        with open(emit_certificate, "w", encoding="utf-8") as stream:
            json.dump(certificate, stream, indent=2, sort_keys=True)
            stream.write("\n")
        print(f"sign chamber certificate: wrote {emit_certificate} "
              f"feasible={'PASS' if certificate_ok else 'FAIL'}")
    print(
        f"sign chamber probe: n={n} bound={bound} mode={mode} "
        f"modulus={modulus} "
        f"chambers={len(edges)} self_loops={self_loops} "
        f"nontrivial_SCCs={nontrivial[:12]}{rank_text}"
    )
    return 0


def main() -> int:
    install_memory_limit()
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--n", type=int, required=True)
    parser.add_argument("--bound", type=int, required=True)
    parser.add_argument(
        "--mode", choices=("sign", "ordered", "gapcap", "gaps", "gaps-parity",
                           "gaps-mod3", "gaps-mod", "gaps-residue",
                           "gaps-mod-scale", "parity"),
        default="ordered",
    )
    parser.add_argument(
        "--modulus", default="auto",
        help="minimum magnitude residue modulus, or 'auto' for n+1",
    )
    parser.add_argument("--rank-min", action="store_true")
    parser.add_argument("--rank-base", choices=("min", "max", "sum", "quotient", "sum-quotient", "facet", "facet-sum"))
    parser.add_argument(
        "--emit-certificate",
        help="write a JSON chamber/rank certificate (requires --rank-base)",
    )
    args = parser.parse_args()
    if args.n < 2 or args.bound < 1:
        parser.error("require n>=2, bound>=1, and modulus>=2")
    try:
        modulus = args.n + 1 if args.modulus == "auto" else int(args.modulus)
    except ValueError:
        parser.error("modulus must be an integer or 'auto'")
    if modulus < 2:
        parser.error("require modulus>=2")
    if args.rank_min and args.rank_base is not None:
        parser.error("choose at most one of --rank-min and --rank-base")
    rank_base = "min" if args.rank_min else args.rank_base
    try:
        if args.emit_certificate is not None and rank_base is None:
            parser.error("--emit-certificate requires --rank-base or --rank-min")
        return run(args.n, args.bound, args.mode, modulus, rank_base,
                   args.emit_certificate)
    except MemoryError:
        print(
            "sign chamber probe: INCONCLUSIVE/MEMORY_LIMIT "
            f"n={args.n} bound={args.bound}"
        )
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
