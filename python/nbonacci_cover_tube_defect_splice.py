#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
from collections import deque, defaultdict
from itertools import product
from pathlib import Path


def step(state, digit):
    return state[1:] + (state[0] - sum(state[1:]) + digit,)


def shell_states(n, bound):
    return {
        x for x in product(range(-bound, bound + 1), repeat=n)
        if max(map(abs, x)) == bound
    }


def first_returns(n, bound):
    shell = shell_states(n, bound)
    out = []
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
                    out.append((source, target, next_digits))
                elif target not in seen:
                    seen.add(target)
                    queue.append((target, next_digits))
    return out


def base_path(source, digits):
    path = [source]
    current = source
    for digit in digits:
        current = step(current, digit)
        path.append(current)
    return path


def boundary_states(n, delta):
    return [
        state for state in product(range(-delta, delta + 1), repeat=n)
        if max(map(abs, state)) == delta
    ]


def synthesize_defect_splice(source, digits, bound, delta, states):
    """Find a bounded translation path with admissible digit-coordinate splices.

    At step k, choose q so that t[k+1] = A*t[k] + q*e_last and
    digits[k] + q remains in {-1,0,1}. Then x+t follows the same system with
    adjusted digit digits[k]+q.
    """
    base = base_path(source, digits)
    outer = bound + delta

    # parents[k][translation_state] = (previous_state, q, adjusted_digit)
    current = {}
    parents = []
    for t0 in states:
        transported0 = tuple(x + t for x, t in zip(base[0], t0))
        if max(map(abs, transported0)) == outer:
            current[t0] = None
    parents.append(current)

    for k, digit in enumerate(digits):
        next_layer = {}
        final_step = k + 1 == len(digits)
        for t in current:
            for adjusted_digit in (-1, 0, 1):
                q = adjusted_digit - digit
                t_next = step(t, q)
                if max(map(abs, t_next)) != delta:
                    continue
                transported = tuple(
                    x + shift for x, shift in zip(base[k + 1], t_next)
                )
                norm = max(map(abs, transported))
                if norm > outer:
                    continue
                if final_step:
                    if norm != outer:
                        continue
                elif norm >= outer:
                    continue
                if t_next not in next_layer:
                    next_layer[t_next] = (t, q, adjusted_digit)
        if not next_layer:
            return None
        current = next_layer
        parents.append(current)

    terminal = next(iter(current))
    translation = [None] * (len(digits) + 1)
    defects = [None] * len(digits)
    adjusted_digits = [None] * len(digits)
    translation[-1] = terminal
    for k in range(len(digits), 0, -1):
        previous, q, adjusted = parents[k][translation[k]]
        translation[k - 1] = previous
        defects[k - 1] = q
        adjusted_digits[k - 1] = adjusted

    transported = [
        tuple(x + t for x, t in zip(base_state, translation_state))
        for base_state, translation_state in zip(base, translation)
    ]
    # Independent replay with adjusted digits.
    replay = [transported[0]]
    current_state = transported[0]
    for digit in adjusted_digits:
        current_state = step(current_state, digit)
        replay.append(current_state)
    assert replay == transported

    splice_positions = [i for i, q in enumerate(defects) if q != 0]
    return {
        "delta": delta,
        "base_path": base,
        "translation_windows": translation,
        "defects": defects,
        "adjusted_digits": adjusted_digits,
        "transported_path": transported,
        "splice_positions": splice_positions,
    }


def audit(n, bound, max_delta):
    edges = first_returns(n, bound)
    state_cache = {delta: boundary_states(n, delta)
                   for delta in range(1, max_delta + 1)}
    explained = 0
    exact_homogeneous = 0
    defect_spliced = 0
    by_delta = defaultdict(int)
    by_length = defaultdict(lambda: {"edges": 0, "explained": 0})
    examples = []
    certificates = []
    unexplained = []

    for source, target, digits in edges:
        by_length[len(digits)]["edges"] += 1
        found = None
        for delta in range(1, max_delta + 1):
            found = synthesize_defect_splice(
                source, digits, bound, delta, state_cache[delta]
            )
            if found:
                break
        if found:
            explained += 1
            by_length[len(digits)]["explained"] += 1
            by_delta[found["delta"]] += 1
            if found["splice_positions"]:
                defect_spliced += 1
            else:
                exact_homogeneous += 1
            record = {
                "source": source,
                "target": target,
                "digits": digits,
                **found,
            }
            certificates.append(record)
            if len(examples) < 20:
                examples.append(record)
        elif len(unexplained) < 20:
            unexplained.append({
                "source": source,
                "target": target,
                "digits": digits,
                "path_length": len(digits),
            })

    return {
        "kind": "nbonacci-source-conditioned-defect-splice-v1",
        "n": n,
        "bound": bound,
        "max_delta": max_delta,
        "edges": len(edges),
        "explained": explained,
        "unexplained": len(edges) - explained,
        "coverage_ratio": explained / len(edges) if edges else 1.0,
        "exact_homogeneous": exact_homogeneous,
        "defect_spliced": defect_spliced,
        "explained_by_delta": dict(sorted(by_delta.items())),
        "coverage_by_path_length": {
            str(length): values for length, values in sorted(by_length.items())
        },
        "certificate_count": len(certificates),
        "certificates": certificates,
        "examples": examples,
        "unexplained_examples": unexplained,
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
        f"defect splice n={args.n} M={args.bound} delta<={args.max_delta}: "
        f"{payload['explained']}/{payload['edges']} "
        f"({payload['coverage_ratio']:.3%}); "
        f"exact={payload['exact_homogeneous']} "
        f"spliced={payload['defect_spliced']}"
    )


if __name__ == "__main__":
    main()
