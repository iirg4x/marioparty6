from __future__ import annotations

import copy
import hashlib
import json
import tempfile
import unittest
from pathlib import Path

from tools import crack_learning_rules as rules
from tools import repeated_opcode_low_level_readiness as readiness
from tools.tests import test_crack_learning_rules as fixtures


def _site(
    *,
    site_id: str,
    function: str,
    start: int,
    payload: bytes,
    operation: str,
    mnemonics: list[str],
    objdiff_sha256: str,
    eligible: bool = True,
) -> dict[str, object]:
    return {
        "id": site_id,
        "function": function,
        "object_start": start,
        "object_end": start + len(payload),
        "bytes": payload.hex(),
        "sha256": hashlib.sha256(payload).hexdigest(),
        "operation": operation,
        "aggregate_type": "HuVecF",
        "target_mnemonics": mnemonics,
        "objdiff_canonical_sha256": objdiff_sha256,
        "unresolved_residual": eligible,
        "producer_consumer_authenticated": eligible,
        "eligible": eligible,
        "exclusion_reason": None if eligible else "unrelated scalar opcode span",
    }


def _context(
    *,
    authorized: bool = True,
    objdiff_sha256: str = "9" * 64,
) -> dict[str, object]:
    copy_payload = bytes.fromhex("102030405060708090a0b0c0d0e0f000")
    mul_payload = bytes.fromhex(
        "00112233445566778899aabbccddeeff"
        "ffeeddccbbaa99887766554433221100"
    )
    unrelated_payload = bytes.fromhex("deadbeef")
    copy_mnemonics = ["psq_l", "lfs", "psq_st", "stfs"]
    mul_mnemonics = [
        "psq_l",
        "psq_l",
        "ps_mul",
        "psq_st",
        "lfs",
        "lfs",
        "fmuls",
        "stfs",
    ]
    inventory = [
        _site(
            site_id="copy_movenum",
            function="MoveNumOMExec",
            start=0x34DC,
            payload=copy_payload,
            operation="aggregate_copy",
            mnemonics=copy_mnemonics,
            objdiff_sha256=objdiff_sha256,
        ),
        _site(
            site_id="copy_colball",
            function="mbev_PlayerColBall",
            start=0x4564,
            payload=copy_payload,
            operation="aggregate_copy",
            mnemonics=copy_mnemonics,
            objdiff_sha256=objdiff_sha256,
        ),
        _site(
            site_id="mul_radius",
            function="GetBiriQEffectRadius",
            start=0x64EC,
            payload=mul_payload,
            operation="componentwise_multiply",
            mnemonics=mul_mnemonics,
            objdiff_sha256=objdiff_sha256,
        ),
        _site(
            site_id="mul_metal",
            function="MetalEffectCreate",
            start=0x69A4,
            payload=mul_payload,
            operation="componentwise_multiply",
            mnemonics=mul_mnemonics,
            objdiff_sha256=objdiff_sha256,
        ),
        _site(
            site_id="unrelated",
            function="UnrelatedFunction",
            start=0x7770,
            payload=unrelated_payload,
            operation="scalar_load",
            mnemonics=["lwz"],
            objdiff_sha256="8" * 64,
            eligible=False,
        ),
    ]
    controls = [
        {
            "id": "copy_assignment_control",
            "operation": "aggregate_copy",
            "source_shape": "pos = posNorm",
            "source_sha256": "1" * 64,
            "object_sha256": "2" * 64,
            "result_class": "different_nonexact_object",
            "target_sequence_emitted": False,
            "admissible": True,
        },
        {
            "id": "multiply_member_control",
            "operation": "componentwise_multiply",
            "source_shape": "dst.x = lhs.x * rhs.x; dst.y = lhs.y * rhs.y; dst.z = lhs.z * rhs.z",
            "source_sha256": "3" * 64,
            "object_sha256": "4" * 64,
            "result_class": "object_identical_nonexact",
            "target_sequence_emitted": False,
            "admissible": True,
        },
    ]
    source_sha256 = "5" * 64
    object_sha256 = "6" * 64
    target_sha256 = "7" * 64
    return {
        "schema": readiness.CONTEXT_SCHEMA,
        "report_artifact_sha256": "a" * 64,
        "owner": "main:board/player",
        "configured_compiler": {
            "version": "GC/2.6",
            "sha256": "b" * 64,
            "wrapper_sha256": "c" * 64,
        },
        "toolchain": {
            "dtk": {"version": "1.4.1", "sha256": "d" * 64},
            "objdiff": {"version": "3.0.0", "sha256": "e" * 64},
        },
        "candidate": {
            "source_sha256": source_sha256,
            "object_sha256": object_sha256,
        },
        "target": {"object_sha256": target_sha256},
        "opcode_inventory": inventory,
        "groups": [
            {
                "operation": "aggregate_copy",
                "helper_symbol": "HuVecCopy",
                "aggregate_type": "HuVecF",
                "fingerprint_sha256": inventory[0]["sha256"],
                "site_ids": ["copy_movenum", "copy_colball"],
                "semantic_contract": "copy three f32 components from one HuVecF to another",
                "expected_source_class": "target_proven_low_level_source",
                "target_mnemonics": copy_mnemonics,
            },
            {
                "operation": "componentwise_multiply",
                "helper_symbol": "HuVecMul",
                "aggregate_type": "HuVecF",
                "fingerprint_sha256": inventory[2]["sha256"],
                "site_ids": ["mul_radius", "mul_metal"],
                "semantic_contract": "multiply corresponding HuVecF components",
                "expected_source_class": "target_proven_low_level_source",
                "target_mnemonics": mul_mnemonics,
            },
        ],
        "natural_c_exhaustion": {
            "bounded": True,
            "all_admissible_controls_exhausted": True,
            "unknown_evidence_used": False,
            "repeat_trace_required": False,
            "control_corpus_sha256": readiness.canonical_sha256(controls),
            "controls": controls,
        },
        "governed_low_level_source": {
            "source_class": "target_proven_low_level_source",
            "policy_sha256": "f" * 64,
            "instance_request_sha256": "0" * 64,
            "validator_sha256": "1" * 64,
            "validation_receipt_sha256": "2" * 64 if authorized else None,
            "explicit_user_authorization": authorized,
            "validator_result": "PASS" if authorized else "NOT_RUN",
            "symbolic_operands_only": True,
            "fixed_physical_registers": False,
            "raw_words": False,
            "object_patching": False,
            "authority_advanced": False,
        },
        "exact_result": {
            "source_sha256": source_sha256,
            "object_sha256": object_sha256,
            "target_object_sha256": target_sha256,
            "strict_report_sha256": "3" * 64,
            "data_report_sha256": "4" * 64,
            "focus_functions": [
                "MoveNumOMExec",
                "mbev_PlayerColBall",
                "GetBiriQEffectRadius",
                "MetalEffectCreate",
            ],
            "helpers": ["HuVecCopy", "HuVecMul"],
            "functions_exact": 165,
            "functions_total": 165,
            "strict_diff_rows": 0,
            "data_diff_rows": 0,
            "physical_relocations": 2249,
            "relocation_identity": True,
            "protected_sibling_losses": 0,
            "configured_outputs_exact": 137,
            "configured_outputs_total": 137,
            "main_dol_sha256": "5" * 64,
            "main_dol_byte_identical": True,
        },
        "telemetry": {
            "telemetry_complete": False,
            "excluded_from_measured_crack_per_hour": True,
            "no_imputation": True,
            "interval_log_sha256": "6" * 64,
        },
        "authority_advanced": False,
    }


class RepeatedOpcodeLowLevelReadinessTests(unittest.TestCase):
    def test_player_acceptance_is_authorized_but_non_authoritative(self) -> None:
        context = _context()
        result = readiness.evaluate(context)

        self.assertTrue(result["matched"])
        self.assertEqual(result["status"], "AUTHORIZED_VALIDATED_INSTANCE")
        self.assertEqual(
            [group["site_count"] for group in result["groups"]],
            [2, 2],
        )
        self.assertFalse(result["candidate_scheduled"])
        self.assertFalse(result["authority_advanced"])
        self.assertEqual(
            result["evidence"]["exact_result"]["main_dol_sha256"],
            "5" * 64,
        )
        sealed = dict(result)
        digest = sealed.pop(readiness.HASH_FIELD)
        self.assertEqual(digest, readiness.canonical_sha256(sealed))

    def test_pending_packet_requires_explicit_authorization(self) -> None:
        result = readiness.evaluate(_context(authorized=False))
        self.assertEqual(result["status"], "READY_FOR_EXPLICIT_AUTHORIZATION")
        self.assertIn("Request explicit user authorization", result["recommendation"])
        self.assertFalse(result["authority_advanced"])

    def test_semantic_collection_order_is_normalized(self) -> None:
        context = _context()
        reordered = copy.deepcopy(context)
        reordered["opcode_inventory"].reverse()
        reordered["groups"].reverse()
        for group in reordered["groups"]:
            group["site_ids"].reverse()
        reordered["natural_c_exhaustion"]["controls"].reverse()

        self.assertEqual(
            readiness.parse_context(reordered),
            readiness.parse_context(context),
        )
        self.assertEqual(
            readiness.evaluate(reordered),
            readiness.evaluate(context),
        )

    def test_focus_and_objdiff_binding(self) -> None:
        context = _context()
        matched = readiness.evaluate(
            context,
            focus_symbol="MoveNumOMExec",
            objdiff_canonical_sha256="9" * 64,
        )
        self.assertTrue(matched["matched"])
        other_function = readiness.evaluate(
            context,
            focus_symbol="NotPlayer",
            objdiff_canonical_sha256="9" * 64,
        )
        self.assertFalse(other_function["matched"])
        other_report = readiness.evaluate(
            context,
            focus_symbol="MoveNumOMExec",
            objdiff_canonical_sha256="8" * 64,
        )
        self.assertFalse(other_report["matched"])
        with self.assertRaises(readiness.RepeatedOpcodeReadinessInputError):
            readiness.evaluate(context, focus_symbol="MoveNumOMExec")

    def test_context_fails_closed(self) -> None:
        cases: list[tuple[str, dict[str, object]]] = []

        extra = _context()
        extra["unexpected"] = True
        cases.append(("extra field", extra))

        bad_fingerprint = _context()
        bad_fingerprint["opcode_inventory"][0]["sha256"] = "0" * 64  # type: ignore[index]
        cases.append(("bad fingerprint", bad_fingerprint))

        missing_site = _context()
        missing_site["groups"][0]["site_ids"] = ["copy_movenum"]  # type: ignore[index]
        cases.append(("singleton group", missing_site))

        bad_controls = _context()
        bad_controls["natural_c_exhaustion"]["control_corpus_sha256"] = "0" * 64  # type: ignore[index]
        cases.append(("unsealed controls", bad_controls))

        unknown = _context()
        unknown["natural_c_exhaustion"]["unknown_evidence_used"] = True  # type: ignore[index]
        cases.append(("unknown evidence", unknown))

        no_receipt = _context()
        no_receipt["governed_low_level_source"]["validation_receipt_sha256"] = None  # type: ignore[index]
        cases.append(("authorized without receipt", no_receipt))

        overlap = _context()
        overlap["opcode_inventory"][1]["function"] = "MoveNumOMExec"  # type: ignore[index]
        overlap["opcode_inventory"][1]["object_start"] = 0x34E0  # type: ignore[index]
        overlap["opcode_inventory"][1]["object_end"] = 0x34F0  # type: ignore[index]
        cases.append(("overlap", overlap))

        for label, context in cases:
            with self.subTest(label=label):
                with self.assertRaises(readiness.RepeatedOpcodeReadinessInputError):
                    readiness.parse_context(context)

    def test_standalone_cli_writes_atomic_self_hashed_output(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            context_path = root / "context.json"
            output_path = root / "result.json"
            context_path.write_text(json.dumps(_context()), encoding="utf-8")

            self.assertEqual(
                readiness.main(
                    [
                        "--context",
                        str(context_path),
                        "--output",
                        str(output_path),
                    ]
                ),
                0,
            )
            result = json.loads(output_path.read_text(encoding="utf-8"))
            self.assertEqual(result["schema"], readiness.RESULT_SCHEMA)
            self.assertFalse(result["authority_advanced"])
            self.assertEqual(
                list(root.glob(f"{output_path.name}.*")),
                [],
            )

    def test_crack_learning_dispatcher_integration(self) -> None:
        report = fixtures._dform_copy_trace_report()
        report_sha256 = rules._sha256(rules._canonical(report))
        context = _context(objdiff_sha256=report_sha256)

        result = rules.diagnose_document(
            report,
            focus_symbol="MoveNumOMExec",
            repeated_opcode_low_level_readiness_context=context,
        )
        diagnosis = next(
            item
            for item in result["evaluations"]
            if item["rule_id"] == readiness.RULE_ID
        )
        self.assertTrue(diagnosis["matched"])
        self.assertEqual(
            diagnosis["evidence"]["readiness"]["status"],
            "AUTHORIZED_VALIDATED_INSTANCE",
        )
        self.assertFalse(
            diagnosis["evidence"]["readiness"]["authority_advanced"]
        )
        self.assertEqual(
            result["inputs"][
                "repeated_opcode_low_level_readiness_context_canonical_sha256"
            ],
            readiness.canonical_sha256(readiness.parse_context(context)),
        )
        self.assertEqual(
            result["implementations"]["repeated_opcode_low_level_readiness"][
                "result_schema"
            ],
            readiness.RESULT_SCHEMA,
        )
        self.assertFalse(result["authority_advanced"])


if __name__ == "__main__":
    unittest.main()
