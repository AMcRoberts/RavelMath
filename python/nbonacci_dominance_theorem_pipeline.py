#!/usr/bin/env python3
"""End-to-end pipeline for the n-bonacci arithmetic-dominance theorem.

Automates the two independent pieces that, together, close
rho(G_B) = rho(core) for a given n (see
docs/NBONACCI_ARITHMETIC_DOMINANCE_THEOREM_N3_N7.md for the derivation):

  1. Carry-bound closure: compute the period-independent bound B(n) via the
     conjugate-height argument (nbonacci_conjugate_height_bound.py's
     mechanism), then run nbonacci_carry_cycle_probe at a box exceeding
     B(n) to confirm zero non-ternary cyclic states. This proves no
     periodic admissible orbit -- hence no cyclic SCC of the arithmetic
     hull, at any coefficient bound -- escapes the ternary box.
  2. Dominance certificate: run nbonacci_arithmetic_hull --exact --bound=1
     to confirm the formula-defined core exactly dominates every other
     cyclic SCC found within the ternary box, via exact rational
     Collatz-Wielandt brackets.

For a given n, piece 1 alone gives the weak (all-n) carry bound
unconditionally; piece 2 is checked per n. n is reported CLOSED only when
both pieces pass for that n. Each external call runs under a wall-clock
timeout so a call that would time out (as the direct corona builder,
nbonacci_dominance_ledger, does past n=6) is reported INCONCLUSIVE rather
than silently hanging the pipeline -- this is a diagnostic tool, not a
substitute for careful reading of what each timeout means.

This is exactly the "run the existing certificates across a range and look
for where the pattern holds or breaks" step this project's methodology
uses to promote per-n finite checks toward a symbolic, uniform-in-n
argument -- see docs/NBONACCI_ARITHMETIC_DOMINANCE_PROGRAM.md's repeated
"exact finite certificate at sampled a" -> "closed-form for every a>=k"
pattern elsewhere in this project. It does not, by itself, produce that
symbolic promotion; it produces the data (corona sizes, SCC counts, gap
sizes) a future session needs to attempt one.
"""

from __future__ import annotations

import argparse
import re
import subprocess
import sys
import time
from pathlib import Path

import mpmath as mp

REPO_ROOT = Path(__file__).resolve().parent.parent
CARRY_CYCLE_PROBE = REPO_ROOT / "out" / "nbonacci_carry_cycle_probe"
ARITHMETIC_HULL = REPO_ROOT / "out" / "nbonacci_arithmetic_hull"

CARRY_CYCLE_STATE_CAP = 100_000_000  # matches the probe's own internal cap


def conjugate_height_bound(n: int, precision: int = 60) -> float:
    """B(n): period-independent bound on |a_t|, via the max-attained
    argument applied to every root of the n-bonacci characteristic
    polynomial (dominant root forward, conjugates backward)."""
    mp.mp.dps = precision
    coeffs = [1] + [-1] * n
    roots = mp.polyroots(coeffs, maxsteps=200, extraprec=10 * precision)
    dominant = max(roots, key=lambda r: r.real)
    others = [r for r in roots if abs(r - dominant) > mp.mpf(10) ** (-(precision // 2))]
    ordered = [dominant] + others

    matrix = mp.matrix(n, n)
    for k, beta_k in enumerate(ordered):
        running = mp.mpc(0)
        for j in range(n):
            matrix[k, j] = beta_k**j - running
            running += beta_k**j
    bounds = [1 / (dominant.real - 1)] + [1 / (1 - abs(b)) for b in others]
    e0 = mp.matrix(n, 1)
    e0[0, 0] = 1
    lam = mp.lu_solve(matrix.T, e0)
    return float(sum(abs(lam[k, 0]) * bounds[k] for k in range(n)).real)


def run(cmd: list[str], timeout_s: float) -> tuple[str, bool]:
    try:
        result = subprocess.run(
            cmd, cwd=REPO_ROOT, capture_output=True, text=True, timeout=timeout_s
        )
        return result.stdout + result.stderr, True
    except subprocess.TimeoutExpired as exc:
        partial = (exc.stdout or "") + (exc.stderr or "")
        return partial, False


def check_carry_bound(n: int, timeout_s: float) -> dict:
    bound = conjugate_height_bound(n)
    box = int(bound) + 1  # smallest integer strictly exceeding B(n)
    states = (2 * box + 1) ** n
    if states > CARRY_CYCLE_STATE_CAP:
        return {
            "status": "INCONCLUSIVE",
            "reason": f"box radius {box} needs {states:,} states, "
            f"over the {CARRY_CYCLE_STATE_CAP:,} probe cap",
            "bound": bound,
            "box": box,
        }
    output, finished = run(
        [str(CARRY_CYCLE_PROBE), f"--n={n}", f"--bound={box}"], timeout_s
    )
    if not finished:
        return {"status": "INCONCLUSIVE", "reason": "timeout", "bound": bound, "box": box}
    match = re.search(r"nonternary_cyclic=(\d+)", output)
    if not match:
        return {"status": "INCONCLUSIVE", "reason": "no match in output",
                 "bound": bound, "box": box}
    nonternary = int(match.group(1))
    return {
        "status": "PASS" if nonternary == 0 else "FAIL",
        "bound": bound,
        "box": box,
        "nonternary_cyclic": nonternary,
        "raw": output.strip(),
    }


def check_dominance(n: int, timeout_s: float) -> dict:
    output, finished = run(
        [str(ARITHMETIC_HULL), "--exact", "--bound=1", str(n)], timeout_s
    )
    if not finished:
        return {"status": "INCONCLUSIVE", "reason": "timeout (hull enumeration too large)"}
    scc_match = re.search(r"cyclic_SCCs=(\d+)\s+generated_core=(\d+)\s+missing=(\d+)\s+core_SCC=(\w+)", output)
    verdict_match = re.search(
        r"arithmetic-supergraph dominance: estimate=(\w+) exact=(\S+)", output
    )
    scc_lines = re.findall(
        r"^SCC (\d+)(\s\[CORE\])? nodes=(\d+) edges=(\d+) q=(\d+) period=(\d+) rho~=([\d.]+)",
        output, re.MULTILINE,
    )
    if not scc_match or not verdict_match:
        return {"status": "INCONCLUSIVE", "reason": "no match in output", "raw": output.strip()}
    cyclic_sccs, generated_core, missing, core_scc = scc_match.groups()
    estimate, exact = verdict_match.groups()
    status = "PASS" if (core_scc == "EXACT" and exact == "PASS") else "FAIL"
    return {
        "status": status,
        "cyclic_SCCs": int(cyclic_sccs),
        "core_size": int(generated_core),
        "core_missing": int(missing),
        "exact_verdict": exact,
        "sccs": [
            {"index": int(idx), "is_core": bool(core_flag), "nodes": int(nodes),
             "edges": int(edges), "quotient": int(q), "period": int(period), "rho": float(rho)}
            for idx, core_flag, nodes, edges, q, period, rho in scc_lines
        ],
    }


def predicted_core_size(n: int) -> int:
    return (n - 1) * (5 * n * n - 10 * n + 6) // 3


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__,
                                      formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--n-min", type=int, default=3)
    parser.add_argument("--n-max", type=int, default=10)
    parser.add_argument("--carry-timeout", type=float, default=120.0)
    parser.add_argument("--dominance-timeout", type=float, default=120.0)
    args = parser.parse_args()

    if not CARRY_CYCLE_PROBE.exists() or not ARITHMETIC_HULL.exists():
        print("build both binaries first: make nbonacci_carry_cycle_probe nbonacci_arithmetic_hull",
              file=sys.stderr)
        return 2

    rows = []
    for n in range(args.n_min, args.n_max + 1):
        start = time.time()
        carry = check_carry_bound(n, args.carry_timeout)
        dominance = check_dominance(n, args.dominance_timeout) if n >= 3 else \
            {"status": "N/A", "reason": "arithmetic_hull requires n>=3"}
        elapsed = time.time() - start
        closed = carry["status"] == "PASS" and dominance["status"] == "PASS"

        core_ok = ""
        if dominance.get("sccs"):
            core = next((s for s in dominance["sccs"] if s["is_core"]), None)
            if core:
                predicted = predicted_core_size(n)
                core_ok = "match" if core["nodes"] == predicted else \
                    f"MISMATCH(got {core['nodes']}, predicted {predicted})"

        rows.append((n, carry, dominance, closed, elapsed, core_ok))
        print(
            f"n={n:2d} carry={carry['status']:12s} "
            f"(B(n)={carry.get('bound', float('nan')):.3f}, box={carry.get('box','-')}) "
            f"dominance={dominance['status']:12s} "
            f"(cyclic_SCCs={dominance.get('cyclic_SCCs','-')}, core_size_check={core_ok or '-'}) "
            f"THEOREM={'CLOSED' if closed else 'open'} [{elapsed:.1f}s]"
        )

    print()
    closed_ns = [n for n, _, _, closed, _, _ in rows if closed]
    print(f"n-bonacci arithmetic-dominance theorem CLOSED for: "
          f"{','.join(map(str, closed_ns)) if closed_ns else '(none in range)'}")

    # Pattern-mining hook: dump competitor SCC data for symbolic promotion.
    print("\nCompetitor SCC data (for closed-form fitting across n):")
    for n, _, dominance, _, _, _ in rows:
        for scc in dominance.get("sccs", []):
            if not scc["is_core"]:
                print(f"  n={n} SCC{scc['index']} nodes={scc['nodes']} "
                      f"edges={scc['edges']} period={scc['period']} rho~={scc['rho']:.9f}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
