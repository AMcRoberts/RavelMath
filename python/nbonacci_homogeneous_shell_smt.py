#!/usr/bin/env python3
"""Exact SMT probe for the homogeneous (q=0) shell automaton.

After scaling a shell by M, the digit term is q=1/M.  At q=0 the update is
the exact shift/tail map below.  This probe asks whether every window stays in
[-1,1]^n and touches the boundary.  It is deliberately independent of the
MILP implementation and uses rational linear arithmetic with a disjunction
for the boundary hit.
"""

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


def check(n: int, transitions: int):
    state = [z3.Real(f"x_{i}") for i in range(n)]
    solver = z3.Solver()
    current = state
    for _ in range(transitions + 1):
        solver.add(*(value >= -1 for value in current))
        solver.add(*(value <= 1 for value in current))
        solver.add(z3.Or(*(value == 1 for value in current),
                         *(value == -1 for value in current)))
        tail = current[0] - sum(current[1:], z3.RealVal(0))
        current = current[1:] + [tail]
    status = solver.check()
    return status, solver.model() if status == z3.sat else None, solver


def main() -> int:
    install_memory_limit()
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--n", type=int, required=True)
    parser.add_argument("--max-n", action="store_true",
                        help="also scan 2..n (default: only --n)")
    parser.add_argument("--emit-proof",
                        help="write the Z3 proof for the n+2 UNSAT query")
    args = parser.parse_args()
    dimensions = range(2, args.n + 1) if args.max_n else (args.n,)
    if args.n < 2:
        parser.error("require n>=2")
    if args.emit_proof:
        # Proof production is a context-wide Z3 option and must be enabled
        # before the first solver is constructed.
        z3.set_param("proof", True)
    for n in dimensions:
        short_status, model, _ = check(n, n + 1)
        long_status, _, long_solver = check(n, n + 2)
        short = str(short_status)
        long = str(long_status)
        witness = ""
        if model is not None:
            witness = " witness=" + ",".join(str(model.eval(z3.Real(f"x_{i}")))
                                               for i in range(n))
        print(f"homogeneous-shell: n={n} transitions={n+1} status={short}"
              f"; transitions={n+2} status={long}{witness}")
        if args.emit_proof and n == args.n and long_status == z3.unsat:
            # Rerun this one query under the proof-enabled context.
            _, _, proof_solver = check(n, n + 2)
            with open(args.emit_proof, "w", encoding="utf-8") as stream:
                stream.write(str(proof_solver.proof()))
                stream.write("\n")
            print(f"  proof={args.emit_proof}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
