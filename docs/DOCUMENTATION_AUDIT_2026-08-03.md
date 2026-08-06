# Documentation audit — 2026-08-03

## Scope

Audited the `.ravel/` continuity tree and `RavelMathPub/` documentation together.

## Authoritative route

Continuity:

1. `.ravel/START_HERE.md`
2. `.ravel/CURRENT_STATE.md`
3. `.ravel/DIARY.md`
4. `.ravel/DOCUMENTATION_MAP.md`

Proof system:

1. `docs/PROOF_SYSTEM_MANUAL.md`
2. `docs/PROOF_SYSTEM_CONTRACTS.md`
3. `docs/PROOF_SYSTEM_EXTENSION_GUIDE.md`
4. the relevant task reading list

## Changes

- Restored a concise continuity bootstrap.
- Added a current diary and diary index.
- Rewrote the Minimax route around executable pivots and installed derivation operations.
- Replaced obsolete generator-era status notes in `THEOREM_STATUS.md` and `RESEARCH_STATUS.md`.
- Clarified that campaign `Closed` is not Lean `KernelChecked`.
- Verified every path named by the proof-system manual exists.
- Moved generator-era architectures, audits, session notes, old runbooks, and long reading lists into `docs/archive/2026-08-03_pre_generic_proof_system/`.
- Moved the old continuity handoffs and long diary into `.ravel/archive/2026-08-03_pre_generic_proof_system/`.

## Archived as stale

The archived material describes one or more superseded approaches:

- flat/chunked Lean proof generation from C++;
- Lua as a proof-schema emitter rather than activation/configuration;
- theorem-specific course handlers;
- raw Lean proof blocks;
- plans whose operations were not already executable;
- outdated session/toolchain claims.

Archive files remain available for provenance but are not instructions.

## Remaining documentation boundary

Many mathematical research notes remain active or parked and were not rewritten merely because their optional cross-references were omitted from the compressed package. Their theorem strength is governed by `THEOREM_STATUS.md`; their research state is governed by `RESEARCH_STATUS.md`.

## Proof-engine API consolidation

`MATHEMATICAL_API.md` now owns the public proof-engine entry point, real symbol list, minimal invocation, operating sequence, and breadcrumb into the manual, contracts, extension guide, and safe checker. `PROOF_SYSTEM_MANUAL.md` no longer duplicates the source-location table, and `SOURCE_FILE_ATLAS.md` records ownership without duplicating usage instructions.
