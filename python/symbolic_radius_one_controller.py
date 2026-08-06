#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
from collections import deque, defaultdict
from itertools import product
from pathlib import Path


DIGITS = (-1, 0, 1)


def states(n):
    return tuple(x for x in product(DIGITS, repeat=n)
                 if max(map(abs, x)) == 1)


def face_states(all_states, coordinate, sign):
    return frozenset(x for x in all_states if x[coordinate] == sign)


def center(t):
    return t[0] - sum(t[1:])


def legal_tail(t, digit, tail):
    # adjusted = tail - center(t) + digit
    return abs(tail - center(t) + digit) <= 1


def successors(t, digit):
    result = []
    for tail in DIGITS:
        if not legal_tail(t, digit, tail):
            continue
        nxt = t[1:] + (tail,)
        if max(map(abs, nxt)) == 1:
            result.append(nxt)
    return tuple(result)


def transition_subset(subset, digit, succ):
    out = set()
    for t in subset:
        out.update(succ[(t, digit)])
    return frozenset(out)


def symmetry_negate(t):
    return tuple(-x for x in t)


def canonical_subset(subset):
    neg = frozenset(symmetry_negate(t) for t in subset)
    a = tuple(sorted(subset))
    b = tuple(sorted(neg))
    return a if a <= b else b


def shortest_arbitrary_counterexample(n, max_depth):
    all_states = states(n)
    succ = {(t, d): successors(t, d) for t in all_states for d in DIGITS}
    counterexamples = []
    max_reachable_subsets = 0

    for source_i in range(n):
        for source_sign in (-1, 1):
            initial = face_states(all_states, source_i, source_sign)
            queue = deque([(initial, ())])
            seen = {initial}
            while queue:
                subset, word = queue.popleft()
                max_reachable_subsets = max(max_reachable_subsets, len(seen))
                for target_i in range(n):
                    for target_sign in (-1, 1):
                        target = face_states(all_states, target_i, target_sign)
                        if not subset.intersection(target):
                            counterexamples.append({
                                "source_face": [source_i, source_sign],
                                "target_face": [target_i, target_sign],
                                "word": list(word),
                                "reachable_states": len(subset),
                            })
                            return counterexamples[0], max_reachable_subsets
                if len(word) == max_depth:
                    continue
                for d in DIGITS:
                    nxt = transition_subset(subset, d, succ)
                    if nxt not in seen:
                        seen.add(nxt)
                        queue.append((nxt, word + (d,)))
    return None, max_reachable_subsets


def exact_reachable_quotient(n, max_depth):
    all_states = states(n)
    succ = {(t, d): successors(t, d) for t in all_states for d in DIGITS}
    initial_sets = []
    for i in range(n):
        for sign in (-1, 1):
            initial_sets.append(face_states(all_states, i, sign))

    layers = [set(initial_sets)]
    all_subsets = set(initial_sets)
    transitions = {}
    for _ in range(max_depth):
        next_layer = set()
        for subset in layers[-1]:
            for d in DIGITS:
                nxt = transition_subset(subset, d, succ)
                transitions[(subset, d)] = nxt
                if nxt not in all_subsets:
                    all_subsets.add(nxt)
                    next_layer.add(nxt)
        layers.append(next_layer)
        if not next_layer:
            break

    canonical_classes = {canonical_subset(s) for s in all_subsets}
    cardinalities = defaultdict(int)
    center_profiles = defaultdict(int)
    for subset in all_subsets:
        cardinalities[len(subset)] += 1
        profile = tuple(sorted(set(center(t) for t in subset)))
        center_profiles[profile] += 1

    return {
        "concrete_states": len(all_states),
        "reachable_subset_states": len(all_subsets),
        "negation_quotient_states": len(canonical_classes),
        "layer_new_states": [len(layer) for layer in layers],
        "subset_cardinality_histogram": dict(sorted(cardinalities.items())),
        "center_profile_classes": len(center_profiles),
    }


def corpus_check(files):
    total = 0
    accepted = 0
    feature_counts = defaultdict(int)
    failures = []
    for filename in files:
        payload = json.loads(Path(filename).read_text())
        n = payload["n"]
        all_states = states(n)
        succ = {(t, d): successors(t, d) for t in all_states for d in DIGITS}
        bound = payload["bound"]
        for cert in payload["certificates"]:
            total += 1
            source_faces = [(i, 1 if v > 0 else -1)
                            for i, v in enumerate(cert["source"])
                            if abs(v) == bound]
            target_faces = [(i, 1 if v > 0 else -1)
                            for i, v in enumerate(cert["target"])
                            if abs(v) == bound]
            subset = frozenset(
                t for t in all_states
                if any(t[i] == sign for i, sign in source_faces)
            )
            min_sizes = [len(subset)]
            for d in cert["digits"]:
                subset = transition_subset(subset, d, succ)
                min_sizes.append(len(subset))
            ok = any(t[i] == sign for t in subset
                     for i, sign in target_faces)
            if ok:
                accepted += 1
            elif len(failures) < 20:
                failures.append({
                    "file": filename,
                    "source": cert["source"],
                    "target": cert["target"],
                    "digits": cert["digits"],
                })
            feature_counts[(
                n,
                len(cert["digits"]),
                len(source_faces),
                len(target_faces),
                min(min_sizes),
            )] += 1
    return {
        "total": total,
        "accepted": accepted,
        "failures": failures,
        "feature_class_count": len(feature_counts),
        "feature_classes": [
            {
                "n": k[0], "word_length": k[1],
                "source_face_count": k[2], "target_face_count": k[3],
                "minimum_reachable_set_size": k[4], "count": v,
            }
            for k, v in sorted(feature_counts.items())
        ],
    }


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--max-n", type=int, default=7)
    parser.add_argument("--depth", type=int, default=8)
    parser.add_argument("--corpus", nargs="*", default=[])
    parser.add_argument("--emit", required=True)
    args = parser.parse_args()

    dimensions = []
    for n in range(2, args.max_n + 1):
        quotient = exact_reachable_quotient(n, args.depth)
        counterexample, visited = shortest_arbitrary_counterexample(n, args.depth)
        quotient["n"] = n
        quotient["shortest_arbitrary_counterexample"] = counterexample
        quotient["counterexample_search_subsets"] = visited
        dimensions.append(quotient)
        print(
            f"n={n}: states={quotient['concrete_states']} "
            f"subsets={quotient['reachable_subset_states']} "
            f"neg-classes={quotient['negation_quotient_states']} "
            f"arbitrary={'FAIL' if counterexample else 'no counterexample'}"
        )

    output = {
        "kind": "symbolic-radius-one-controller-v1",
        "transition_normal_form":
            "next=t.tail++[r], r in {-1,0,1}, "
            "abs(r-(t0-sum(tail))+digit)<=1",
        "dimensions": dimensions,
        "corpus": corpus_check(args.corpus) if args.corpus else None,
    }
    Path(args.emit).write_text(json.dumps(output, indent=2, sort_keys=True) + "\n")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
