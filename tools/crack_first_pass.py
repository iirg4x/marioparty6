#!/usr/bin/env python3
"""Route one objdiff residual to a first-mismatch P0 recovery cell.

The router composes the installed typed-pool decoder with the strict
typed-owner manifest gate.  It does not generate source or launch a compiler.
Its purpose is to identify the earliest mismatch, rank one evidence-backed
natural-C cell that owns it, and force a recompute/pivot after that result.
Donor, history, and trace evidence are optional support rather than gates.
"""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path
from typing import Any, Mapping, Sequence

if __package__ in {None, ""}:
    sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

from tools import complete_stack_home_exchange as stack_home
from tools import typed_pool_owner_manifest as owner_manifest


SCHEMA = "crack_first_pass/v1"


def _compile_action() -> str:
    return (
        "Compile one highest-ranked evidence-backed cell that owns the earliest "
        "mismatch; after the result, retain measurable gain or pivot/recompute "
        "from the new earliest mismatch."
    )


def _optional_evidence_action() -> str:
    return (
        "Use donor/history/trace evidence only when available as optional support; "
        "none is a prerequisite for the first compile."
    )


def route_manifest(
    manifest: Mapping[str, Any],
    stack_home_diagnosis: Mapping[str, Any] | None = None,
) -> dict[str, Any]:
    facts = manifest.get("facts") if isinstance(manifest.get("facts"), Mapping) else {}
    strict_count = int(facts.get("strict_residual_row_count") or 0)
    pool_count = int(facts.get("nonexact_pool_row_count") or 0)
    classification_counts = facts.get("classification_counts")
    if not isinstance(classification_counts, Mapping):
        classification_counts = {}

    stack_home_facts = (
        stack_home_diagnosis.get("facts")
        if isinstance(stack_home_diagnosis, Mapping)
        and isinstance(stack_home_diagnosis.get("facts"), Mapping)
        else {}
    )

    if manifest.get("status") == "matched":
        route = "typed_pool_owner_manifest"
        actions = [
            "Confirm the earliest listed instruction row maps to a truthful semantic source consumer; donor/history/trace evidence remains optional.",
            _compile_action(),
            "Run strict/data/physical-relocation/protected-sibling proof and write CRACK_REPORT/v1.",
        ]
        trace_budget = 0
    elif isinstance(stack_home_diagnosis, Mapping) and stack_home_diagnosis.get("status") == "matched":
        route = stack_home.ROUTE
        actions = [
            "Use the closed stack-home mapping to rank the one natural owner cell for the earliest mismatch.",
            _compile_action(),
            "Recompute the residual after proof; decode optional pool rows only if still earliest, suppressing source matrices.",
        ]
        trace_budget = 0
    elif pool_count == strict_count and pool_count > 0:
        route = "typed_pool_decoder"
        actions = [
            "Reduce pool evidence around the earliest mismatch and rank one truthful typed owner cell.",
            _compile_action(),
            _optional_evidence_action(),
        ]
        trace_budget = 0
    elif pool_count == 0:
        route = "causal_reducer"
        actions = [
            "Use the causal reducer to identify the earliest non-cascade mismatch and rank one owning source boundary.",
            _compile_action(),
            _optional_evidence_action(),
        ]
        trace_budget = 1
    else:
        route = "causal_reducer_then_typed_pool_decoder"
        actions = [
            "Identify the earliest non-pool mismatch and rank one owner before considering typed-pool evidence.",
            _compile_action(),
            "Recompute after proof; use optional typed-pool evidence only if it becomes the new earliest mismatch.",
        ]
        trace_budget = 1

    # Every route gets exactly one compile opportunity.  A neutral or regressing
    # result is handled by recomputing the residual and pivoting, never by
    # expanding a syntax matrix in this first pass.
    candidate_budget = 1

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
            "stack_home_diagnosis_status": (
                stack_home_diagnosis.get("status")
                if isinstance(stack_home_diagnosis, Mapping)
                else None
            ),
            "stack_home_diagnosis_sha256": (
                stack_home_diagnosis.get("diagnosis_sha256")
                if isinstance(stack_home_diagnosis, Mapping)
                else None
            ),
            "stack_home_row_count": stack_home_facts.get("stack_home_row_count"),
            "stack_home_mapping_count": stack_home_facts.get("mapping_count"),
            "stack_home_pool_handoff_row_count": stack_home_facts.get(
                "pool_handoff_row_count"
            ),
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
    diagnosis = stack_home.build_from_paths(**kwargs)
    return route_manifest(manifest, diagnosis)


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
