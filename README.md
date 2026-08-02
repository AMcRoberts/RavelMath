# RavelMathPub

A research and computation library for Pisot substitutions, Rauzy
fractals, contact-boundary graphs, exact algebraic arithmetic, and
selected experiments around the Spectre aperiodic monotile.

Authored by **Ravel**. Coordinated and architected by **AM**. See
[`AUTHORS.md`](AUTHORS.md) for the project roles and
[`CITATION.cff`](CITATION.cff) for machine-readable citation metadata.

The repository contains source code, deterministic fixtures, formal
proofs, theorem/certificate status, literature notes, and open research
questions.

## Current mathematical boundary

The strongest current results include:

- an explicit eight-state recurrent balanced-pair core for the
  Class-II family `sigma_{a,1}`, with exact characteristic polynomial;
- explicit Class-II contact-core and affine-shell catalogues;
- universal stable, penultimate, terminal, and repeated-terminal
  neighbor-2 corona/Red certificates on their stated domains;
- a kernel-checked global partition of legal rounds into five phases;
- exact arithmetic over `Q`, polynomial rings, `Q(beta)`, matrices,
  Sturm sequences, root isolation, and Perron certificates;
- an adelic/non-unit Pisot classification pipeline with explicit
  limitations;
- C++/Lua implementations and retained Python prototypes for several
  finite dynamical and selection experiments.

The active Class-II occurrence seam is deliberately narrow: four
exceptional base transitions still need reverse-inclusion and Red
exclusion proofs before the global catalogue occurrence theorem can be
claimed. See
[`docs/GLOBAL_CATALOGUE_OCCURRENCE_EXHAUSTION.md`](docs/GLOBAL_CATALOGUE_OCCURRENCE_EXHAUSTION.md).

The authoritative classification of claims is
[`docs/THEOREM_STATUS.md`](docs/THEOREM_STATUS.md). A passing bounded
sweep is not presented as a universal proof.

## Layout

| Path | Contents |
|---|---|
| `include/ravel/` | Header-oriented substitution, graph, boundary, spectral, and finite-dynamics library. |
| `include/adelic/` | Dedekind factorization, ideal arithmetic, local fields, prefix automata, coincidence, and property-(F) machinery. |
| `math/` | Exact arithmetic, algebraic-number, matrix, Sturm, root-isolation, FFT, and Perron tools. |
| `src/` | Non-header-only implementations and Lua bindings. |
| `app/` | Standalone research and diagnostic drivers. |
| `tests/` | Enrolled native tests. |
| `lean/` | Lean statements and proofs, including enrolled sorry-free modules and clearly marked drafts. |
| `lua/` | Lua package, deterministic geometry tables, tests, and orchestration. |
| `python/` | Legacy prototypes and result fixtures awaiting capability-by-capability retirement. |
| `tatham_svg_parsed/` | Small parsed Spectre geometry tables used by the data pipeline. |
| `refs/references.bib` | Canonical citation metadata. |
| `refs/FULLTEXT_ARCHIVE.md`, `refs/FULLTEXT_MANIFEST.tsv` | Policy and checksums for the adjacent, Git-excluded local `refs/FullText/` literature archive. |
| `docs/` | Theorem status, proofs, audits, design notes, literature boundaries, and open questions. |

## Build and validation

The native build expects a C++20 compiler, `make`, Lua 5.4 development
headers, and the vendored mini-gmp sources already present under
`math/`.

```sh
make
make check
```

Useful focused targets include:

```sh
make math
make lean-check
make class_ii_corona_literature_probe
make class_ii_symmetry_probe
make class_ii_bp_family_probe
make class_ii_terminal_transport_probe
```

`make lean-check` uses `LEAN_ENV`, defaulting to the workspace-relative
`../LEAN/free_involution_perron/free_involution_perron`. Set `LEAN_ENV`
to a compatible Lean 4 + Mathlib environment if that path is not
available.

Some exact neighbor tests are intentionally slower than ordinary unit
tests. Long-running exploratory applications are not enrolled in
`make check`.

## Reading map

- [`docs/RESEARCH_STATUS.md`](docs/RESEARCH_STATUS.md) — concise current
  results, open seams, and validation boundary.
- [`docs/SOURCE_FILE_ATLAS.md`](docs/SOURCE_FILE_ATLAS.md) — exhaustive
  map of code-bearing files and their responsibilities.
- [`docs/MATHEMATICAL_API.md`](docs/MATHEMATICAL_API.md) — direct
  mathematical object, C++/Lua, CLI, Lean, and reference guide.
- [`docs/THEOREM_STATUS.md`](docs/THEOREM_STATUS.md) — claim-by-claim
  theorem and certificate classification.
- [`docs/GLOBAL_CATALOGUE_OCCURRENCE_EXHAUSTION.md`](docs/GLOBAL_CATALOGUE_OCCURRENCE_EXHAUSTION.md)
  — exact active Class-II proof seam.
- [`docs/CLASS_II_BP_FAMILY_THEOREM.md`](docs/CLASS_II_BP_FAMILY_THEOREM.md),
  [`docs/CLASS_II_CONTACT_MATRIX.md`](docs/CLASS_II_CONTACT_MATRIX.md),
  and
  [`docs/CLASS_II_NEIGHBOR2_TRANSPORT_THEOREM.md`](docs/CLASS_II_NEIGHBOR2_TRANSPORT_THEOREM.md)
  — the principal parametric family documents.
- [`docs/ADELIC_TILING_PLAN.md`](docs/ADELIC_TILING_PLAN.md) and
  [`docs/CSY_MEMORY_EXACTNESS_AUDIT.md`](docs/CSY_MEMORY_EXACTNESS_AUDIT.md)
  — the non-unit/automata direction and its resource boundary.
- [`docs/DIRECTION_AND_OPEN_THREADS.md`](docs/DIRECTION_AND_OPEN_THREADS.md)
  and [`docs/FAMILY_OF_FAMILIES.md`](docs/FAMILY_OF_FAMILIES.md) —
  research questions and representation-family program.
- [`docs/CPP_DESIGN_PHILOSOPHY.md`](docs/CPP_DESIGN_PHILOSOPHY.md),
  [`docs/LUA_USAGE.md`](docs/LUA_USAGE.md), and
  [`math/TOOLS.md`](math/TOOLS.md) — engineering and tool maps.
- [`docs/LITERATURE_AUDIT_CLASS_II.md`](docs/LITERATURE_AUDIT_CLASS_II.md)
  and [`refs/references.bib`](refs/references.bib) — calibrated
  literature and citation boundary.

## Scope and limitations

This is a research library, not a packaged product. Several executables
are probes rather than theorem certificates; documentation distinguishes
those roles. The Lean environment is not yet self-contained, no license
has been selected, and the remaining Python sources are
retained prototypes rather than the preferred architecture for new work.
