#!/usr/bin/env python3
"""Bounded UNSAT probe for the n-bonacci periodic carry recurrence.

Writing x_t=(a_t,...,a_{t+n-1}), the inverse-incidence update is equivalent
to

    delta_t = -a_t + a_{t+1} + ... + a_{t+n},   delta_t in {-1,0,1}.

This script asks Z3 whether a periodic integer word of period p can satisfy
that recurrence while containing a coefficient of absolute value at least 2.
It is deliberately a finite probe: its output is evidence for the carry
lemma, not a universal proof.  The useful artifact is the explicit UNSAT
rectangle (n,p) that was checked.
"""

from __future__ import annotations

import argparse
from dataclasses import dataclass

from z3 import Int, Or, Solver, sat


@dataclass(frozen=True)
class ProbeResult:
    n: int
    period: int
    status: str
    witness: tuple[int, ...] = ()
    deltas: tuple[int, ...] = ()


def probe(n: int, period: int, timeout_ms: int) -> ProbeResult:
    word = [Int(f"a_{n}_{period}_{t}") for t in range(period)]
    solver = Solver()
    solver.set(timeout=timeout_ms)
    for t in range(period):
        carry = -word[t] + sum(word[(t + k) % period] for k in range(1, n + 1))
        solver.add(carry >= -1, carry <= 1)
    solver.add(Or(*(Or(value >= 2, value <= -2) for value in word)))
    status = solver.check()
    if str(status) == "unknown":
        return ProbeResult(n, period, "TIMEOUT")
    if status != sat:
        return ProbeResult(n, period, "UNSAT")
    model = solver.model()
    values = tuple(model.eval(value).as_long() for value in word)
    deltas = tuple(
        -values[t] + sum(values[(t + k) % period] for k in range(1, n + 1))
        for t in range(period)
    )
    return ProbeResult(n, period, "SAT", values, deltas)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--min-n", type=int, default=3)
    parser.add_argument("--max-n", type=int, default=8)
    parser.add_argument("--max-period", type=int, default=24)
    parser.add_argument(
        "--timeout-ms", type=int, default=5000,
        help="per-instance Z3 timeout (default: 5000)",
    )
    args = parser.parse_args()
    if (args.min_n < 2 or args.max_n < args.min_n or args.max_period < 1
            or args.timeout_ms < 1):
        parser.error(
            "require 2 <= min-n <= max-n, max-period >= 1, timeout-ms >= 1"
        )

    checked = 0
    for n in range(args.min_n, args.max_n + 1):
        for period in range(1, args.max_period + 1):
            result = probe(n, period, args.timeout_ms)
            checked += 1
            if result.status == "TIMEOUT":
                print(f"TIMEOUT n={n} period={period}")
                print(f"periodic carry probe: INCONCLUSIVE after {checked} checks")
                return 2
            if result.status == "SAT":
                print(
                    f"SAT n={n} period={period} word={list(result.witness)} "
                    f"delta={list(result.deltas)}"
                )
                print(f"periodic carry probe: FAIL after {checked} checks")
                return 1
        print(f"n={n}: UNSAT periods=1..{args.max_period}")
    print(
        "periodic carry probe: PASS "
        f"(no |a_t|>=2 in {checked} bounded instances)"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
