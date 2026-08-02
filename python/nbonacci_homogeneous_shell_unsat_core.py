#!/usr/bin/env python3
"""Extract and report the MINIMAL Z3 unsat core for the homogeneous-shell
survival-depth query (see nbonacci_homogeneous_shell_smt.py and
docs/NBONACCI_CODE_MECHANISM.md, "the homogeneous boundary automaton has
survival depth exactly n+1").

The full n+2-transition infeasibility query has O(n^2) box constraints and
n+3 shell disjunctions. Z3's minimal unsat core is dramatically smaller and
reveals real structure: across every tested n=2..6, the core only ever
needs (a) the two-sided bound on a_0, (b) a two-sided bound on a run of
LATER, CONSECUTIVE scalar values of the sequence (not spread across the
whole window machinery -- "coordinate n-1 at step t" is literally a_{t+n-1},
so ranging t over consecutive steps ranges the underlying index
consecutively too), and (c) exactly one or two shell (boundary-touch)
disjunctions, not one per window. This narrows the actual proof obligation
from "reason about n+3 whole windows" to "reason about a_0 and one later
run of consecutive scalar values" -- a real simplification, not yet turned
into the general-n closed-form survival-depth proof itself.

This is a discovery/diagnostic tool: it reports what Z3 found, which is
not guaranteed to be *the* minimal core (multiple minimal cores can exist;
Z3's choice depends on internal heuristics) or canonical across n. Treat
patterns in the printed core structure as a lead, not a theorem.
"""

from __future__ import annotations

import argparse
import os
import re
import resource

import z3


def install_memory_limit() -> None:
    megabytes = int(os.environ.get("RAVEL_PROBE_MEMORY_MB", "10240"))
    if megabytes > 0:
        limit = megabytes * 1024 * 1024
        resource.setrlimit(resource.RLIMIT_AS, (limit, limit))


def extract_core(n: int, transitions: int):
    state = [z3.Real(f"x_{i}") for i in range(n)]
    solver = z3.Solver()
    current = state
    for step in range(transitions + 1):
        for idx, value in enumerate(current):
            solver.assert_and_track(value >= -1, f"ge_{step}_{idx}")
            solver.assert_and_track(value <= 1, f"le_{step}_{idx}")
        boundary_lits = []
        for idx, value in enumerate(current):
            eq1 = z3.Bool(f"eq1_{step}_{idx}")
            eqm1 = z3.Bool(f"eqm1_{step}_{idx}")
            solver.add(eq1 == (value == 1))
            solver.add(eqm1 == (value == -1))
            boundary_lits.append(eq1)
            boundary_lits.append(eqm1)
        solver.assert_and_track(z3.Or(*boundary_lits), f"shell_{step}")
        tail = current[0] - sum(current[1:], z3.RealVal(0))
        current = current[1:] + [tail]
    status = solver.check()
    if status != z3.unsat:
        return status, None
    return status, solver.unsat_core()


def describe_core(n: int, core) -> str:
    box_bounds: dict[int, dict[str, bool]] = {}
    shell_steps = []
    for tag in core:
        name = str(tag)
        match = re.match(r"(ge|le)_(\d+)_(\d+)", name)
        if match:
            kind, step, coord = match.group(1), int(match.group(2)), int(match.group(3))
            actual_index = step + coord  # window coordinate `coord` at step `step` is a_{step+coord}
            box_bounds.setdefault(actual_index, {})[kind] = True
            continue
        match = re.match(r"shell_(\d+)", name)
        if match:
            shell_steps.append(int(match.group(1)))
    indices = sorted(box_bounds)
    lines = [f"  scalar indices bounded: {indices}"]
    for idx in indices:
        sides = box_bounds[idx]
        two_sided = "ge" in sides and "le" in sides
        lines.append(f"    a_{idx}: {'both bounds' if two_sided else next(iter(sides))+' bound only'}")
    lines.append(f"  shell disjunctions used at steps: {sorted(shell_steps)}")
    lines.append(f"  core size: {len(core)} (out of "
                  f"{2*n*(len(indices) and max(indices)+1 or 0)}+ possible box constraints)")
    return "\n".join(lines)


def main() -> int:
    install_memory_limit()
    parser = argparse.ArgumentParser(description=__doc__,
                                      formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--n-min", type=int, default=2)
    parser.add_argument("--n-max", type=int, default=6)
    args = parser.parse_args()

    for n in range(args.n_min, args.n_max + 1):
        status, core = extract_core(n, n + 2)
        print(f"n={n} transitions={n+2}: {status}")
        if core is not None:
            print(describe_core(n, core))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
