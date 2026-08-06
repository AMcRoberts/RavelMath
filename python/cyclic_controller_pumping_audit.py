#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
from collections import defaultdict
from itertools import product
from pathlib import Path

DIGITS = (-1, 0, 1)

def translation_states(n):
    return tuple(
        state for state in product(DIGITS, repeat=n)
        if max(map(abs, state)) <= 1
    )

def center(t):
    return t[0] - sum(t[1:])

def successors(t, digit):
    result = []
    for adjusted in DIGITS:
        defect = adjusted - digit
        nxt = t[1:] + (center(t) + defect,)
        if all(-1 <= x <= 1 for x in nxt):
            result.append((nxt, adjusted, defect))
    return tuple(result)

def cyclic_fixed_points(n, word):
    states = translation_states(n)
    fixed = []
    for start in states:
        frontier = {start: []}
        for digit in word:
            nxt_frontier = {}
            for state, trace in frontier.items():
                for nxt, adjusted, defect in successors(state, digit):
                    nxt_frontier.setdefault(
                        nxt, trace + [(nxt, adjusted, defect)]
                    )
            frontier = nxt_frontier
            if not frontier:
                break
        if start in frontier:
            fixed.append({
                "start": start,
                "trace": frontier[start],
            })
    return fixed

def load_cycles(path):
    payload = json.loads(Path(path).read_text())
    cycles = []
    # Support several likely schemas from prior graph/carry exports.
    for key in ("cycles", "recurrent_cycles", "cycle_certificates"):
        if key in payload:
            for cycle in payload[key]:
                word = cycle.get("digits") or cycle.get("word") or cycle.get("digit_word")
                if word is not None:
                    cycles.append({
                        "id": cycle.get("id", f"{key}.{len(cycles)}"),
                        "digits": tuple(word),
                        "source_radius": cycle.get("radius"),
                    })
    return cycles

def audit_words(n, words):
    results = []
    for item in words:
        fixed = cyclic_fixed_points(n, item["digits"])
        results.append({
            **item,
            "fixed_controller_runs": len(fixed),
            "example": fixed[0] if fixed else None,
            "accepted": bool(fixed),
        })
    return results

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--n", type=int, required=True)
    parser.add_argument("--input")
    parser.add_argument("--emit", required=True)
    parser.add_argument("--max-word-length", type=int, default=10)
    args = parser.parse_args()

    words = []
    if args.input and Path(args.input).exists():
        words.extend(load_cycles(args.input))

    # If no explicit cycle corpus is available, audit all words through a
    # bounded length. This is characterization, not a universal claim.
    if not words:
        for length in range(1, args.max_word_length + 1):
            for word in product(DIGITS, repeat=length):
                words.append({
                    "id": f"all_words.L{length}.{len(words)}",
                    "digits": word,
                    "source_radius": None,
                })

    results = audit_words(args.n, words)
    payload = {
        "kind": "cyclic-controller-pumping-audit-v1",
        "n": args.n,
        "words": len(results),
        "accepted": sum(r["accepted"] for r in results),
        "rejected": sum(not r["accepted"] for r in results),
        "results": results,
    }
    Path(args.emit).write_text(
        json.dumps(payload, indent=2, sort_keys=True) + "\n"
    )
    print(
        f"cyclic controller n={args.n}: "
        f"{payload['accepted']}/{payload['words']} words have fixed runs"
    )

if __name__ == "__main__":
    main()
