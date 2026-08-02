#!/usr/bin/env python3
"""Capped vectorized diagnostic for raw L1 growth under the n+1 block map."""

from __future__ import annotations

import argparse
import itertools
import os
import resource

import numpy as np


def install_memory_limit() -> None:
    megabytes = int(os.environ.get("RAVEL_PROBE_MEMORY_MB", "10240"))
    if megabytes > 0:
        limit = megabytes * 1024 * 1024
        resource.setrlimit(resource.RLIMIT_AS, (limit, limit))


def block_matrix(n: int) -> np.ndarray:
    a = np.zeros((n, n), dtype=np.int64)
    for column in range(n - 1):
        a[column, column + 1] = 1
    a[-1, 0] = 1
    a[-1, 1:] = -1
    return np.linalg.matrix_power(a, n + 1)


def forcing_words(n: int) -> np.ndarray:
    values = []
    for digits in itertools.product((-1, 0, 1), repeat=n + 1):
        vector = np.zeros(n, dtype=np.int64)
        for r, digit in enumerate(digits):
            if r == 0:
                vector[0] -= digit
                vector[-1] += 2 * digit
            elif r < n:
                vector[r - 1] += digit
                vector[r] -= digit
            else:
                vector[-1] += digit
        values.append(vector)
    return np.asarray(values, dtype=np.int64)


def state_chunks(n: int, bound: int, chunk_size: int):
    values = range(-bound, bound + 1)
    chunk = []
    for state in itertools.product(values, repeat=n):
        chunk.append(state)
        if len(chunk) == chunk_size:
            yield np.asarray(chunk, dtype=np.int64)
            chunk = []
    if chunk:
        yield np.asarray(chunk, dtype=np.int64)


def run(n: int, bound: int, chunk_size: int) -> None:
    matrix = block_matrix(n)
    forcing = forcing_words(n)
    best_delta = None
    best_state = None
    best_forcing = None
    checked = 0
    for states in state_chunks(n, bound, chunk_size):
        original = np.abs(states).sum(axis=1)
        # Stream forcing words so the temporary chunk never scales as
        # (states * 3^(n+1)).
        minimum = np.full(len(states), np.iinfo(np.int64).max, dtype=np.int64)
        minimum_word = np.zeros(len(states), dtype=np.int64)
        image = states @ matrix.T
        for word_index, word in enumerate(forcing):
            delta = np.abs(image + word).sum(axis=1) - original
            improved = delta < minimum
            minimum[improved] = delta[improved]
            minimum_word[improved] = word_index
        exterior = np.max(np.abs(states), axis=1) > 1
        if np.any(exterior):
            indices = np.flatnonzero(exterior)
            local = int(minimum[indices].min())
            if best_delta is None or local < best_delta:
                winner = indices[int(np.argmin(minimum[indices]))]
                best_delta = local
                best_state = states[winner].tolist()
                best_forcing = forcing[minimum_word[winner]].tolist()
        checked += len(states)
    print(
        f"raw block L1 probe: n={n} bound={bound} states={checked} "
        f"forcing_words={len(forcing)} min_delta={best_delta} "
        f"state={best_state} forcing={best_forcing}"
    )


def main() -> int:
    install_memory_limit()
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--n", type=int, required=True)
    parser.add_argument("--bound", type=int, required=True)
    parser.add_argument("--chunk-size", type=int, default=8192)
    args = parser.parse_args()
    if args.n < 2 or args.bound < 1 or args.chunk_size < 1:
        parser.error("require n>=2, bound>=1, chunk-size>=1")
    run(args.n, args.bound, args.chunk_size)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

