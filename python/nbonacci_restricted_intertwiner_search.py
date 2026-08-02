#!/usr/bin/env python3
"""Search displacement-decomposition state-level n-bonacci intertwiners.

This is the structurally meaningful refinement of
``nbonacci_intertwiner_search.py``. Variables exist only when the core
translation is an elementary summand of the shell translation and the
remainder has grade at most two. Thus a successful P is local in the graded
displacement skew product rather than an arbitrary cone map whose existence
merely restates rho(S)<rho(C).
"""

from __future__ import annotations

import re
import sys
import argparse
from dataclasses import dataclass

import numpy as np
from scipy.optimize import linprog
from scipy.sparse import coo_matrix


@dataclass
class Relation:
    n: int
    component: int
    shell_n: int
    core_n: int
    core_edges: list[tuple[int, int, float]]
    shell_edges: list[tuple[int, int, float]]
    allowed: list[tuple[int, int]]


def parse(lines: list[str]) -> list[Relation]:
    header = re.compile(
        r"RBEGIN n=(\d+) component=(\d+) shell=(\d+) core=(\d+) allowed=(\d+)"
    )
    result: list[Relation] = []
    current = None
    core_edges: list[tuple[int, int, float]] = []
    shell_edges: list[tuple[int, int, float]] = []
    allowed: list[tuple[int, int]] = []
    for raw in lines:
        line = raw.strip()
        match = header.fullmatch(line)
        if match:
            current = tuple(map(int, match.groups()))
            core_edges, shell_edges, allowed = [], [], []
        elif line.startswith("RCE ") and current is not None:
            _, u, v, w = line.split()
            core_edges.append((int(u), int(v), float(w)))
        elif line.startswith("RSE ") and current is not None:
            _, u, v, w = line.split()
            shell_edges.append((int(u), int(v), float(w)))
        elif line.startswith("RALLOW ") and current is not None:
            _, u, v = line.split()
            allowed.append((int(u), int(v)))
        elif line == "REND" and current is not None:
            n, component, shell_n, core_n, expected_allowed = current
            if len(allowed) != expected_allowed:
                raise ValueError("truncated RALLOW stream")
            result.append(Relation(
                n, component, shell_n, core_n,
                core_edges, shell_edges, allowed,
            ))
            current = None
    return result


def search(relation: Relation):
    allowed_index = {pair: k for k, pair in enumerate(relation.allowed)}
    variables = len(relation.allowed)
    constraint_index: dict[tuple[int, int], int] = {}
    rows: list[int] = []
    columns: list[int] = []
    values: list[float] = []

    def constraint(pair: tuple[int, int]) -> int:
        if pair not in constraint_index:
            constraint_index[pair] = len(constraint_index)
        return constraint_index[pair]

    allowed_by_shell: list[list[tuple[int, int]]] = [
        [] for _ in range(relation.shell_n)
    ]
    for variable, (shell, core) in enumerate(relation.allowed):
        allowed_by_shell[shell].append((core, variable))

    # S P term: shell edge s->t transports every permitted P[t,c].
    for source, target, weight in relation.shell_edges:
        for core, variable in allowed_by_shell[target]:
            rows.append(constraint((source, core)))
            columns.append(variable)
            values.append(weight)
    # -P C term: P[s,d] transports along every core edge d->c.
    core_out: list[list[tuple[int, float]]] = [
        [] for _ in range(relation.core_n)
    ]
    for source, target, weight in relation.core_edges:
        core_out[source].append((target, weight))
    for variable, (shell, core) in enumerate(relation.allowed):
        for target, weight in core_out[core]:
            rows.append(constraint((shell, target)))
            columns.append(variable)
            values.append(-weight)

    inequalities = coo_matrix(
        (values, (rows, columns)),
        shape=(len(constraint_index), variables),
    ).tocsr()
    eq_rows = [shell for shell, _ in relation.allowed]
    eq_columns = list(range(variables))
    equalities = coo_matrix(
        (np.ones(variables), (eq_rows, eq_columns)),
        shape=(relation.shell_n, variables),
    ).tocsr()

    shell_degree = np.zeros(relation.shell_n)
    core_degree = np.zeros(relation.core_n)
    for source, _, weight in relation.shell_edges:
        shell_degree[source] += weight
    for source, _, weight in relation.core_edges:
        core_degree[source] += weight
    objective = np.array([
        shell_degree[shell] - core_degree[core]
        for shell, core in relation.allowed
    ])
    uniform = np.zeros(variables)
    for entries in allowed_by_shell:
        for _, variable in entries:
            uniform[variable] = 1.0 / len(entries)
    uniform_residual = inequalities @ uniform
    uniform_diagnostic = (
        float(uniform_residual.max(initial=0.0)),
        int(np.count_nonzero(uniform_residual > 1e-10)),
        -float(objective @ uniform),
    )
    result = linprog(
        objective,
        A_ub=inequalities,
        b_ub=np.zeros(inequalities.shape[0]),
        A_eq=equalities,
        b_eq=np.ones(relation.shell_n),
        bounds=(0.0, None),
        method="highs",
        options={"presolve": True},
    )
    if not result.success:
        return result, 0, 0.0, uniform_diagnostic
    support = int(np.count_nonzero(result.x > 1e-9))
    total_slack = -float(result.fun)
    return result, support, total_slack, uniform_diagnostic


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--component", type=int)
    parser.add_argument("--require-all", action="store_true")
    args = parser.parse_args()
    relations = parse(sys.stdin.readlines())
    failed = False
    for relation in relations:
        if args.component is not None and relation.component != args.component:
            continue
        result, support, total_slack, uniform = search(relation)
        uniform_text = (
            f"uniform(max_violation={uniform[0]:.6g},"
            f"bad={uniform[1]},total_slack={uniform[2]:.6g})"
        )
        if result.success:
            strict = total_slack > 1e-8
            print(
                f"n={relation.n} shell={relation.component}: FOUND "
                f"allowed={len(relation.allowed)} support={support} "
                f"total_slack={total_slack:.9g} "
                f"strict={'YES' if strict else 'no'} {uniform_text}"
            )
            failed = failed or not strict
        else:
            print(
                f"n={relation.n} shell={relation.component}: "
                f"NO RESTRICTED INTERTWINER status={result.status} "
                f"{result.message} {uniform_text}"
            )
            failed = True
    return 1 if failed and args.require_all else 0


if __name__ == "__main__":
    raise SystemExit(main())
