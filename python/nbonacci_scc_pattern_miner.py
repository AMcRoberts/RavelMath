#!/usr/bin/env python3
"""Batch-run the arithmetic hull across a range of n (in parallel), then mine
the resulting SCC data for closed-form patterns, with out-of-sample
validation rather than curve-fitting to every available point.

This automates the "promote a finite certificate to a symbolic formula"
step this project's methodology uses everywhere else (e.g. the Class-II
"20a+8"-style discoveries) but had not yet applied to the n-bonacci
dominance program's competitor-SCC structure
(docs/NBONACCI_ARITHMETIC_DOMINANCE_PROGRAM.md, "Graded recurrent
structure").

Pipeline:
  1. Run `nbonacci_arithmetic_hull --bound=1 <n>` for every n in range, in
     parallel, with a per-process memory cap that grows with n (state
     count grows roughly like 3^n) and a shared total budget across all
     concurrently-running jobs -- small n run many-at-once with a small
     cap each, large n run few-at-once (possibly alone) with a large cap,
     rather than one flat per-worker limit that either starves large n or
     wastes memory reserving it for small n.
  2. Parse every SCC line (nodes, edges, rho, grade-by-support) into a flat
     dataset, tagged by (n, grade, rank-within-grade).
  3. For each stable family (same grade, same rank, present at every
     sampled n), fit an exact-integer polynomial through all but the
     largest n, then check the held-out largest n against the fitted
     formula. Report MATCH or MISMATCH, never silently accept a fit that
     was not checked out-of-sample.

Grade classification matches the project's own convention: grade =
min(support) where support is the number of nonzero displacement
coordinates recorded in the SCC's smallest-support member (parsed from the
"support=k:count,..." field the hull tool already prints).
"""

from __future__ import annotations

import argparse
import re
import subprocess
from collections import defaultdict
from fractions import Fraction
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
ARITHMETIC_HULL = REPO_ROOT / "out" / "nbonacci_arithmetic_hull"

SCC_LINE = re.compile(
    r"^SCC (\d+)(\s\[CORE\])? nodes=(\d+) edges=(\d+) q=(\d+) period=(\d+) "
    r"rho~=([\d.]+).*?support=(\S+)$"
)


def _cap_child_memory(megabytes: int):
    """preexec_fn: apply a per-process RLIMIT_AS before exec, so N parallel
    workers cannot jointly exceed N * megabytes regardless of how large a
    single n's enumeration turns out to be."""
    import resource

    def _apply():
        limit = megabytes * 1024 * 1024
        resource.setrlimit(resource.RLIMIT_AS, (limit, limit))

    return _apply


def run_hull(n: int, timeout_s: float, memory_mb: int,
             cache_dir: Path | None) -> tuple[int, str, bool]:
    """Re-running `--bound=1 9` alone costs ~1.5-2 minutes; this script was
    rerun three times in one session while iterating on the fitting logic,
    recomputing the same n=3..9 outputs from scratch every time. Cache by
    n (the hull's output depends only on n and the binary's own git state,
    not on anything this script's CLI flags control), and invalidate by
    binary mtime so a rebuilt nbonacci_arithmetic_hull is never served a
    stale cached run."""
    cache_path = None
    if cache_dir is not None:
        cache_dir.mkdir(parents=True, exist_ok=True)
        binary_mtime = int(ARITHMETIC_HULL.stat().st_mtime)
        cache_path = cache_dir / f"n{n}_bound1_mtime{binary_mtime}.txt"
        if cache_path.exists():
            return n, cache_path.read_text(), True
    try:
        result = subprocess.run(
            [str(ARITHMETIC_HULL), "--bound=1", str(n)],
            cwd=REPO_ROOT, capture_output=True, text=True, timeout=timeout_s,
            preexec_fn=_cap_child_memory(memory_mb),
        )
        if cache_path is not None:
            cache_path.write_text(result.stdout)
        return n, result.stdout, True
    except subprocess.TimeoutExpired as exc:
        return n, exc.stdout or "", False


def memory_budget_mb(n: int, n_min: int, base_mb: int) -> int:
    """Grows geometrically with n: exact-window labelled-state count grows
    roughly like 3^n (see the exact-enumeration table in
    docs/NBONACCI_ARITHMETIC_DOMINANCE_PROGRAM.md), so a flat per-worker
    cap either starves large n or wastes memory on small n. Doubling per
    step is a coarse but safe proxy for that growth."""
    return base_mb * (2 ** (n - n_min))


def run_batch(n_values: list[int], timeout_s: float, total_budget_mb: int,
              base_mb: int, cache_dir: Path | None) -> dict[int, tuple[str, bool]]:
    """Bucket by memory budget rather than a flat worker count: run as many
    concurrent jobs as fit under total_budget_mb given each job's own
    (n-dependent) cap, largest n last so small/cheap n finish fast and
    free up budget for the few expensive large-n runs."""
    from concurrent.futures import ProcessPoolExecutor, as_completed

    n_min = min(n_values)
    budgets = {n: memory_budget_mb(n, n_min, base_mb) for n in n_values}
    print("Per-n memory budgets (MB): " +
          ", ".join(f"n={n}:{budgets[n]}" for n in sorted(n_values)))

    results: dict[int, tuple[str, bool]] = {}
    pending = sorted(n_values)  # cheapest first
    running: dict = {}
    with ProcessPoolExecutor(max_workers=len(n_values)) as pool:
        def used_budget() -> int:
            return sum(budgets[n] for n in running.values())

        while pending or running:
            while pending and used_budget() + budgets[pending[0]] <= total_budget_mb:
                n = pending.pop(0)
                future = pool.submit(run_hull, n, timeout_s, budgets[n], cache_dir)
                running[future] = n
            if not running:
                # single job already exceeds the whole budget; run it alone
                n = pending.pop(0)
                future = pool.submit(run_hull, n, timeout_s, budgets[n], cache_dir)
                running[future] = n
            done, _ = __import__("concurrent.futures", fromlist=["wait"]).wait(
                running.keys(), return_when="FIRST_COMPLETED")
            for future in done:
                n = running.pop(future)
                n2, output, finished = future.result()
                results[n2] = (output, finished)
                print(f"  n={n2} {'done' if finished else 'TIMEOUT'} "
                      f"({len(output)} bytes, budget was {budgets[n2]} MB)")
    return results


def parse_sccs(n: int, output: str) -> list[dict]:
    sccs = []
    for line in output.splitlines():
        match = SCC_LINE.match(line.strip())
        if not match:
            continue
        idx, is_core, nodes, edges, q, period, rho, support = match.groups()
        min_support = min(int(part.split(":")[0]) for part in support.split(","))
        # Project convention (docs/NBONACCI_ARITHMETIC_DOMINANCE_PROGRAM.md,
        # "Graded recurrent structure"): support 2/3 -> grade 1, 4/5 -> grade
        # 2, 6/7 -> grade 3, etc.
        grade = (min_support - 2) // 2 + 1
        sccs.append({
            "n": n, "index": int(idx), "is_core": bool(is_core),
            "nodes": int(nodes), "edges": int(edges), "quotient": int(q),
            "period": int(period), "rho": float(rho), "grade": grade,
        })
    return sccs


def rank_within_grade(sccs: list[dict]) -> None:
    """Assign rank 0,1,2,... within each (n, grade, period) bucket, largest
    first, so the same structural family gets the same rank across
    different n. `period` is included because it is a robust, exact,
    small-integer structural invariant -- grouping by size-rank alone
    conflates genuinely different objects (e.g. n=4's grade-2 slot holds
    only trivial period-2/4 permutation cycles, while n>=5's grade-2 holds
    genuine period-1 competitors; sorting by size alone silently merges
    these into one "family" across n, which is exactly why the first
    fitting pass produced false-looking patterns that failed holdout --
    see docs/NBONACCI_SCC_PATTERN_MINING_NEGATIVE_RESULT.md)."""
    buckets: dict[tuple[int, int, int], list[dict]] = defaultdict(list)
    for scc in sccs:
        buckets[(scc["n"], scc["grade"], scc["period"])].append(scc)
    for bucket in buckets.values():
        bucket.sort(key=lambda s: -s["nodes"])
        for rank, scc in enumerate(bucket):
            scc["rank"] = rank


def fit_polynomial_exact(points: list[tuple[int, int]]) -> list[Fraction] | None:
    """Exact Lagrange interpolation. Returns coefficients (low to high
    degree) if they come out as a low-degree exact-integer-ish polynomial,
    else None. Degree = len(points) - 1."""
    xs = [Fraction(x) for x, _ in points]
    ys = [Fraction(y) for _, y in points]
    n = len(points)
    # Newton's divided differences -> polynomial coefficients.
    coef = ys[:]
    for j in range(1, n):
        for i in range(n - 1, j - 1, -1):
            coef[i] = (coef[i] - coef[i - 1]) / (xs[i] - xs[i - j])
    # Expand Newton form into standard polynomial coefficients.
    result = [Fraction(0)] * n
    result[0] = coef[0]
    current = [Fraction(1)]
    for j in range(1, n):
        # multiply current by (x - xs[j-1])
        new_current = [Fraction(0)] * (len(current) + 1)
        for k, c in enumerate(current):
            new_current[k] += c * (-xs[j - 1])
            new_current[k + 1] += c
        current = new_current
        for k, c in enumerate(current):
            result[k] += coef[j] * c
    return result


def evaluate_polynomial(coeffs: list[Fraction], x: int) -> Fraction:
    total = Fraction(0)
    for power, c in enumerate(coeffs):
        total += c * Fraction(x) ** power
    return total


def describe_polynomial(coeffs: list[Fraction]) -> str:
    terms = []
    for power in range(len(coeffs) - 1, -1, -1):
        c = coeffs[power]
        if c == 0:
            continue
        if power == 0:
            terms.append(f"{c}")
        elif power == 1:
            terms.append(f"{c}*n")
        else:
            terms.append(f"{c}*n^{power}")
    return " + ".join(terms) if terms else "0"


def fit_rational_close(points: list[tuple[int, float]], max_denominator: int = 10000):
    """Fit an exact-rational polynomial to a *floating* rho sequence by
    rounding each value to the nearest fraction with bounded denominator,
    then reuse the exact integer machinery. This is a coarse tool: rho is
    algebraic (root of a polynomial over Q(beta)), not literally rational,
    so a "MATCH" here only ever means "matches to the printed ~12-digit
    precision," not an exact algebraic identity -- report it as such."""
    rounded = [(x, Fraction(y).limit_denominator(max_denominator)) for x, y in points]
    return fit_polynomial_exact(rounded)


def mine_families(all_sccs: list[dict], min_family_size: int) -> dict[str, str]:
    """Returns {family_label: 'MATCH'|'MISMATCH'|'insufficient'} for the
    node-count fit specifically (the primary target), so a caller can
    decide whether to keep extending n or stop."""
    families: dict[tuple[int, int, int, bool], list[dict]] = defaultdict(list)
    for scc in all_sccs:
        families[(scc["grade"], scc["period"], scc["rank"], scc["is_core"])].append(scc)

    verdicts: dict[str, str] = {}
    print("\n=== Family-by-family closed-form mining "
          "(grouped by grade+period+rank, node counts) ===")
    for (grade, period, rank, is_core), members in sorted(families.items()):
        members = sorted(members, key=lambda s: s["n"])
        label = (f"grade={grade} period={period} rank={rank}"
                  f"{' [CORE]' if is_core else ''}")
        if len(members) < min_family_size:
            continue
        n_values = [m["n"] for m in members]
        print(f"\n{label}: present at n={n_values}")

        fit_members = members[:-1]
        holdout = members[-1]
        if len(fit_members) < 2:
            print("  not enough points to fit + validate")
            verdicts[label] = "insufficient"
            continue

        points = [(m["n"], m["nodes"]) for m in fit_members]
        coeffs = fit_polynomial_exact(points)
        predicted = evaluate_polynomial(coeffs, holdout["n"])
        match = predicted == holdout["nodes"]
        print(f"  fit on n={[m['n'] for m in fit_members]}: "
              f"nodes(n) = {describe_polynomial(coeffs)}")
        print(f"  holdout n={holdout['n']}: predicted={predicted} "
              f"actual={holdout['nodes']} -> {'MATCH' if match else 'MISMATCH'}")
        verdicts[label] = "MATCH" if match else "MISMATCH"

        # quotient (coarsest equitable partition size) is often a cleaner
        # invariant than raw node count -- try it too.
        q_points = [(m["n"], m["quotient"]) for m in fit_members]
        q_coeffs = fit_polynomial_exact(q_points)
        q_predicted = evaluate_polynomial(q_coeffs, holdout["n"])
        q_match = q_predicted == holdout["quotient"]
        print(f"  fit: quotient(n) = {describe_polynomial(q_coeffs)}")
        print(f"  holdout n={holdout['n']}: predicted={q_predicted} "
              f"actual={holdout['quotient']} -> {'MATCH' if q_match else 'MISMATCH'}")

        if not is_core and period == 1:
            # rho is the quantity the dominance theorem actually needs;
            # try it too, clearly labelled as an approximate rational fit.
            rho_points = [(m["n"], m["rho"]) for m in fit_members]
            rho_coeffs = fit_rational_close(rho_points)
            rho_predicted = evaluate_polynomial(rho_coeffs, holdout["n"])
            rho_actual = Fraction(holdout["rho"]).limit_denominator(10000)
            rho_match = abs(float(rho_predicted) - holdout["rho"]) < 1e-6
            print(f"  [approximate] rho(n) ~= {describe_polynomial(rho_coeffs)}")
            print(f"  holdout n={holdout['n']}: predicted rho~={float(rho_predicted):.9f} "
                  f"actual rho~={holdout['rho']:.9f} -> "
                  f"{'MATCH(~1e-6)' if rho_match else 'MISMATCH'}")

    return verdicts


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__,
                                      formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--n-min", type=int, default=3)
    parser.add_argument("--n-max", type=int, default=9)
    parser.add_argument("--total-budget-mb", type=int, default=9000,
                         help="sum of concurrent per-process memory caps never exceeds this")
    parser.add_argument("--base-mb", type=int, default=64,
                         help="memory cap for the smallest n; doubles per step up in n")
    parser.add_argument("--timeout", type=float, default=600.0)
    parser.add_argument("--min-family-size", type=int, default=4,
                         help="minimum number of n-samples before attempting a fit")
    parser.add_argument("--cache-dir", type=Path,
                         default=REPO_ROOT / "out" / "nbonacci_hull_cache",
                         help="cache hull outputs by (n, binary mtime) here; "
                              "pass --no-cache to force recomputation")
    parser.add_argument("--no-cache", action="store_true")
    parser.add_argument("--auto-extend", action="store_true",
                         help="if every non-core family with enough points still shows "
                              "MISMATCH, push n-max up by one and rerun (reusing the "
                              "cache, so already-computed n cost nothing) until either a "
                              "MATCH appears, --n-max-cap is hit, or no family reaches "
                              "--min-family-size at the new n-max")
    parser.add_argument("--n-max-cap", type=int, default=14,
                         help="hard stop for --auto-extend regardless of match status; "
                              "state-count/runtime growth makes n>~12 impractical without "
                              "a sharper approach (see docs/NBONACCI_CONJUGATE_HEIGHT_BOUND.md)")
    args = parser.parse_args()

    if not ARITHMETIC_HULL.exists():
        print("build first: make nbonacci_arithmetic_hull")
        return 2

    cache_dir = None if args.no_cache else args.cache_dir
    n_max = args.n_max
    while True:
        n_values = list(range(args.n_min, n_max + 1))
        print(f"\n### auto-extend round: n={args.n_min}..{n_max} ###" if args.auto_extend else "")
        print(f"Running {len(n_values)} hull computations, total memory budget "
              f"{args.total_budget_mb} MB, cache={'off' if cache_dir is None else cache_dir}...")
        results = run_batch(n_values, args.timeout, args.total_budget_mb, args.base_mb, cache_dir)

        all_sccs: list[dict] = []
        for n, (output, finished) in sorted(results.items()):
            if not finished:
                print(f"n={n}: TIMEOUT, excluded from mining")
                continue
            sccs = parse_sccs(n, output)
            all_sccs.extend(sccs)
            print(f"n={n}: {len(sccs)} SCCs parsed")

        rank_within_grade(all_sccs)
        verdicts = mine_families(all_sccs, args.min_family_size)

        if not args.auto_extend:
            return 0

        non_core_checked = {label: v for label, v in verdicts.items()
                             if "[CORE]" not in label and v != "insufficient"}
        if any(v == "MATCH" for v in non_core_checked.values()):
            print(f"\nauto-extend: MATCH found at n_max={n_max}, stopping.")
            return 0
        if not non_core_checked:
            print(f"\nauto-extend: no non-core family reached "
                  f"--min-family-size={args.min_family_size} at n_max={n_max}, "
                  f"nothing left to validate against -- stopping.")
            return 0
        if n_max >= args.n_max_cap:
            print(f"\nauto-extend: hit --n-max-cap={args.n_max_cap} with only "
                  f"MISMATCH/insufficient verdicts. Not closing -- this is real negative "
                  f"evidence (see docs/NBONACCI_SCC_PATTERN_MINING_NEGATIVE_RESULT.md), "
                  f"not a tool limitation to push past blindly.")
            return 0
        print(f"\nauto-extend: all {len(non_core_checked)} checked non-core "
              f"families MISMATCH at n_max={n_max}, extending to {n_max + 1}...")
        n_max += 1


if __name__ == "__main__":
    raise SystemExit(main())
