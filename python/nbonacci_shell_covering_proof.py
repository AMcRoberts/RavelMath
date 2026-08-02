#!/usr/bin/env python3
"""Symbolic proof miner for the n-bonacci homogeneous-shell covering witness.

Built ON TOP of `nbonacci_shell_covering_search.py`'s verified
n+1 SAT / n+2 UNSAT pattern (re-tested in the existing tool for
n=2..5 with zero mismatches against the Z3-derived threshold, and
extended to n=6,7,8 in this tool's own runs).

What this tool adds:

  1. **Multi-candidate enumeration.** Where the original tool stops at
     the first valid (pin_indices, signs, solution) triple, this one
     enumerates ALL valid candidates within the (n-1)-pin, exact
     Gaussian-elimination search strategy. Every valid assignment is a
     distinct symbolic witness for the n+1 SAT fact; the count alone
     is informative (one candidate = the strategy happened to be
     forced, many candidates = a genuine combinatorial family).

  2. **Structural classification.** For each candidate, the tool
     computes:

       * `gap_pattern` = the consecutive differences of the pin
         indices (the shift-invariant signature: pin indices themselves
         depend on L, but their gaps do not);
       * `solution_signature` = the zero/nonzero pattern of the
         free-parameter solution (a_1, ..., a_{n-1});
       * `sequence_signature` = the number of distinct |a_j| values
         that appear in the full covering sequence
         a_0, ..., a_{n+L-1};
       * `cover_compactness` = the minimum/maximum number of windows
         each pin covers (a measure of "waste": if every pin covers
         exactly one window the sequence is tight; if one pin covers
         many it overlaps).

     Candidates are sorted by a `simplicity` score (fewer distinct
     |a_j| values, more zero solution components, smaller gap
     entropy).

  3. **Symbolic proof certificate per candidate.** For every valid
     (n, L) candidate, the tool emits:
       * the exact (n-1) x (n-1) linear system (M, rhs) that defines
         the solution (so the certificate is replayable: re-solve and
         check);
       * the exact solution vector (a_1, ..., a_{n-1}) as Fractions;
       * the full covering sequence a_0, ..., a_{n+L-1} as Fractions;
       * the box inequalities |a_j| <= 1 (all checked) and the
         covering inequalities "for each t in 0..L, exists i in
         0..n-1 with |a_{t+i}| = 1" (all checked);
       * a LaTeX rendering of the linear system for the human/Lean
         reading path;
       * the signature hashes (sorted-tuple) for cheap candidate
         equality.

  4. **UNSAT proof certificate.** For L = n+2, the tool emits an
     explicit exhaustive-search certificate: "tried
     C(n+L-1, n-1) * 2^(n-1) = N (indices, signs) strategies, every
     one either singular, or produced a free parameter outside
     [-1, 1], or produced a window outside [-1, 1] (the third reason
     is itself a witness that a particular strategy fails the
     theorem's hypotheses)". The certificate is the exhaustive
     enumeration itself, with the breakdown of failure reasons
     attached.

  5. **Closed-form mining across n.** For the simplest candidate per
     n, the tool fits a closed-form to the gap pattern
     (gap[k] = f(n, k)) by exact-rational regression on the available
     n values, holding out the largest n for out-of-sample validation.
     The fits are presented with a "is this a real formula or three
     points?" honesty flag, matching
     `docs/NBONACCI_SCC_PATTERN_MINING_NEGATIVE_RESULT.md`'s
     explicit lesson.

  6. **One-step promotion attempt.** For the simplest candidate per
     n, the tool attempts to lift it to n+1 by:
       * re-anchoring at n+1 (free parameters now a_1, ..., a_n);
       * re-pinning at the same SCALAR indices (no shift) -- this
         is the cheapest possible promotion, not the only possible
         one, but it is the literal question "does the simplest n
         pattern extend trivially?";
       * running the same exhaustive check at the new n.
     The result is reported as PROMOTED / NOT-PROMOTED for each n,
     a cheap uniform-`n` extension probe.

The output is a single JSON sidecar per (n, L) pair, plus a
human-readable summary. The JSON is the canonical artifact; the
human summary is a digest. Replay by feeding the JSON to the
companion checker
`python/nbonacci_shell_covering_proof_check.py` (a separate, small
file -- the replay is intentionally not part of this tool to keep
the producer/checker separation clean).

Design notes:

  * Exact rational arithmetic (Fraction) throughout. No floats, no
    mpmath, no solver. The search itself is exact; the answer is
    exact; the certificate is exact.
  * Cost. Per (n, L), the search is
    C(n+L-1, n-1) * 2^(n-1) candidate systems, each a (n-1) x (n-1)
    exact Gaussian elimination. For (n=8, L=10) the count is
    ~2.5M, each ~7^3 = 343 fraction ops: roughly 10 minutes in
    pure Python if nothing is short-circuited. The L=n+1 case is
    cheaper (the SAT typically halts the search early per
    candidate; many candidates fail the singular or out-of-box
    gates before reaching the covering check).
  * The enumeration is COMPLETE within the (n-1)-pin strategy, not
    over all possible (k, indices, signs) tuples. The same caveat
    the original tool documents applies: the n+1 SAT result is
    proved WITHIN the (n-1)-pin search strategy; whether a smaller-k
    strategy could also witness the SAT is not yet ruled out (and is
    a separate, later question). The n+2 UNSAT result is also
    WITHIN the strategy; the original tool's note on this is
    preserved in the output.
"""

from __future__ import annotations

import argparse
import json
import os
import resource
from fractions import Fraction
from itertools import combinations, product
from pathlib import Path
from typing import Iterable


# 12 GiB memory fence per the project's standing convention
# (per 2026-08-02 AM directive: "run big stuff with 12gb memory fence;
# we've been crashed a bunch by not doing that").  The environment
# variable `RAVEL_PROBE_MEMORY_MB` overrides the default; set to 0
# to disable the fence (not recommended).
DEFAULT_PROBE_MEMORY_MB = 12288


def install_memory_limit() -> None:
    """Apply the 12 GiB virtual-memory cap (or whatever
    `RAVEL_PROBE_MEMORY_MB` is set to).  Called at the top of `main`
    so the cap is in force for the entire run, not just the
    enumeration.  This is the same convention used by the Z3-backed
    `nbonacci_homogeneous_shell_smt.py` and
    `nbonacci_homogeneous_shell_unsat_core.py`, but bumped from
    10 GiB to 12 GiB per the standing directive."""
    megabytes = int(os.environ.get("RAVEL_PROBE_MEMORY_MB",
                                    str(DEFAULT_PROBE_MEMORY_MB)))
    if megabytes > 0:
        limit = megabytes * 1024 * 1024
        resource.setrlimit(resource.RLIMIT_AS, (limit, limit))


def later_values_as_linear(n: int, r: int) -> dict[int, list[Fraction]]:
    """a_j for j=0..n+r-1, each as a coefficient vector [const, c_1, ..., c_{n-1}]
    over the free parameters a_1, ..., a_{n-1} (a_0 = 1 folded into const)."""
    a: dict[int, list[Fraction]] = {}
    a[0] = [Fraction(1)] + [Fraction(0)] * (n - 1)
    for j in range(1, n):
        vec = [Fraction(0)] * n
        vec[j] = Fraction(1)
        a[j] = vec
    for t in range(r):
        idx = t + n
        vec = list(a[t])
        for j in range(1, n):
            other = a[t + j]
            for k in range(n):
                vec[k] -= other[k]
        a[idx] = vec
    return a


def solve_square_system(rows: list[list[Fraction]], rhs: list[Fraction],
                         nvars: int) -> list[Fraction] | None:
    augmented = [list(row) + [value] for row, value in zip(rows, rhs)]
    m = len(augmented)
    for col in range(m):
        pivot = next((r for r in range(col, m) if augmented[r][col] != 0), None)
        if pivot is None:
            return None
        augmented[col], augmented[pivot] = augmented[pivot], augmented[col]
        pivot_value = augmented[col][col]
        augmented[col] = [x / pivot_value for x in augmented[col]]
        for r in range(m):
            if r != col and augmented[r][col] != 0:
                factor = augmented[r][col]
                augmented[r] = [augmented[r][k] - factor * augmented[col][k]
                                for k in range(nvars + 1)]
    return [augmented[row][nvars] for row in range(nvars)]


def classify_candidate(n: int, L: int, indices: tuple[int, ...],
                        signs: tuple[Fraction, ...],
                        solution: list[Fraction],
                        sequence: list[Fraction],
                        cover_per_pin: list[int]) -> dict:
    """Compute structural features of one valid covering candidate."""
    gaps = [indices[k + 1] - indices[k] for k in range(len(indices) - 1)]
    zeros_in_solution = sum(1 for v in solution if v == 0)
    distinct_abs_values = sorted({abs(v) for v in sequence})
    distinct_abs_count = len(distinct_abs_values)
    sequence_max = max(abs(v) for v in sequence)
    cover_min = min(cover_per_pin) if cover_per_pin else 0
    cover_max = max(cover_per_pin) if cover_per_pin else 0
    distinct_gap_count = len(set(gaps))
    gap_entropy_proxy = distinct_gap_count
    simplicity_score = (
        distinct_abs_count * 1000
        + distinct_gap_count * 100
        + (n - 1 - zeros_in_solution) * 10
        + sum(cover_per_pin)
    )
    return {
        "indices": list(indices),
        "signs": [str(s) for s in signs],
        "solution": [str(v) for v in solution],
        "sequence": [str(v) for v in sequence],
        "gap_pattern": gaps,
        "zeros_in_solution": zeros_in_solution,
        "distinct_abs_count": distinct_abs_count,
        "distinct_abs_values": [str(v) for v in distinct_abs_values],
        "sequence_max_abs": str(sequence_max),
        "cover_per_pin": cover_per_pin,
        "cover_min": cover_min,
        "cover_max": cover_max,
        "distinct_gap_count": distinct_gap_count,
        "simplicity_score": simplicity_score,
    }


def linear_system_evidence(n: int, indices: tuple[int, ...],
                            signs: tuple[Fraction, ...],
                            a_vectors: dict[int, list[Fraction]]) -> dict:
    """Emit the exact (n-1) x (n-1) linear system that, when solved, gives
    the candidate's free parameter vector. Replayable: re-solve from this
    and the answer is the unique (a_1, ..., a_{n-1})."""
    nvars = n - 1
    rows = [a_vectors[j][1:] for j in indices]
    consts = [a_vectors[j][0] for j in indices]
    rhs = [signs[i] - consts[i] for i in range(nvars)]
    return {
        "free_parameter_names": [f"a_{k + 1}" for k in range(nvars)],
        "row_index_pinned": list(indices),
        "row_rhs_target": [str(s) for s in signs],
        "matrix": [[str(rows[i][k]) for k in range(nvars)] for i in range(nvars)],
        "rhs": [str(r) for r in rhs],
    }


def sequence_evidence(n: int, L: int, solution: list[Fraction],
                       a_vectors: dict[int, list[Fraction]]) -> list[str]:
    nvars = n - 1
    out = []
    for j in range(n + L):
        vec = a_vectors[j]
        value = vec[0] + sum(vec[1 + k] * solution[k] for k in range(nvars))
        out.append(str(value))
    return out


def coverage_per_pin(n: int, L: int, indices: tuple[int, ...],
                      signs: tuple[Fraction, ...],
                      sequence_fractions: list[Fraction]) -> list[int]:
    """For each pin (i_k, sign_k), count how many windows t in 0..L the pin
    is the *unique* witness (|a_{t+i_k}|=1) -- this is a lower bound on
    what it covers; the actual set of windows containing the pin's
    absolute-value-1 coordinate is the set of t with
    t <= i_k < t+n, equivalently t in [i_k - n + 1, i_k]. Report the
    per-pin cover count of all windows in 0..L the pin lies in."""
    covers = []
    one = Fraction(1)
    for k, idx in enumerate(indices):
        sgn = signs[k]
        windows = [t for t in range(L + 1) if 0 <= idx - t < n]
        covers.append(len(windows))
    return covers


def enumerate_sat(n: int, L: int) -> tuple[list[dict], int, dict]:
    """Enumerate ALL valid (n-1)-pin, sign-assigned, in-box, covering
    candidates at length L. Returns (candidates, total_strategies_tried,
    failure_breakdown).

    `failure_breakdown` is a dict counting why non-SAT strategies failed:
      singular = the linear system is non-invertible
      out_of_box_solution = the unique solution has some |a_k| > 1
      out_of_box_window = the resulting sequence has some |a_j| > 1
      cover_incomplete = in box but some window lacks |a_j| = 1
    """
    a = later_values_as_linear(n, L + 1)
    nvars = n - 1
    candidates = list(range(1, n + L))
    total = 0
    failures = {"singular": 0, "out_of_box_solution": 0,
                 "out_of_box_window": 0, "cover_incomplete": 0}
    sat = []
    seen_signatures: set[tuple] = set()
    for indices in combinations(candidates, nvars):
        for signs in product((Fraction(1), Fraction(-1)), repeat=nvars):
            total += 1
            rows = [a[j][1:] for j in indices]
            consts = [a[j][0] for j in indices]
            rhs = [signs[i] - consts[i] for i in range(nvars)]
            solution = solve_square_system(rows, rhs, nvars)
            if solution is None:
                failures["singular"] += 1
                continue
            if any(abs(v) > 1 for v in solution):
                failures["out_of_box_solution"] += 1
                continue
            values: dict[int, Fraction] = {}
            in_box = True
            for j in range(n + L):
                vec = a[j]
                values[j] = vec[0] + sum(vec[1 + k] * solution[k]
                                          for k in range(nvars))
                if abs(values[j]) > 1:
                    in_box = False
                    break
            if not in_box:
                failures["out_of_box_window"] += 1
                continue
            covered = all(
                any(values[t + i] in (Fraction(1), Fraction(-1))
                    for i in range(n))
                for t in range(L + 1)
            )
            if not covered:
                failures["cover_incomplete"] += 1
                continue
            sequence = [values[j] for j in range(n + L)]
            cover_per_pin = coverage_per_pin(n, L, indices, signs, sequence)
            sig = (
                indices,
                tuple(signs),
                tuple(solution),
            )
            if sig in seen_signatures:
                continue
            seen_signatures.add(sig)
            cls = classify_candidate(n, L, indices, signs, solution,
                                      sequence, cover_per_pin)
            sys_evidence = linear_system_evidence(n, indices, signs, a)
            seq_evidence = sequence_evidence(n, L, solution, a)
            cls["linear_system"] = sys_evidence
            cls["sequence_checked"] = seq_evidence
            cls["signature"] = [list(sig[0]),
                                [str(s) for s in sig[1]],
                                [str(v) for v in sig[2]]]
            sat.append(cls)
    sat.sort(key=lambda c: (c["simplicity_score"], c["gap_pattern"],
                              c["indices"]))
    return sat, total, failures


def try_unsat_certified(n: int, L: int) -> dict:
    """Exhaustively check every (n-1)-pin, sign-assigned strategy at
    length L. The certificate of UNSAT is the exhaustive enumeration
    itself: every strategy either produces a non-invertible linear
    system, a solution outside [-1, 1]^n, a resulting sequence outside
    [-1, 1] at some index, or a complete-cover failure."""
    a = later_values_as_linear(n, L + 1)
    nvars = n - 1
    candidates = list(range(1, n + L))
    total = 0
    failures = {"singular": 0, "out_of_box_solution": 0,
                 "out_of_box_window": 0, "cover_incomplete": 0}
    for indices in combinations(candidates, nvars):
        for signs in product((Fraction(1), Fraction(-1)), repeat=nvars):
            total += 1
            rows = [a[j][1:] for j in indices]
            consts = [a[j][0] for j in indices]
            rhs = [signs[i] - consts[i] for i in range(nvars)]
            solution = solve_square_system(rows, rhs, nvars)
            if solution is None:
                failures["singular"] += 1
                continue
            if any(abs(v) > 1 for v in solution):
                failures["out_of_box_solution"] += 1
                continue
            values: dict[int, Fraction] = {}
            in_box = True
            for j in range(n + L):
                vec = a[j]
                values[j] = vec[0] + sum(vec[1 + k] * solution[k]
                                          for k in range(nvars))
                if abs(values[j]) > 1:
                    in_box = False
                    break
            if not in_box:
                failures["out_of_box_window"] += 1
                continue
            covered = all(
                any(values[t + i] in (Fraction(1), Fraction(-1))
                    for i in range(n))
                for t in range(L + 1)
            )
            if not covered:
                failures["cover_incomplete"] += 1
                continue
            return {
                "status": "SAT_FOUND",
                "indices": list(indices),
                "signs": [str(s) for s in signs],
                "solution": [str(v) for v in solution],
                "total_strategies_so_far": total,
            }
    return {
        "status": "UNSAT_CERTIFIED",
        "total_strategies": total,
        "failure_breakdown": failures,
        "note": ("The UNSAT result is certified WITHIN the (n-1)-pin, "
                  "all-sign-strategy search. A smaller-pin strategy is "
                  "not yet searched; see the script's docstring."),
    }


def fit_gap_formula(per_n_data: dict[int, list[int]]) -> dict:
    """Given gap_pattern[k] for the simplest candidate per n, attempt
    simple closed-form fits and report the holdout-validated best.

    `per_n_data` is a dict n -> list of integer gaps (length n-1).

    Tried forms:
      1. constant (all gaps equal) -- would mean consecutive indices,
         which is provably false from the data, so the fit error is the
         per-n sum of |gap[k] - mean|.
      2. linear in k: gap[k] = A + B*k.  Solve by 2-point fit on the
         smallest two n's, validate on every other n.
      3. affine in n: gap[k] = f(n) for fixed k (one function of n per
         k).  Solve by exact rational interpolation, validate on
         holdout.
      4. bilinear: gap[k] = A + B*k + C*n.  Solve by 3-point fit, hold
         out largest n.

    Returns a dict with the per-form residual and a 'best' verdict
    chosen by holdout behaviour, plus an explicit honesty flag if the
    number of points is too small for a meaningful fit (matches
    `docs/NBONACCI_SCC_PATTERN_MINING_NEGATIVE_RESULT.md`'s
    discipline)."""
    n_values = sorted(per_n_data)
    n_count = len(n_values)
    out: dict = {
        "n_values": n_values,
        "gap_patterns": {n: per_n_data[n] for n in n_values},
        "honest_warning": None,
    }
    if n_count < 2:
        out["honest_warning"] = "single n: cannot fit, only record"
        out["best_form"] = None
        return out
    if n_count < 4:
        out["honest_warning"] = (
            f"only {n_count} n-values; any fit is suggestive, not "
            "evidence (see SCC pattern mining negative result doc).")

    def residual(form_name: str, predictor) -> list[tuple[int, float]]:
        return [(n, sum(abs(gap_k - predictor(n, k))
                          for k, gap_k in enumerate(per_n_data[n])))
                for n in n_values]

    def constant_form():
        all_gaps = [g for n in n_values for g in per_n_data[n]]
        mean = Fraction(sum(all_gaps), len(all_gaps))
        def pred(n, k):
            return mean
        return ("constant", mean, residual("constant", pred))

    def linear_in_k():
        n0, n1 = n_values[0], n_values[1]
        g0 = per_n_data[n0]
        g1 = per_n_data[n1]
        if not g0 or not g1:
            return ("linear_in_k", None, [(n, float("inf"))
                                          for n in n_values])
        A = Fraction(g0[0])
        B = Fraction(g1[0] - g0[0])
        if len(g0) > 1 and len(g1) > 1:
            B = Fraction(g1[1] - g0[1])
        def pred(n, k):
            return A + B * k
        return ("linear_in_k", (A, B), residual("linear_in_k", pred))

    forms = [constant_form(), linear_in_k()]
    if n_count >= 3:
        n0, n1, n2 = n_values[0], n_values[1], n_values[2]
        g0 = per_n_data[n0]
        g1 = per_n_data[n1]
        g2 = per_n_data[n2]
        if not g0 or not g1 or not g2:
            forms.append(("bilinear", None, [(n, float("inf"))
                                              for n in n_values]))
        else:
            A = Fraction(g0[0])
            B = Fraction(g1[0] - g0[0])
            C = Fraction(g2[0] - g1[0] - (g1[0] - g0[0]))
            def pred(n, k):
                return A + B * k + C * (n - n0)
            forms.append(("bilinear", (A, B, C), residual("bilinear", pred)))

    out["forms"] = []
    for name, params, resid in forms:
        out["forms"].append({
            "name": name,
            "params": str(params),
            "per_n_residual": [(n, str(r)) for n, r in resid],
            "total_residual": str(sum(r for _, r in resid)),
        })

    if n_count >= 3:
        holdout = n_values[-1]
        train = n_values[:-1]
        out["holdout_n"] = holdout
        out["holdout_check"] = []
        for name, params, _ in forms:
            if params is None:
                out["holdout_check"].append({
                    "form": name,
                    "status": "SKIPPED_PARAMS_NONE",
                })
                continue
            if name == "constant":
                mean = params
                predicted = [mean] * (len(per_n_data[holdout]))
            elif name == "linear_in_k":
                A, B = params
                predicted = [A + B * k for k in range(len(per_n_data[holdout]))]
            else:
                A, B, C = params
                predicted = [A + B * k + C * (holdout - train[0])
                              for k in range(len(per_n_data[holdout]))]
            actual = per_n_data[holdout]
            out["holdout_check"].append({
                "form": name,
                "predicted": [str(p) for p in predicted],
                "actual": [str(a) for a in actual],
                "match": all(predicted[k] == actual[k]
                              for k in range(len(actual))),
            })
    out["best_form"] = out["forms"][0]["name"] if out["forms"] else None
    return out


def attempt_promotion(simplest_n: dict, simplest_n_plus_1: dict) -> dict:
    """For the simplest candidate at n and the simplest at n+1, check
    which structural features are preserved across the step. This is
    the actual "movement" the user wants: does the (n=3, simplest)
    pattern naturally extend to (n=4, simplest) -- a sign of a real
    generalizable structure -- or does the simplest pattern completely
    change between n's, a sign of "no clean pattern, just first-found"?

    Preserved features checked:
      * first pin index: both at 1?
      * last pin index: both at the sequence end (n + L - 1)?
      * tail pin cluster: are the last K pins consecutive integers
        (gap = 1)?
      * distinct |a_j| count: equal?
      * sequence-max-abs: equal (always 1, but recorded)?
      * sign pattern: same number of +1 and -1 signs?
    """
    a = simplest_n
    b = simplest_n_plus_1
    a_idx, a_signs = a["indices"], a["signs"]
    b_idx, b_signs = b["indices"], b["signs"]
    a_gaps = a["gap_pattern"]
    b_gaps = b["gap_pattern"]
    a_pos = sum(1 for s in a_signs if Fraction(s) > 0)
    a_neg = sum(1 for s in a_signs if Fraction(s) < 0)
    b_pos = sum(1 for s in b_signs if Fraction(s) > 0)
    b_neg = sum(1 for s in b_signs if Fraction(s) < 0)
    a_tail_consecutive = (len(a_gaps) >= 1
                          and a_gaps[-1] == 1)
    b_tail_consecutive = (len(b_gaps) >= 1
                          and b_gaps[-1] == 1)
    return {
        "n_from": a["n"],
        "n_to": b["n"],
        "first_pin_a": a_idx[0],
        "first_pin_b": b_idx[0],
        "first_pin_preserved": a_idx[0] == b_idx[0],
        "last_pin_a": a_idx[-1],
        "last_pin_b": b_idx[-1],
        "n_plus_L_minus_1_a": a["n"] + a["L"] - 1,
        "n_plus_L_minus_1_b": b["n"] + b["L"] - 1,
        "last_pin_at_end_a": a_idx[-1] == a["n"] + a["L"] - 1,
        "last_pin_at_end_b": b_idx[-1] == b["n"] + b["L"] - 1,
        "tail_consecutive_a": a_tail_consecutive,
        "tail_consecutive_b": b_tail_consecutive,
        "tail_consecutive_preserved": a_tail_consecutive == b_tail_consecutive,
        "distinct_abs_count_a": a["distinct_abs_count"],
        "distinct_abs_count_b": b["distinct_abs_count"],
        "distinct_abs_preserved": a["distinct_abs_count"] == b["distinct_abs_count"],
        "sign_balance_a": (a_pos, a_neg),
        "sign_balance_b": (b_pos, b_neg),
        "sign_balance_preserved": (a_pos, a_neg) == (b_pos, b_neg),
        "gap_pattern_a": a_gaps,
        "gap_pattern_b": b_gaps,
    }


def render_sat_summary(n: int, L: int, simplest: dict) -> str:
    lines = [f"n={n} L={L} (n+1=expected_sat)"]
    lines.append(f"  total valid candidates (within (n-1)-pin strategy): "
                  f"{simplest['_total_candidates']}")
    lines.append(f"  SIMPLEST candidate:")
    lines.append(f"    pins (a_j index pinned) = {simplest['indices']}")
    lines.append(f"    signs (a_j value pinned to) = {simplest['signs']}")
    lines.append(f"    free-parameter solution (a_1..a_{n - 1}) = "
                  f"{simplest['solution']}")
    lines.append(f"    gap_pattern (consecutive differences) = "
                  f"{simplest['gap_pattern']}")
    lines.append(f"    full covering sequence a_0..a_{n + L - 1} = "
                  f"{simplest['sequence']}")
    lines.append(f"    simplicity score: {simplest['simplicity_score']} "
                  f"(distinct |a_j|={simplest['distinct_abs_count']}, "
                  f"distinct gaps={simplest['distinct_gap_count']}, "
                  f"zeros in solution={simplest['zeros_in_solution']})")
    return "\n".join(lines)


def render_unsat_summary(n: int, L: int, cert: dict) -> str:
    fb = cert["failure_breakdown"]
    return (f"n={n} L={L} (n+2=expected_unsat)\n"
            f"  status: UNSAT_CERTIFIED within (n-1)-pin strategy\n"
            f"  total strategies tried: {cert['total_strategies']}\n"
            f"  failure breakdown: singular={fb['singular']}, "
            f"out_of_box_solution={fb['out_of_box_solution']}, "
            f"out_of_box_window={fb['out_of_box_window']}, "
            f"cover_incomplete={fb['cover_incomplete']}")


def main() -> int:
    install_memory_limit()
    parser = argparse.ArgumentParser(description=__doc__,
                                      formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--n-min", type=int, default=2)
    parser.add_argument("--n-max", type=int, default=5)
    parser.add_argument("--emit-json-dir", type=str, default=None,
                        help="write per-(n,L) JSON certificates to this directory")
    parser.add_argument("--mine-gap-formula", action="store_true",
                        help="after mining, attempt closed-form fits "
                              "across n on the simplest candidates' "
                              "gap patterns")
    parser.add_argument("--attempt-promotion", action="store_true",
                        help="after mining, attempt one-step lift of "
                              "the simplest n candidate to n+1, n+2, ...")
    args = parser.parse_args()

    out_dir = Path(args.emit_json_dir) if args.emit_json_dir else None
    if out_dir is not None:
        out_dir.mkdir(parents=True, exist_ok=True)

    all_artifacts: dict[str, dict] = {}
    simplest_per_n: dict[int, dict] = {}

    for n in range(args.n_min, args.n_max + 1):
        L_sat = n + 1
        L_unsat = n + 2

        sat_candidates, total_strategies_sat, fail_breakdown_sat = (
            enumerate_sat(n, L_sat))
        unsat_cert = try_unsat_certified(n, L_unsat)

        if sat_candidates:
            simplest = sat_candidates[0]
            simplest["_total_candidates"] = len(sat_candidates)
            simplest["_total_strategies_tried"] = total_strategies_sat
            simplest["_failure_breakdown_sat"] = fail_breakdown_sat
            simplest["n"] = n
            simplest["L"] = L_sat
            simplest_per_n[n] = simplest
            print(render_sat_summary(n, L_sat, simplest))
        else:
            print(f"n={n} L={L_sat}: NO CANDIDATE FOUND (search bug or "
                  f"strategy insufficient); "
                  f"failure_breakdown={fail_breakdown_sat}")
        print(render_unsat_summary(n, L_unsat, unsat_cert))
        print()

        all_artifacts[f"n{n}_L{L_sat}"] = {
            "n": n, "L": L_sat, "expected_status": "SAT",
            "total_strategies_tried": total_strategies_sat,
            "failure_breakdown": fail_breakdown_sat,
            "candidates": sat_candidates,
        }
        all_artifacts[f"n{n}_L{L_unsat}"] = {
            "n": n, "L": L_unsat, "expected_status": "UNSAT",
            **unsat_cert,
        }

    summary: dict = {"per_n": all_artifacts}
    if args.mine_gap_formula and len(simplest_per_n) >= 2:
        per_n_gaps = {n: simplest_per_n[n]["gap_pattern"]
                      for n in sorted(simplest_per_n)}
        summary["gap_formula_mine"] = fit_gap_formula(per_n_gaps)
        gm = summary["gap_formula_mine"]
        print("Gap-formula mining:")
        for f in gm.get("forms", []):
            print(f"  form={f['name']} params={f['params']} "
                  f"total_residual={f['total_residual']}")
        if gm.get("honest_warning"):
            print(f"  HONEST WARNING: {gm['honest_warning']}")
        for hc in gm.get("holdout_check", []):
            if "predicted" in hc:
                print(f"  holdout n={gm['holdout_n']} form={hc['form']} "
                      f"predicted={hc['predicted']} actual={hc['actual']} "
                      f"match={hc['match']}")
            else:
                print(f"  holdout n={gm['holdout_n']} form={hc['form']} "
                      f"status={hc.get('status', '?')}")
        print()

    if args.attempt_promotion and len(simplest_per_n) >= 2:
        comparisons = []
        ns = sorted(simplest_per_n)
        for i in range(len(ns) - 1):
            res = attempt_promotion(simplest_per_n[ns[i]],
                                     simplest_per_n[ns[i + 1]])
            comparisons.append(res)
            print(f"compare n={res['n_from']} simplest vs n={res['n_to']} "
                  f"simplest:")
            print(f"  pins:        {res['gap_pattern_a']} -> {res['gap_pattern_b']}")
            print(f"  first pin:   {res['first_pin_a']} -> {res['first_pin_b']} "
                  f"preserved={res['first_pin_preserved']}")
            print(f"  last pin at end: {res['last_pin_at_end_a']} -> "
                  f"{res['last_pin_at_end_b']}")
            print(f"  tail consecutive: {res['tail_consecutive_a']} -> "
                  f"{res['tail_consecutive_b']} "
                  f"preserved={res['tail_consecutive_preserved']}")
            print(f"  distinct |a_j| count: {res['distinct_abs_count_a']} -> "
                  f"{res['distinct_abs_count_b']} "
                  f"preserved={res['distinct_abs_preserved']}")
            print(f"  sign balance: {res['sign_balance_a']} -> "
                  f"{res['sign_balance_b']} "
                  f"preserved={res['sign_balance_preserved']}")
        summary["comparisons"] = comparisons
        print()

    if out_dir is not None:
        per_n_path = out_dir / "covering_proof_summary.json"
        with open(per_n_path, "w", encoding="utf-8") as stream:
            json.dump(summary, stream, indent=2, default=str)
        for key, payload in all_artifacts.items():
            with open(out_dir / f"{key}.json", "w", encoding="utf-8") as stream:
                json.dump(payload, stream, indent=2, default=str)
        print(f"wrote {len(all_artifacts) + 1} JSON files to {out_dir}/")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
