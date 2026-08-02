#!/usr/bin/env python3
"""Check all digit words at once in one exact mixed-integer SMT formula."""

from __future__ import annotations

import argparse
import os
import resource

import z3


def install_memory_limit() -> None:
    megabytes = int(os.environ.get("RAVEL_PROBE_MEMORY_MB", "10240"))
    if megabytes > 0:
        limit = megabytes * 1024 * 1024
        resource.setrlimit(resource.RLIMIT_AS, (limit, limit))


def solve(n: int, length: int, upper_q: str, timeout_ms: int):
    q = z3.Real("q")
    positive = [z3.Int(f"p_{i}") for i in range(length)]
    negative = [z3.Int(f"m_{i}") for i in range(length)]
    pos_effect = [z3.Real(f"ep_{i}") for i in range(length)]
    neg_effect = [z3.Real(f"em_{i}") for i in range(length)]
    digit_effect = [z3.Real(f"e_{i}") for i in range(length)]
    current = [z3.Real(f"x_{i}") for i in range(n)]
    solver = z3.Solver()
    if timeout_ms:
        solver.set(timeout=timeout_ms)
    solver.add(q >= 0, q < z3.RealVal(upper_q), q <= 1)
    for pos, neg, pos_value, neg_value, effect in zip(
            positive, negative, pos_effect, neg_effect, digit_effect):
        solver.add(pos >= 0, pos <= 1, neg >= 0, neg <= 1,
                   pos + neg <= 1)
        # Exact binary-times-continuous linearization: pos_value=pos*q and
        # neg_value=neg*q.  The difference is q, -q, or 0.
        for selector, value in ((pos, pos_value), (neg, neg_value)):
            solver.add(value >= 0, value <= q, value <= selector,
                       value >= q - (1 - selector))
        solver.add(effect == pos_value - neg_value)
    current_by_step = current
    for step in range(length + 1):
        solver.add(*(value >= -1 for value in current_by_step))
        solver.add(*(value <= 1 for value in current_by_step))
        solver.add(z3.Or(*(value == 1 for value in current_by_step),
                         *(value == -1 for value in current_by_step)))
        if step < length:
            tail = current_by_step[0] - sum(current_by_step[1:], z3.RealVal(0))
            current_by_step = current_by_step[1:] + [tail + digit_effect[step]]
    return solver.check(), solver


def main() -> int:
    install_memory_limit()
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--n", type=int, required=True)
    parser.add_argument("--length", type=int, required=True)
    parser.add_argument("--upper-q", default="1/7")
    parser.add_argument("--timeout-ms", type=int, default=0)
    parser.add_argument("--emit-proof")
    args = parser.parse_args()
    if args.n < 2 or args.length < 1:
        parser.error("require n>=2 and length>=1")
    if args.emit_proof:
        z3.set_param("proof", True)
    status, solver = solve(args.n, args.length, args.upper_q, args.timeout_ms)
    print(f"shared-interval-smt: n={args.n} length={args.length} "
          f"upper_q={args.upper_q} status={status}")
    if status == z3.sat:
        model = solver.model()
        word_parts = []
        for index in range(args.length):
            if model.eval(z3.Int(f"p_{index}")).as_long() == 1:
                word_parts.append("1")
            elif model.eval(z3.Int(f"m_{index}")).as_long() == 1:
                word_parts.append("-1")
            else:
                word_parts.append("0")
        word = ",".join(word_parts)
        print(f"  witness_word={word} q={model.eval(z3.Real('q'))}")
    if args.emit_proof and status == z3.unsat:
        with open(args.emit_proof, "w", encoding="utf-8") as stream:
            stream.write(str(solver.proof()) + "\n")
        print(f"  proof={args.emit_proof}")
    return 0 if status == z3.unsat else 1


if __name__ == "__main__":
    raise SystemExit(main())
