from __future__ import annotations

import hashlib
import importlib.util
import json
import os
import tempfile
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
TOOL = ROOT / "tools" / "player_gc27_historical_replay.py"
SPEC = importlib.util.spec_from_file_location("player_gc27_historical_replay", TOOL)
assert SPEC and SPEC.loader
replay = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(replay)


def write(path: Path, value: object) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    if isinstance(value, (dict, list)):
        path.write_text(json.dumps(value, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    else:
        path.write_bytes(bytes(value))


def desc(path: Path) -> dict[str, object]:
    return {"path": str(path), "sha256": hashlib.sha256(path.read_bytes()).hexdigest(), "size": path.stat().st_size}


def hooks() -> list[dict[str, object]]:
    return [replay._hook_row(name) for name in replay.HOOK_ORDER]


def replace_session(value: object, old: str, new: str) -> object:
    if isinstance(value, str):
        return value.replace(old, new)
    if isinstance(value, list):
        return [replace_session(item, old, new) for item in value]
    if isinstance(value, dict):
        return {key: replace_session(item, old, new) for key, item in value.items()}
    return value


class Fixture:
    def __init__(self, root: Path) -> None:
        self.root = root
        self.runtime = root / "runtime"
        self.session = "session-0123456789abcdef"
        runtime_files = {
            "backend-request.json": {
                "schema": replay.RUNTIME_REQUEST_SCHEMA,
                "session_id": "session-1111111111111111",
                "function": "MoveNumOMExec",
                "diagnostic_only": True,
                "board_admission": False,
                "exactness_claim": False,
                "sources": {"retained": {}, "v491": {}},
                "outputs": {},
                "argv": ["compiler", "-O0,p", "-c", "old.c", "-o", "old.o"],
                "custom_hook_union": {"count": 13, "rows": hooks()},
                "hooks": [],
                "request_sha256": "0" * 64,
            },
            "manifest.json": {"hooks": hooks(), "session_id": "session-1111111111111111"},
            "inputs/launch_movenum_capture.py": b"launcher\n",
            "backend/movenum-pcode-color-v523/physical_capture_backend_v523.py": b"backend\n",
            "backend/movenum-pcode-color-v523/player-phys-reg-trace-v490/gc27_phys_reg_adapter.py": b"adapter\n",
        }
        for relative, value in runtime_files.items():
            write(self.runtime / relative, value)
        files = {relative: desc(self.runtime / relative) | {"path": relative} for relative in runtime_files}
        receipt = {
            "schema": "player_gc27_current_source_capture_package/v2",
            "diagnostic_only": True,
            "authority_advanced": False,
            "files": files,
        }
        write(self.runtime / "package-receipt.json", receipt)
        self.lanes: dict[str, dict[str, object]] = {}
        for index, lane in enumerate(replay.LANES):
            lane_root = root / lane
            source = lane_root / "player.c"
            expected = lane_root / "player.o"
            template = lane_root / "template.json"
            historical = lane_root / "physical-reg.envelope.json"
            write(source, f"source-{lane}".encode())
            write(expected, f"object-{lane}".encode())
            function_hash = hashlib.sha256(f"function-{lane}".encode()).hexdigest()
            write(template, {
                "schema": "mwcc_source_spans/v2",
                "function": "MoveNumOMExec",
                "function_sha256": function_hash,
                "source": desc(source),
                "unsealed": True,
                "authority_advanced": False,
                "spans": [],
            })
            envelope = self.envelope(lane, expected, f"session-{index + 2:016x}")
            write(historical, envelope)
            self.lanes[lane] = {
                "label": lane,
                "source": desc(source),
                "function": {"name": "MoveNumOMExec", "sha256": function_hash},
                "expected_object": desc(expected),
                "source_span_template": desc(template),
                "historical_envelope": desc(historical),
            }
        self.request = root / "request.json"
        write(self.request, {
            "schema": replay.REQUEST_SCHEMA,
            "diagnostic_only": True,
            "authority_advanced": False,
            "board_admission": False,
            "exactness_claim": False,
            "session_id": self.session,
            "runtime": {"package_root": str(self.runtime), "package_receipt": desc(self.runtime / "package-receipt.json")},
            "lanes": self.lanes,
        })

    @staticmethod
    def envelope(lane: str, expected: Path, session: str) -> dict[str, object]:
        object_id = f"local-{session}-000000"
        varinfo_id = f"var-{session}-000000"
        return {
            "schema": "player_gc27_phys_reg_capture/v490",
            "backend": {"path": f"C:/run/{session}/backend.py", "sha256": "1" * 64},
            "board_admission": False,
            "capture_label": lane,
            "color_nodes": [],
            "diagnostic_only": True,
            "event_count": 1,
            "events": [{
                "sequence": 0,
                "hook_id": "physical_single_commit",
                "commit_kind": "single",
                "object_id": object_id,
                "varinfo_id": varinfo_id,
                "assignment": {"class": 4, "slot": 31},
                "varinfo": {"flags": 2, "noregister": 0, "rclass": 4, "reg": 31, "reg_hi": 0},
            }],
            "exactness_claim": False,
            "function": "MoveNumOMExec",
            "object": desc(expected) | {"path": f"C:/run/{session}/player.o"},
            "pcode_color_rows": [{
                "sequence": 0,
                "confirmed": True,
                "has_object": True,
                "object_id": object_id,
                "object_kind": "local",
                "source_name": "modelNo",
                "pcode_token": "pcode-000000",
                "ig_token": "ig-000000",
                "register_class": 4,
                "final_color": 31,
            }],
            "physical_register_range": [0, 31],
            "request": {"path": f"C:/run/{session}/request.json", "sha256": "2" * 64},
            "semantic_gate": {
                "status": "SEMANTIC_OBJECT_EXACT",
                "function_count": 167,
                "semantic_sections": [".text", ".data"],
                "actual_object": desc(expected) | {"path": f"C:/run/{session}/player.o"},
                "expected_object": desc(expected),
                "report": {"path": f"C:/run/{session}/semantic.json", "sha256": "3" * 64, "size": 1},
            },
            "session_id": session,
            "source_bindings": [{"object_id": object_id, "varinfo_id": varinfo_id, "object_kind": "local", "source_name": "modelNo"}],
            "status": "CAPTURED_PHYSICAL_REGISTERS",
            "varinfo_layout": {"reg": {"encoding": "s16le", "offset": 38}},
            "virtual_register_rejected": [32, 66],
        }

    def prepare(self) -> Path:
        output = self.root / "prepared"
        replay.prepare(self.request, output)
        return output

    def populate_actual(self, package: Path) -> None:
        for lane in replay.LANES:
            baseline = json.loads(Path(self.lanes[lane]["historical_envelope"]["path"]).read_text(encoding="utf-8"))
            actual = replace_session(baseline, baseline["session_id"], self.session)
            write(package / "backend-output" / lane / "physical-reg.envelope.json", actual)


class HistoricalReplayTests(unittest.TestCase):
    def fixture(self) -> tuple[tempfile.TemporaryDirectory[str], Fixture]:
        temporary = tempfile.TemporaryDirectory()
        return temporary, Fixture(Path(temporary.name))

    def test_prepare_binds_two_distinct_lanes_and_exact_hook_runtime(self) -> None:
        temporary, fixture = self.fixture()
        with temporary:
            package = fixture.prepare()
            receipt = json.loads((package / "package-receipt.json").read_text(encoding="utf-8"))
            request = json.loads((package / "backend-request.json").read_text(encoding="utf-8"))
            plan = json.loads((package / "historical-replay-plan.json").read_text(encoding="utf-8"))
            self.assertEqual(receipt["status"], "READY_NOT_EXECUTED")
            self.assertFalse(receipt["compiler_run"])
            self.assertFalse(receipt["capture_run"])
            self.assertEqual(request["session_id"], fixture.session)
            self.assertNotEqual(request["sources"]["retained"]["sha256"], request["sources"]["v491"]["sha256"])
            self.assertEqual(plan["one_shot"]["argv"][-1], "--execute")
            self.assertFalse((package / "backend-output").exists())

    def test_prepare_preserves_distinct_historical_semantic_reference(self) -> None:
        temporary, fixture = self.fixture()
        with temporary:
            semantic_reference = fixture.root / "retained-semantic-reference.o"
            write(semantic_reference, b"semantic-reference")
            envelope_path = Path(fixture.lanes["retained"]["historical_envelope"]["path"])
            envelope = json.loads(envelope_path.read_text(encoding="utf-8"))
            envelope["semantic_gate"]["expected_object"] = desc(semantic_reference)
            write(envelope_path, envelope)
            fixture.lanes["retained"]["historical_envelope"] = desc(envelope_path)
            request = json.loads(fixture.request.read_text(encoding="utf-8"))
            request["lanes"]["retained"]["historical_envelope"] = desc(envelope_path)
            write(fixture.request, request)

            package = fixture.prepare()
            replay_request = json.loads((package / "backend-request.json").read_text(encoding="utf-8"))
            plan = json.loads((package / "historical-replay-plan.json").read_text(encoding="utf-8"))
            self.assertEqual(
                replay_request["sources"]["retained"]["expected_object"]["sha256"],
                desc(semantic_reference)["sha256"],
            )
            self.assertEqual(
                plan["lanes"]["retained"]["expected_object"]["sha256"],
                fixture.lanes["retained"]["expected_object"]["sha256"],
            )
            self.assertEqual(
                plan["lanes"]["retained"]["semantic_expected_object"]["sha256"],
                desc(semantic_reference)["sha256"],
            )
            fixture.populate_actual(package)
            result = replay.compare(package, package / "comparison.json")
            self.assertEqual(result["status"], "MATCH")

    def test_prepare_rejects_historical_semantic_reference_drift(self) -> None:
        temporary, fixture = self.fixture()
        with temporary:
            semantic_reference = fixture.root / "retained-semantic-reference.o"
            write(semantic_reference, b"semantic-reference")
            envelope_path = Path(fixture.lanes["retained"]["historical_envelope"]["path"])
            envelope = json.loads(envelope_path.read_text(encoding="utf-8"))
            envelope["semantic_gate"]["expected_object"] = desc(semantic_reference)
            write(envelope_path, envelope)
            request = json.loads(fixture.request.read_text(encoding="utf-8"))
            request["lanes"]["retained"]["historical_envelope"] = desc(envelope_path)
            write(fixture.request, request)
            write(semantic_reference, b"drift")
            with self.assertRaisesRegex(replay.ReplayError, "semantic expected object descriptor drift"):
                fixture.prepare()

    def test_prepare_seals_absolute_paths_and_compare_works_cross_cwd(self) -> None:
        temporary, fixture = self.fixture()
        with temporary:
            caller = fixture.root / "caller"
            caller.mkdir()
            previous = Path.cwd()
            try:
                os.chdir(caller)
                replay.prepare(Path("..") / "request.json", Path("..") / "prepared")
            finally:
                os.chdir(previous)
            package = fixture.root / "prepared"
            plan = json.loads((package / "historical-replay-plan.json").read_text(encoding="utf-8"))
            receipt = json.loads((package / "package-receipt.json").read_text(encoding="utf-8"))
            self.assertEqual(Path(plan["package_root"]), package.resolve())
            self.assertEqual(Path(receipt["package_root"]), package.resolve())
            self.assertTrue(Path(plan["request"]["path"]).is_absolute())
            for value in plan["one_shot"]["argv"]:
                if isinstance(value, str) and ("backend-request" in value or "backend-output" in value or "live-execution-plan" in value):
                    self.assertTrue(Path(value).is_absolute())
            fixture.populate_actual(package)
            try:
                os.chdir(caller)
                result = replay.compare(Path("..") / "prepared", package / "comparison.json")
            finally:
                os.chdir(previous)
            self.assertEqual(result["status"], "MATCH")

    def test_compare_rejects_relative_sealed_package_root(self) -> None:
        temporary, fixture = self.fixture()
        with temporary:
            package = fixture.prepare()
            fixture.populate_actual(package)
            plan_path = package / "historical-replay-plan.json"
            receipt_path = package / "package-receipt.json"
            plan = json.loads(plan_path.read_text(encoding="utf-8"))
            plan["package_root"] = "prepared"
            write(plan_path, plan)
            receipt = json.loads(receipt_path.read_text(encoding="utf-8"))
            receipt["plan_sha256"] = hashlib.sha256(replay._canonical(plan)).hexdigest()
            write(receipt_path, receipt)
            with self.assertRaisesRegex(replay.ReplayError, "plan package_root is not absolute"):
                replay.compare(package, package / "comparison.json")

    def test_prepare_rejects_noncanonical_session(self) -> None:
        temporary, fixture = self.fixture()
        with temporary:
            value = json.loads(fixture.request.read_text())
            value["session_id"] = "session-" + "a" * 24
            write(fixture.request, value)
            with self.assertRaisesRegex(replay.ReplayError, "session_id"):
                fixture.prepare()

    def test_prepare_rejects_same_source_identity(self) -> None:
        temporary, fixture = self.fixture()
        with temporary:
            value = json.loads(fixture.request.read_text())
            value["lanes"]["v491"]["source"] = value["lanes"]["retained"]["source"]
            write(fixture.request, value)
            with self.assertRaisesRegex(replay.ReplayError, "template source binding|source identities"):
                fixture.prepare()

    def test_prepare_rejects_same_function_identity(self) -> None:
        temporary, fixture = self.fixture()
        with temporary:
            value = json.loads(fixture.request.read_text())
            value["lanes"]["v491"]["function"] = value["lanes"]["retained"]["function"]
            template_path = Path(value["lanes"]["v491"]["source_span_template"]["path"])
            template = json.loads(template_path.read_text())
            template["function_sha256"] = value["lanes"]["retained"]["function"]["sha256"]
            write(template_path, template)
            value["lanes"]["v491"]["source_span_template"] = desc(template_path)
            write(fixture.request, value)
            with self.assertRaisesRegex(replay.ReplayError, "function identities"):
                fixture.prepare()

    def test_prepare_rejects_twelve_hook_runtime(self) -> None:
        temporary, fixture = self.fixture()
        with temporary:
            manifest = fixture.runtime / "manifest.json"
            value = json.loads(manifest.read_text())
            value["hooks"] = value["hooks"][:-1]
            write(manifest, value)
            receipt_path = fixture.runtime / "package-receipt.json"
            receipt = json.loads(receipt_path.read_text())
            receipt["files"]["manifest.json"] = desc(manifest) | {"path": "manifest.json"}
            write(receipt_path, receipt)
            request = json.loads(fixture.request.read_text())
            request["runtime"]["package_receipt"] = desc(receipt_path)
            write(fixture.request, request)
            with self.assertRaisesRegex(replay.ReplayError, "13-hook"):
                fixture.prepare()

    def test_prepare_rejects_descriptor_drift_before_output(self) -> None:
        temporary, fixture = self.fixture()
        with temporary:
            Path(fixture.lanes["retained"]["source"]["path"]).write_bytes(b"drift")
            output = fixture.root / "prepared"
            with self.assertRaisesRegex(replay.ReplayError, "descriptor drift"):
                replay.prepare(fixture.request, output)
            self.assertFalse(output.exists())

    def test_prepare_never_overwrites(self) -> None:
        temporary, fixture = self.fixture()
        with temporary:
            fixture.prepare()
            with self.assertRaisesRegex(replay.ReplayError, "overwrite"):
                fixture.prepare()

    def test_compare_ignores_only_run_local_session_paths(self) -> None:
        temporary, fixture = self.fixture()
        with temporary:
            package = fixture.prepare()
            fixture.populate_actual(package)
            result = replay.compare(package, package / "comparison.json")
            self.assertEqual(result["status"], "MATCH")
            self.assertEqual(result["lanes"]["retained"]["event_count"], 1)
            self.assertEqual(result["lanes"]["retained"]["pcode_color_row_count"], 1)

    def test_compare_detects_register_assignment_change(self) -> None:
        temporary, fixture = self.fixture()
        with temporary:
            package = fixture.prepare()
            fixture.populate_actual(package)
            path = package / "backend-output" / "retained" / "physical-reg.envelope.json"
            value = json.loads(path.read_text())
            value["events"][0]["assignment"]["slot"] = 30
            write(path, value)
            result = replay.compare(package, package / "comparison.json")
            self.assertEqual(result["status"], "MISMATCH")
            self.assertTrue(any("assignment.slot" in row for row in result["lanes"]["retained"]["differences"]))

    def test_compare_detects_source_name_change(self) -> None:
        temporary, fixture = self.fixture()
        with temporary:
            package = fixture.prepare()
            fixture.populate_actual(package)
            path = package / "backend-output" / "v491" / "physical-reg.envelope.json"
            value = json.loads(path.read_text())
            value["source_bindings"][0]["source_name"] = "other"
            value["pcode_color_rows"][0]["source_name"] = "other"
            write(path, value)
            result = replay.compare(package, package / "comparison.json")
            self.assertEqual(result["lanes"]["v491"]["status"], "MISMATCH")

    def test_compare_detects_pcode_row_count_change(self) -> None:
        temporary, fixture = self.fixture()
        with temporary:
            package = fixture.prepare()
            fixture.populate_actual(package)
            path = package / "backend-output" / "retained" / "physical-reg.envelope.json"
            value = json.loads(path.read_text())
            value["pcode_color_rows"].append(dict(value["pcode_color_rows"][0], sequence=1))
            write(path, value)
            result = replay.compare(package, package / "comparison.json")
            self.assertEqual(result["status"], "MISMATCH")
            self.assertTrue(any("pcode_color_row_count" in row or "pcode_color_rows" in row for row in result["lanes"]["retained"]["differences"]))

    def test_compare_detects_semantic_gate_change(self) -> None:
        temporary, fixture = self.fixture()
        with temporary:
            package = fixture.prepare()
            fixture.populate_actual(package)
            path = package / "backend-output" / "v491" / "physical-reg.envelope.json"
            value = json.loads(path.read_text())
            value["semantic_gate"]["function_count"] = 166
            write(path, value)
            result = replay.compare(package, package / "comparison.json")
            self.assertEqual(result["status"], "MISMATCH")

    def test_compare_rejects_missing_source_binding(self) -> None:
        temporary, fixture = self.fixture()
        with temporary:
            package = fixture.prepare()
            fixture.populate_actual(package)
            path = package / "backend-output" / "retained" / "physical-reg.envelope.json"
            value = json.loads(path.read_text())
            value["source_bindings"] = []
            write(path, value)
            with self.assertRaisesRegex(replay.ReplayError, "inventory|source binding"):
                replay.compare(package, package / "comparison.json")

    def test_compare_never_overwrites_receipt(self) -> None:
        temporary, fixture = self.fixture()
        with temporary:
            package = fixture.prepare()
            fixture.populate_actual(package)
            output = package / "comparison.json"
            replay.compare(package, output)
            with self.assertRaises(FileExistsError):
                replay.compare(package, output)


if __name__ == "__main__":
    unittest.main()
