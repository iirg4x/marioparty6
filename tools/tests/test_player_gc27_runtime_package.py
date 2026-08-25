from __future__ import annotations

import hashlib
import importlib.util
import json
import os
from pathlib import Path
import sys
import tempfile
import unittest


MODULE_PATH = Path(__file__).resolve().parents[1] / "player_gc27_runtime_package.py"
SPEC = importlib.util.spec_from_file_location("player_gc27_runtime_package", MODULE_PATH)
assert SPEC is not None and SPEC.loader is not None
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)


class PackageMaterializerTests(unittest.TestCase):
    def setUp(self) -> None:
        self.original_profiles = json.loads(json.dumps(MODULE.FUNCTION_PROFILES))
        self.temporary = tempfile.TemporaryDirectory()
        self.root = Path(self.temporary.name)
        self.forensic = self.root / "forensic"
        self.external = self.root / "external"
        self.current_tool = self.root / "worktree" / "tools" / "capsule_same_session_capture.py"
        self.source = self.root / "immutable" / "player.c"
        self.template = self.root / "templates" / "MoveNumOMExec.source-spans.unsealed.json"
        self.template_manifest = self.template.parent / "manifest.json"
        self.output = self.root / "materialized"
        self._make_fixture()

    def tearDown(self) -> None:
        MODULE.FUNCTION_PROFILES.clear()
        MODULE.FUNCTION_PROFILES.update(self.original_profiles)
        self.temporary.cleanup()

    def _write(self, path: Path, data: bytes | str) -> Path:
        path.parent.mkdir(parents=True, exist_ok=True)
        if isinstance(data, str):
            data = data.encode("utf-8")
        path.write_bytes(data)
        return path

    def _sha(self, path: Path) -> str:
        return hashlib.sha256(path.read_bytes()).hexdigest()

    def _desc(self, path: Path, *, display: str | None = None) -> dict[str, object]:
        return {"path": display if display is not None else str(path), "sha256": self._sha(path), "size": path.stat().st_size}

    def _make_fixture(self) -> None:
        self.forensic.mkdir(parents=True)
        self.external.mkdir(parents=True)
        old_tool = self._write(self.external / "old-same-session.py", b"old tool\n")
        self._write(self.current_tool, b"current same-session tool\n")
        compiler = self._write(self.external / "mwcceppc.exe", b"compiler\n")
        wrapper = self._write(self.external / "sjiswrap.exe", b"wrapper\n")
        transport = self._write(self.external / "transport.py", b"transport\n")
        authority = self._write(self.external / "authority.md", b"authenticated authority\n")
        target = self._write(self.external / "target.bin", b"target\n")
        self.expected_object = self._write(self.external / "player.o", b"object\n")
        source_bytes = b"/* immutable source */\nvoid MoveNumOMExec(void) {}\n"
        self._write(self.source, source_bytes)
        old_hash = self._sha(old_tool)
        function_start = source_bytes.index(b"void MoveNumOMExec")
        function_end = len(source_bytes)
        body_start = source_bytes.index(b"{}")
        function_hash = hashlib.sha256(source_bytes[function_start:function_end]).hexdigest()
        body_hash = hashlib.sha256(source_bytes[body_start:function_end]).hexdigest()
        source_hash = self._sha(self.source)

        source_copy = self._write(self.forensic / "inputs" / "player.c", self.source.read_bytes())
        old_template = {
            "schema": "mwcc_source_span_bindings/v1",
            "template_schema": "player_source_span_template/v1",
            "function": "MoveNumOMExec",
            "function_sha256": function_hash,
            "session_id": "<CAPTURE_SESSION_ID>",
            "source": {"path": str(self.source), "size": self.source.stat().st_size, "sha256": source_hash},
            "source_anchor": {
                "byte_start": function_start,
                "byte_end": function_end,
                "line_start": 2,
                "line_end": 2,
                "text_sha256": function_hash,
                "body_byte_start": body_start,
                "body_byte_end": function_end,
                "body_line_start": 2,
                "body_line_end": 2,
                "body_sha256": body_hash,
            },
            "spans": [{"object_token": "<OBJECT_TOKEN_CAPTURE_LOCAL>", "identity": "<SOURCE_IDENTITY>", "byte_start": function_start, "byte_end": function_start + 4, "line_start": 2, "line_end": 2, "text_sha256": hashlib.sha256(b"void").hexdigest()}],
            "diagnostic_only": True,
            "board_admission": False,
            "exactness_claim": False,
            "authority_advanced": False,
            "unsealed": True,
        }
        old_template_path = self.forensic / "inputs" / "MoveNumOMExec.source-spans.unsealed.json"
        self._write(old_template_path, json.dumps(old_template, sort_keys=True, indent=2))
        corrected = dict(old_template)
        corrected["spans"] = [{"object_token": "<OBJECT_TOKEN_CAPTURE_LOCAL>", "identity": "pos/write", "byte_start": function_start, "byte_end": function_start + 4, "line_start": 2, "line_end": 2, "text_sha256": hashlib.sha256(b"void").hexdigest()}]
        self._write(self.template, json.dumps(corrected, sort_keys=True, indent=2))
        self._write(self.template_manifest, json.dumps({"schema": "player_source_span_templates/v1", "functions": [{"function": "MoveNumOMExec", "template_sha256": self._sha(self.template), "span_count": 1, "function_sha256": function_hash, "body_sha256": body_hash}]}, sort_keys=True, indent=2))
        old_template_manifest = self._write(self.forensic / "inputs" / "source-span-template-manifest.json", b"forensic template manifest\n")
        launcher = self._write(
            self.forensic / "inputs" / "launch_movenum_capture.py",
            b'from pathlib import Path\nROOT = Path(r"C:\\Users\\Anony\\.codex\\mp6-wt-player-full-closure-v459")\nCOMPILER_LANE_LOCK = ROOT / "build/.compiler-lane.lock"\n',
        )
        backend = self._write(
            self.forensic / "backend" / "movenum-pcode-color-v523" / "physical_capture_backend_v523.py",
            (
                "import hashlib\n"
                "class BackendError(RuntimeError):\n"
                "    pass\n"
                "OLD_TOOL_HASH=\"" + old_hash + "\"\n"
                "FUNCTION = \"MoveNumOMExec\"\n"
                "def _central_manifest(request, label, source_descriptor):\n"
                + MODULE.BACKEND_DERIVED_SESSION
                + "    return session\n"
            ).encode(),
        )
        adapter = self._write(
            self.forensic / "backend" / "movenum-pcode-color-v523" / "player-phys-reg-trace-v490" / "gc27_phys_reg_adapter.py",
            (
                "from dataclasses import dataclass\n"
                "import hashlib\n"
                "import json\n"
                "OLD_TOOL_HASH=\"" + old_hash + "\"\n"
                "@dataclass\n"
                "class CaptureBinding:\n"
                "    expected_sha256: str\n"
                "def validate_request(request):\n"
                "    if request.get(\"function\") != \"MoveNumOMExec\":\n"
                "        raise ValidationError(\"request is not bound to MoveNumOMExec\")\n"
                "    capture_tool = request.get('same_session_capture')\n"
                "    binding = CaptureBinding(capture_tool.get('expected_sha256'))\n"
                "    if binding.expected_sha256 != capture_tool.get('sha256'):\n"
                "        raise ValueError('same-session capture expected_sha256 mismatch')\n"
                "    unsigned = dict(request)\n"
                "    unsigned.pop('request_sha256', None)\n"
                "    encoded = json.dumps(unsigned, ensure_ascii=True, sort_keys=True, separators=(',', ':')).encode('utf-8')\n"
                "    if request.get('request_sha256') != hashlib.sha256(encoded).hexdigest():\n"
                "        raise ValueError('request_sha256 does not bind the request contents')\n"
            ).encode(),
        )
        old_root = str(self.forensic)
        old_output = str(self.forensic / "backend-output")
        session = "session-forensic"
        hooks = [MODULE._hook_row(name) for name in MODULE.HOOK_ORDER]
        manifest = {
            "schema": MODULE.MANIFEST_SCHEMA,
            "session_id": session,
            "function": "MoveNumOMExec",
            "function_sha256": function_hash,
            "cwd": r"C:\Users\Anony\.codex\mp6-wt-player-full-closure-v459",
            "source": self._desc(source_copy, display="inputs/player.c"),
            "debugger": self._desc(old_tool),
            "hooks": hooks,
            "argv": [str(wrapper), str(compiler), "-c", str(self.forensic / "inputs" / "player.c"), "-o", old_output + "\\movenum.o"],
            "authority": self._desc(authority),
            "compiler": self._desc(compiler),
            "diagnostic_only": True,
            "board_admission": False,
            "exactness_claim": False,
        }
        request = {
            "schema": MODULE.REQUEST_SCHEMA,
            "session_id": session,
            "function": "MoveNumOMExec",
            "function_sha256": function_hash,
            "argv": list(manifest["argv"]),
            "cwd": r"C:\Users\Anony\.codex\mp6-wt-player-full-closure-v459",
            "debugger": self._desc(old_tool),
            "transport": self._desc(transport),
            "adapter": {**self._desc(adapter, display="backend/movenum-pcode-color-v523/player-phys-reg-trace-v490/gc27_phys_reg_adapter.py"), "schema": "player_gc27_phys_reg_adapter/v490"},
            "backend": {**self._desc(backend, display="backend/movenum-pcode-color-v523/physical_capture_backend_v523.py"), "schema": "player_gc27_phys_reg_backend/v523"},
            "sources": {"retained": {"path": "inputs/player.c", "sha256": source_hash, "size": self.source.stat().st_size, "expected_object": self._desc(self.expected_object)}, "v491": {"path": "inputs/player.c", "sha256": source_hash, "size": self.source.stat().st_size, "expected_object": self._desc(self.expected_object)}},
            "target": self._desc(target),
            "hooks": [{"id": name, "address": MODULE.EXPECTED_HOOKS[name][0], "prefix": MODULE.EXPECTED_HOOKS[name][1]} for name in MODULE.PHYSICAL_HOOK_ORDER],
            "custom_hook_union": {"count": 13, "rows": hooks, "machine_emit": MODULE._hook_row("gc27_machine_emit")},
            "same_session": {"session_id": session},
            "diagnostic_only": True,
            "board_admission": False,
            "exactness_claim": False,
            "outputs": {"run_output": old_output},
        }
        receipt = {
            "schema": "player_gc27_current_source_capture_package/v1",
            "status": "READY_NOT_EXECUTED",
            "diagnostic_only": True,
            "board_admission": False,
            "exactness_claim": False,
            "compiler_or_live_capture_run": False,
            "session_id": session,
            "function": "MoveNumOMExec",
            "function_sha256": function_hash,
            "source": self._desc(source_copy, display="inputs/player.c"),
            "template": self._desc(old_template_path, display="inputs/MoveNumOMExec.source-spans.unsealed.json"),
            "template_manifest": self._desc(old_template_manifest, display="inputs/source-span-template-manifest.json"),
            "launcher": self._desc(launcher, display="inputs/launch_movenum_capture.py"),
            "backend": self._desc(backend, display="backend/movenum-pcode-color-v523/physical_capture_backend_v523.py"),
            "adapter": self._desc(adapter, display="backend/movenum-pcode-color-v523/player-phys-reg-trace-v490/gc27_phys_reg_adapter.py"),
            "manifest": {"path": "manifest.json"},
            "request": {"path": "backend-request.json"},
            "target": self._desc(target),
            "expected_object": self._desc(self.expected_object),
            "debugger": self._desc(old_tool),
        }
        self._write(self.forensic / "manifest.json", json.dumps(manifest, sort_keys=True, indent=2))
        self._write(self.forensic / "backend-request.json", json.dumps(request, sort_keys=True, indent=2))
        self._write(self.forensic / "trust-root.json", json.dumps({"session_id": session, "diagnostic_only": True, "request": {"path": "backend-request.json"}}, sort_keys=True, indent=2))
        self._write(self.forensic / "preflight-trust-root.json", json.dumps({"session_id": session, "diagnostic_only": True, "request": {"path": "backend-request.json"}}, sort_keys=True, indent=2))
        self._write(self.forensic / "package-receipt.json", json.dumps(receipt, sort_keys=True, indent=2))
        MODULE.FUNCTION_PROFILES["MoveNumOMExec"] = {
            "source_sha256": source_hash,
            "source_size": self.source.stat().st_size,
            "object_sha256": self._sha(self.expected_object),
            "object_size": self.expected_object.stat().st_size,
            "target_sha256": self._sha(target),
            "target_size": target.stat().st_size,
            "function_sha256": function_hash,
            "body_sha256": body_hash,
            "template_input_sha256": self._sha(self.template),
            "template_span_count": 1,
            "template_source_sha256": source_hash,
            "byte_delta": 0,
            "line_delta": 0,
            "output_object_name": "movenum.o",
        }

    def _materialize(self) -> Path:
        MODULE.materialize_package(self.forensic, self.output, self.current_tool, self.template, template_manifest=self.template_manifest, session_id="session-1111111111111111")
        return self.output

    def _rewrite_forensic_launcher_for_test(self, text: str) -> None:
        path = self.forensic / "inputs" / "launch_movenum_capture.py"
        path.write_text(text)
        receipt_path = self.forensic / "package-receipt.json"
        receipt = json.loads(receipt_path.read_text())
        receipt["launcher"]["sha256"] = self._sha(path)
        receipt["launcher"]["size"] = path.stat().st_size
        receipt_path.write_text(json.dumps(receipt, sort_keys=True, indent=2))

    def _install_radius_profile_fixture(self) -> tuple[Path, Path, Path]:
        current_source = self._write(
            self.root / "radius" / "current-player.c",
            b"head\nint GetBiriQEffectRadius(void) { return 1; }\n",
        )
        old_source = self._write(
            self.root / "radius" / "old-player.c",
            b"extra\n" + current_source.read_bytes(),
        )
        old_bytes = old_source.read_bytes()
        current_bytes = current_source.read_bytes()
        old_start = old_bytes.index(b"int GetBiriQEffectRadius")
        old_end = len(old_bytes)
        old_body = old_bytes.index(b"{ return")
        current_start = current_bytes.index(b"int GetBiriQEffectRadius")
        current_body = current_bytes.index(b"{ return")
        function_hash = hashlib.sha256(current_bytes[current_start:]).hexdigest()
        body_hash = hashlib.sha256(current_bytes[current_body:]).hexdigest()
        radius_template = {
            "schema": "mwcc_source_span_bindings/v1",
            "template_schema": "player_source_span_template/v1",
            "function": "GetBiriQEffectRadius",
            "function_sha256": function_hash,
            "session_id": "<CAPTURE_SESSION_ID>",
            "source": self._desc(old_source),
            "source_anchor": {
                "byte_start": old_start,
                "byte_end": old_end,
                "line_start": 2,
                "line_end": 2,
                "text_sha256": function_hash,
                "body_byte_start": old_body,
                "body_byte_end": old_end,
                "body_line_start": 2,
                "body_line_end": 2,
                "body_sha256": body_hash,
            },
            "spans": [{
                "object_token": "<CAPTURE_OBJECT_TOKEN_RADIUS>",
                "identity": "radius",
                "role": "declaration",
                "byte_start": old_start,
                "byte_end": old_start + 3,
                "line_start": 2,
                "line_end": 2,
                "text_sha256": hashlib.sha256(b"int").hexdigest(),
            }],
            "diagnostic_only": True,
            "board_admission": False,
            "exactness_claim": False,
            "authority_advanced": False,
            "unsealed": True,
        }
        template = self._write(
            self.root / "radius" / "GetBiriQEffectRadius.stack-interval.source-spans.unsealed.json",
            json.dumps(radius_template, sort_keys=True, indent=2),
        )
        manifest = self._write(
            self.root / "radius" / "manifest.json",
            json.dumps({
                "schema": "player_source_span_templates/v1",
                "source": self._desc(old_source),
                "functions": [{
                    "function": "GetBiriQEffectRadius",
                    "template": template.name,
                    "template_sha256": self._sha(template),
                    "span_count": 1,
                    "function_sha256": function_hash,
                    "body_sha256": body_hash,
                }],
                "diagnostic_only": True,
                "authority_advanced": False,
            }, sort_keys=True, indent=2),
        )
        MODULE.FUNCTION_PROFILES["GetBiriQEffectRadius"] = {
            "source_sha256": self._sha(current_source),
            "source_size": current_source.stat().st_size,
            "object_sha256": self._sha(self.expected_object),
            "object_size": self.expected_object.stat().st_size,
            "target_sha256": self._sha(self.external / "target.bin"),
            "target_size": (self.external / "target.bin").stat().st_size,
            "function_sha256": function_hash,
            "body_sha256": body_hash,
            "template_input_sha256": self._sha(template),
            "template_span_count": 1,
            "template_source_sha256": self._sha(old_source),
            "byte_delta": -len(b"extra\n"),
            "line_delta": -1,
            "output_object_name": "radius.o",
        }
        return current_source, template, manifest

    def _materialize_radius(self) -> tuple[Path, Path, Path]:
        current_source, template, manifest = self._install_radius_profile_fixture()
        MODULE.materialize_package(
            self.forensic,
            self.output,
            self.current_tool,
            template,
            source=current_source,
            template_manifest=manifest,
            session_id="session-1212121212121212",
            function_profile="GetBiriQEffectRadius",
        )
        return current_source, template, manifest

    def test_materialize_rebases_and_validates_without_execution(self) -> None:
        receipt = MODULE.materialize_package(self.forensic, self.output, self.current_tool, self.template, template_manifest=self.template_manifest, session_id="session-1111111111111111")
        self.assertEqual(receipt["status"], "READY_NOT_EXECUTED")
        self.assertFalse((self.output / "backend-output").exists())
        self.assertFalse((self.output / "live-execution-plan.json").exists())
        self.assertFalse((self.output / "inputs" / "launch-plan.json").exists())
        request = json.loads((self.output / "backend-request.json").read_text())
        self.assertEqual(request["session_id"], "session-1111111111111111")
        self.assertEqual(
            request["hooks"],
            [
                {
                    "id": name,
                    "address": MODULE.EXPECTED_HOOKS[name][0],
                    "address_hex": f"0x{MODULE.EXPECTED_HOOKS[name][0]:08x}",
                    "prefix": MODULE.EXPECTED_HOOKS[name][1],
                    "commit_kind": "pair" if name == "physical_pair_commit" else "single" if name == "physical_single_commit" else "precolored",
                    "optional": name == "precolored_commit",
                }
                for name in MODULE.PHYSICAL_HOOK_ORDER
            ],
        )
        self.assertEqual(request["sources"]["retained"]["path"], str(self.source))
        self.assertIn(str(self.source), request["argv"])
        one_shot = receipt["one_shot"]
        expected_interpreter = str(Path(MODULE.sys.executable).resolve())
        expected_plan = str(self.output / "live-execution-plan.json")
        self.assertEqual(one_shot["interpreter"], expected_interpreter)
        self.assertEqual(one_shot["argv"][0], expected_interpreter)
        self.assertIn("--plan-output", one_shot["argv"])
        self.assertEqual(one_shot["plan_output"], expected_plan)
        self.assertEqual(one_shot["argv"][one_shot["argv"].index("--plan-output") + 1], expected_plan)
        self.assertEqual(MODULE.validate_package(self.output)["request_sha256"], receipt["request_sha256"])
        copied_backend = self.output / "backend" / "movenum-pcode-color-v523" / "physical_capture_backend_v523.py"
        self.assertEqual(copied_backend.read_text().count(self._sha(self.current_tool)), 1)
        self.assertEqual(copied_backend.read_text().count(MODULE.BACKEND_REQUEST_SESSION), 1)
        self.assertNotIn(MODULE.BACKEND_DERIVED_SESSION, copied_backend.read_text())
        launcher = (self.output / "inputs" / "launch_movenum_capture.py").read_text()
        self.assertIn(f'ROOT = Path(r"{self.current_tool.parent.parent}")', launcher)
        self.assertIn('COMPILER_LANE_LOCK = ROOT / "build/.compiler-lane.lock"', launcher)
        self.assertNotIn("mp6-wt-player-full-closure-v459", launcher)

    def test_radius_profile_rebases_source_and_rewrites_both_producer_guards(self) -> None:
        current_source, _, _ = self._materialize_radius()
        receipt = MODULE.validate_package(self.output)
        self.assertEqual(receipt["function"], "GetBiriQEffectRadius")
        self.assertEqual(receipt["template"]["span_count"], 1)
        request = json.loads((self.output / "backend-request.json").read_text())
        self.assertEqual(request["function_profile"], "GetBiriQEffectRadius")
        self.assertIn(str(current_source), request["argv"])
        self.assertIn(str(self.output / "backend-output" / "radius.o"), request["argv"])
        prepared = json.loads((self.output / "inputs" / "GetBiriQEffectRadius.source-spans.unsealed.json").read_text())
        self.assertEqual(prepared["source"]["sha256"], self._sha(current_source))
        self.assertEqual(prepared["source_anchor"]["line_start"], 1)
        self.assertEqual(current_source.read_bytes()[prepared["spans"][0]["byte_start"]:prepared["spans"][0]["byte_end"]], b"int")
        backend = (self.output / "backend/movenum-pcode-color-v523/physical_capture_backend_v523.py").read_text()
        adapter = (self.output / "backend/movenum-pcode-color-v523/player-phys-reg-trace-v490/gc27_phys_reg_adapter.py").read_text()
        self.assertIn('FUNCTION = "GetBiriQEffectRadius"', backend)
        self.assertNotIn('FUNCTION = "MoveNumOMExec"', backend)
        self.assertIn('request.get("function") != "GetBiriQEffectRadius"', adapter)
        self.assertNotIn('request.get("function") != "MoveNumOMExec"', adapter)

    def test_radius_profile_rejects_wrong_function_template_source_and_object(self) -> None:
        current_source, template, manifest = self._install_radius_profile_fixture()
        with self.subTest("wrong function"):
            with self.assertRaisesRegex(MODULE.PackageError, "does not match requested profile"):
                MODULE.materialize_package(
                    self.forensic, self.output, self.current_tool, template,
                    source=current_source, template_manifest=manifest,
                    function_profile="MoveNumOMExec",
                )
        with self.subTest("wrong template"):
            value = json.loads(template.read_text())
            value["spans"][0]["identity"] = "tampered"
            template.write_text(json.dumps(value, sort_keys=True))
            with self.assertRaisesRegex(MODULE.PackageError, "template hash"):
                MODULE.materialize_package(
                    self.forensic, self.output, self.current_tool, template,
                    source=current_source, template_manifest=manifest,
                    function_profile="GetBiriQEffectRadius",
                )
        current_source, template, manifest = self._install_radius_profile_fixture()
        with self.subTest("wrong source"):
            current_source.write_bytes(current_source.read_bytes() + b"drift")
            with self.assertRaisesRegex(MODULE.PackageError, "immutable Player source"):
                MODULE.materialize_package(
                    self.forensic, self.output, self.current_tool, template,
                    source=current_source, template_manifest=manifest,
                    function_profile="GetBiriQEffectRadius",
                )
        current_source, template, manifest = self._install_radius_profile_fixture()
        with self.subTest("wrong object"):
            MODULE.FUNCTION_PROFILES["GetBiriQEffectRadius"]["object_sha256"] = "0" * 64
            with self.assertRaisesRegex(MODULE.PackageError, "forensic expected object"):
                MODULE.materialize_package(
                    self.forensic, self.output, self.current_tool, template,
                    source=current_source, template_manifest=manifest,
                    function_profile="GetBiriQEffectRadius",
                )

    def test_radius_profile_rejects_wrong_hook_and_stale_movenum_guard(self) -> None:
        self._materialize_radius()
        request_path = self.output / "backend-request.json"
        request = json.loads(request_path.read_text())
        request["custom_hook_union"]["rows"][0]["address"] += 1
        request["request_sha256"] = MODULE._request_digest(request)
        request_path.write_text(json.dumps(request, sort_keys=True))
        self._reauthenticate_materialized_request(self.output)
        with self.assertRaisesRegex(MODULE.PackageError, "hook profile mismatch"):
            MODULE.validate_package(self.output)

        second_output = self.root / "radius-stale-guard"
        self.output = second_output
        self._materialize_radius()
        backend = self.output / "backend/movenum-pcode-color-v523/physical_capture_backend_v523.py"
        backend.write_text(backend.read_text().replace('FUNCTION = "GetBiriQEffectRadius"', 'FUNCTION = "MoveNumOMExec"'))
        with self.assertRaisesRegex(MODULE.PackageError, "function binding|stale MoveNum"):
            MODULE._validate_copied_function_guards(self.output, "GetBiriQEffectRadius")

    def test_validate_runs_copied_adapter_validate_request(self) -> None:
        receipt = MODULE.materialize_package(
            self.forensic,
            self.output,
            self.current_tool,
            self.template,
            template_manifest=self.template_manifest,
            session_id="session-aaaaaaaaaaaaaaaa",
        )
        request = json.loads((self.output / "backend-request.json").read_text())
        self.assertEqual(request["same_session_capture"]["expected_sha256"], self._sha(self.current_tool))
        adapter_path = self.output / "backend/movenum-pcode-color-v523/player-phys-reg-trace-v490/gc27_phys_reg_adapter.py"
        module_name = "_player_gc27_runtime_adapter_" + MODULE._sha256(adapter_path)[:16]
        self.assertNotIn(module_name, sys.modules)
        called: list[bool] = []
        original = MODULE._validate_copied_adapter

        def recording_validator(root: Path, value: dict[str, object]) -> None:
            called.append(True)
            original(root, value)

        MODULE._validate_copied_adapter = recording_validator
        try:
            self.assertEqual(MODULE.validate_package(self.output)["request_sha256"], receipt["request_sha256"])
        finally:
            MODULE._validate_copied_adapter = original
        self.assertEqual(called, [True])
        self.assertNotIn(module_name, sys.modules)

    def test_validate_rejects_copied_adapter_import_failure_and_cleans_module(self) -> None:
        self._materialize()
        adapter_path = self.output / "backend/movenum-pcode-color-v523/player-phys-reg-trace-v490/gc27_phys_reg_adapter.py"
        adapter_path.write_text("def validate_request(:\n")
        self._reauthenticate_materialized_adapter(self.output)
        module_name = "_player_gc27_runtime_adapter_" + MODULE._sha256(adapter_path)[:16]
        with self.assertRaisesRegex(MODULE.PackageError, "copied physical-register adapter import failed"):
            MODULE.validate_package(self.output)
        self.assertNotIn(module_name, sys.modules)

    def test_validate_rejects_copied_adapter_module_name_collision(self) -> None:
        self._materialize()
        adapter_path = self.output / "backend/movenum-pcode-color-v523/player-phys-reg-trace-v490/gc27_phys_reg_adapter.py"
        module_name = "_player_gc27_runtime_adapter_" + MODULE._sha256(adapter_path)[:16]
        collision = object()
        sys.modules[module_name] = collision
        try:
            with self.assertRaisesRegex(MODULE.PackageError, "module-name collision"):
                MODULE.validate_package(self.output)
            self.assertIs(sys.modules[module_name], collision)
        finally:
            sys.modules.pop(module_name, None)

    def test_validate_rejects_missing_or_changed_expected_sha256(self) -> None:
        self._materialize()
        request_path = self.output / "backend-request.json"
        request = json.loads(request_path.read_text())
        request["same_session_capture"].pop("expected_sha256")
        request["request_sha256"] = MODULE._request_digest(request)
        request_path.write_text(json.dumps(request, sort_keys=True))
        self._reauthenticate_materialized_request(self.output)
        with self.assertRaisesRegex(MODULE.PackageError, "same-session capture binding"):
            MODULE.validate_package(self.output)

        second = self.root / "materialized-expected-sha-drift"
        MODULE.materialize_package(
            self.forensic,
            second,
            self.current_tool,
            self.template,
            template_manifest=self.template_manifest,
            session_id="session-aaaaaaaaaaaaaaaa",
        )
        second_request_path = second / "backend-request.json"
        second_request = json.loads(second_request_path.read_text())
        second_request["same_session_capture"]["expected_sha256"] = "0" * 64
        second_request["request_sha256"] = MODULE._request_digest(second_request)
        second_request_path.write_text(json.dumps(second_request, sort_keys=True))
        self._reauthenticate_materialized_request(second)
        with self.assertRaisesRegex(MODULE.PackageError, "same-session capture binding"):
            MODULE.validate_package(second)

    def test_validate_rejects_physical_hook_serialization_drift(self) -> None:
        self._materialize()
        request_path = self.output / "backend-request.json"
        request = json.loads(request_path.read_text())
        request["hooks"][0]["address_hex"] = "0x" + request["hooks"][0]["address_hex"][2:].upper()
        request["request_sha256"] = MODULE._request_digest(request)
        request_path.write_text(json.dumps(request, sort_keys=True))
        self._reauthenticate_materialized_request(self.output)
        with self.assertRaisesRegex(MODULE.PackageError, "physical hook serialization mismatch"):
            MODULE.validate_package(self.output)

    def test_validate_rejects_request_mutation_after_hash_sealing(self) -> None:
        self._materialize()
        request_path = self.output / "backend-request.json"
        request = json.loads(request_path.read_text())
        request["function"] = "TamperedMoveNumOMExec"
        request_path.write_text(json.dumps(request, sort_keys=True))
        self._reauthenticate_materialized_request(self.output)
        with self.assertRaisesRegex(MODULE.PackageError, "backend request digest mismatch"):
            MODULE.validate_package(self.output)

    def _reauthenticate_materialized_request(self, package_root: Path) -> None:
        request_path = package_root / "backend-request.json"
        receipt_path = package_root / "package-receipt.json"
        request = json.loads(request_path.read_text())
        receipt = json.loads(receipt_path.read_text())
        descriptor = receipt["files"]["backend-request.json"]
        descriptor["sha256"] = self._sha(request_path)
        descriptor["size"] = request_path.stat().st_size
        receipt["request_sha256"] = request["request_sha256"]
        receipt_path.write_text(json.dumps(receipt, sort_keys=True))

    def _reauthenticate_materialized_adapter(self, package_root: Path) -> None:
        relative = "backend/movenum-pcode-color-v523/player-phys-reg-trace-v490/gc27_phys_reg_adapter.py"
        adapter_path = package_root / relative
        receipt_path = package_root / "package-receipt.json"
        receipt = json.loads(receipt_path.read_text())
        descriptor = receipt["files"][relative]
        descriptor["sha256"] = self._sha(adapter_path)
        descriptor["size"] = adapter_path.stat().st_size
        receipt["adapter"]["sha256"] = descriptor["sha256"]
        receipt["adapter"]["size"] = descriptor["size"]
        receipt_path.write_text(json.dumps(receipt, sort_keys=True))

    def test_expected_object_override_rebinds_after_old_path_drift(self) -> None:
        forensic_before = (self.forensic / "package-receipt.json").read_bytes()
        replacement = self._write(self.root / "replacement" / "player.o", self.expected_object.read_bytes())
        self.expected_object.write_bytes(b"mutable old path drift\n")

        receipt = MODULE.materialize_package(
            self.forensic,
            self.output,
            self.current_tool,
            self.template,
            template_manifest=self.template_manifest,
            session_id="session-3333333333333333",
            expected_object_path=replacement,
        )

        self.assertEqual((self.forensic / "package-receipt.json").read_bytes(), forensic_before)
        self.assertEqual(receipt["expected_object"]["path"], str(replacement))
        self.assertEqual(receipt["forensic_expected_object"]["path"], str(self.expected_object))
        self.assertEqual(receipt["expected_object_binding"]["mode"], "path_override")
        request = json.loads((self.output / "backend-request.json").read_text())
        manifest = json.loads((self.output / "manifest.json").read_text())
        self.assertEqual(request["expected_object"]["path"], str(replacement))
        self.assertEqual(request["sources"]["retained"]["expected_object"]["path"], str(replacement))
        self.assertEqual(manifest["expected_object"]["path"], str(replacement))
        self.assertEqual(MODULE.validate_package(self.output)["expected_object"]["path"], str(replacement))

    def test_expected_object_override_accepts_pathless_v1_forensic_identity(self) -> None:
        receipt_path = self.forensic / "package-receipt.json"
        receipt = json.loads(receipt_path.read_text())
        receipt["expected_object"].pop("path")
        receipt["expected_object"]["both_mirrored_lanes"] = True
        receipt_path.write_text(json.dumps(receipt, sort_keys=True))
        request_path = self.forensic / "backend-request.json"
        request = json.loads(request_path.read_text())
        for source_value in request["sources"].values():
            source_value["expected_object"].pop("path")
            source_value["expected_object"]["both_mirrored_lanes"] = True
        request_path.write_text(json.dumps(request, sort_keys=True))
        replacement = self._write(self.root / "replacement" / "player.o", self.expected_object.read_bytes())

        result = MODULE.materialize_package(
            self.forensic,
            self.output,
            self.current_tool,
            self.template,
            template_manifest=self.template_manifest,
            session_id="session-9999999999999999",
            expected_object_path=replacement,
        )

        self.assertEqual(result["forensic_expected_object"]["sha256"], self._sha(self.expected_object))
        self.assertEqual(result["forensic_expected_object"]["size"], self.expected_object.stat().st_size)
        self.assertNotIn("path", result["forensic_expected_object"])
        self.assertTrue(result["forensic_expected_object"]["both_mirrored_lanes"])
        self.assertEqual(result["expected_object"]["path"], str(replacement))
        self.assertTrue(result["expected_object"]["both_mirrored_lanes"])
        self.assertEqual(MODULE.validate_package(self.output)["expected_object"]["path"], str(replacement))

    def test_pathless_v1_forensic_identity_requires_explicit_override(self) -> None:
        receipt_path = self.forensic / "package-receipt.json"
        receipt = json.loads(receipt_path.read_text())
        receipt["expected_object"].pop("path")
        receipt["expected_object"]["both_mirrored_lanes"] = True
        receipt_path.write_text(json.dumps(receipt, sort_keys=True))
        request_path = self.forensic / "backend-request.json"
        request = json.loads(request_path.read_text())
        for source_value in request["sources"].values():
            source_value["expected_object"].pop("path")
            source_value["expected_object"]["both_mirrored_lanes"] = True
        request_path.write_text(json.dumps(request, sort_keys=True))

        with self.assertRaisesRegex(MODULE.PackageError, "forensic expected object path missing"):
            self._materialize()
        self.assertFalse(self.output.exists())

    def test_expected_object_override_hash_mismatch_fails_before_output(self) -> None:
        replacement = self._write(self.root / "replacement" / "wrong.o", b"wrong bytes\n")
        with self.assertRaisesRegex(MODULE.PackageError, "expected-object override hash drift"):
            MODULE.materialize_package(
                self.forensic,
                self.output,
                self.current_tool,
                self.template,
                template_manifest=self.template_manifest,
                session_id="session-4444444444444444",
                expected_object_path=replacement,
            )
        self.assertFalse(self.output.exists())

    def test_expected_object_override_size_mismatch_fails_before_output(self) -> None:
        for relative in ("package-receipt.json", "backend-request.json"):
            path = self.forensic / relative
            value = json.loads(path.read_text())
            if relative == "package-receipt.json":
                value["expected_object"]["size"] += 1
            else:
                for source_value in value["sources"].values():
                    source_value["expected_object"]["size"] += 1
            path.write_text(json.dumps(value, sort_keys=True))
        with self.assertRaisesRegex(MODULE.PackageError, "forensic expected object is not authenticated"):
            MODULE.materialize_package(
                self.forensic,
                self.output,
                self.current_tool,
                self.template,
                template_manifest=self.template_manifest,
                session_id="session-5555555555555555",
                expected_object_path=self.expected_object,
            )
        self.assertFalse(self.output.exists())

    def test_expected_object_override_requires_absolute_path(self) -> None:
        with self.assertRaisesRegex(MODULE.PackageError, "expected-object override path must be absolute"):
            MODULE.materialize_package(
                self.forensic,
                self.output,
                self.current_tool,
                self.template,
                template_manifest=self.template_manifest,
                session_id="session-6666666666666666",
                expected_object_path=Path("relative-player.o"),
            )
        self.assertFalse(self.output.exists())

    def test_expected_object_override_symlink_is_rejected_when_supported(self) -> None:
        link = self.root / "replacement-link.o"
        try:
            os.symlink(self.expected_object, link)
        except (OSError, NotImplementedError):
            self.skipTest("symlinks unavailable")
        with self.assertRaisesRegex(MODULE.PackageError, "expected-object override path contains a symlink"):
            MODULE.materialize_package(
                self.forensic,
                self.output,
                self.current_tool,
                self.template,
                template_manifest=self.template_manifest,
                session_id="session-7777777777777777",
                expected_object_path=link,
            )
        self.assertFalse(self.output.exists())

    def test_expected_object_override_validation_reauthenticates_live_bytes(self) -> None:
        replacement = self._write(self.root / "replacement" / "player.o", self.expected_object.read_bytes())
        self._materialize_with_expected_object(replacement)
        replacement.write_bytes(b"validation-time drift\n")
        with self.assertRaisesRegex(MODULE.PackageError, "hash drift"):
            MODULE.validate_package(self.output)

    def _materialize_with_expected_object(self, expected_object_path: Path) -> Path:
        MODULE.materialize_package(
            self.forensic,
            self.output,
            self.current_tool,
            self.template,
            template_manifest=self.template_manifest,
            session_id="session-8888888888888888",
            expected_object_path=expected_object_path,
        )
        return self.output

    def test_relative_inputs_are_sealed_for_cross_cwd_validation(self) -> None:
        original_cwd = Path.cwd()
        materialize_cwd = self.root
        unrelated_cwd = self.root / "independent-validator"
        unrelated_cwd.mkdir()
        try:
            os.chdir(materialize_cwd)
            receipt = MODULE.materialize_package(
                self.forensic.relative_to(materialize_cwd),
                self.output.relative_to(materialize_cwd),
                self.current_tool.relative_to(materialize_cwd),
                self.template.relative_to(materialize_cwd),
                template_manifest=self.template_manifest.relative_to(materialize_cwd),
                session_id="session-abababababababab",
            )
            self.assertTrue(Path(receipt["root"]).is_absolute())
            for role in ("current_tool", "source", "target", "expected_object"):
                self.assertTrue(Path(receipt[role]["path"]).is_absolute(), role)
            for item in receipt["one_shot"]["argv"]:
                if isinstance(item, str) and (
                    "launch_movenum_capture.py" in item
                    or "backend-request.json" in item
                    or "physical_capture_backend_v523.py" in item
                ):
                    self.assertTrue(Path(item).is_absolute(), item)
            os.chdir(unrelated_cwd)
            validated = MODULE.validate_package(self.output.resolve())
            self.assertEqual(validated["root"], str(self.output.resolve()))
        finally:
            os.chdir(original_cwd)

    def test_validate_rejects_relative_sealed_root(self) -> None:
        self._materialize()
        receipt_path = self.output / "package-receipt.json"
        receipt = json.loads(receipt_path.read_text())
        receipt["root"] = "materialized"
        receipt_path.write_text(json.dumps(receipt, sort_keys=True))
        with self.assertRaisesRegex(MODULE.PackageError, "receipt root is not an absolute path"):
            MODULE.validate_package(self.output)

    def test_forensic_backend_hash_drift_is_rejected(self) -> None:
        backend = self.forensic / "backend" / "movenum-pcode-color-v523" / "physical_capture_backend_v523.py"
        backend.write_text(backend.read_text() + "drift\n")
        with self.assertRaisesRegex(MODULE.PackageError, "hash drift"):
            self._materialize()

    def test_materialized_backend_uses_only_the_canonical_request_session(self) -> None:
        self._materialize()
        backend_path = self.output / "backend" / "movenum-pcode-color-v523" / "physical_capture_backend_v523.py"
        spec = importlib.util.spec_from_file_location("materialized_backend_session_test", backend_path)
        assert spec is not None and spec.loader is not None
        backend = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(backend)
        self.assertEqual(
            backend._central_manifest(
                {"session_id": "session-1111111111111111"},
                "retained",
                {"sha256": "a" * 64},
            ),
            "session-1111111111111111",
        )
        with self.assertRaisesRegex(backend.BackendError, "session_id is not canonical"):
            backend._central_manifest(
                {"session_id": "session-" + "2" * 24},
                "retained",
                {"sha256": "a" * 64},
            )

    def test_copied_backend_source_derived_session_is_rejected(self) -> None:
        self._materialize()
        backend = self.output / "backend" / "movenum-pcode-color-v523" / "physical_capture_backend_v523.py"
        backend.write_text(
            backend.read_text().replace(
                MODULE.BACKEND_REQUEST_SESSION,
                MODULE.BACKEND_DERIVED_SESSION,
                1,
            )
        )
        with self.assertRaisesRegex(MODULE.PackageError, "source-based sessions"):
            MODULE._validate_copied_backend_session_binding(self.output)

    def test_backend_session_rewrite_rejects_missing_or_duplicate_marker(self) -> None:
        old_hash = "a" * 64
        new_hash = "b" * 64
        for label, marker_count in (("missing", 0), ("duplicate", 2)):
            with self.subTest(label=label):
                source = self.root / f"{label}-backend.py"
                destination = self.root / f"{label}-rewritten.py"
                source.write_text(
                    f'OLD_TOOL_HASH="{old_hash}"\n'
                    + 'FUNCTION = "MoveNumOMExec"\n'
                    + MODULE.BACKEND_DERIVED_SESSION * marker_count,
                    encoding="utf-8",
                )
                with self.assertRaisesRegex(
                    MODULE.PackageError,
                    "session derivation marker is missing or ambiguous",
                ):
                    MODULE._rewrite_backend(source, destination, old_hash, new_hash, "MoveNumOMExec", "MoveNumOMExec")
                self.assertFalse(destination.exists())

    def test_launcher_missing_root_literal_is_rejected(self) -> None:
        self._rewrite_forensic_launcher_for_test('from pathlib import Path\nCOMPILER_LANE_LOCK = ROOT / "build/.compiler-lane.lock"\n')
        with self.assertRaisesRegex(MODULE.PackageError, "exactly one ROOT literal"):
            self._materialize()

    def test_launcher_multiple_root_literals_are_rejected(self) -> None:
        text = (self.forensic / "inputs" / "launch_movenum_capture.py").read_text()
        self._rewrite_forensic_launcher_for_test(text + text.split("COMPILER_LANE_LOCK", 1)[0])
        with self.assertRaisesRegex(MODULE.PackageError, "exactly one ROOT literal"):
            self._materialize()

    def test_launcher_root_drift_is_rejected(self) -> None:
        text = (self.forensic / "inputs" / "launch_movenum_capture.py").read_text()
        self._rewrite_forensic_launcher_for_test(text.replace("mp6-wt-player-full-closure-v459", "mp6-wt-player-other-owner"))
        with self.assertRaisesRegex(MODULE.PackageError, "ROOT literal drift"):
            self._materialize()

    def test_session_mismatch_is_rejected_on_validation(self) -> None:
        self._materialize()
        manifest_path = self.output / "manifest.json"
        value = json.loads(manifest_path.read_text())
        value["session_id"] = "session-other"
        manifest_path.write_text(json.dumps(value, sort_keys=True))
        with self.assertRaises(MODULE.PackageError):
            MODULE.validate_package(self.output)

    def test_stale_hook_is_rejected(self) -> None:
        self._materialize()
        path = self.output / "backend-request.json"
        path.write_text(path.read_text().replace("gc27_machine_emit", "0x004D03E8"))
        with self.assertRaises(MODULE.PackageError):
            MODULE.validate_package(self.output)

    def test_output_collision_is_rejected(self) -> None:
        self._materialize()
        (self.output / "backend-output").mkdir()
        with self.assertRaises(MODULE.PackageError):
            MODULE.validate_package(self.output)

    def test_live_execution_plan_collision_is_rejected(self) -> None:
        self._materialize()
        (self.output / "live-execution-plan.json").write_text("{}")
        with self.assertRaisesRegex(MODULE.PackageError, "live execution plan"):
            MODULE.validate_package(self.output)

    def test_unexpected_file_is_rejected(self) -> None:
        self._materialize()
        (self.output / "unexpected.txt").write_text("unexpected")
        with self.assertRaises(MODULE.PackageError):
            MODULE.validate_package(self.output)

    def test_output_root_overwrite_is_rejected(self) -> None:
        self.output.mkdir()
        with self.assertRaisesRegex(MODULE.PackageError, "overwrite"):
            self._materialize()

    def test_materialize_rejects_noncanonical_session_id(self) -> None:
        with self.assertRaisesRegex(MODULE.PackageError, "invalid session ID"):
            MODULE.materialize_package(
                self.forensic,
                self.output,
                self.current_tool,
                self.template,
                template_manifest=self.template_manifest,
                session_id="session-1234567890abcdef12345678",
            )

    def test_validate_rejects_noncanonical_session_id(self) -> None:
        self._materialize()
        receipt_path = self.output / "package-receipt.json"
        receipt = json.loads(receipt_path.read_text())
        receipt["session_id"] = "session-1234567890abcdef12345678"
        receipt_path.write_text(json.dumps(receipt, sort_keys=True))
        with self.assertRaisesRegex(MODULE.PackageError, "invalid session ID"):
            MODULE.validate_package(self.output)

    def test_validate_requires_authenticated_schemas(self) -> None:
        self._materialize()
        manifest_path = self.output / "manifest.json"
        manifest = json.loads(manifest_path.read_text())
        manifest["schema"] = "wrong"
        manifest_path.write_text(json.dumps(manifest, sort_keys=True))
        with self.assertRaisesRegex(MODULE.PackageError, "manifest schema"):
            MODULE.validate_package(self.output)

        # Re-materialize in a fresh root so the request-schema branch is
        # independently exercised rather than masked by the manifest change.
        second = self.root / "materialized-request-schema"
        MODULE.materialize_package(self.forensic, second, self.current_tool, self.template, template_manifest=self.template_manifest, session_id="session-2222222222222222")
        request_path = second / "backend-request.json"
        request = json.loads(request_path.read_text())
        request["schema"] = "wrong"
        request_path.write_text(json.dumps(request, sort_keys=True))
        with self.assertRaisesRegex(MODULE.PackageError, "backend request schema"):
            MODULE.validate_package(second)

    def test_symlink_is_rejected_when_supported(self) -> None:
        link = self.forensic / "inputs" / "player.c.link"
        try:
            os.symlink(self.forensic / "inputs" / "player.c", link)
        except (OSError, NotImplementedError):
            self.skipTest("symlinks unavailable")
        with self.assertRaises(MODULE.PackageError):
            self._materialize()


if __name__ == "__main__":
    unittest.main()
