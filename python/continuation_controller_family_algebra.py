#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
from collections import deque
from itertools import product
from pathlib import Path

DIGITS = (-1, 0, 1)


def controller_states(n):
    return tuple(
        state for state in product(DIGITS, repeat=n)
        if max(map(abs, state)) == 1
    )


def center(state):
    return state[0] - sum(state[1:])


def successors(state, digit):
    result = []
    for tail in DIGITS:
        nxt = state[1:] + (tail,)
        if abs(tail - center(state) + digit) <= 1 \
                and max(map(abs, nxt)) == 1:
            result.append(nxt)
    return tuple(result)


def predecessor(continuation, digit, states, transition):
    return frozenset(
        state for state in states
        if any(nxt in continuation for nxt in transition[(state, digit)])
    )


def endpoint_monitors(n, states):
    monitors = {}
    for face_profile in product((None, -1, 1), repeat=n):
        if all(value is None for value in face_profile):
            continue
        monitor = frozenset(
            state for state in states
            if any(
                sign is not None and state[i] == sign
                for i, sign in enumerate(face_profile)
            )
        )
        monitors.setdefault(monitor, []).append(face_profile)
    return monitors


def close_family(n):
    states = controller_states(n)
    transition = {
        (state, digit): successors(state, digit)
        for state in states for digit in DIGITS
    }
    monitors = endpoint_monitors(n, states)

    members = []
    ids = {}
    queue = deque()

    def intern(member):
        if member in ids:
            return ids[member]
        idx = len(members)
        ids[member] = idx
        members.append(member)
        queue.append(member)
        return idx

    terminal_ids = {}
    for monitor, profiles in monitors.items():
        terminal_ids[intern(monitor)] = profiles

    table = {}
    while queue:
        member = queue.popleft()
        source_id = ids[member]
        for digit in DIGITS:
            target = predecessor(member, digit, states, transition)
            target_id = intern(target)
            table[(source_id, digit)] = target_id

    checked = True
    for member_id, member in enumerate(members):
        for digit in DIGITS:
            target_id = table.get((member_id, digit))
            expected = predecessor(member, digit, states, transition)
            checked &= target_id is not None
            checked &= members[target_id] == expected

    return {
        "n": n,
        "controller_states": len(states),
        "terminal_generators": len(terminal_ids),
        "family_members": len(members),
        "predecessor_transitions": len(table),
        "family_closed": checked,
        "member_size_histogram": {
            str(size): sum(len(member) == size for member in members)
            for size in sorted(set(map(len, members)))
        },
        "terminal_profiles_per_generator": {
            str(member_id): [list(profile) for profile in profiles]
            for member_id, profiles in terminal_ids.items()
        },
        "members": [
            [states.index(state) for state in sorted(member)]
            for member in members
        ],
        "predecessor_table": [
            {"source": source, "digit": digit, "target": target}
            for (source, digit), target in sorted(table.items())
        ],
    }


def corpus_residual_membership(corpus_files, closures):
    result = {}
    for filename in corpus_files:
        payload = json.loads(Path(filename).read_text())
        n = payload["n"]
        closure = closures[n]
        members = {
            frozenset(member): idx
            for idx, member in enumerate(closure["members"])
        }
        states = controller_states(n)
        transition = {
            (state, digit): successors(state, digit)
            for state in states for digit in DIGITS
        }

        occurrences = 0
        missing = 0
        used = set()
        for cert in payload["certificates"]:
            bound = payload["bound"]
            target = cert["target"]
            target_faces = tuple(
                (i, 1 if value > 0 else -1)
                for i, value in enumerate(target)
                if abs(value) == bound
            )
            residual = frozenset(
                i for i, state in enumerate(states)
                if any(state[coord] == sign for coord, sign in target_faces)
            )
            if residual not in members:
                missing += 1
            else:
                used.add(members[residual])
            occurrences += 1

            residual_states = frozenset(states[i] for i in residual)
            for digit in reversed(cert["digits"]):
                residual_states = predecessor(
                    residual_states, digit, states, transition)
                residual = frozenset(states.index(s) for s in residual_states)
                if residual not in members:
                    missing += 1
                else:
                    used.add(members[residual])
                occurrences += 1

        result[Path(filename).name] = {
            "residual_occurrences": occurrences,
            "missing_from_closed_family": missing,
            "distinct_family_members_used": len(used),
        }
    return result


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--dimensions", nargs="+", type=int, default=[3, 4])
    parser.add_argument("--corpus", nargs="*", default=[])
    parser.add_argument("--emit", required=True)
    args = parser.parse_args()

    closures = {}
    compact = {}
    for n in args.dimensions:
        closure = close_family(n)
        closures[n] = closure
        compact[str(n)] = {
            key: value for key, value in closure.items()
            if key not in {
                "members", "predecessor_table",
                "terminal_profiles_per_generator",
            }
        }
        print(
            f"n={n}: states={closure['controller_states']} "
            f"generators={closure['terminal_generators']} "
            f"family={closure['family_members']} "
            f"transitions={closure['predecessor_transitions']} "
            f"closed={closure['family_closed']}"
        )

    payload = {
        "kind": "continuation-controller-family-algebra-v1",
        "dimensions": closures,
        "summary": compact,
        "corpus_membership": corpus_residual_membership(
            args.corpus, closures) if args.corpus else {},
    }
    Path(args.emit).write_text(
        json.dumps(payload, indent=2, sort_keys=True) + "\n",
        encoding="utf-8",
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
