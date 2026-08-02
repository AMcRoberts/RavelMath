#!/usr/bin/env python3
"""Search normalized shell digit words with a bounded MILP.

After scaling a shell of radius M to radius one, the forcing amplitude is
q=1/M in [0,1].  For a fixed digit word, shell membership at every window is
a disjunction saying that some coordinate equals +/-1.  The MILP encodes
those boundary choices exactly.  A word reported infeasible is forbidden for
every M>=1; timeouts are explicitly inconclusive.
"""

from __future__ import annotations

import argparse
import itertools
import os
import resource

import numpy as np
from scipy import sparse
from scipy.optimize import Bounds, LinearConstraint, milp

from nbonacci_block_spectrum_probe import carry_matrix


def install_memory_limit() -> None:
    megabytes = int(os.environ.get("RAVEL_PROBE_MEMORY_MB", "10240"))
    if megabytes > 0:
        limit = megabytes * 1024 * 1024
        resource.setrlimit(resource.RLIMIT_AS, (limit, limit))


def canonical_words(length: int):
    seen = set()
    for word in itertools.product((-1, 0, 1), repeat=length):
        mirror = tuple(-digit for digit in word)
        if word in seen or mirror in seen:
            continue
        seen.add(word)
        yield word


def solve_word(n: int, word: tuple[int, ...], time_limit: float,
               max_q: float):
    A = np.linalg.inv(carry_matrix(n))
    matrices = []
    current = np.zeros((n, n + 1))
    current[:, :n] = np.eye(n)
    matrices.append(current.copy())
    for digit in word:
        current = A @ current
        current[:, -1] += np.eye(n)[:, -1] * digit
        matrices.append(current.copy())

    windows = len(word) + 1
    binary_count = windows * 2 * n
    variable_count = n + 1 + binary_count
    rows, lower, upper = [], [], []

    def add(row, lo=-np.inf, hi=np.inf):
        rows.append(row)
        lower.append(lo)
        upper.append(hi)

    for window, matrix in enumerate(matrices):
        for coordinate in range(n):
            row = np.zeros(variable_count)
            row[:n + 1] = matrix[coordinate]
            add(row, -1, 1)
            add(-row, -1, 1)
        for coordinate in range(n):
            for sign_index, sign in enumerate((-1, 1)):
                binary = n + 1 + window * 2 * n + coordinate * 2 + sign_index
                row = np.zeros(variable_count)
                row[:n + 1] = matrix[coordinate]
                row[binary] = 2
                add(row, -np.inf, sign + 2)
                row = np.zeros(variable_count)
                row[:n + 1] = -matrix[coordinate]
                row[binary] = 2
                add(row, -np.inf, 2 - sign)
        row = np.zeros(variable_count)
        start = n + 1 + window * 2 * n
        row[start:start + 2 * n] = 1
        add(row, 1, np.inf)

    integrality = np.zeros(variable_count)
    integrality[n + 1:] = 1
    lower_bounds = np.r_[np.full(n, -1.0), 0.0, np.zeros(binary_count)]
    upper_bounds = np.r_[np.full(n, 1.0), max_q, np.ones(binary_count)]
    return milp(
        np.zeros(variable_count), integrality=integrality,
        bounds=Bounds(lower_bounds, upper_bounds),
        constraints=LinearConstraint(sparse.csr_matrix(np.asarray(rows)),
                                     np.asarray(lower), np.asarray(upper)),
        options={"time_limit": time_limit}), matrices


def main() -> int:
    install_memory_limit()
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--n", type=int, required=True)
    parser.add_argument("--length", type=int, required=True,
                        help="number of digit transitions")
    parser.add_argument("--time-limit", type=float, default=2.0)
    parser.add_argument("--max-words", type=int, default=0,
                        help="stop after this many canonical words (0=all)")
    parser.add_argument("--max-q", type=float, default=1.0,
                        help="upper bound on q=1/M (use .5 for M>=2)")
    args = parser.parse_args()
    if args.n < 2 or args.length < 1 or not 0 < args.max_q <= 1:
        parser.error("require n>=2, length>=1, and 0<max-q<=1")
    counts = {"feasible": 0, "infeasible": 0, "inconclusive": 0}
    witnesses = []
    q_samples = []
    for index, word in enumerate(canonical_words(args.length)):
        if args.max_words and index >= args.max_words:
            break
        result, _ = solve_word(args.n, word, args.time_limit, args.max_q)
        if result.status == 0:
            counts["feasible"] += 1
            if len(witnesses) < 8:
                witnesses.append(word)
                q_samples.append(float(result.x[args.n]))
        elif result.status == 2:
            counts["infeasible"] += 1
        else:
            counts["inconclusive"] += 1
    print(f"forbidden-block MILP: n={args.n} length={args.length} "
          f"tested={sum(counts.values())} {counts}")
    if witnesses:
        print("  feasible_words=" + " ".join("".join(map(str, word))
                                            for word in witnesses))
        print("  witness_q=" + " ".join(f"{value:.6g}" for value in q_samples))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
