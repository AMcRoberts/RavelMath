#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
from collections import defaultdict, deque
from itertools import product
from pathlib import Path


def step(state: tuple[int, ...], digit: int) -> tuple[int, ...]:
    return state[1:] + (state[0] - sum(state[1:]) + digit,)


def shell_states(n: int, bound: int):
    return {
        state
        for state in product(range(-bound, bound + 1), repeat=n)
        if max(map(abs, state)) == bound
    }


def first_returns(n: int, bound: int):
    shell = shell_states(n, bound)
    returns = []
    for source in shell:
        queue = deque([(source, ())])
        seen = {source}
        while queue:
            current, digits = queue.popleft()
            for digit in (-1, 0, 1):
                target = step(current, digit)
                if not all(-bound <= value <= bound for value in target):
                    continue
                next_digits = digits + (digit,)
                if target in shell:
                    returns.append((source, target, next_digits))
                elif target not in seen:
                    seen.add(target)
                    queue.append((target, next_digits))
    return returns


def base_path(source: tuple[int, ...], digits: tuple[int, ...]):
    path = [source]
    current = source
    for digit in digits:
        current = step(current, digit)
        path.append(current)
    return path


def homogeneous_path(initial: tuple[int, ...], length: int):
    path = [initial]
    current = initial
    for _ in range(length):
        current = step(current, 0)
        path.append(current)
    return path


def bounded_covering_trajectories(n: int, delta: int, max_length: int):
    """All homogeneous trajectories with sup norm exactly delta at every window."""
    result_by_length: dict[int, list[tuple[tuple[int, ...], ...]]] = {
        length: [] for length in range(1, max_length + 1)
    }
    for initial in product(range(-delta, delta + 1), repeat=n):
        if max(map(abs, initial)) != delta:
            continue
        path = homogeneous_path(initial, max_length)
        valid_through = 0
        for index, state in enumerate(path):
            if max(map(abs, state)) != delta:
                break
            valid_through = index
        for length in range(1, min(valid_through, max_length) + 1):
            result_by_length[length].append(tuple(path[: length + 1]))
    return result_by_length


def transports_first_return(base, translation, bound: int, delta: int):
    outer = bound + delta
    transported = [
        tuple(x + t for x, t in zip(base_state, translation_state))
        for base_state, translation_state in zip(base, translation)
    ]
    if any(max(map(abs, state)) > outer for state in transported):
        return None
    if max(map(abs, transported[0])) != outer:
        return None
    if max(map(abs, transported[-1])) != outer:
        return None
    if any(max(map(abs, state)) == outer for state in transported[1:-1]):
        return None
    return transported


def fragment_ids(translation, fragment_horizon: int):
    """Canonical exact-overlap splice of a trajectory into short fragments."""
    transitions = len(translation) - 1
    fragments = []
    start = 0
    while start < transitions:
        stop = min(transitions, start + fragment_horizon)
        fragment = translation[start : stop + 1]
        fragments.append({
            "start_transition": start,
            "stop_transition": stop,
            "start_window": fragment[0],
            "end_window": fragment[-1],
            "windows": fragment,
        })
        start = stop
    for left, right in zip(fragments, fragments[1:]):
        assert tuple(left["end_window"]) == tuple(right["start_window"])
    return fragments


def audit(n: int, bound: int, max_delta: int):
    edges = first_returns(n, bound)
    max_length = max(len(digits) for _, _, digits in edges)
    fragment_horizon = n + 1

    trajectories = {}
    trajectory_counts = {}
    for delta in range(1, max_delta + 1):
        by_length = bounded_covering_trajectories(n, delta, max_length)
        trajectories[delta] = by_length
        trajectory_counts[delta] = {
            str(length): len(paths) for length, paths in by_length.items()
        }

    explained = 0
    synthesized = 0
    spliced = 0
    by_length = defaultdict(lambda: {"edges": 0, "explained": 0})
    by_delta = defaultdict(int)
    examples = []
    unexplained_examples = []

    for source, target, digits in edges:
        length = len(digits)
        by_length[length]["edges"] += 1
        base = base_path(source, digits)
        found = None
        for delta in range(1, max_delta + 1):
            for translation in trajectories[delta][length]:
                transported = transports_first_return(base, translation, bound, delta)
                if transported is None:
                    continue
                fragments = fragment_ids(translation, fragment_horizon)
                found = {
                    "delta": delta,
                    "translation_windows": translation,
                    "transported_path": transported,
                    "fragments": fragments,
                }
                break
            if found:
                break

        if found:
            explained += 1
            synthesized += 1
            by_length[length]["explained"] += 1
            by_delta[found["delta"]] += 1
            if len(found["fragments"]) > 1:
                spliced += 1
            if len(examples) < 16:
                examples.append({
                    "source": source,
                    "target": target,
                    "digits": digits,
                    "base_path": base,
                    **found,
                })
        elif len(unexplained_examples) < 16:
            unexplained_examples.append({
                "source": source,
                "target": target,
                "digits": digits,
                "path_length": length,
            })

    return {
        "kind": "nbonacci-source-conditioned-cover-tube-splice-v1",
        "n": n,
        "bound": bound,
        "max_delta": max_delta,
        "fragment_horizon": fragment_horizon,
        "max_return_length": max_length,
        "edges": len(edges),
        "explained": explained,
        "unexplained": len(edges) - explained,
        "coverage_ratio": explained / len(edges) if edges else 1.0,
        "source_conditioned_syntheses": synthesized,
        "multi_fragment_splices": spliced,
        "trajectory_counts": trajectory_counts,
        "explained_by_delta": dict(sorted(by_delta.items())),
        "coverage_by_path_length": {
            str(length): values for length, values in sorted(by_length.items())
        },
        "examples": examples,
        "unexplained_examples": unexplained_examples,
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--n", type=int, required=True)
    parser.add_argument("--bound", type=int, required=True)
    parser.add_argument("--max-delta", type=int, default=4)
    parser.add_argument("--emit", required=True)
    args = parser.parse_args()

    payload = audit(args.n, args.bound, args.max_delta)
    Path(args.emit).write_text(json.dumps(payload, indent=2, sort_keys=True) + "\n")
    print(
        f"synth+splice n={args.n} M={args.bound} delta<= {args.max_delta}: "
        f"{payload['explained']}/{payload['edges']} "
        f"({payload['coverage_ratio']:.3%}), "
        f"multi-fragment={payload['multi_fragment_splices']}"
    )


if __name__ == "__main__":
    main()
