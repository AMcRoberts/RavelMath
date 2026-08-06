#!/bin/sh
set -eu

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
cd "$ROOT"

mkdir -p out/shoot_the_moon

printf '%s\n' '[1/3] Building the Lua-driven C++ proof runner...'
make ravel_proof_runner

printf '%s\n' '[2/3] Validating the Lua proof schema and deep pivot tree...'
./out/ravel_proof_runner --plan-only

printf '%s\n' '[3/3] Running the available foundation gates and recording the frontier...'
./out/ravel_proof_runner --execute-foundation

LATEST=$(find out/shoot_the_moon -mindepth 1 -maxdepth 1 -type d | sort | tail -n 1)
printf '\nRun package: %s\n' "$LATEST"
printf 'Open first: %s/NEXT_MOVES.md\n' "$LATEST"
printf 'Then inspect: %s/FRONTIER.md and %s/logs/\n' "$LATEST" "$LATEST"
