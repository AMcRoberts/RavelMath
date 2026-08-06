#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
from itertools import product
from pathlib import Path


def step(t, q):
    return t[1:] + (t[0] - sum(t[1:]) + q,)


def states(n):
    return [x for x in product((-1, 0, 1), repeat=n)
            if max(map(abs, x)) == 1]


def faces(x, bound):
    return [(i, 1 if value > 0 else -1)
            for i, value in enumerate(x) if abs(value) == bound]


def synthesize(n, source, target, digits, bound):
    source_faces = faces(source, bound)
    target_faces = faces(target, bound)
    current = {t: None for t in states(n)
               if any(t[i] == sign for i, sign in source_faces)}
    parents = [current]
    for digit in digits:
        next_layer = {}
        for t in current:
            for adjusted in (-1, 0, 1):
                q = adjusted - digit
                u = step(t, q)
                if max(map(abs, u)) != 1:
                    continue
                if u not in next_layer:
                    next_layer[u] = (t, q, adjusted)
        if not next_layer:
            return None
        current = next_layer
        parents.append(current)

    terminal = next((t for t in current
                     if any(t[i] == sign for i, sign in target_faces)), None)
    if terminal is None:
        return None

    translation = [None] * (len(digits) + 1)
    defects = [None] * len(digits)
    adjusted = [None] * len(digits)
    translation[-1] = terminal
    for k in range(len(digits), 0, -1):
        previous, q, digit2 = parents[k][translation[k]]
        translation[k - 1] = previous
        defects[k - 1] = q
        adjusted[k - 1] = digit2
    return translation, defects, adjusted


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("inputs", nargs="+")
    parser.add_argument("--emit", required=True)
    args = parser.parse_args()

    total = 0
    closed = 0
    by_instance = []
    failures = []
    for filename in args.inputs:
        payload = json.loads(Path(filename).read_text())
        instance_total = 0
        instance_closed = 0
        for certificate in payload["certificates"]:
            total += 1
            instance_total += 1
            result = synthesize(
                payload["n"],
                tuple(certificate["source"]),
                tuple(certificate["target"]),
                tuple(certificate["digits"]),
                payload["bound"],
            )
            if result is not None:
                closed += 1
                instance_closed += 1
            elif len(failures) < 20:
                failures.append({
                    "file": filename,
                    "source": certificate["source"],
                    "target": certificate["target"],
                    "digits": certificate["digits"],
                })
        by_instance.append({
            "file": filename,
            "closed": instance_closed,
            "total": instance_total,
        })

    output = {
        "kind": "uniform-radius-one-synthesis-replay-v1",
        "closed": closed,
        "total": total,
        "coverage": closed / total if total else 1.0,
        "instances": by_instance,
        "failures": failures,
    }
    Path(args.emit).write_text(json.dumps(output, indent=2) + "\n")
    print(f"uniform controller replay: {closed}/{total}")
    return 0 if closed == total else 1


if __name__ == "__main__":
    raise SystemExit(main())
