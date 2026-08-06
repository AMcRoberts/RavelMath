#!/usr/bin/env bash
set -euo pipefail

# Run Lean/Lake against disposable, ordinary-directory copies only.
# This intentionally never exposes the authoritative ridealong dependency tree
# through a symlink or a manifest-managed package directory.

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUNDLE_ROOT="${RAVEL_LEAN_BUNDLE_ROOT:-/home/anonymous/ravel_seam_zip/ravel_work_2026-08-03_LEAN_dependencies_2026-08-03}"
TOOLCHAIN_ROOT="${RAVEL_LEAN_TOOLCHAIN_ROOT:-/home/anonymous/.elan/toolchains/leanprover--lean4---v4.32.1}"
TARGET="${1:-$ROOT/out/GeneratedCampaign.lean}"
KEEP_WORKSPACE="${RAVEL_KEEP_LEAN_WORKSPACE:-0}"

if [[ ! -f "$TARGET" ]]; then
  echo "safe_lean_check: target not found: $TARGET" >&2
  exit 2
fi
if [[ ! -d "$BUNDLE_ROOT/mathlib" ]]; then
  echo "safe_lean_check: Mathlib ridealong not found under: $BUNDLE_ROOT" >&2
  exit 2
fi
if [[ ! -x "$TOOLCHAIN_ROOT/bin/lake" ]]; then
  echo "safe_lean_check: packaged Lake not found under: $TOOLCHAIN_ROOT" >&2
  exit 2
fi
if [[ ! -f "$TOOLCHAIN_ROOT/lib/lean/Init.olean.private" ]]; then
  echo "safe_lean_check: packaged Lean toolchain is incomplete: missing lib/lean/Init.olean.private" >&2
  echo "safe_lean_check: the compiler binary is present, but Lean 4.32 cannot elaborate even its Lake configuration without private core artifacts" >&2
  exit 4
fi

# Refuse symlinked authoritative inputs. Lake has package-repair behavior that
# can follow and replace links, so only real directories are accepted here.
for p in "$BUNDLE_ROOT" "$BUNDLE_ROOT/mathlib" "$TOOLCHAIN_ROOT"; do
  if [[ -L "$p" ]]; then
    echo "safe_lean_check: refusing symlinked authoritative input: $p" >&2
    exit 3
  fi
done

WORK="$(mktemp -d /tmp/ravel-lean-check.XXXXXX)"
cleanup() {
  if [[ "$KEEP_WORKSPACE" == "1" ]]; then
    echo "safe_lean_check: retained disposable workspace: $WORK" >&2
  else
    rm -rf -- "$WORK"
  fi
}
trap cleanup EXIT

mkdir -p "$WORK/vendor" "$WORK/check/Ravel"

# Copy, never link. Lake may freely repair/build/delete inside this disposable
# copy without touching the packaged dependency snapshot.
cp -a --reflink=auto "$BUNDLE_ROOT"/. "$WORK/vendor"/
# The top-level Ravel/ tree is the superset of committed hand-written Lean
# support modules (e.g. Ravel/Polynomial/Normalization.lean, which generated
# campaigns import but which does not live under lean/Ravel/).
cp -a "$ROOT/Ravel"/. "$WORK/check/Ravel"/
cp -a "$TARGET" "$WORK/check/GeneratedCampaign.lean"

cat > "$WORK/check/lakefile.toml" <<LAKE
name = "ravel_safe_check"
version = "0.1.0"
defaultTargets = ["RavelCheck"]

[[require]]
name = "mathlib"
path = "$WORK/vendor/mathlib"

[[lean_lib]]
name = "Ravel"

[[lean_lib]]
name = "RavelCheck"
roots = ["GeneratedCampaign"]
LAKE

# Pin to the packaged compiler and isolate all user/global state.
cp "$BUNDLE_ROOT/mathlib/lean-toolchain" "$WORK/check/lean-toolchain"
export HOME="$WORK/home"
export ELAN_HOME="$WORK/elan-home"
export PATH="$TOOLCHAIN_ROOT/bin:$PATH"
mkdir -p "$HOME" "$ELAN_HOME"

cd "$WORK/check"
# Plain `lake env lean` only exports LEAN_PATH for already-built dependencies;
# our own Ravel/RavelCheck libraries are local and unbuilt, so `lean` alone
# cannot resolve `import Ravel...`. `lake build` compiles (and thereby kernel
# checks) the whole local package, Ravel included.
"$TOOLCHAIN_ROOT/bin/lake" build RavelCheck
