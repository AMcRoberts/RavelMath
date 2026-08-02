#!/usr/bin/env python3
"""Replay an emitted maximum-shell first-return rank certificate.

The certificate is finite and exact: every recorded first-return edge must
strictly increase the supplied integer rank.  A periodic orbit whose global
maximum is the shell bound would induce a cycle of such returns, so this is
the machine-checkable finite escape step.
"""

from __future__ import annotations

import argparse
import json


def key(state: list[int] | tuple[int, ...]) -> str:
    return ",".join(map(str, state))


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("certificate")
    args = parser.parse_args()
    with open(args.certificate, encoding="utf-8") as stream:
        data = json.load(stream)
    if data.get("kind") != "nbonacci-max-shell-return-rank-v1":
        raise SystemExit("wrong certificate kind")
    shell = {key(state) for state in data["shell"]}
    ranks = data["rank"]
    if set(ranks) != shell:
        raise SystemExit("rank/shell vertex mismatch")
    for source, destination in data["edges"]:
        source_key, destination_key = key(source), key(destination)
        if source_key not in shell or destination_key not in shell:
            raise SystemExit("edge endpoint is not on shell")
        if ranks[destination_key] <= ranks[source_key]:
            raise SystemExit(f"non-increasing shell edge: {source}->{destination}")
    if data.get("checked") is not True:
        raise SystemExit("certificate was not marked checked")
    print(f"max-shell return certificate PASS: n={data['n']} "
          f"bound={data['bound']} shell={len(shell)} "
          f"edges={len(data['edges'])} rank_height={max(ranks.values(), default=0)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
