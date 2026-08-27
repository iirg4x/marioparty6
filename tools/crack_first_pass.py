#!/usr/bin/env python3
"""Route one objdiff residual to at most three P0 recovery actions.

The router composes the installed typed-pool decoder with the strict
typed-owner manifest gate.  It does not generate source or launch a compiler.
Its purpose is to decide the first diagnostic path in seconds and keep the
initial candidate budget bounded.
"""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path
from typing import Any, Mapping, Sequence

if __package__ in {None, ""}:
    sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

from tools import typed_pool_owner_manifest as owner_manifest


SCHEMA = "crack_first_pass/v1"


def route_manifest(manifest: Mapping[str, Any]) -> dict[str, Any]:
    facts = manifest.get("facts") if isinstance(manifest.get("facts"), Mapping) else {}
    strict_count = int(facts.get("strict_residual_row_count") or 0)
    pool_count = int(facts.get("nonexact_pool_row_count") or 0)
    classification_counts = facts.get("classification_counts")
    if not isinstance(classification_counts, Mapping):
        classification_counts = {}

    if manifest.get("status") == "matched":
        route = "typed_pool_owner_manifest"
        candidate_budget = 1
        actions = [
            "Confirm each listed instruction row maps to a truthful semantic source consumer.",
            "Compile the single composed named-owner binding cell.",
            "Run strict/data/physical-relocation/protected-sibling proof and write CRACK_REPORT/v1.",
        ]
        trace_budget = 0
    elif pool_count == strict_count and pool_count > 0:
        route = "typed_pool_decoder"
        candidate_budget = 3
        actions = [
            "Reduce pool groups by value, type, relocation addend, owner identity, and TU chronology.",
            "Compose all disjoint target-authenticated pool causes into the first candidate.",
            "Compile at most three cells; trace remains disabled for a pool-only residual.",
        ]
        trace_budget = 0
    elif pool_count == 0:
        route = "causal_reducer"
        candidate_budget = 3
        actions = [
            "Run the causal cascade reducer for size/frame/CFG/ABI/aggregate ownership.",
            "Query Graphify and one narrow same-TU/history donor before source permutations.",
            "Compile at most three evidence-composed cells; trace only after structure is exact.",
        ]
        trace_budget = 1
    else:
        route = "causal_reducer_then_typed_pool_decoder"
        candidate_budget = 3
        actions = [
            "Close non-pool topology and owner rows first with the causal reducer.",
            "Decode the remaining typed-pool rows after structure is exact.",
            "Compose disjoint causes; compile at most three cells and trace at most once.",
        ]
        trace_budget = 1

    result: dict[str, Any] = {
        "schema": SCHEMA,
        "schema_version": 1,
        "function": manifest.get("function"),
        "route": route,
        "analysis_deadline_minutes": 5,
        "candidate_budget": candidate_budget,
        "trace_budget": trace_budget,
        "actions": actions,
        "facts": {
            "function_size_exact": facts.get("function_size_exact"),
            "data_values_exact": facts.get("data_values_exact"),
            "strict_residual_row_count": strict_count,
            "nonexact_pool_row_count": pool_count,
            "classification_counts": dict(sorted(classification_counts.items())),
            "typed_owner_manifest_status": manifest.get("status"),
            "typed_owner_manifest_sha256": manifest.get("manifest_sha256"),
            "typed_owner_manifest_blockers": list(manifest.get("blockers", [])),
        },
        "p0_scope": "crack_assigned_function_and_complete_CRACK_REPORT_v1",
        "source_patch_emitted": False,
        "retention_authorized": False,
        "promotion_authorized": False,
        "authority_advanced": False,
    }
    result["triage_sha256"] = owner_manifest.canonical_sha256(result)
    return result


def build_from_paths(**kwargs: Any) -> dict[str, Any]:
    manifest = owner_manifest.build_from_paths(**kwargs)
    return route_manifest(manifest)


def _parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("strict_report", type=Path)
    parser.add_argument("data_report", type=Path)
    parser.add_argument("function")
    parser.add_argument("--target-object", type=Path, required=True)
    parser.add_argument("--candidate-object", type=Path, required=True)
    parser.add_argument("--expect-strict-report-sha256", required=True)
    parser.add_argument("--expect-data-report-sha256", required=True)
    parser.add_argument("--expect-target-object-sha256", required=True)
    parser.add_argument("--expect-candidate-object-sha256", required=True)
    parser.add_argument("--output", type=Path)
    return parser


def main(argv: Sequence[str] | None = None) -> int:
    parser = _parser()
    args = parser.parse_args(argv)
    try:
        result = build_from_paths(
            strict_report_path=args.strict_report,
            data_report_path=args.data_report,
            target_object_path=args.target_object,
            candidate_object_path=args.candidate_object,
            function=args.function,
            expected_strict_report_sha256=args.expect_strict_report_sha256,
            expected_data_report_sha256=args.expect_data_report_sha256,
            expected_target_object_sha256=args.expect_target_object_sha256,
            expected_candidate_object_sha256=args.expect_candidate_object_sha256,
        )
    except owner_manifest.TypedPoolManifestInputError as exc:
        parser.error(str(exc))
    rendered = json.dumps(result, indent=2, ensure_ascii=False, sort_keys=True) + "\n"
    if args.output is not None:
        try:
            args.output.parent.mkdir(parents=True, exist_ok=True)
            args.output.write_text(rendered, encoding="utf-8")
        except OSError as exc:
            parser.error(f"cannot write {args.output}: {exc}")
    else:
        print(rendered, end="")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
