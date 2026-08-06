#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
from pathlib import Path

def summarize(path):
    payload = json.loads(Path(path).read_text())
    summary = {
        "file": str(path),
        "keys": sorted(payload.keys()),
        "candidate_counts": {},
    }
    for key, value in payload.items():
        if isinstance(value, list):
            summary["candidate_counts"][key] = len(value)
        elif isinstance(value, dict):
            summary["candidate_counts"][key] = len(value)
    return summary

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--inputs", nargs="*", default=[])
    parser.add_argument("--emit", required=True)
    args = parser.parse_args()

    reports = []
    for item in args.inputs:
        p = Path(item)
        if p.exists() and p.suffix == ".json":
            try:
                reports.append(summarize(p))
            except Exception:
                pass

    payload = {
        "kind": "next-axis-ternary-recurrence-audit-v1",
        "files_examined": len(reports),
        "reports": reports,
        "recommended_next_axis":
            "universal predicted-core SCC identification before spectral dominance",
        "reason":
            "once cyclic shell pumping closes ternary recurrence, the next structural theorem must identify the grade-one recurrent component independently of dimension",
    }
    Path(args.emit).write_text(
        json.dumps(payload, indent=2, sort_keys=True) + "\n"
    )
    print(f"next-axis audit examined {len(reports)} JSON artifacts")

if __name__ == "__main__":
    main()
