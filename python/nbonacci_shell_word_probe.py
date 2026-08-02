#!/usr/bin/env python3
"""Mine digit words that keep consecutive carry windows on a max shell."""

from __future__ import annotations

import argparse
import os
import resource
from collections import defaultdict
from itertools import product


def install_memory_limit() -> None:
    megabytes = int(os.environ.get("RAVEL_PROBE_MEMORY_MB", "10240"))
    if megabytes > 0:
        limit = megabytes * 1024 * 1024
        resource.setrlimit(resource.RLIMIT_AS, (limit, limit))


def transitions(state: tuple[int, ...], bound: int):
    tail = state[0] - sum(state[1:])
    for digit in (-1, 0, 1):
        target = state[1:] + (tail + digit,)
        if all(-bound <= value <= bound for value in target):
            yield digit, target


def shell(state: tuple[int, ...], bound: int) -> bool:
    return max(map(abs, state)) == bound


def mine(n: int, bound: int, max_steps: int):
    states = product(range(-bound, bound + 1), repeat=n)
    frontier = {state for state in states if shell(state, bound)}
    words: dict[int, set[tuple[int, ...]]] = defaultdict(set)
    total_paths = 0
    for state in frontier:
        stack = [(state, ())]
        while stack:
            current, word = stack.pop()
            if word:
                words[len(word)].add(word)
                total_paths += 1
            if len(word) >= max_steps:
                continue
            for digit, target in transitions(current, bound):
                if shell(target, bound):
                    stack.append((target, word + (digit,)))
    longest = max(words, default=0)
    return len(frontier), total_paths, words, longest


def main() -> int:
    install_memory_limit()
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--n", type=int, required=True)
    parser.add_argument("--bound", type=int, required=True)
    parser.add_argument("--max-steps", type=int, default=12)
    args = parser.parse_args()
    if args.n < 2 or args.bound < 1 or args.max_steps < 1:
        parser.error("require n>=2, bound>=1, max-steps>=1")
    shell_count, total_paths, words, longest = mine(
        args.n, args.bound, args.max_steps)
    print(f"shell words: n={args.n} bound={args.bound} shell={shell_count} "
          f"paths={total_paths} longest_transitions={longest}")
    for length in sorted(words):
        print(f"  length={length} distinct_words={len(words[length])}")
    if longest >= args.max_steps:
        print("  status=TRUNCATED_AT_MAX_STEPS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
