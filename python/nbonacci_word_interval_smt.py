#!/usr/bin/env python3
"""Exact interval check for normalized shell digit words.

For a fixed digit word, encode the carry windows over real variables and ask
whether any normalized shell chain exists for ``0 <= q < upper_q``.  Unlike
the exploratory MILP, this uses exact QF_LRA and a strict rational endpoint.
The canonical sign quotient is sound because negating every state and digit
preserves the shell constraints.
"""

from __future__ import annotations

import argparse
import itertools
import os
import re
import resource

import z3


def install_memory_limit() -> None:
    megabytes = int(os.environ.get("RAVEL_PROBE_MEMORY_MB", "10240"))
    if megabytes > 0:
        limit = megabytes * 1024 * 1024
        resource.setrlimit(resource.RLIMIT_AS, (limit, limit))


def canonical_words(length: int):
    for word in itertools.product((-1, 0, 1), repeat=length):
        mirror = tuple(-digit for digit in word)
        if word <= mirror:
            yield word


def solve_word(n: int, word: tuple[int, ...], upper_q: str,
               timeout_ms: int = 0):
    q = z3.Real("q")
    state = [z3.Real(f"x_{i}") for i in range(n)]
    solver = z3.Solver()
    if timeout_ms:
        solver.set(timeout=timeout_ms)
    solver.add(q >= 0, q < z3.RealVal(upper_q))
    current = state
    for index in range(len(word) + 1):
        solver.add(*(value >= -1 for value in current))
        solver.add(*(value <= 1 for value in current))
        solver.add(z3.Or(*(value == 1 for value in current),
                         *(value == -1 for value in current)))
        if index < len(word):
            tail = current[0] - sum(current[1:], z3.RealVal(0))
            current = current[1:] + [tail + q * word[index]]
    return solver.check(), solver


def parse_word(text: str) -> tuple[int, ...]:
    try:
        tokens = re.findall(r"-1|0|1", text)
        if not tokens or "".join(tokens) != text.replace(",", "").replace(" ", ""):
            raise ValueError
        word = tuple(int(token) for token in tokens)
    except ValueError as error:
        raise argparse.ArgumentTypeError("word must contain only -1, 0, 1") from error
    if any(value not in (-1, 0, 1) for value in word):
        raise argparse.ArgumentTypeError("word must contain only -1, 0, 1")
    return word


def main() -> int:
    install_memory_limit()
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--n", type=int, required=True)
    parser.add_argument("--length", type=int, required=True)
    parser.add_argument("--upper-q", default="1/7")
    parser.add_argument("--timeout-ms", type=int, default=0)
    parser.add_argument("--word", type=parse_word,
                        help="check one word instead of all canonical words")
    parser.add_argument("--emit-proof")
    args = parser.parse_args()
    if args.n < 2 or args.length < 1:
        parser.error("require n>=2 and length>=1")
    words = [args.word] if args.word is not None else list(canonical_words(args.length))
    if any(len(word) != args.length for word in words):
        parser.error("--word length must equal --length")
    if args.emit_proof:
        z3.set_param("proof", True)
    counts = {"unsat": 0, "sat": 0, "unknown": 0}
    for word in words:
        status, solver = solve_word(args.n, word, args.upper_q, args.timeout_ms)
        counts[str(status)] += 1
        if status != z3.unsat:
            print(f"  non-UNSAT word={''.join(map(str, word))} status={status}")
        if args.emit_proof and word == words[0] and status == z3.unsat:
            with open(args.emit_proof, "w", encoding="utf-8") as stream:
                stream.write(str(solver.proof()) + "\n")
    print(f"word-interval-smt: n={args.n} length={args.length} "
          f"upper_q={args.upper_q} tested={len(words)} {counts}")
    return 0 if counts["sat"] == 0 and counts["unknown"] == 0 else 1


if __name__ == "__main__":
    raise SystemExit(main())
