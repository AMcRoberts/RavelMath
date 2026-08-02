#!/usr/bin/env python3
"""Compare arithmetic-hull shell roots with core exterior-power products.

Reads the quotient stream emitted by nbonacci_arithmetic_hull.  This is a
numerical structure probe only: equality of moduli is not an exact compound-
matrix identification.  A match points the subsequent exact search toward
minors/Pluecker coordinates rather than arbitrary Perron intertwiners.
"""

from __future__ import annotations

import sys

import numpy as np

from nbonacci_intertwiner_search import parse_stream


def main() -> int:
    records = parse_stream(sys.stdin.readlines())
    by_dimension = {}
    for record in records:
        by_dimension.setdefault(record.n, []).append(record)
    for n in sorted(by_dimension):
        group = by_dimension[n]
        core = next(record for record in group if record.kind == "core")
        core_eigenvalues = np.linalg.eigvals(core.matrix.toarray())
        core_moduli = sorted((abs(value) for value in core_eigenvalues), reverse=True)
        products = []
        product = 1.0
        for modulus in core_moduli[: min(5, len(core_moduli))]:
            product *= modulus
            products.append(product)
        print(
            f"n={n} core_moduli="
            + ",".join(f"{value:.12g}" for value in core_moduli[:6])
            + " exterior_products="
            + ",".join(f"{value:.12g}" for value in products)
        )
        for shell in [record for record in group if record.kind == "shell"]:
            # Dense eigenvalues are acceptable for the enrolled n<=7 probe;
            # callers should cap dimensions before feeding thousand-state
            # shell quotients here.
            if shell.rows > 450:
                print(
                    f"  shell={shell.component} rows={shell.rows} skipped-dense-spectrum"
                )
                continue
            eigenvalues = np.linalg.eigvals(shell.matrix.toarray())
            rho = max(abs(value) for value in eigenvalues)
            ratios = [rho / value for value in products if value != 0]
            print(
                f"  shell={shell.component} rho={rho:.12g} "
                + "rho/products="
                + ",".join(f"{value:.9g}" for value in ratios)
            )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
