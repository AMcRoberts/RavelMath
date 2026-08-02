#!/usr/bin/env python3
"""Merge chamber rank constraints from several finite certificates.

Independent Bellman--Ford runs choose different canonical translations and
can acquire different longest-path offsets as a box grows.  This tool instead
identifies chambers by name, unions all weighted constraints, and searches for
one common integer potential.  A positive difference cycle is a sound
obstruction to that shared rank.
"""

from __future__ import annotations

import argparse
import json
from collections import deque


def load(path: str) -> dict:
    with open(path, encoding="utf-8") as stream:
        data = json.load(stream)
    if data.get("kind") != "nbonacci-sign-chamber-rank-v1":
        raise SystemExit(f"{path}: wrong certificate kind")
    return data


def solve_difference_constraints(vertex_count: int,
                                 edges: list[tuple[int, int, int]]) -> tuple[bool, list[int]]:
    """Solve h[v] >= h[u]+w by SCC-local Bellman--Ford propagation."""
    graph = [[] for _ in range(vertex_count)]
    reverse = [[] for _ in range(vertex_count)]
    for source, destination, weight in edges:
        graph[source].append((destination, weight))
        reverse[destination].append(source)

    # Iterative Kosaraju avoids the O(VE) global relaxation that dominates
    # large merged boxes while retaining an exact positive-cycle test.
    seen = [False] * vertex_count
    order: list[int] = []
    for root in range(vertex_count):
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
    component = [-1] * vertex_count
    components: list[list[int]] = []
    for root in reversed(order):
        if component[root] >= 0:
            continue
        component[root] = len(components)
        vertices = [root]
        stack = [root]
        while stack:
            vertex = stack.pop()
            for source in reverse[vertex]:
                if component[source] < 0:
                    component[source] = component[root]
                    vertices.append(source)
                    stack.append(source)
        components.append(vertices)

    dag = [[] for _ in components]
    indegree = [0] * len(components)
    for source, destination, _ in edges:
        left, right = component[source], component[destination]
        if left != right:
            dag[left].append(right)
    for source in range(len(dag)):
        dag[source] = sorted(set(dag[source]))
        for destination in dag[source]:
            indegree[destination] += 1
    queue = deque(index for index, degree in enumerate(indegree) if degree == 0)
    topological: list[int] = []
    while queue:
        source = queue.popleft()
        topological.append(source)
        for destination in dag[source]:
            indegree[destination] -= 1
            if indegree[destination] == 0:
                queue.append(destination)
    heights = [0] * vertex_count
    internal = [[] for _ in components]
    for source, destination, weight in edges:
        if component[source] == component[destination]:
            internal[component[source]].append((source, destination, weight))
    for component_id in topological:
        vertices = components[component_id]
        local_edges = internal[component_id]
        for iteration in range(len(vertices)):
            changed = False
            for source, destination, weight in local_edges:
                candidate = heights[source] + weight
                if heights[destination] < candidate:
                    heights[destination] = candidate
                    changed = True
            if not changed:
                break
        else:
            return False, []
        # Push the settled component height across its outgoing DAG edges;
        # those components occur later in the topological order.
        for source in vertices:
            for destination, weight in graph[source]:
                if component[destination] != component_id:
                    candidate = heights[source] + weight
                    if heights[destination] < candidate:
                        heights[destination] = candidate
    return True, heights


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("certificates", nargs="+")
    parser.add_argument("--emit")
    args = parser.parse_args()
    records = [load(path) for path in args.certificates]
    for key in ("n", "mode", "modulus", "rank_base"):
        if len({record.get(key) for record in records}) != 1:
            raise SystemExit(f"metadata mismatch for {key}")

    names: dict[str, int] = {}
    edges: set[tuple[int, int, int]] = set()
    bounds = []
    sources = []
    for record, path in zip(records, args.certificates):
        bounds.append(record["bound"])
        sources.append(path)
        for name in record["chambers"]:
            names.setdefault(name, len(names))
        for source, destination, weight in record["weighted_edges"]:
            edges.add((names[record["chambers"][source]],
                       names[record["chambers"][destination]], weight))

    edge_list = sorted(edges)
    feasible, offsets = solve_difference_constraints(len(names), edge_list)

    print(f"chamber merge: n={records[0]['n']} mode={records[0]['mode']} "
          f"bounds={bounds} chambers={len(names)} edges={len(edges)} "
          f"shared_rank={'PASS' if feasible else 'FAIL'}")
    if not feasible:
        return 1
    if args.emit:
        ordered = [name for name, _ in sorted(names.items(),
                                               key=lambda item: item[1])]
        output = {
            "kind": "nbonacci-sign-chamber-rank-merged-v1",
            "n": records[0]["n"], "mode": records[0]["mode"],
            "modulus": records[0]["modulus"],
            "rank_base": records[0]["rank_base"], "bounds": bounds,
            "sources": sources, "chambers": ordered,
            "weighted_edges": [list(edge) for edge in edge_list],
            "rank_certificate": {"feasible": True, "offsets": offsets},
        }
        with open(args.emit, "w", encoding="utf-8") as stream:
            json.dump(output, stream, indent=2, sort_keys=True)
            stream.write("\n")
        print(f"chamber merge: wrote {args.emit}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
