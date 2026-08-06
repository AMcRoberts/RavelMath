#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
from pathlib import Path


def step(state, digit):
    return tuple(state[1:]) + (state[0] - sum(state[1:]) + digit,)


def check_certificate(record, bound):
    source = tuple(record["source"])
    digits = tuple(record["digits"])
    base = [tuple(x) for x in record["base_path"]]
    translation = [tuple(x) for x in record["translation_windows"]]
    defects = tuple(record["defects"])
    adjusted = tuple(record["adjusted_digits"])
    transported = [tuple(x) for x in record["transported_path"]]
    delta = record["delta"]

    assert base[0] == source
    assert len(base) == len(digits) + 1
    assert len(translation) == len(base)
    assert len(defects) == len(digits)
    assert len(adjusted) == len(digits)
    assert len(transported) == len(base)

    current = source
    replay_base = [current]
    for digit in digits:
        current = step(current, digit)
        replay_base.append(current)
    assert replay_base == base

    for k in range(len(digits)):
        assert adjusted[k] == digits[k] + defects[k]
        assert -1 <= adjusted[k] <= 1
        assert step(translation[k], defects[k]) == translation[k + 1]
        assert step(transported[k], adjusted[k]) == transported[k + 1]

    assert transported == [
        tuple(x + t for x, t in zip(base_state, translation_state))
        for base_state, translation_state in zip(base, translation)
    ]
    assert all(max(map(abs, t)) == delta for t in translation)
    outer = bound + delta
    assert max(map(abs, transported[0])) == outer
    assert max(map(abs, transported[-1])) == outer
    assert all(max(map(abs, state)) < outer for state in transported[1:-1])


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("certificate")
    args = parser.parse_args()
    payload = json.loads(Path(args.certificate).read_text())
    records = payload["certificates"]
    assert len(records) == payload["certificate_count"]
    assert len(records) == payload["explained"]
    for record in records:
        check_certificate(record, payload["bound"])
    print(
        f"checked {len(records)} defect-spliced covering-tube certificates "
        f"for n={payload['n']} M={payload['bound']}"
    )


if __name__ == "__main__":
    main()
