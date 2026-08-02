#!/usr/bin/env python3
"""Search quotient-level nonnegative Perron intertwiners.

Input is the QBEGIN/QEDGE/QEND stream emitted by
``out/nbonacci_arithmetic_hull --dump-quotients``.  For every shell quotient S
and core quotient C in the same dimension, solve

    S P <= P C,       P >= 0,       each row of P sums to 1.

The objective maximizes the total entrywise slack.  A feasible solution gives
a positive core-Perron pullback on the shell; positive total slack makes the
Perron comparison strict because the shell is irreducible.

This is a discovery engine.  Floating HiGHS output is not itself the final
proof object; successful supports are meant to be rationally reconstructed
and emitted as a sparse parametric certificate by the next synthesis stage.
"""

from __future__ import annotations

import argparse
import re
import sys
from dataclasses import dataclass

import numpy as np
from scipy.optimize import linprog
from scipy.sparse import coo_matrix, csr_matrix


@dataclass
class MatrixRecord:
    n: int
    kind: str
    component: int
    rows: int
    matrix: csr_matrix


def parse_stream(lines: list[str]) -> list[MatrixRecord]:
    records: list[MatrixRecord] = []
    header_re = re.compile(
        r"QBEGIN n=(\d+) kind=(\w+) component=(\d+) rows=(\d+)"
    )
    current: tuple[int, str, int, int] | None = None
    edge_rows: list[int] = []
    edge_columns: list[int] = []
    edge_values: list[float] = []
    for raw in lines:
        line = raw.strip()
        match = header_re.fullmatch(line)
        if match:
            current = (
                int(match.group(1)), match.group(2),
                int(match.group(3)), int(match.group(4)),
            )
            edge_rows, edge_columns, edge_values = [], [], []
            continue
        if line.startswith("QEDGE ") and current is not None:
            _, row, column, value = line.split()
            edge_rows.append(int(row))
            edge_columns.append(int(column))
            edge_values.append(float(value))
            continue
        if line == "QEND" and current is not None:
            n, kind, component, rows = current
            matrix = coo_matrix(
                (edge_values, (edge_rows, edge_columns)), shape=(rows, rows)
            ).tocsr()
            records.append(MatrixRecord(n, kind, component, rows, matrix))
            current = None
    return records


def search(shell: csr_matrix, core: csr_matrix):
    shell_n, core_n = shell.shape[0], core.shape[0]
    variables = shell_n * core_n

    # One inequality for each entry of S P - P C.
    rows: list[int] = []
    columns: list[int] = []
    values: list[float] = []
    shell_coo = shell.tocoo()
    core_coo = core.tocoo()
    for source, target, weight in zip(
        shell_coo.row, shell_coo.col, shell_coo.data
    ):
        base_constraint = source * core_n
        base_variable = target * core_n
        for core_column in range(core_n):
            rows.append(base_constraint + core_column)
            columns.append(base_variable + core_column)
            values.append(float(weight))
    for core_source, core_target, weight in zip(
        core_coo.row, core_coo.col, core_coo.data
    ):
        for shell_source in range(shell_n):
            rows.append(shell_source * core_n + core_target)
            columns.append(shell_source * core_n + core_source)
            values.append(-float(weight))
    inequalities = coo_matrix(
        (values, (rows, columns)),
        shape=(shell_n * core_n, variables),
    ).tocsr()

    # Stochastic rows rule out the trivial P=0 and ensure P r_core > 0.
    eq_rows = np.repeat(np.arange(shell_n), core_n)
    eq_columns = np.arange(variables)
    equalities = coo_matrix(
        (np.ones(variables), (eq_rows, eq_columns)),
        shape=(shell_n, variables),
    ).tocsr()

    # Minimize negative total slack.  Summing P C - S P over columns
    # leaves outdegree(core_target)-outdegree(shell_source).
    shell_degree = np.asarray(shell.sum(axis=1)).ravel()
    core_degree = np.asarray(core.sum(axis=1)).ravel()
    objective = np.empty(variables)
    for source in range(shell_n):
        objective[source * core_n : (source + 1) * core_n] = (
            shell_degree[source] - core_degree
        )

    result = linprog(
        objective,
        A_ub=inequalities,
        b_ub=np.zeros(shell_n * core_n),
        A_eq=equalities,
        b_eq=np.ones(shell_n),
        bounds=(0.0, None),
        method="highs",
        options={"presolve": True},
    )
    if not result.success:
        return result, 0.0, 0, 0.0
    vector = result.x
    product = vector.reshape(shell_n, core_n)
    slack = product @ core.toarray() - shell @ product
    support = int(np.count_nonzero(vector > 1e-9))
    min_slack = float(slack.min())
    total_slack = float(slack.sum())
    return result, total_slack, support, min_slack


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--max-variables", type=int, default=350_000)
    args = parser.parse_args()
    records = parse_stream(sys.stdin.readlines())
    by_dimension: dict[int, list[MatrixRecord]] = {}
    for record in records:
        by_dimension.setdefault(record.n, []).append(record)
    failure = False
    for n in sorted(by_dimension):
        group = by_dimension[n]
        cores = [record for record in group if record.kind == "core"]
        if len(cores) != 1:
            print(f"n={n}: expected one core quotient, found {len(cores)}")
            failure = True
            continue
        core = cores[0]
        for shell in [record for record in group if record.kind == "shell"]:
            variables = shell.rows * core.rows
            if variables > args.max_variables:
                print(
                    f"n={n} shell={shell.component}: SKIP variables={variables} "
                    f"> cap={args.max_variables}"
                )
                continue
            result, total_slack, support, min_slack = search(
                shell.matrix, core.matrix
            )
            if not result.success:
                print(
                    f"n={n} shell={shell.component}: NO INTERTWINER "
                    f"status={result.status} {result.message}"
                )
                failure = True
            else:
                strict = total_slack > 1e-8 and min_slack > -1e-7
                print(
                    f"n={n} shell={shell.component}: FOUND "
                    f"shape={shell.rows}x{core.rows} support={support} "
                    f"total_slack={total_slack:.9g} min_slack={min_slack:.3g} "
                    f"strict={'YES' if strict else 'no'}"
                )
                failure = failure or not strict
    return 1 if failure else 0


if __name__ == "__main__":
    raise SystemExit(main())
