from __future__ import annotations

import hashlib
import json
import subprocess
import tempfile
import unittest
from copy import deepcopy
from pathlib import Path

from tools.promote_original_data import (
    PromotionError,
    audit_original_data,
    create_original_data,
    find_record,
    load_manifest,
    parse_original_data,
    plan_original_data,
    validate_record,
)
from tools.promote_recovered_c import source_quality_errors


OWNER = "main:test/original_data#fixture"
SOURCE_PATH = "src/test/original_data.c"
SOURCE_TEXT = """#include \"test/original_data.h\"
#if TEST_TARGET == TEST_TARGET_DOLPHIN
char testPayload[] ATTRIBUTE_ALIGN(4) = {
    0x00, 0x01, 0xFE, 0xFF,
};
u16 testPayloadLength = sizeof(testPayload);
#endif
"""
PAYLOAD = bytes((0x00, 0x01, 0xFE, 0xFF))


def run(root: Path, *args: str) -> str:
    process = subprocess.run(
        args,
        cwd=root,
        check=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
    )
    return process.stdout.strip()


def record_for(text: str = SOURCE_TEXT) -> dict[str, object]:
    return {
        "id": "fixture-original-data",
        "classification": "authenticated",
        "kind": "original_data",
        "status": "static_authenticated_pending_native",
        "owner": OWNER,
        "path": SOURCE_PATH,
        "source_sha256": hashlib.sha256(text.encode()).hexdigest(),
        "payload": {
            "symbol": "testPayload",
            "length": len(PAYLOAD),
            "alignment": 4,
            "sha256": hashlib.sha256(PAYLOAD).hexdigest(),
        },
        "length_symbol": {"symbol": "testPayloadLength", "value": len(PAYLOAD)},
        "target": {
            "section": ".data",
            "address": "0x80001000",
            "size": len(PAYLOAD),
            "length_section": ".sdata",
            "length_address": "0x80002000",
            "length_size": 4,
            "relocations": [],
        },
        "donor": {
            "repo": "repos/donor",
            "path": "src/test/original_data.c",
            "commit": "1" * 40,
            "blob": "2" * 40,
        },
        "evidence": ["docs/evidence.md"],
    }


class OriginalDataPromotionTests(unittest.TestCase):
    def fixture(self) -> tuple[tempfile.TemporaryDirectory[str], Path, str, str]:
        temporary = tempfile.TemporaryDirectory()
        root = Path(temporary.name) / "repo"
        root.mkdir()
        run(root, "git", "init", "-q", "-b", "main")
        run(root, "git", "config", "user.name", "Test User")
        run(root, "git", "config", "user.email", "test@example.com")
        manifest = root / "config/recovery/original_data.json"
        manifest.parent.mkdir(parents=True)
        manifest.write_text(
            json.dumps({"schema_version": 1, "records": [record_for()]}, indent=2) + "\n",
            encoding="utf-8",
        )
        (root / "README.md").write_text("fixture\n", encoding="utf-8")
        run(root, "git", "add", ".")
        run(root, "git", "commit", "-qm", "Initial project")
        base = run(root, "git", "rev-parse", "HEAD")
        source = root / SOURCE_PATH
        source.parent.mkdir(parents=True)
        source.write_bytes(SOURCE_TEXT.encode("utf-8"))
        run(root, "git", "add", SOURCE_PATH)
        run(root, "git", "commit", "-qm", "Add authenticated original data")
        head = run(root, "git", "rev-parse", "HEAD")
        return temporary, root, base, head

    def test_plan_create_and_audit_exact_blob(self) -> None:
        temporary, root, base, head = self.fixture()
        promotion = Path(temporary.name) / "promotion"
        try:
            plan = plan_original_data(
                root,
                base_ref=base,
                source_ref=head,
                owner=OWNER,
                path=SOURCE_PATH,
                allow_unverified=True,
            )
            self.assertEqual(plan["parsed"]["length"], 4)
            result = create_original_data(
                root,
                base_ref=base,
                source_ref=head,
                owner=OWNER,
                path=SOURCE_PATH,
                branch="recovery/original-data-fixture",
                worktree=promotion,
                title="Recover authenticated original data",
                allow_unverified=True,
            )
            self.assertTrue(result["promotion"]["audit"]["clean_human_promotion"])
            self.assertEqual(
                run(root, "git", "rev-parse", f"{head}:{SOURCE_PATH}"),
                run(promotion, "git", "rev-parse", f"HEAD:{SOURCE_PATH}"),
            )
            audit = audit_original_data(
                promotion,
                base_ref=base,
                head_ref="HEAD",
                source_ref=head,
                owner=OWNER,
                path=SOURCE_PATH,
                policy_root=root,
            )
            self.assertEqual(audit["errors"], [])
        finally:
            subprocess.run(
                ["git", "worktree", "remove", "--force", str(promotion)],
                cwd=root,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                check=False,
            )
            subprocess.run(
                ["git", "branch", "-D", "recovery/original-data-fixture"],
                cwd=root,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                check=False,
            )
            temporary.cleanup()

    def test_record_requires_exact_non_wildcard_schema_and_donor(self) -> None:
        cases: list[tuple[str, dict[str, object]]] = []
        wildcard = record_for()
        wildcard["path"] = "src/test/*.c"
        cases.append(("wildcards", wildcard))
        donor_missing = record_for()
        del donor_missing["donor"]["blob"]  # type: ignore[index]
        cases.append(("keys differ", donor_missing))
        bad_commit = record_for()
        bad_commit["donor"]["commit"] = "short"  # type: ignore[index]
        cases.append(("full lowercase Git hash", bad_commit))
        donor_traversal = record_for()
        donor_traversal["donor"]["repo"] = "../outside"  # type: ignore[index]
        cases.append(("repository-relative", donor_traversal))
        bad_target = record_for()
        bad_target["target"]["size"] = 5  # type: ignore[index]
        cases.append(("target size", bad_target))
        bad_length = record_for()
        bad_length["length_symbol"]["value"] = 5  # type: ignore[index]
        cases.append(("length symbol", bad_length))
        for expected, record in cases:
            with self.subTest(expected=expected):
                with self.assertRaisesRegex(PromotionError, expected):
                    validate_record(record)

    def test_parser_rejects_symbol_hash_length_function_and_extra_declaration(self) -> None:
        valid = validate_record(record_for())
        array_start = SOURCE_TEXT.index("char testPayload")
        length_start = SOURCE_TEXT.index("u16 testPayloadLength")
        length_end = SOURCE_TEXT.index("\n", length_start) + 1
        swapped = (
            SOURCE_TEXT[:array_start]
            + SOURCE_TEXT[length_start:length_end]
            + SOURCE_TEXT[array_start:length_start]
            + SOURCE_TEXT[length_end:]
        )
        cases = [
            ("payload symbol", SOURCE_TEXT.replace("char testPayload", "char wrongPayload", 1), valid),
            (
                "payload SHA-256",
                SOURCE_TEXT,
                {**valid, "payload": {**valid["payload"], "sha256": "0" * 64}},
            ),
            (
                "payload length",
                SOURCE_TEXT,
                {
                    **valid,
                    "payload": {**valid["payload"], "length": 5},
                    "length_symbol": {**valid["length_symbol"], "value": 5},
                    "target": {**valid["target"], "size": 5},
                },
            ),
            ("extra declaration", SOURCE_TEXT + "int extra;\n", valid),
            ("extra declaration", SOURCE_TEXT + "int Function(void) { return 1; }\n", valid),
            ("must precede", swapped, valid),
        ]
        for expected, text, record in cases:
            with self.subTest(expected=expected):
                with self.assertRaisesRegex(PromotionError, expected):
                    parse_original_data(text, record)

    def test_plan_rejects_unlisted_path_and_source_hash_mismatch(self) -> None:
        temporary, root, base, head = self.fixture()
        try:
            with self.assertRaisesRegex(PromotionError, "found 0"):
                find_record(root, owner=OWNER, path="src/test/other.c")
            manifest_path = root / "config/recovery/original_data.json"
            payload = json.loads(manifest_path.read_text(encoding="utf-8"))
            payload["records"][0]["source_sha256"] = "0" * 64
            manifest_path.write_text(json.dumps(payload), encoding="utf-8")
            with self.assertRaisesRegex(PromotionError, "source SHA-256"):
                plan_original_data(
                    root,
                    base_ref=base,
                    source_ref=head,
                    owner=OWNER,
                    path=SOURCE_PATH,
                    allow_unverified=True,
                )
        finally:
            temporary.cleanup()

    def test_manifest_rejects_boolean_schema_and_alternate_policy_path(self) -> None:
        temporary, root, _, _ = self.fixture()
        try:
            manifest_path = root / "config/recovery/original_data.json"
            payload = json.loads(manifest_path.read_text(encoding="utf-8"))
            payload["schema_version"] = True
            manifest_path.write_text(json.dumps(payload), encoding="utf-8")
            with self.assertRaisesRegex(PromotionError, "schema_version"):
                load_manifest(root)
            with self.assertRaisesRegex(PromotionError, "must use"):
                load_manifest(root, "config/recovery/other.json")
        finally:
            temporary.cleanup()

    def test_audit_rejects_extra_paths_and_create_requires_recovery_branch(self) -> None:
        temporary, root, base, head = self.fixture()
        promotion = Path(temporary.name) / "malicious-promotion"
        try:
            with self.assertRaisesRegex(PromotionError, "must start with recovery"):
                create_original_data(
                    root,
                    base_ref=base,
                    source_ref=head,
                    owner=OWNER,
                    path=SOURCE_PATH,
                    branch="project/original-data-fixture",
                    worktree=promotion,
                    title="Recover authenticated original data",
                    allow_unverified=True,
                )

            run(
                root,
                "git",
                "worktree",
                "add",
                "-q",
                "-b",
                "recovery/original-data-extra-path",
                str(promotion),
                base,
            )
            source = promotion / SOURCE_PATH
            source.parent.mkdir(parents=True)
            source.write_bytes(SOURCE_TEXT.encode("utf-8"))
            (promotion / "AI_WORKSPACE.md").write_text("must not transfer\n", encoding="utf-8")
            run(promotion, "git", "add", SOURCE_PATH, "AI_WORKSPACE.md")
            run(promotion, "git", "commit", "-qm", "Recover authenticated data")
            audit = audit_original_data(
                promotion,
                base_ref=base,
                head_ref="HEAD",
                source_ref=head,
                owner=OWNER,
                path=SOURCE_PATH,
                policy_root=root,
            )
            self.assertFalse(audit["clean_human_promotion"])
            self.assertTrue(any("AI_WORKSPACE.md" in item for item in audit["errors"]))
        finally:
            subprocess.run(
                ["git", "worktree", "remove", "--force", str(promotion)],
                cwd=root,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                check=False,
            )
            subprocess.run(
                ["git", "branch", "-D", "recovery/original-data-extra-path"],
                cwd=root,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                check=False,
            )
            temporary.cleanup()

    def test_ordinary_c_promotion_still_rejects_original_data_hex(self) -> None:
        temporary, root, _, _ = self.fixture()
        try:
            findings = source_quality_errors(root, SOURCE_PATH, SOURCE_TEXT)
            self.assertTrue(any("raw_hex_literal" in item for item in findings))
        finally:
            temporary.cleanup()


if __name__ == "__main__":
    unittest.main()
