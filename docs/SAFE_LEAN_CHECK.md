# Safe Lean checking with the ridealong Mathlib tree

Use `scripts/safe_lean_check.sh <generated-file.lean>`.

The script never points Lake at the authoritative dependency bundle through a
symlink. It creates a disposable ordinary-directory copy of the complete
ridealong dependency tree, copies the Ravel Lean modules and generated theorem
into a separate check workspace, and runs the packaged Lean/Lake toolchain
there. Lake may repair or delete files only inside that disposable workspace.

Set `RAVEL_KEEP_LEAN_WORKSPACE=1` to retain the disposable workspace for
inspection after a failure.

## Toolchain completeness preflight

The wrapper checks for `lib/lean/Init.olean.private` before invoking Lake. Lean
4.32 requires the private core artifacts to elaborate Lake package files. A
bundle containing `lean`, `lake`, and ordinary `.olean` files but omitting the
`.olean.private` files is source/tooling evidence, not an executable Lean
installation; the wrapper reports this as exit status 4 instead of producing
misleading Mathlib parser errors.
