import hashlib
import hmac
import contextlib
import inspect
import json
import os
import shutil
import sqlite3
from datetime import datetime, timedelta, timezone
from pathlib import Path
import subprocess
import sys
import tempfile
import time
import unittest

from tools import crack_harness as harness
from tools.recovery_memory import RecoveryMemory
from unittest.mock import patch

TARGET_SHA = hashlib.sha256(b"target").hexdigest()
STRICT_ROW_0 = (
    "strict:Owner:row:0:kind=DIFF_ARG_MISMATCH:"
    "target=0x100:candidate=0x100"
)
STRICT_ROW_1 = (
    "strict:Owner:row:1:kind=DIFF_ARG_MISMATCH:"
    "target=0x104:candidate=0x104"
)


def sha(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


class CrackHarnessTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temp = tempfile.TemporaryDirectory()
        self.root = Path(self.temp.name).resolve()
        self.source = self.root / "src/owner.c"
        self.admission = self.root / "tools/candidate_compile_admission.py"
        self.hook = self.root / "tools/crack_harness.py"
        self.compile_hook = self.root / "tools/crack_evidence_bundle.py"
        self.residual_producer = self.root / "tools/crack_current_residual.py"
        self.source.parent.mkdir(parents=True)
        self.admission.parent.mkdir()
        self.evidence = self.root / "evidence/selection.json"
        self.current_residual = self.root / "evidence/current-residual.json"
        self.evidence.parent.mkdir()
        self.source.write_text("int Owner(void) {\n    return 1;\n}\n", encoding="utf-8")
        self.base = self.root / "base.c"
        self.candidate = self.root / "candidate.c"
        self.base.write_bytes(self.source.read_bytes())
        self.candidate.write_text("int Owner(void) {\n    return 2;\n}\n", encoding="utf-8")
        self.admission.write_text(
            self._admission_source(),
            encoding="utf-8",
        )
        self.hook.write_text(self._hook_source(), encoding="utf-8")
        self.compile_hook.write_text(self._hook_source(), encoding="utf-8")
        self.residual_producer.write_text("# test residual producer\n", encoding="utf-8")
        self._git("init", "-q")
        self._git("config", "user.email", "test@example.invalid")
        self._git("config", "user.name", "Harness Test")
        self._git("add", "src/owner.c", "tools/candidate_compile_admission.py", "tools/crack_harness.py", "tools/crack_evidence_bundle.py", "tools/crack_current_residual.py")
        self._git("commit", "-qm", "fixture")
        self.commit = self._git("rev-parse", "HEAD")
        self._write_selection_evidence()
        self.luna_audit = self.root / "evidence/luna5.json"
        self._write_luna5_audit()
        self.state = self.root / "state"
        self.state.mkdir()
        self.approval = self.root / "approval.json"
        self.permit = self.root / "permit.json"
        self.secret_temp = tempfile.TemporaryDirectory()
        self.manager_key = Path(self.secret_temp.name) / "manager.key"
        self.manager_key.write_bytes(b"T" * 32)

    def tearDown(self) -> None:
        subprocess.run(["git", "worktree", "prune"], cwd=self.root, capture_output=True)
        self.temp.cleanup()
        self.secret_temp.cleanup()

    def _git(self, *args: str) -> str:
        return subprocess.run(["git", *args], cwd=self.root, text=True, capture_output=True, check=True).stdout.strip()

    def _write_selection_evidence(
        self, *, expected_terminal: str = "exact",
        include_live_source: bool = False,
        residual_rows: list[str] | None = None,
        predicted_rows: list[str] | None = None,
    ) -> None:
        predicted_rows = predicted_rows or [STRICT_ROW_0]
        residual_rows = residual_rows or [STRICT_ROW_0]
        self._write_current_residual(residual_rows)
        inputs = [{
            "path": "base.c", "sha256": sha(self.base),
            "role": "sealed_baseline_source",
        }]
        if include_live_source:
            inputs.append({
                "path": "src/owner.c", "sha256": sha(self.source),
                "role": "live_source_context",
            })
        value = {
            "schema": harness.WINNING_CELL_EVIDENCE_SCHEMA,
            "owner": "main:board/test", "function": "Owner",
            "strategy": "winning_cell_first", "rank": 1,
            "expected_terminal": expected_terminal,
            "candidate_sha256": sha(self.candidate),
            "predicted_rows_sha256": harness._digest_json(predicted_rows),
            "alternatives_compiled": 0, "negative_controls": 0,
            "pivot_if_unranked": True, "source_class": "test-natural-cell",
            "current_residual": {
                "path": self.current_residual.relative_to(self.root).as_posix(),
                "sha256": sha(self.current_residual),
            },
            "inputs": inputs,
            "causal_prediction": {
                "earliest_divergence": "Owner return-value producer",
                "predicted_effect": "close the one predicted focus row",
                "predicted_rows": predicted_rows,
            },
        }
        self.evidence.write_text(
            json.dumps(value, sort_keys=True, separators=(",", ":")) + "\n",
            encoding="utf-8",
        )

    def _write_current_residual(
        self, residual_rows: list[str], **overrides: object,
    ) -> None:
        base_object_path = self.root / "evidence/current-base.o"
        target_object_path = self.root / "evidence/current-target.o"
        focus_path = self.root / "evidence/current-focus.json"
        physical_path = self.root / "evidence/current-physical.json"
        base_object_path.write_bytes(b"current base object")
        target_object_path.write_bytes(b"target")

        def file_descriptor(path: Path) -> dict[str, object]:
            return {
                "path": path.relative_to(self.root).as_posix(),
                "sha256": sha(path),
                "size_bytes": path.stat().st_size,
            }

        strict_descriptor = {"sha256": "1" * 64, "size_bytes": 101}
        data_descriptor = {"sha256": "2" * 64, "size_bytes": 102}
        receipt_descriptor = {"sha256": "3" * 64, "size_bytes": 103}
        base_descriptor = file_descriptor(base_object_path)
        target_descriptor = file_descriptor(target_object_path)
        row_material = {
            STRICT_ROW_0: {
                "index": 0, "diff_kind": "DIFF_ARG_MISMATCH",
                "instruction": {"address": "0x100", "formatted": "li r3,0"},
            },
            STRICT_ROW_1: {
                "index": 1, "diff_kind": "DIFF_ARG_MISMATCH",
                "instruction": {"address": "0x104", "formatted": "li r4,0"},
            },
        }
        try:
            strict_rows = [row_material[row] for row in residual_rows]
        except KeyError as exc:
            raise AssertionError(f"test fixture lacks focus material for {exc.args[0]}") from exc
        focus_body = {
            "schema": "focus_symbol_report/v1",
            "function": "Owner",
            "input_binding": {
                "strict_report": strict_descriptor,
                "data_report": data_descriptor,
                "physical_relocation_receipt": receipt_descriptor,
            },
            "channels": {
                "strict": {
                    "target": {"rows_kind": "diff_only", "rows": strict_rows},
                    "candidate": {"rows_kind": "diff_only", "rows": strict_rows},
                },
                "data": {
                    "target": {"rows_kind": "diff_only", "rows": []},
                    "candidate": {"rows_kind": "diff_only", "rows": []},
                },
            },
            "physical_relocations": {
                "status": "exact",
                "target": {"object": target_descriptor},
                "candidate": {"object": base_descriptor},
                "physical_relocation_differences": [],
            },
            "authority_advanced": False,
        }
        focus_path.write_text(
            json.dumps({
                **focus_body,
                "artifact_sha256": harness._digest_json(focus_body),
            }, sort_keys=True, separators=(",", ":")) + "\n",
            encoding="utf-8",
        )
        focus_descriptor = file_descriptor(focus_path)
        physical_body = {
            "schema": "crack_current_residual_physical_summary/v1",
            "owner": "main:board/test",
            "unit": "main/board/test",
            "function": "Owner",
            "base_commit": self.commit,
            "target_object": target_descriptor,
            "base_object": base_descriptor,
            "focus_report": focus_descriptor,
            "strict_report": strict_descriptor,
            "data_report": data_descriptor,
            "physical_relocations_exact": True,
            "physical_difference_count": 0,
            "physical_difference_sha256": harness._digest_json([]),
            "authority_advanced": False,
        }
        physical_path.write_text(
            json.dumps({
                **physical_body,
                "physical_summary_sha256": harness._digest_json(physical_body),
            }, sort_keys=True, separators=(",", ":")) + "\n",
            encoding="utf-8",
        )
        residual_body = {
            "schema": harness.CURRENT_RESIDUAL_EVIDENCE_SCHEMA,
            "owner": "main:board/test", "function": "Owner",
            "base_commit": self.commit, "unit": "main/board/test",
            "base_sha256": sha(self.base), "source_sha256": sha(self.source),
            "target_sha256": TARGET_SHA,
            "function_span": {
                "start_line": 1, "end_line": 3,
                "base_span_sha256": sha(self.base),
            },
            "toolchain_key": harness.TOOLCHAIN_MANIFEST_KEY,
            "base_object": base_descriptor,
            "target_object": target_descriptor,
            "focus_report": focus_descriptor,
            "physical_summary": file_descriptor(physical_path),
            "strict_report": strict_descriptor,
            "data_report": data_descriptor,
            "physical_receipt": receipt_descriptor,
            "producer": file_descriptor(self.residual_producer),
            "residual_rows": residual_rows,
            "current_source_bound": True, "authority_advanced": False,
        }
        residual_body.update(overrides)
        self.current_residual.write_text(
            json.dumps({
                **residual_body,
                "residual_sha256": harness._digest_json(residual_body),
            }, sort_keys=True, separators=(",", ":")) + "\n",
            encoding="utf-8",
        )

    def _rebind_current_residual(self, approval_path: Path) -> None:
        evidence = json.loads(self.evidence.read_text(encoding="utf-8"))
        evidence["current_residual"]["sha256"] = sha(self.current_residual)
        self.evidence.write_text(
            json.dumps(evidence, sort_keys=True, separators=(",", ":")) + "\n",
            encoding="utf-8",
        )
        approval = json.loads(approval_path.read_text(encoding="utf-8"))
        approval["selection"]["current_residual"]["sha256"] = sha(
            self.current_residual
        )
        approval["selection"]["evidence"]["sha256"] = sha(self.evidence)
        approval_path.write_text(json.dumps(approval), encoding="utf-8")

    def _write_luna5_audit(self) -> None:
        receipts = []
        for index, role in enumerate(sorted(harness.LUNA5_ROLES)):
            artifact = self.root / f"evidence/luna-{index}.json"
            artifact.write_text(
                json.dumps({
                    "schema": harness.LUNA5_ARTIFACT_SCHEMA,
                    "role": role, "agent_id": f"luna-{index}",
                    "model": "gpt-5.6-luna", "reasoning_effort": "max",
                    "status": "PASS", "owner": "main:board/test",
                    "function": "Owner", "controller_commit": self.commit,
                    "candidate_sha256": sha(self.candidate),
                    "checks": {
                        name: True for name in harness.LUNA5_ROLE_CHECKS[role]
                    },
                    "findings": [f"{role} audit passed"],
                    "mutations": False, "compiled": False,
                    "authority_advanced": False,
                }, sort_keys=True, separators=(",", ":")) + "\n",
                encoding="utf-8",
            )
            receipts.append({
                "role": role, "agent_id": f"luna-{index}",
                "model": "gpt-5.6-luna", "reasoning_effort": "max",
                "status": "PASS", "controller_commit": self.commit,
                "artifact": {
                    "path": artifact.relative_to(self.root).as_posix(),
                    "sha256": sha(artifact),
                },
                "mutations": False, "compiled": False,
            })
        value = {
            "schema": harness.LUNA5_AUDIT_SCHEMA,
            "owner": "main:board/test", "function": "Owner",
            "controller_commit": self.commit,
            "candidate_sha256": sha(self.candidate),
            "receipts": receipts, "authority_advanced": False,
        }
        self.luna_audit.write_text(
            json.dumps(value, sort_keys=True, separators=(",", ":")) + "\n",
            encoding="utf-8",
        )

    def _commit_expected_terminal_fixture(self, expected_terminal: str) -> None:
        self._write_selection_evidence(expected_terminal=expected_terminal)
        self._write_luna5_audit()

    def _commit_live_source_evidence_fixture(self) -> None:
        self._write_selection_evidence(include_live_source=True)
        self._git("add", "evidence/selection.json")
        self._git("commit", "-qm", "bind live source evidence")
        self.commit = self._git("rev-parse", "HEAD")
        self._write_luna5_audit()

    def _hook_source(self) -> str:
        return r'''import hashlib,json,os,pathlib,sys,time
if pathlib.Path(sys.argv[0]).name=='crack_evidence_bundle.py':
 out=pathlib.Path(os.environ['CRACK_HARNESS_OUT_ROOT']); out.mkdir(parents=True,exist_ok=True)
 if os.environ.get('HARNESS_TEST_COMMAND_FAIL')=='1':
  (out/'observed.o').write_bytes(b'partial-object'); print('known stdout'); print('known stderr',file=sys.stderr); raise SystemExit(7)
 desc=lambda p:{'sha256':hashlib.sha256(p.read_bytes()).hexdigest(),'size_bytes':p.stat().st_size}
 digest=lambda v:hashlib.sha256(json.dumps(v,sort_keys=True,separators=(',',':')).encode()).hexdigest()
 phase=os.environ['CRACK_HARNESS_PHASE']; source_rel=os.environ['CRACK_HARNESS_SOURCE_PATH']
 def receipt(phase,names):
  value={'schema':'crack_evidence_phase_receipt/v1','phase':phase,'owner':os.environ['CRACK_HARNESS_OWNER'],'function':os.environ['CRACK_HARNESS_FUNCTION'],'unit':os.environ['CRACK_HARNESS_UNIT'],'source_relpath':source_rel,'base_commit':os.environ['CRACK_HARNESS_BASE_COMMIT'],'approval_sha256':os.environ['CRACK_HARNESS_APPROVAL_SHA256'],'approval_context_sha256':os.environ['CRACK_HARNESS_CONTEXT_SHA256'],'phase_nonce':os.environ['CRACK_HARNESS_PHASE_NONCE'],'issued_at':os.environ['CRACK_HARNESS_ISSUED_AT'],'artifacts':{name:desc(out/name) for name in names},'tools':{'fixture':{'sha256':'f'*64}},'authority_advanced':False}
  value['receipt_sha256']=digest(value); (out/(phase+'-receipt.json')).write_text(json.dumps(value)); return value
 if os.environ['CRACK_HARNESS_PHASE']=='baseline':
  (out/'target.o').write_bytes(b'target'); (out/'baseline-candidate.o').write_bytes(b'baseline'); (out/'baseline-strict.json').write_text('{}'); (out/'baseline-data.json').write_text('{}'); (out/'baseline-physical.json').write_text('{}')
  receipt('baseline',['target.o','baseline-candidate.o','baseline-strict.json','baseline-data.json','baseline-physical.json'])
  value={'schema':'crack_evidence_bundle_context/v1','owner':os.environ['CRACK_HARNESS_OWNER'],'function':os.environ['CRACK_HARNESS_FUNCTION'],'unit':os.environ['CRACK_HARNESS_UNIT'],'source_relpath':source_rel,'target_sha256':os.environ['CRACK_HARNESS_TARGET_SHA256'],'base_commit':os.environ['CRACK_HARNESS_BASE_COMMIT'],'approval_sha256':os.environ['CRACK_HARNESS_APPROVAL_SHA256'],'approval_context_sha256':os.environ['CRACK_HARNESS_CONTEXT_SHA256'],'phase_nonces':{'baseline':os.environ['CRACK_HARNESS_PHASE_NONCE'],'candidate':hashlib.sha256((os.environ['CRACK_HARNESS_CONTEXT_SHA256']+':candidate').encode()).hexdigest()},'baseline_receipt':desc(out/'baseline-receipt.json'),'authority_advanced':False}; value['evidence_context_sha256']=digest(value); (out/'evidence-context.json').write_text(json.dumps(value))
 else:
  (out/'candidate.o').write_bytes(b'candidate'); (out/'candidate-strict.json').write_text('{}'); (out/'candidate-data.json').write_text('{}'); (out/'physical.json').write_text('{}'); (out/'fixture.json').write_text(json.dumps({'gain':float(os.environ['HARNESS_TEST_GAIN']),'focus':int(os.environ['HARNESS_TEST_FOCUS']),'data_gain':float(os.environ.get('HARNESS_TEST_DATA_GAIN','0')),'sibling_losses':int(os.environ.get('HARNESS_TEST_SIBLING_LOSSES','0')),'baseline_physical_diff':int(os.environ.get('HARNESS_TEST_BASELINE_PHYSICAL_DIFF','0')),'physical_diff':int(os.environ.get('HARNESS_TEST_PHYSICAL_DIFF','0')),'baseline_size_delta':int(os.environ.get('HARNESS_TEST_BASELINE_SIZE_DELTA','0')),'size_delta':int(os.environ.get('HARNESS_TEST_SIZE_DELTA','0'))}))
  candidate_receipt=receipt('candidate',['target.o','candidate.o','candidate-strict.json','candidate-data.json','physical.json'])
  if os.environ.get('HARNESS_TEST_BAD_RECEIPT')=='1': candidate_receipt['receipt_sha256']='0'*64; (out/'candidate-receipt.json').write_text(json.dumps(candidate_receipt))
  value=json.loads((out/'evidence-context.json').read_text()); value.pop('evidence_context_sha256'); value['candidate_receipt']=desc(out/'candidate-receipt.json'); value['completed']=True; value['evidence_context_sha256']=digest(value); (out/'evidence-context.json').write_text(json.dumps(value))
 print('compiled '+os.environ['CRACK_HARNESS_PHASE']); raise SystemExit
if sys.argv[1]=='proof-adapter':
 args=sys.argv[1:]; get=lambda name:args[args.index(name)+1]
 kind=get('--kind'); source=pathlib.Path(get('--candidate-source')); candidate_source=hashlib.sha256(source.read_bytes()).hexdigest()
 fixture=json.loads((pathlib.Path(get('--target-object')).parent/'fixture.json').read_text()); target=hashlib.sha256(b'target').hexdigest(); candidate='c'*64
 common={'owner':get('--owner'),'function':get('--function'),'candidate_source_sha256':candidate_source,'target_object_sha256':target,'candidate_object_sha256':candidate,'report_sha256':'d'*64}
 if kind=='strict': value=common|{'schema':'crack_proof_strict/v1','strict_percent':100,'target_bytes':32,'candidate_bytes':32+fixture['size_delta'],'differences':0}
 elif kind=='data': value=common|{'schema':'crack_proof_data/v1','data_percent':100,'target_bytes':8,'candidate_bytes':8+fixture['size_delta'],'differences':0}
 elif kind=='focus': value=common|{'schema':'crack_proof_focus/v1','differing_rows':fixture['focus']}
 elif kind=='siblings': value=common|{'schema':'crack_proof_siblings/v1','protected_total':18,'protected_losses':fixture['sibling_losses']}
 elif kind=='physical': value=common|{'schema':'crack_proof_physical/v1','target_count':3,'candidate_count':3,'differences':fixture['physical_diff']}
 else: value={'schema':'crack_assessment/v1','owner':get('--owner'),'function':get('--function'),'candidate_source_sha256':candidate_source,'target_object_sha256':target,'candidate_object_sha256':candidate,'owner_gain':fixture['gain'],'data_gain':fixture['data_gain'],'data_diff_delta':1 if fixture['data_gain']<0 else 0,'baseline_data_target_bytes':8,'baseline_data_candidate_bytes':8+fixture['baseline_size_delta'],'data_target_bytes':8,'data_candidate_bytes':8+fixture['size_delta'],'size_diff_delta':abs(fixture['size_delta'])-abs(fixture['baseline_size_delta']),'physical_diff_delta':fixture['physical_diff']-fixture['baseline_physical_diff']}
 print(json.dumps(value)); raise SystemExit
run,out,kind,*rest=sys.argv[1:]
source=pathlib.Path(run,'src/owner.c')
candidate_source=hashlib.sha256(source.read_bytes()).hexdigest()
target=hashlib.sha256(b'target').hexdigest(); candidate='c'*64; common={'owner':'main:board/test','function':'Owner','candidate_source_sha256':candidate_source,'target_object_sha256':target,'candidate_object_sha256':candidate,'report_sha256':'d'*64}
if kind=='compile':
 pathlib.Path(out,'fixture.json').write_text(json.dumps({'gain':float(rest[0]),'focus':int(rest[1])})); print('compiled')
elif kind=='strict': print(json.dumps(common|{'schema':'crack_proof_strict/v1','strict_percent':100,'target_bytes':32,'candidate_bytes':32,'differences':0}))
elif kind=='data': print(json.dumps(common|{'schema':'crack_proof_data/v1','data_percent':100,'target_bytes':8,'candidate_bytes':8,'differences':0}))
elif kind=='focus': print(json.dumps(common|{'schema':'crack_proof_focus/v1','differing_rows':int(rest[0]) if rest else 0}))
elif kind=='siblings': print(json.dumps(common|{'schema':'crack_proof_siblings/v1','protected_total':9,'protected_losses':0}))
elif kind=='physical': print(json.dumps(common|{'schema':'crack_proof_physical/v1','target_count':3,'candidate_count':3,'differences':0}))
elif kind=='assess': print(json.dumps({'schema':'crack_assessment/v1','owner':'main:board/test','function':'Owner','candidate_source_sha256':candidate_source,'target_object_sha256':target,'candidate_object_sha256':candidate,'owner_gain':float(rest[0]),'data_gain':0.0,'data_diff_delta':0,'baseline_data_target_bytes':8,'baseline_data_candidate_bytes':8,'data_target_bytes':8,'data_candidate_bytes':8,'size_diff_delta':0,'physical_diff_delta':0}))
elif kind=='record': print(json.dumps({'schema':'crack_central_record_receipt/v1','recorded':True,'owner':'main:board/test','function':'Owner','candidate_source_sha256':candidate_source,'target_object_sha256':target,'candidate_object_sha256':os.environ['CRACK_HARNESS_CANDIDATE_OBJECT_SHA256'],'outcome':os.environ['CRACK_HARNESS_OUTCOME'],'admission_token_sha256':hashlib.sha256(os.environ['CRACK_HARNESS_ADMISSION_TOKEN'].encode()).hexdigest(),'admission_input_key':'a'*64,'record_sha256':'e'*64}))
'''

    def _admission_source(self) -> str:
        return r'''import hashlib,json,os,pathlib,sys
if 'discard' in sys.argv:
 print(json.dumps({'status':'discarded','input_key':'a'*64,'authority_advanced':False}))
elif 'record' in sys.argv:
 args=sys.argv; get=lambda name:args[args.index(name)+1]
 print(json.dumps({'status':'recorded','authority_advanced':False,'experiment':{'input_key':'a'*64,'owner':get('--owner'),'function_name':get('--function'),'target_sha256':get('--target-sha256'),'source_sha256':get('--source-sha256'),'object_sha256':get('--object-sha256'),'status':get('--status'),'record_sha256':'e'*64}}))
elif 'admit' in sys.argv:
 print(json.dumps({'status':'admitted','reused':False,'skip_compile':False,'admission_token':'central','input_key':'a'*64,'expires_at':'2099-01-01T00:00:00+00:00','authority_advanced':False}))
'''

    def descriptor(self, kind: str, hook_kind: str, *extra: str) -> dict:
        script = self.compile_hook if kind == "compile" else self.admission if kind == "canonical_record" else self.hook
        if kind.startswith("proof_") or kind == "assessment":
            adapter_kind = kind.removeprefix("proof_") if kind != "assessment" else "assess"
            argv = [str(Path(sys.executable).resolve()), "{CONTROLLER_ROOT}/tools/crack_harness.py", "proof-adapter", "--kind", adapter_kind, "--owner", "main:board/test", "--function", "Owner", "--candidate-source", "{RUN_ROOT}/src/owner.c", "--candidate-source-sha256", sha(self.candidate), "--approved-target-object-sha256", TARGET_SHA, "--target-object", "{OUT_ROOT}/target.o", "--candidate-object", "{OUT_ROOT}/candidate.o", "--baseline-strict-report", "{OUT_ROOT}/baseline-strict.json", "--baseline-data-report", "{OUT_ROOT}/baseline-data.json", "--candidate-strict-report", "{OUT_ROOT}/candidate-strict.json", "--candidate-data-report", "{OUT_ROOT}/candidate-data.json", "--baseline-physical-receipt", "{OUT_ROOT}/baseline-physical.json", "--physical-receipt", "{OUT_ROOT}/physical.json"]
        elif kind == "compile":
            argv = [str(Path(sys.executable).resolve()), "{CONTROLLER_ROOT}/tools/crack_evidence_bundle.py", "--root", "{RUN_ROOT}", "--context", "{OUT_ROOT}/approval-context.json", "--out", "{OUT_ROOT}"]
        elif kind == "canonical_record":
            argv = [str(Path(sys.executable).resolve()), str(self.admission)]
        else:
            argv = [str(Path(sys.executable).resolve()), "{RUN_ROOT}/" + script.relative_to(self.root).as_posix(), "{RUN_ROOT}", "{OUT_ROOT}", hook_kind, *extra]
        return {
            "kind": kind,
            "argv": argv,
            "executable": {"path": str(Path(sys.executable).resolve()), "sha256": sha(Path(sys.executable))},
            "script": {"path": str(script), "sha256": sha(script)},
        }

    def write_inputs(
        self, *, gain: int = 1, focus_rows: int = 0, data_gain: int = 0,
        sibling_losses: int = 0, baseline_physical_diff: int = 0,
        physical_diff: int = 0, baseline_size_delta: int = 0,
        size_delta: int = 0,
        campaign_id: str = "campaign", expected_terminal: str = "exact",
        live_source_evidence: bool = False,
    ) -> tuple[Path, Path]:
        self._write_selection_evidence(
            expected_terminal=expected_terminal,
            include_live_source=live_source_evidence,
        )
        issued = datetime.now(timezone.utc).replace(microsecond=0)
        deadline = issued + timedelta(minutes=20)
        stop_nonce = "f" * 64
        precompile = {
            "kind": "canonical_admission",
            "argv": [str(Path(sys.executable).resolve()), str(self.admission), "--root", str(self.root), "admit", "--owner", "main:board/test", "--function", "Owner", "--base-commit", self.commit, "--toolchain-key", harness.TOOLCHAIN_MANIFEST_KEY, "--target-sha256", TARGET_SHA, "--source-sha256", sha(self.candidate), "--source-path", str(self.source), "--json"],
            "executable": {"path": str(Path(sys.executable).resolve()), "sha256": sha(Path(sys.executable))},
            "script": {"path": str(self.admission), "sha256": sha(self.admission)},
        }
        predicted_rows = [STRICT_ROW_0]
        value = {
            "schema": harness.APPROVAL_SCHEMA, "approval_id": "cell-1", "owner": "main:board/test", "task_id": "task", "function": "Owner", "unit": "main/board/test", "base_commit": self.commit, "toolchain_key": harness.TOOLCHAIN_MANIFEST_KEY, "target_sha256": TARGET_SHA, "permit_sha256": "0" * 64, "issued_at": issued.isoformat(), "expires_at": deadline.isoformat(),
            "source": {"path": str(self.source), "sha256": sha(self.source)}, "base": {"path": str(self.base), "sha256": sha(self.base)}, "candidate": {"path": str(self.candidate), "sha256": sha(self.candidate)}, "function_span": {"start_line": 1, "end_line": 3, "base_span_sha256": hashlib.sha256(self.base.read_bytes()).hexdigest()}, "predicted_rows": predicted_rows,
            "selection": {"strategy": "winning_cell_first", "rank": 1, "expected_terminal": expected_terminal, "evidence": {"path": self.evidence.relative_to(self.root).as_posix(), "sha256": sha(self.evidence)}, "current_residual": {"path": self.current_residual.relative_to(self.root).as_posix(), "sha256": sha(self.current_residual)}, "candidate_sha256": sha(self.candidate), "predicted_rows_sha256": harness._digest_json(predicted_rows), "alternatives_compiled": 0, "negative_controls": 0, "pivot_if_unranked": True, "source_class": "test-natural-cell", "luna5_audit": {"path": self.luna_audit.relative_to(self.root).as_posix(), "sha256": sha(self.luna_audit)}},
            "commands": {"precompile": precompile, "compile": self.descriptor("compile", "compile", str(gain), str(focus_rows)), "strict": self.descriptor("proof_strict", "strict"), "data": self.descriptor("proof_data", "data"), "focus": self.descriptor("proof_focus", "focus"), "siblings": self.descriptor("proof_siblings", "siblings"), "physical": self.descriptor("proof_physical", "physical"), "assess": self.descriptor("assessment", "assess"), "record": self.descriptor("canonical_record", "record")},
            "campaign": {"id": campaign_id, "quota": 1}, "limits": {"active_seconds": 20, "temporary_bytes": 1048576, "candidates": 1}
        }
        permit = {
            "schema": harness.PERMIT_SCHEMA, "permit_id": "permit",
            "issuer": harness.MANAGER_ISSUER, "resume": True,
            "owner": value["owner"], "task_id": value["task_id"],
            "function": value["function"], "campaign_id": value["campaign"]["id"],
            "approval_id": value["approval_id"],
            "approval_identity_sha256": harness._approval_permit_identity(value),
            "commands_sha256": harness._digest_json(value["commands"]),
            "source_relpath": self.source.relative_to(self.root).as_posix(),
            "source_sha256": value["source"]["sha256"],
            "base_sha256": value["base"]["sha256"],
            "candidate_sha256": value["candidate"]["sha256"],
            "base_commit": value["base_commit"],
            "toolchain_key": value["toolchain_key"],
            "target_sha256": value["target_sha256"],
            "stop_nonce": stop_nonce, "issued_at": issued.isoformat(),
            "deadline": deadline.isoformat(), "key_id": sha(self.manager_key),
        }
        permit["signature"] = hmac.new(
            self.manager_key.read_bytes(), harness._canonical(permit), hashlib.sha256
        ).hexdigest()
        self.permit.write_text(json.dumps(permit), encoding="utf-8")
        value["permit_sha256"] = sha(self.permit)
        (self.state / "STOP").write_text(json.dumps({"schema":"crack_harness_stop/v1","stopped":True,"authorized_permit_sha256":sha(self.permit),"stop_nonce":stop_nonce}), encoding="utf-8")
        self.approval.write_text(json.dumps(value), encoding="utf-8")
        return self.approval, self.permit

    def add_legacy_retry(
        self, approval_path: Path, *, strict_percent: int = 100,
        tombstone_schema: str = "crack_harness_function_tombstone/v1",
    ) -> tuple[Path, dict]:
        approval = json.loads(approval_path.read_text(encoding="utf-8"))
        loaded = harness.load_approval(self.root, approval_path)
        run_dir = harness._run_dir(self.state, loaded)
        function_dir = run_dir.parent
        function_dir.mkdir(parents=True, exist_ok=True)
        tombstone = {
            "schema": tombstone_schema,
            "function_key": harness._function_key(loaded),
            "owner": loaded["owner"], "function": loaded["function"],
            "first_campaign_id": "legacy-campaign", "consumed": True,
        }
        if tombstone_schema.endswith("/v2"):
            tombstone.update({
                "approval_sha256": "1" * 64,
                "base_sha256": loaded["base"]["sha256"],
                "candidate_sha256": loaded["candidate"]["sha256"],
                "candidate_execution_started": True,
                "consumed_at": harness._now(),
            })
        tombstone_path = function_dir / "latest-function.json"
        tombstone_path.write_text(json.dumps(tombstone), encoding="utf-8")

        prior_approval = "2" * 64
        failure_body = {
            "schema": "crack_harness_failure_diagnostic/v1",
            "approval_sha256": prior_approval,
            "owner": loaded["owner"], "function": loaded["function"],
            "primary_reason": "legacy infrastructure failed before proof",
            "cleanup_errors": [], "finished_at": harness._now(),
        }
        failure_path = function_dir / "latest-failure.json"
        failure_path.write_text(json.dumps({
            **failure_body,
            "diagnostic_sha256": harness._digest_json(failure_body),
        }), encoding="utf-8")
        historical = {
            "schema": harness.HISTORICAL_EXACT_EVIDENCE_SCHEMA,
            "owner": loaded["owner"], "function": loaded["function"],
            "candidate_sha256": loaded["candidate"]["sha256"],
            "legacy_controller_commit": self.commit,
            "target_bytes": 32, "candidate_bytes": 32,
            "strict_percent": strict_percent, "data_percent": 100,
            "strict_diff_rows": 0, "data_diff_rows": 0,
        }
        evidence_path = self.root / "evidence/legacy-exact.json"
        evidence_path.write_text(json.dumps(historical), encoding="utf-8")
        rel = lambda path: path.relative_to(self.root).as_posix()
        approval["retry"] = {
            "schema": harness.RETRY_SCHEMA,
            "tombstone": {"path": rel(tombstone_path), "sha256": sha(tombstone_path)},
            "failure": {"path": rel(failure_path), "sha256": sha(failure_path)},
            "prior_approval_sha256": prior_approval,
            "candidate_sha256": loaded["candidate"]["sha256"],
            "legacy_controller_commit": self.commit,
            "historical_exact_evidence": {
                "path": rel(evidence_path), "sha256": sha(evidence_path),
            },
        }
        approval_path.write_text(json.dumps(approval), encoding="utf-8")
        return run_dir, approval

    def manager_draft(self, **kwargs: int) -> Path:
        approval, permit = self.write_inputs(**kwargs)
        value = json.loads(approval.read_text(encoding="utf-8"))
        value["permit_sha256"] = "0" * 64
        draft = self.root / "manager-draft.json"
        draft.write_text(json.dumps(value), encoding="utf-8")
        approval.unlink()
        permit.unlink()
        return draft

    def execute(self, **kwargs: int) -> dict:
        approval, permit = self.write_inputs(**kwargs)
        settings = {
            "HARNESS_TEST_GAIN": kwargs.get("gain", 1),
            "HARNESS_TEST_FOCUS": kwargs.get("focus_rows", 0),
            "HARNESS_TEST_DATA_GAIN": kwargs.get("data_gain", 0),
            "HARNESS_TEST_SIBLING_LOSSES": kwargs.get("sibling_losses", 0),
            "HARNESS_TEST_BASELINE_PHYSICAL_DIFF": kwargs.get(
                "baseline_physical_diff", 0
            ),
            "HARNESS_TEST_PHYSICAL_DIFF": kwargs.get("physical_diff", 0),
            "HARNESS_TEST_SIZE_DELTA": kwargs.get("size_delta", 0),
            "HARNESS_TEST_BASELINE_SIZE_DELTA": kwargs.get("baseline_size_delta", 0),
        }
        previous = {name: os.environ.get(name) for name in settings}
        os.environ.update({name: str(value) for name, value in settings.items()})
        try:
            return harness._run_approved_for_test(self.root, approval, permit_path=permit, state_root=self.state, manager_key_path=self.manager_key)
        finally:
            for name, value in previous.items():
                if value is None: os.environ.pop(name, None)
                else: os.environ[name] = value

    def _write_recovery_journal(
        self, *, result: dict, exact_commit: bool = False,
        central_record_binding: dict | None = None,
    ) -> None:
        run = self.state / "owners/test/Owner/latest"; recovery = run / "temp"
        recovery.mkdir(parents=True)
        baseline = recovery / "baseline.snapshot"; baseline.write_bytes(self.base.read_bytes())
        result_path = run / "result.json"; result_path.write_text(json.dumps(result), encoding="utf-8")
        commit_path = run / "record.commit.json"
        if exact_commit:
            commit_body = {"schema":"crack_harness_record_commit/v1","outcome":"exact","candidate_sha256":sha(self.candidate),"record_payload_sha256":"e"*64,"record_sha256":"e"*64}
            commit_path.write_text(json.dumps(commit_body | {"commit_sha256":harness._digest_json(commit_body)}), encoding="utf-8")
        body = {"schema":"crack_harness_transaction/v1","source_relpath":"src/owner.c","baseline_snapshot":str(baseline),"baseline_sha256":sha(self.base),"approval_sha256":"a"*64,"target_object_sha256":TARGET_SHA,"candidate_sha256":sha(self.candidate),"result_path":str(result_path),"record_commit_path":str(commit_path),"worktree":str(recovery / "worktree"),"central_record_binding":central_record_binding}
        (self.state / "transaction.json").write_text(json.dumps(body | {"transaction_sha256":harness._digest_json(body)}), encoding="utf-8")

    def test_exact_uses_disposable_worktree_and_keeps_no_source_duplicate(self) -> None:
        result = self.execute()
        self.assertEqual(result["status"], "exact", result)
        self.assertEqual(result["expected_terminal"], "exact")
        self.assertTrue(result["terminal_expectation_met"])
        self.assertEqual(self.source.read_text(), "int Owner(void) {\n    return 2;\n}\n", result)
        self.assertFalse(self.base.exists()); self.assertFalse(self.candidate.exists()); self.assertFalse(self.permit.exists()); self.assertFalse(self.approval.exists())
        run = next((self.state / "owners").glob("*/*/*"))
        self.assertFalse((run / "temp").exists()); self.assertFalse((run / "retained_winner.c").exists())
        self.assertTrue((run / "CRACK_REPORT_v1.json").is_file())
        self.assertTrue((run / "root-cleanup.receipt.json").is_file())

    def test_unregistered_orphan_worktree_is_removed_after_git_rejects_it(self) -> None:
        run = self.state / "owners/test/Owner/latest"
        destination = run / "temp/worktree"
        destination.mkdir(parents=True)
        (destination / "orphan.bin").write_bytes(b"orphan")

        harness._remove_disposable_worktree(self.root, destination)

        self.assertFalse(destination.exists())
        registered = subprocess.run(
            ["git", "worktree", "list", "--porcelain"],
            cwd=self.root, text=True, capture_output=True, check=True,
        ).stdout
        self.assertNotIn(str(destination), registered)

    def test_missing_unregistered_worktree_cleanup_is_idempotent(self) -> None:
        run = self.state / "owners/test/Owner/latest"
        destination = run / "temp/worktree"
        destination.parent.mkdir(parents=True)

        harness._remove_disposable_worktree(self.root, destination)

        self.assertFalse(destination.exists())
        registered = subprocess.run(
            ["git", "worktree", "list", "--porcelain"],
            cwd=self.root, text=True, capture_output=True, check=True,
        ).stdout
        self.assertNotIn(str(destination), registered)

    def test_registered_worktree_removal_failure_fails_closed(self) -> None:
        run = self.state / "owners/test/Owner/latest"
        destination = run / "temp/worktree"
        destination.parent.mkdir(parents=True)
        self._git("worktree", "add", "--detach", str(destination), "HEAD")
        marker = destination / "registered.bin"
        marker.write_bytes(b"registered")

        original_run = harness.subprocess.run

        def fail_registered_remove(argv: object, *args: object, **kwargs: object) -> object:
            if list(argv)[:4] == ["git", "worktree", "remove", "--force"]:
                return subprocess.CompletedProcess(argv, 1, "", "remove sentinel")
            return original_run(argv, *args, **kwargs)

        try:
            with patch.object(
                harness.subprocess, "run", side_effect=fail_registered_remove,
            ):
                with self.assertRaisesRegex(
                    harness.CrackHarnessError, "registered disposable worktree cleanup failed",
                ):
                    harness._remove_disposable_worktree(self.root, destination)
            self.assertTrue(destination.is_dir())
            self.assertTrue(marker.is_file())
        finally:
            subprocess.run(
                ["git", "worktree", "remove", "--force", str(destination)],
                cwd=self.root, text=True, capture_output=True, check=False,
            )

    def test_selection_evidence_cannot_bind_mutable_live_source(self) -> None:
        self._commit_live_source_evidence_fixture()
        approval, _ = self.write_inputs(live_source_evidence=True)
        with self.assertRaisesRegex(
            harness.CrackHarnessError,
            "selection evidence cannot bind the mutable live source",
        ):
            harness.load_approval(self.root, approval)

    def test_selection_evidence_cannot_bind_mutable_harness_state(self) -> None:
        approval, _ = self.write_inputs()
        frontier = (
            self.root / harness.DEFAULT_STATE_ROOT
            / "owners/owner/function/latest-frontier.json"
        )
        frontier.parent.mkdir(parents=True)
        frontier.write_text("{}\n", encoding="utf-8")
        evidence = json.loads(self.evidence.read_text(encoding="utf-8"))
        evidence["inputs"].append({
            "path": frontier.relative_to(self.root).as_posix(),
            "sha256": sha(frontier),
            "role": "signed_parent_frontier",
        })
        self.evidence.write_text(
            json.dumps(evidence, sort_keys=True, separators=(",", ":")) + "\n",
            encoding="utf-8",
        )
        approval_value = json.loads(approval.read_text(encoding="utf-8"))
        approval_value["selection"]["evidence"]["sha256"] = sha(self.evidence)
        approval.write_text(json.dumps(approval_value), encoding="utf-8")
        with self.assertRaisesRegex(
            harness.CrackHarnessError,
            "selection evidence cannot bind mutable harness state",
        ):
            harness.load_approval(self.root, approval)

    def test_legacy_winning_packet_without_current_residual_fails_closed(self) -> None:
        approval, _ = self.write_inputs()
        value = json.loads(approval.read_text(encoding="utf-8"))
        value["selection"].pop("current_residual")
        approval.write_text(json.dumps(value), encoding="utf-8")
        with self.assertRaisesRegex(
            harness.CrackHarnessError, "strict closed winning-cell-first",
        ):
            harness.load_approval(self.root, approval)

    def test_current_residual_must_bind_current_target_and_base(self) -> None:
        approval, _ = self.write_inputs()
        self._write_current_residual(
            [STRICT_ROW_0], target_sha256="0" * 64,
        )
        self._rebind_current_residual(approval)
        with self.assertRaisesRegex(
            harness.CrackHarnessError,
            "current residual evidence does not bind approval target_sha256",
        ):
            harness.load_approval(self.root, approval)

    def test_current_residual_rejects_self_hashed_forged_rows(self) -> None:
        approval, _ = self.write_inputs()
        residual = json.loads(self.current_residual.read_text(encoding="utf-8"))
        residual["residual_rows"] = ["FORGED:ROW"]
        unsigned = {
            key: value for key, value in residual.items()
            if key != "residual_sha256"
        }
        residual["residual_sha256"] = harness._digest_json(unsigned)
        self.current_residual.write_text(
            json.dumps(residual, sort_keys=True, separators=(",", ":")) + "\n",
            encoding="utf-8",
        )
        self._rebind_current_residual(approval)
        with self.assertRaisesRegex(
            harness.CrackHarnessError,
            "rows do not match the bound focus evidence",
        ):
            harness.load_approval(self.root, approval)

    def test_predicted_rows_must_come_from_current_residual(self) -> None:
        approval, _ = self.write_inputs()
        self._write_current_residual([STRICT_ROW_1])
        self._rebind_current_residual(approval)
        with self.assertRaisesRegex(
            harness.CrackHarnessError,
            "predicted_rows must be a subset of current residual rows",
        ):
            harness.load_approval(self.root, approval)

    def test_exact_prediction_must_cover_complete_current_residual(self) -> None:
        approval, _ = self.write_inputs(expected_terminal="exact")
        self._write_current_residual([STRICT_ROW_0, STRICT_ROW_1])
        self._rebind_current_residual(approval)
        with self.assertRaisesRegex(
            harness.CrackHarnessError,
            "expected exact requires complete current residual row coverage",
        ):
            harness.load_approval(self.root, approval)

    def test_improved_prediction_may_cover_current_residual_subset(self) -> None:
        approval, _ = self.write_inputs(expected_terminal="improved")
        self._write_current_residual([STRICT_ROW_0, STRICT_ROW_1])
        self._rebind_current_residual(approval)
        loaded = harness.load_approval(self.root, approval)
        self.assertEqual(loaded["selection"]["expected_terminal"], "improved")

    def test_positive_nonexact_retains_signed_frontier_without_report_or_record(self) -> None:
        result = self.execute(gain=2, focus_rows=1)
        self.assertEqual(result["status"], "improved", result)
        self.assertEqual(result["expected_terminal"], "exact")
        self.assertFalse(result["terminal_expectation_met"])
        self.assertIn("exact terminal expectation unmet", result["reason"])
        self.assertIn("partial frontier retained", result["reason"])
        self.assertFalse(any(self.state.glob("owners/*/*/latest/CRACK_REPORT_v1.json")))
        self.assertFalse(any(self.state.glob("owners/*/*/latest/result.json")))
        self.assertEqual(
            self.source.read_text(), "int Owner(void) {\n    return 2;\n}\n"
        )
        self.assertIn("discard", result["receipts"])
        self.assertNotIn("record", result["receipts"])
        frontier_path = next(self.state.glob("owners/*/*/latest-frontier.json"))
        frontier = harness._validate_frontier(
            self.root, frontier_path, manager_key_path=self.manager_key,
            expected_key_id=sha(self.manager_key),
        )
        self.assertEqual(frontier["frontier_sha256"], result["frontier_sha256"])
        self.assertEqual(frontier["candidate_sha256"], sha(self.source))

    def test_improved_prediction_is_met_by_an_improved_result(self) -> None:
        self._commit_expected_terminal_fixture("improved")
        result = self.execute(
            gain=2, focus_rows=1, expected_terminal="improved"
        )
        self.assertEqual(result["status"], "improved", result)
        self.assertEqual(result["expected_terminal"], "improved")
        self.assertTrue(result["terminal_expectation_met"])

    def test_improved_prediction_is_met_by_an_exact_result(self) -> None:
        self._commit_expected_terminal_fixture("improved")
        result = self.execute(expected_terminal="improved")
        self.assertEqual(result["status"], "exact", result)
        self.assertEqual(result["expected_terminal"], "improved")
        self.assertTrue(result["terminal_expectation_met"])

    def test_positive_nonexact_with_unchanged_physical_residual_is_retained(self) -> None:
        result = self.execute(
            gain=2, focus_rows=1,
            baseline_physical_diff=1, physical_diff=1,
        )
        self.assertEqual(result["status"], "improved", result)
        frontier_path = next(self.state.glob("owners/*/*/latest-frontier.json"))
        frontier = harness._validate_frontier(
            self.root, frontier_path, manager_key_path=self.manager_key,
            expected_key_id=sha(self.manager_key),
        )
        self.assertEqual(frontier["physical_differences"], 1)
        self.assertEqual(frontier["physical_diff_delta"], 0)
        self.assertEqual(
            self.source.read_text(), "int Owner(void) {\n    return 2;\n}\n"
        )

    def test_positive_nonexact_with_improved_physical_residual_is_retained(self) -> None:
        result = self.execute(
            gain=2, focus_rows=1,
            baseline_physical_diff=2, physical_diff=1,
        )
        self.assertEqual(result["status"], "improved", result)
        frontier_path = next(self.state.glob("owners/*/*/latest-frontier.json"))
        frontier = harness._validate_frontier(
            self.root, frontier_path, manager_key_path=self.manager_key,
            expected_key_id=sha(self.manager_key),
        )
        self.assertEqual(frontier["physical_differences"], 1)
        self.assertEqual(frontier["physical_diff_delta"], -1)

    def test_closed_size_channel_regression_is_not_retained(self) -> None:
        result = self.execute(
            gain=2, focus_rows=1,
            baseline_size_delta=0, size_delta=16,
        )
        self.assertEqual(result["status"], "no_gain", result)
        self.assertEqual(result["owner_gain"], 2)
        self.assertFalse(any(self.state.glob("owners/*/*/latest-frontier.json")))
        self.assertIn("return 1", self.source.read_text())

    def test_open_size_distance_improvement_is_retained(self) -> None:
        result = self.execute(
            gain=2, focus_rows=1,
            baseline_size_delta=132, size_delta=120,
        )
        self.assertEqual(result["status"], "improved", result)
        frontier_path = next(self.state.glob("owners/*/*/latest-frontier.json"))
        frontier = harness._validate_frontier(
            self.root, frontier_path, manager_key_path=self.manager_key,
            expected_key_id=sha(self.manager_key),
        )
        self.assertEqual(frontier["baseline_data_target_bytes"], 8)
        self.assertEqual(frontier["baseline_data_candidate_bytes"], 140)
        self.assertEqual(frontier["data_target_bytes"], 8)
        self.assertEqual(frontier["data_candidate_bytes"], 128)
        self.assertEqual(frontier["size_diff_delta"], -12)

    def test_equal_open_size_distance_requires_other_positive_gain(self) -> None:
        retained = self.execute(
            gain=2, focus_rows=1,
            baseline_size_delta=132, size_delta=132,
        )
        self.assertEqual(retained["status"], "improved", retained)

    def test_equal_open_size_distance_without_gain_is_not_retained(self) -> None:
        result = self.execute(
            gain=0, focus_rows=1,
            baseline_size_delta=132, size_delta=132,
        )
        self.assertEqual(result["status"], "no_gain", result)
        self.assertFalse(any(self.state.glob("owners/*/*/latest-frontier.json")))

    def test_open_size_distance_regression_is_not_retained(self) -> None:
        result = self.execute(
            gain=2, focus_rows=1,
            baseline_size_delta=132, size_delta=144,
        )
        self.assertEqual(result["status"], "no_gain", result)
        self.assertFalse(any(self.state.glob("owners/*/*/latest-frontier.json")))

    def test_assessment_rejects_missing_or_inconsistent_size_evidence(self) -> None:
        approval_path, _ = self.write_inputs()
        approval = harness.load_approval(self.root, approval_path)
        payload = {
            "schema": "crack_assessment/v1",
            "owner": "main:board/test",
            "function": "Owner",
            "candidate_source_sha256": sha(self.candidate),
            "target_object_sha256": "b" * 64,
            "candidate_object_sha256": "c" * 64,
            "owner_gain": 2,
            "data_gain": 0,
            "data_diff_delta": 0,
            "baseline_data_target_bytes": 8,
            "baseline_data_candidate_bytes": 140,
            "data_target_bytes": 8,
            "data_candidate_bytes": 128,
            "size_diff_delta": -12,
            "physical_diff_delta": 0,
        }
        missing = dict(payload)
        missing.pop("baseline_data_candidate_bytes")
        with self.assertRaisesRegex(
            harness.CrackHarnessError, "strict typed schema"
        ):
            harness._validate_assessment(
                missing, approval, ("b" * 64, "c" * 64)
            )
        inconsistent = dict(payload)
        inconsistent["size_diff_delta"] = -13
        with self.assertRaisesRegex(
            harness.CrackHarnessError, "inconsistent with bound byte counts"
        ):
            harness._validate_assessment(
                inconsistent, approval, ("b" * 64, "c" * 64)
            )

    def test_frontier_rejects_wrong_directory_and_tampering(self) -> None:
        result = self.execute(gain=2, focus_rows=1)
        self.assertEqual(result["status"], "improved", result)
        frontier_path = next(self.state.glob("owners/*/*/latest-frontier.json"))

        wrong_dir = frontier_path.parent.parent / "WrongFunction"
        wrong_dir.mkdir()
        wrong_path = wrong_dir / "latest-frontier.json"
        shutil.copyfile(frontier_path, wrong_path)
        with self.assertRaisesRegex(
            harness.CrackHarnessError, "stored under the wrong function"
        ):
            harness._validate_frontier(
                self.root, wrong_path, manager_key_path=self.manager_key,
                expected_key_id=sha(self.manager_key),
            )

        tampered = json.loads(frontier_path.read_text(encoding="utf-8"))
        tampered["physical_diff_delta"] = 1
        frontier_path.write_text(json.dumps(tampered), encoding="utf-8")
        with self.assertRaisesRegex(
            harness.CrackHarnessError, "frontier digest is invalid"
        ):
            harness._validate_frontier(
                self.root, frontier_path, manager_key_path=self.manager_key,
                expected_key_id=sha(self.manager_key),
            )

    def test_frontier_rejects_correctly_signed_negative_schema_field(self) -> None:
        result = self.execute(gain=2, focus_rows=1)
        self.assertEqual(result["status"], "improved", result)
        frontier_path = next(self.state.glob("owners/*/*/latest-frontier.json"))
        frontier = json.loads(frontier_path.read_text(encoding="utf-8"))
        frontier.pop("frontier_sha256")
        frontier.pop("signature")
        frontier["strict_differences"] = -1
        signed = {
            **frontier,
            "signature": hmac.new(
                self.manager_key.read_bytes(), harness._canonical(frontier),
                hashlib.sha256,
            ).hexdigest(),
        }
        sealed = {
            **signed, "frontier_sha256": harness._digest_json(signed),
        }
        frontier_path.write_text(json.dumps(sealed), encoding="utf-8")
        with self.assertRaisesRegex(
            harness.CrackHarnessError, "strict_differences is invalid"
        ):
            harness._validate_frontier(
                self.root, frontier_path, manager_key_path=self.manager_key,
                expected_key_id=sha(self.manager_key),
            )

    def test_frontier_rejects_resigned_inconsistent_size_evidence(self) -> None:
        result = self.execute(
            gain=2, focus_rows=1,
            baseline_size_delta=132, size_delta=120,
        )
        self.assertEqual(result["status"], "improved", result)
        frontier_path = next(self.state.glob("owners/*/*/latest-frontier.json"))
        frontier = json.loads(frontier_path.read_text(encoding="utf-8"))
        frontier.pop("frontier_sha256")
        frontier.pop("signature")
        frontier["size_diff_delta"] = -13
        signed = {
            **frontier,
            "signature": hmac.new(
                self.manager_key.read_bytes(), harness._canonical(frontier),
                hashlib.sha256,
            ).hexdigest(),
        }
        sealed = {**signed, "frontier_sha256": harness._digest_json(signed)}
        frontier_path.write_text(json.dumps(sealed), encoding="utf-8")
        with self.assertRaisesRegex(
            harness.CrackHarnessError, "inconsistent with bound byte counts"
        ):
            harness._validate_frontier(
                self.root, frontier_path, manager_key_path=self.manager_key,
                expected_key_id=sha(self.manager_key),
            )

    def test_no_gain_restores(self) -> None:
        result = self.execute(gain=0)
        self.assertEqual(result["status"], "no_gain", result)
        self.assertEqual(result["cleanup_status"], "complete")
        self.assertEqual(result["cleanup_errors"], [])
        self.assertIn("return 1", self.source.read_text())
        self.assertFalse(any(self.state.glob("owners/*/*/latest/result.json")))
        self.assertFalse(any(self.state.glob("owners/*/*/latest/temp")))
        self.assertFalse(any(self.state.glob("owners/*/*/latest")))
        self.assertTrue(any(self.state.glob("owners/*/*/latest-function.json")))
        self.assertIn("discard", result["receipts"])
        self.assertNotIn("record", result["receipts"])

    def test_positive_focus_with_closed_channel_regression_is_not_retained(self) -> None:
        result = self.execute(
            gain=2, focus_rows=1, sibling_losses=1, data_gain=-1,
            physical_diff=1, size_delta=1,
        )
        self.assertEqual(result["status"], "no_gain", result)
        self.assertIn("closed proof channel regressed", result["reason"])
        self.assertIn("return 1", self.source.read_text())
        self.assertIn("discard", result["receipts"])
        self.assertNotIn("record", result["receipts"])

    def test_production_apis_expose_no_state_root_override(self) -> None:
        for function in (harness.run_approved, harness.dry_run, harness.status):
            self.assertNotIn("state_root", inspect.signature(function).parameters)
        with self.assertRaisesRegex(harness.CrackHarnessError, "fixed"):
            harness._state_root(self.root, self.state)

    def test_manager_issuer_atomically_materializes_dry_run_ready_packet(self) -> None:
        draft = self.manager_draft()
        issued_approval = self.root / "issued-approval.json"
        issued_permit = self.root / "issued-permit.json"
        packet = harness._issue_manager_packet_for_test(
            self.root, draft, issued_approval, issued_permit,
            state_root=self.state, manager_key_path=self.manager_key,
        )
        self.assertEqual(packet["status"], "ready")
        self.assertEqual(packet["approval"]["sha256"], sha(issued_approval))
        self.assertEqual(packet["permit"]["sha256"], sha(issued_permit))
        loaded = harness.load_approval(self.root, issued_approval)
        harness._load_permit(
            self.root, loaded, issued_permit, self.state,
            manager_key_path=self.manager_key,
            expected_key_id=sha(self.manager_key),
        )
        self.assertEqual(
            harness._dry_run_for_test(
                self.root, issued_approval, state_root=self.state
            )["status"],
            "ready",
        )

    def test_manager_issuer_rolls_back_partial_publication(self) -> None:
        draft = self.manager_draft()
        issued_approval = self.root / "issued-approval.json"
        issued_permit = self.root / "issued-permit.json"
        prior_stop = (self.state / "STOP").read_bytes()
        with patch.object(
            harness, "_load_permit", side_effect=harness.CrackHarnessError("seal failed")
        ):
            with self.assertRaisesRegex(harness.CrackHarnessError, "seal failed"):
                harness._issue_manager_packet_for_test(
                    self.root, draft, issued_approval, issued_permit,
                    state_root=self.state, manager_key_path=self.manager_key,
                )
        self.assertFalse(issued_approval.exists())
        self.assertFalse(issued_permit.exists())
        self.assertEqual((self.state / "STOP").read_bytes(), prior_stop)

    def test_manager_issuer_rejects_outputs_inside_harness_state(self) -> None:
        draft = self.manager_draft()
        reserved = (
            self.state / "transaction.json",
            self.state / "RECOVERY_REQUIRED.json",
            self.state / "attempt.json",
            self.state / "owners/test/Owner/latest/result.json",
        )
        for path in reserved:
            with self.subTest(path=path):
                path.parent.mkdir(parents=True, exist_ok=True)
                with self.assertRaisesRegex(
                    harness.CrackHarnessError, "outside the harness state tree"
                ):
                    harness._issue_manager_packet_for_test(
                        self.root, draft, path,
                        self.root / f"permit-{path.name}.json",
                        state_root=self.state, manager_key_path=self.manager_key,
                    )
                self.assertFalse(path.exists())

        with self.assertRaisesRegex(
            harness.CrackHarnessError, "outside the harness state tree"
        ):
            harness._issue_manager_packet_for_test(
                self.root, draft,
                self.root / "approval-outside-state.json",
                self.state / "owners/test/Owner/latest/result.json",
                state_root=self.state, manager_key_path=self.manager_key,
            )

    def test_manager_issuer_preserves_primary_when_artifact_rollback_fails(self) -> None:
        draft = self.manager_draft()
        issued_approval = self.root / "issued-approval.json"
        issued_permit = self.root / "issued-permit.json"
        original_unlink = harness._safe_unlink

        def fail_artifact_unlink(path: Path) -> None:
            if path in {issued_approval, issued_permit}:
                raise OSError("artifact rollback denied")
            original_unlink(path)

        with patch.object(
            harness, "_load_permit", side_effect=harness.CrackHarnessError("primary seal failure")
        ), patch.object(harness, "_safe_unlink", side_effect=fail_artifact_unlink):
            with self.assertRaisesRegex(harness.CrackHarnessError, "primary seal failure") as raised:
                harness._issue_manager_packet_for_test(
                    self.root, draft, issued_approval, issued_permit,
                    state_root=self.state, manager_key_path=self.manager_key,
                )
        self.assertTrue(any("artifact rollback denied" in note for note in raised.exception.__notes__))
        stop = json.loads((self.state / "STOP").read_text(encoding="utf-8"))
        self.assertEqual(stop["authorized_permit_sha256"], "0" * 64)
        self.assertTrue(issued_approval.is_file())
        self.assertTrue(issued_permit.is_file())
        marker_path = self.state / "PACKET_ROLLBACK_REQUIRED.json"
        marker = json.loads(marker_path.read_text(encoding="utf-8"))
        marker_body = dict(marker)
        marker_sha256 = marker_body.pop("rollback_sha256")
        self.assertEqual(marker["schema"], harness.PACKET_ROLLBACK_REQUIRED_SCHEMA)
        self.assertEqual(marker_sha256, harness._digest_json(marker_body))
        self.assertTrue(
            harness._status_for_test(
                self.root, state_root=self.state,
                manager_key_path=self.manager_key,
            )["packet_rollback_required"]
        )
        self.assertEqual(
            harness._dry_run_for_test(
                self.root, issued_approval, state_root=self.state,
            )["status"],
            "blocked",
        )
        with self.assertRaisesRegex(harness.CrackHarnessError, "recovery state"):
            harness._issue_manager_packet_for_test(
                self.root, draft,
                self.root / "other-issued-approval.json",
                self.root / "other-issued-permit.json",
                state_root=self.state, manager_key_path=self.manager_key,
            )

    def test_atomic_json_flushes_file_and_directory(self) -> None:
        output = self.root / "state" / "durable.json"
        with patch.object(harness, "_directory_fsync") as directory_fsync, patch.object(
            harness.os, "fsync", wraps=harness.os.fsync
        ) as file_fsync:
            harness._atomic_json(output, {"durable": True})
        self.assertGreaterEqual(file_fsync.call_count, 1)
        directory_fsync.assert_called_once_with(output.parent)
        self.assertEqual(json.loads(output.read_text(encoding="utf-8")), {"durable": True})

    def test_manager_key_path_rejects_in_tree_and_symlink_paths(self) -> None:
        in_tree = self.root / "manager.key"
        in_tree.write_bytes(b"T" * 32)
        with self.assertRaisesRegex(harness.CrackHarnessError, "outside the repository"):
            harness._manager_key_file(self.root, in_tree)
        link = self.root / "manager-link"
        try:
            link.symlink_to(self.manager_key)
        except OSError as exc:
            self.skipTest(f"symlink creation unavailable: {exc}")
        with self.assertRaisesRegex(harness.CrackHarnessError, "plain file"):
            harness._manager_key_file(self.root, link)

    def test_legacy_guard_survives_history_pruning(self) -> None:
        function_dir = self.state / "owners/legacy-owner/Legacy"
        function_dir.mkdir(parents=True)
        legacy = function_dir / "latest-campaign.json"
        legacy.write_text(json.dumps({
            "schema": "crack_harness_campaign_tombstone/v1",
            "campaign_key": "a" * 64, "campaign_id": "old", "consumed": True,
        }), encoding="utf-8")
        (function_dir / "bulky.bin").write_bytes(b"x" * 1024)
        harness._prune_function_state(function_dir)
        self.assertTrue(legacy.is_file())
        self.assertFalse((function_dir / "bulky.bin").exists())

    def test_local_and_central_consumed_markers_independently_fail_closed(self) -> None:
        result = self.execute(gain=0)
        self.assertEqual(result["status"], "no_gain")
        function_dir = next((self.state / "owners").glob("*/*"))
        local = function_dir / "latest-function.json"
        local_bytes = local.read_bytes()
        central = self.state / "consumed-cells.json"
        central_bytes = central.read_bytes()

        self.base.write_bytes(self.source.read_bytes())
        self.candidate.write_text(
            "int Owner(void) {\n    return 2;\n}\n", encoding="utf-8"
        )
        self._write_luna5_audit()
        approval_path, _ = self.write_inputs(campaign_id="second")
        local.unlink()
        self.assertEqual(
            harness._dry_run_for_test(
                self.root, approval_path, state_root=self.state
            )["status"],
            "blocked",
        )
        local.write_bytes(local_bytes)
        central.unlink()
        self.assertEqual(
            harness._dry_run_for_test(
                self.root, approval_path, state_root=self.state
            )["status"],
            "blocked",
        )
        central.write_bytes(central_bytes)

    def test_signed_permit_rejects_approval_identity_drift(self) -> None:
        approval_path, permit_path = self.write_inputs()
        value = json.loads(approval_path.read_text(encoding="utf-8"))
        value["predicted_rows"] = [STRICT_ROW_1]
        value["selection"]["predicted_rows_sha256"] = harness._digest_json(
            value["predicted_rows"]
        )
        evidence = json.loads(self.evidence.read_text(encoding="utf-8"))
        evidence["predicted_rows_sha256"] = value["selection"]["predicted_rows_sha256"]
        evidence["causal_prediction"]["predicted_rows"] = value["predicted_rows"]
        self._write_current_residual(value["predicted_rows"])
        evidence["current_residual"]["sha256"] = sha(self.current_residual)
        self.evidence.write_text(json.dumps(evidence), encoding="utf-8")
        value["selection"]["evidence"]["sha256"] = sha(self.evidence)
        value["selection"]["current_residual"]["sha256"] = sha(
            self.current_residual
        )
        approval_path.write_text(json.dumps(value), encoding="utf-8")
        approval = harness.load_approval(self.root, approval_path)
        with self.assertRaisesRegex(harness.CrackHarnessError, "approval_identity_sha256"):
            harness._load_permit(
                self.root, approval, permit_path, self.state,
                manager_key_path=self.manager_key,
                expected_key_id=sha(self.manager_key),
            )

    def test_winning_cell_selection_is_required_and_rank_one(self) -> None:
        for mutation, message in (
            (lambda value: value.pop("selection"), "strict closed"),
            (lambda value: value["selection"].__setitem__("rank", 2), "rank"),
        ):
            with self.subTest(message=message):
                approval_path, _ = self.write_inputs()
                value = json.loads(approval_path.read_text(encoding="utf-8"))
                mutation(value)
                approval_path.write_text(json.dumps(value), encoding="utf-8")
                with self.assertRaisesRegex(harness.CrackHarnessError, message):
                    harness.load_approval(self.root, approval_path)

    def test_winning_cell_selection_rejects_evidence_candidate_and_rows_drift(self) -> None:
        approval_path, _ = self.write_inputs()
        self.evidence.write_text('{"evidence":"changed"}\n', encoding="utf-8")
        with self.assertRaisesRegex(harness.CrackHarnessError, "evidence hash"):
            harness.load_approval(self.root, approval_path)

        for field, expected_message in (
            ("candidate_sha256", "candidate"),
            ("predicted_rows_sha256", "predicted_rows"),
        ):
            with self.subTest(field=field):
                approval_path, _ = self.write_inputs()
                value = json.loads(approval_path.read_text(encoding="utf-8"))
                value["selection"][field] = "e" * 64
                approval_path.write_text(json.dumps(value), encoding="utf-8")
                with self.assertRaisesRegex(harness.CrackHarnessError, expected_message):
                    harness.load_approval(self.root, approval_path)

    def test_winning_cell_selection_rejects_alternatives_and_negative_controls(self) -> None:
        for field in ("alternatives_compiled", "negative_controls"):
            with self.subTest(field=field):
                approval_path, _ = self.write_inputs()
                value = json.loads(approval_path.read_text(encoding="utf-8"))
                value["selection"][field] = 1
                approval_path.write_text(json.dumps(value), encoding="utf-8")
                with self.assertRaisesRegex(harness.CrackHarnessError, field):
                    harness.load_approval(self.root, approval_path)

    def test_winning_cell_accepts_exact_or_improved_terminal(self) -> None:
        approval_path, _ = self.write_inputs()
        value = json.loads(approval_path.read_text(encoding="utf-8"))
        value["selection"]["expected_terminal"] = "improved"
        approval_path.write_text(json.dumps(value), encoding="utf-8")
        with self.assertRaisesRegex(harness.CrackHarnessError, "expected_terminal"):
            harness.load_approval(self.root, approval_path)

        approval_path, _ = self.write_inputs(expected_terminal="improved")
        self.assertEqual(harness.load_approval(self.root, approval_path)["selection"]["expected_terminal"], "improved")

        value = json.loads(approval_path.read_text(encoding="utf-8"))
        evidence = json.loads(self.evidence.read_text(encoding="utf-8"))
        value["selection"]["expected_terminal"] = "partial"
        evidence["expected_terminal"] = "partial"
        self.evidence.write_text(json.dumps(evidence), encoding="utf-8")
        approval = value
        approval["selection"]["evidence"]["sha256"] = sha(self.evidence)
        approval_path.write_text(json.dumps(approval), encoding="utf-8")
        with self.assertRaisesRegex(harness.CrackHarnessError, "exact or improved"):
            harness.load_approval(self.root, approval_path)

    def test_winning_cell_evidence_semantics_are_owner_function_and_rows_bound(self) -> None:
        for mutation, expected_message in (
            (lambda value: value.__setitem__("owner", "main:board/other"), "owner"),
            (lambda value: value.__setitem__("function", "Other"), "function"),
            (
                lambda value: value["causal_prediction"].__setitem__(
                    "predicted_rows", [STRICT_ROW_1]
                ),
                "causal_prediction",
            ),
        ):
            with self.subTest(expected_message=expected_message):
                approval_path, _ = self.write_inputs()
                evidence = json.loads(self.evidence.read_text(encoding="utf-8"))
                mutation(evidence)
                self.evidence.write_text(json.dumps(evidence), encoding="utf-8")
                approval = json.loads(approval_path.read_text(encoding="utf-8"))
                approval["selection"]["evidence"]["sha256"] = sha(self.evidence)
                approval_path.write_text(json.dumps(approval), encoding="utf-8")
                with self.assertRaisesRegex(harness.CrackHarnessError, expected_message):
                    harness.load_approval(self.root, approval_path)

    def test_winning_cell_evidence_inputs_are_hash_bound(self) -> None:
        approval_path, _ = self.write_inputs()
        evidence = json.loads(self.evidence.read_text(encoding="utf-8"))
        evidence["inputs"][0]["sha256"] = "e" * 64
        self.evidence.write_text(json.dumps(evidence), encoding="utf-8")
        approval = json.loads(approval_path.read_text(encoding="utf-8"))
        approval["selection"]["evidence"]["sha256"] = sha(self.evidence)
        approval_path.write_text(json.dumps(approval), encoding="utf-8")
        with self.assertRaisesRegex(harness.CrackHarnessError, "input hash mismatch"):
            harness.load_approval(self.root, approval_path)

    def test_valid_winning_cell_selection_is_bound_to_permit_identity(self) -> None:
        approval_path, permit_path = self.write_inputs()
        loaded = harness.load_approval(self.root, approval_path)
        self.assertEqual(loaded["selection"]["strategy"], "winning_cell_first")
        self.assertEqual(loaded["selection"]["rank"], 1)
        harness._load_permit(
            self.root, loaded, permit_path, self.state,
            manager_key_path=self.manager_key,
            expected_key_id=sha(self.manager_key),
        )
        value = json.loads(approval_path.read_text(encoding="utf-8"))
        value["selection"]["source_class"] = "different-winning-cell"
        evidence = json.loads(self.evidence.read_text(encoding="utf-8"))
        evidence["source_class"] = value["selection"]["source_class"]
        self.evidence.write_text(json.dumps(evidence), encoding="utf-8")
        value["selection"]["evidence"]["sha256"] = sha(self.evidence)
        approval_path.write_text(json.dumps(value), encoding="utf-8")
        drifted = harness.load_approval(self.root, approval_path)
        with self.assertRaisesRegex(harness.CrackHarnessError, "approval_identity_sha256"):
            harness._load_permit(
                self.root, drifted, permit_path, self.state,
                manager_key_path=self.manager_key,
                expected_key_id=sha(self.manager_key),
            )

    def test_five_luna_audit_rejects_duplicate_agent_and_commit_drift(self) -> None:
        for mutation, message in (
            (
                lambda value: value["receipts"][1].__setitem__(
                    "agent_id", value["receipts"][0]["agent_id"]
                ),
                "artifact content is unbound|duplicated, drifted",
            ),
            (
                lambda value: value["receipts"][0].__setitem__(
                    "controller_commit", "0" * 40
                ),
                "artifact content is unbound|duplicated, drifted",
            ),
        ):
            with self.subTest(message=message):
                self._write_luna5_audit()
                value = json.loads(self.luna_audit.read_text(encoding="utf-8"))
                mutation(value)
                self.luna_audit.write_text(json.dumps(value), encoding="utf-8")
                approval_path, _ = self.write_inputs()
                approval = json.loads(approval_path.read_text(encoding="utf-8"))
                approval["selection"]["luna5_audit"]["sha256"] = sha(self.luna_audit)
                approval_path.write_text(json.dumps(approval), encoding="utf-8")
                with self.assertRaisesRegex(harness.CrackHarnessError, message):
                    harness.load_approval(self.root, approval_path)
        self._write_luna5_audit()

    def test_five_luna_audit_rejects_hash_bound_but_incomplete_artifact(self) -> None:
        self._write_luna5_audit()
        audit = json.loads(self.luna_audit.read_text(encoding="utf-8"))
        descriptor = audit["receipts"][0]["artifact"]
        artifact = self.root / descriptor["path"]
        value = json.loads(artifact.read_text(encoding="utf-8"))
        value["checks"].pop(next(iter(value["checks"])))
        artifact.write_text(json.dumps(value), encoding="utf-8")
        descriptor["sha256"] = sha(artifact)
        self.luna_audit.write_text(json.dumps(audit), encoding="utf-8")
        approval_path, _ = self.write_inputs()
        approval = json.loads(approval_path.read_text(encoding="utf-8"))
        approval["selection"]["luna5_audit"]["sha256"] = sha(self.luna_audit)
        approval_path.write_text(json.dumps(approval), encoding="utf-8")
        with self.assertRaisesRegex(
            harness.CrackHarnessError, "artifact checks are incomplete"
        ):
            harness.load_approval(self.root, approval_path)
        self._write_luna5_audit()

    def test_signed_permit_rejects_every_runtime_identity_drift(self) -> None:
        approval_path, permit_path = self.write_inputs()
        mutations = {
            "candidate_sha256": lambda value: value["candidate"].__setitem__("sha256", "e" * 64),
            "source_sha256": lambda value: value["source"].__setitem__("sha256", "e" * 64),
            "base_sha256": lambda value: value["base"].__setitem__("sha256", "e" * 64),
            "base_commit": lambda value: value.__setitem__("base_commit", "other"),
            "toolchain_key": lambda value: value.__setitem__("toolchain_key", "e" * 64),
            "target_sha256": lambda value: value.__setitem__("target_sha256", "e" * 64),
            "approval_id": lambda value: value.__setitem__("approval_id", "other"),
            "commands_sha256": lambda value: value.__setitem__("_commands_sha256", "e" * 64),
        }
        for field, mutate in mutations.items():
            with self.subTest(field=field):
                approval = harness.load_approval(self.root, approval_path)
                mutate(approval)
                with self.assertRaisesRegex(harness.CrackHarnessError, field):
                    harness._load_permit(
                        self.root, approval, permit_path, self.state,
                        manager_key_path=self.manager_key,
                        expected_key_id=sha(self.manager_key),
                    )

    def test_nonfinite_assessment_and_proof_are_rejected(self) -> None:
        approval = {
            "owner": "main:board/test", "function": "Owner",
            "candidate": {"sha256": "a" * 64}, "target_sha256": "b" * 64,
        }
        assessment = {
            "schema": "crack_assessment/v1", "owner": approval["owner"],
            "function": "Owner", "candidate_source_sha256": "a" * 64,
            "target_object_sha256": "b" * 64,
            "candidate_object_sha256": "c" * 64,
            "owner_gain": float("nan"), "data_gain": 0.0,
            "data_diff_delta": 0,
            "baseline_data_target_bytes": 8,
            "baseline_data_candidate_bytes": 8,
            "data_target_bytes": 8,
            "data_candidate_bytes": 8,
            "size_diff_delta": 0,
            "physical_diff_delta": 0,
        }
        with self.assertRaisesRegex(harness.CrackHarnessError, "finite"):
            harness._validate_assessment(assessment, approval, ("b" * 64, "c" * 64))
        proof = {
            "schema": "crack_proof_strict/v1", "owner": approval["owner"],
            "function": "Owner", "candidate_source_sha256": "a" * 64,
            "target_object_sha256": "b" * 64,
            "candidate_object_sha256": "c" * 64, "report_sha256": "d" * 64,
            "strict_percent": float("inf"), "target_bytes": 4,
            "candidate_bytes": 4, "differences": 0,
        }
        with self.assertRaisesRegex(harness.CrackHarnessError, "non-negative numeric"):
            harness._validate_proof("strict", proof, approval, None)

    def test_unbound_legacy_journal_never_invalidates_central_record(self) -> None:
        candidate_record = "d" * 64
        store = RecoveryMemory(self.root / "memory.sqlite3")
        identity = RecoveryMemory.identity(
            owner="main:board/test", function="Owner", base_commit=self.commit,
            toolchain_key=harness.TOOLCHAIN_MANIFEST_KEY,
            target_sha256=TARGET_SHA, source_sha256=sha(self.candidate),
        )
        admitted = store.admit(identity, requester="lane")
        store.record(
            identity, requester="lane", object_sha256="c" * 64,
            status="improved", reason="retained",
            admission_token=admitted["admission_token"],
            candidate_record_sha256=candidate_record,
        )
        self.source.write_bytes(self.candidate.read_bytes())
        self._write_recovery_journal(result={}, central_record_binding={
            "input_key": identity["input_key"], "owner": "main:board/test",
            "function": "Owner", "source_sha256": sha(self.candidate),
            "target_object_sha256": TARGET_SHA,
            "object_sha256": "c" * 64,
            "candidate_record_sha256": candidate_record, "status": "improved",
        })
        with patch.object(RecoveryMemory, "for_root", return_value=store):
            with self.assertRaisesRegex(harness.CrackHarnessError, "central record retained"):
                harness._recover_interrupted(self.root, self.state)
        self.assertEqual(self.source.read_bytes(), self.candidate.read_bytes())
        with contextlib.closing(sqlite3.connect(store.path)) as connection:
            self.assertEqual(connection.execute("SELECT COUNT(*) FROM experiments").fetchone()[0], 1)

    def _write_bound_recovery_journal(
        self, *, central_record_binding: dict | None = None,
        exact_commit: bool = False,
    ) -> tuple[Path, dict]:
        """Create a v1 journal whose identity is bound to the real approval."""

        with patch.object(harness, "_validate_natural_cell", return_value=None):
            approval = harness.load_approval(self.root, self.approval)
        run = harness._run_dir(self.state, approval)
        temp = run / "temp"
        temp.mkdir(parents=True)
        baseline = temp / "baseline.snapshot"
        baseline.write_bytes(self.base.read_bytes())
        result_path = run / "result.json"
        report_path = run / "CRACK_REPORT_v1.json"
        commit_path = run / "record.commit.json"
        if exact_commit:
            commit_body = {
                "schema": "crack_harness_record_commit/v1",
                "outcome": "exact",
                "candidate_sha256": approval["candidate"]["sha256"],
                "record_payload_sha256": "e" * 64,
                "record_sha256": "e" * 64,
            }
            commit_path.write_text(
                json.dumps({**commit_body, "commit_sha256": harness._digest_json(commit_body)}),
                encoding="utf-8",
            )
        body = {
            "schema": harness.TRANSACTION_SCHEMA,
            "approval_path": str(self.approval),
            "approval_id": approval["approval_id"],
            "approval_identity_sha256": approval["_permit_identity_sha256"],
            "approval_sha256": approval["_approval_sha256"],
            "owner": approval["owner"],
            "function": approval["function"],
            "source_relpath": approval["_paths"]["source"].relative_to(self.root).as_posix(),
            "source_sha256": approval["source"]["sha256"],
            "base_relpath": approval["_paths"]["base"].relative_to(self.root).as_posix(),
            "base_sha256": approval["base"]["sha256"],
            "base_commit": approval["base_commit"],
            "candidate_relpath": approval["_paths"]["candidate"].relative_to(self.root).as_posix(),
            "candidate_sha256": approval["candidate"]["sha256"],
            "baseline_snapshot": str(baseline),
            "baseline_sha256": approval["base"]["sha256"],
            "target_object_sha256": approval["target_sha256"],
            "result_path": str(result_path),
            "report_path": str(report_path),
            "worktree": str(temp / "worktree"),
            "record_commit_path": str(commit_path),
            "central_record_binding": central_record_binding,
        }
        journal = self.state / "transaction.json"
        journal.write_text(
            json.dumps({**body, "transaction_sha256": harness._digest_json(body)}),
            encoding="utf-8",
        )
        return journal, approval

    def test_interrupted_journal_must_bind_the_actual_approval_file(self) -> None:
        self.write_inputs()
        journal, _ = self._write_bound_recovery_journal()
        value = json.loads(journal.read_text(encoding="utf-8"))
        value["owner"] = "main:board/forged"
        unsigned = dict(value)
        unsigned.pop("transaction_sha256")
        value["transaction_sha256"] = harness._digest_json(unsigned)
        journal.write_text(json.dumps(value), encoding="utf-8")
        self.source.write_bytes(self.candidate.read_bytes())
        with patch.object(harness, "_validate_natural_cell", return_value=None):
            with self.assertRaisesRegex(harness.CrackHarnessError, "owner.*approval"):
                harness._recover_interrupted(self.root, self.state)
        self.assertEqual(self.source.read_bytes(), self.candidate.read_bytes())
        self.assertTrue(journal.is_file())

    def test_unavailable_central_query_never_rolls_back_interrupted_source(self) -> None:
        self.write_inputs()
        binding = {
            "input_key": "a" * 64,
            "owner": "main:board/test",
            "function": "Owner",
            "source_sha256": sha(self.candidate),
            "target_object_sha256": TARGET_SHA,
            "object_sha256": "c" * 64,
            "candidate_record_sha256": "d" * 64,
            "status": "exact",
        }
        journal, _ = self._write_bound_recovery_journal(
            central_record_binding=binding, exact_commit=True,
        )
        self.source.write_bytes(self.candidate.read_bytes())
        with patch.object(
            harness, "_central_record_matches",
            side_effect=harness.CrackHarnessError("central unavailable"),
        ), patch.object(harness, "_validate_natural_cell", return_value=None):
            with self.assertRaisesRegex(
                harness.CrackHarnessError, "central unavailable"
            ):
                harness._recover_interrupted(self.root, self.state)
        self.assertEqual(self.source.read_bytes(), self.candidate.read_bytes())
        self.assertTrue(journal.is_file())

    def test_live_transaction_without_approval_fails_closed(self) -> None:
        self.write_inputs()
        journal, _ = self._write_bound_recovery_journal()
        self.source.write_bytes(self.candidate.read_bytes())
        self.approval.unlink()
        with self.assertRaisesRegex(
            harness.CrackHarnessError, "approval is missing"
        ):
            harness._recover_interrupted(self.root, self.state)
        self.assertEqual(self.source.read_bytes(), self.candidate.read_bytes())
        self.assertTrue(journal.is_file())

    def test_untrusted_journal_binding_never_invalidates_central_record(self) -> None:
        self.write_inputs()
        store = RecoveryMemory(self.root / "memory.sqlite3")
        with patch.object(RecoveryMemory, "for_root", return_value=store):
            with patch.object(harness, "_validate_natural_cell", return_value=None):
                approval = harness.load_approval(self.root, self.approval)
            identity = RecoveryMemory.identity(
                owner=approval["owner"], function=approval["function"],
                base_commit=approval["base_commit"],
                toolchain_key=harness.TOOLCHAIN_MANIFEST_KEY,
                target_sha256=approval["target_sha256"],
                source_sha256=approval["candidate"]["sha256"],
            )
            admitted = store.admit(identity, requester="lane")
            recorded = store.record(
                identity, requester="lane", object_sha256="c" * 64,
                status="exact", reason="retained",
                admission_token=admitted["admission_token"],
                candidate_record_sha256="d" * 64,
            )
            row = recorded["experiment"]
            journal, _ = self._write_bound_recovery_journal(
                central_record_binding={
                    "input_key": row["input_key"], "owner": row["owner"],
                    "function": row["function_name"],
                    "source_sha256": row["source_sha256"],
                    "target_object_sha256": row["target_sha256"],
                    "object_sha256": row["object_sha256"],
                    "candidate_record_sha256": row["candidate_record_sha256"],
                    "status": row["status"],
                },
                exact_commit=True,
            )
            self.source.write_bytes(self.candidate.read_bytes())
            with patch.object(harness, "_validate_natural_cell", return_value=None):
                with self.assertRaisesRegex(
                    harness.CrackHarnessError,
                    "central retained-record query is inconclusive",
                ):
                    harness._recover_interrupted(self.root, self.state)
            self.assertEqual(self.source.read_bytes(), self.candidate.read_bytes())
            self.assertTrue(journal.is_file())
            with contextlib.closing(sqlite3.connect(store.path)) as connection:
                self.assertEqual(
                    connection.execute("SELECT COUNT(*) FROM experiments").fetchone()[0],
                    1,
                )

    def test_recovery_binding_or_local_commit_mismatch_never_deletes_central_row(self) -> None:
        candidate_record = "d" * 64
        store = RecoveryMemory(self.root / "memory.sqlite3")
        identity = RecoveryMemory.identity(
            owner="main:board/test", function="Owner", base_commit=self.commit,
            toolchain_key=harness.TOOLCHAIN_MANIFEST_KEY,
            target_sha256=TARGET_SHA, source_sha256=sha(self.candidate),
        )
        admitted = store.admit(identity, requester="lane")
        recorded = store.record(
            identity, requester="lane", object_sha256="c" * 64,
            status="improved", reason="retained",
            admission_token=admitted["admission_token"],
            candidate_record_sha256=candidate_record,
        )
        row = recorded["experiment"]
        valid_binding = {
            "input_key": identity["input_key"], "owner": "main:board/test",
            "function": "Owner", "source_sha256": sha(self.candidate),
            "target_object_sha256": TARGET_SHA, "object_sha256": "c" * 64,
            "candidate_record_sha256": candidate_record, "status": "improved",
        }

        self._write_recovery_journal(
            result={},
            central_record_binding={
                **valid_binding, "target_object_sha256": "f" * 64,
            },
        )
        journal = self.state / "transaction.json"
        value = json.loads(journal.read_text(encoding="utf-8"))
        value["target_object_sha256"] = "f" * 64
        unsigned = dict(value)
        unsigned.pop("transaction_sha256", None)
        value["transaction_sha256"] = harness._digest_json(unsigned)
        journal.write_text(json.dumps(value), encoding="utf-8")
        with patch.object(RecoveryMemory, "for_root", return_value=store):
            with self.assertRaisesRegex(harness.CrackHarnessError, "cannot reconcile"):
                harness._recover_interrupted(self.root, self.state)
        with contextlib.closing(sqlite3.connect(store.path)) as connection:
            self.assertEqual(connection.execute("SELECT COUNT(*) FROM experiments").fetchone()[0], 1)

        forged_record_sha = "e" * 64
        run = self.state / "owners/test/Owner/latest"
        commit_body = {
            "schema": "crack_harness_record_commit/v1",
            "outcome": "improved",
            "candidate_sha256": sha(self.candidate),
            "record_payload_sha256": "e" * 64,
            "record_sha256": forged_record_sha,
        }
        (run / "record.commit.json").write_text(
            json.dumps({
                **commit_body,
                "commit_sha256": harness._digest_json(commit_body),
            }),
            encoding="utf-8",
        )
        value = json.loads(journal.read_text(encoding="utf-8"))
        value["target_object_sha256"] = TARGET_SHA
        value["central_record_binding"] = valid_binding
        unsigned = dict(value)
        unsigned.pop("transaction_sha256", None)
        value["transaction_sha256"] = harness._digest_json(unsigned)
        journal.write_text(json.dumps(value), encoding="utf-8")
        with patch.object(RecoveryMemory, "for_root", return_value=store):
            with self.assertRaisesRegex(harness.CrackHarnessError, "cannot reconcile"):
                harness._recover_interrupted(self.root, self.state)
        with contextlib.closing(sqlite3.connect(store.path)) as connection:
            self.assertEqual(connection.execute("SELECT COUNT(*) FROM experiments").fetchone()[0], 1)
        self.assertEqual(row["record_sha256"], store.verify_retained(**{
            "input_key": identity["input_key"], "owner": identity["owner"],
            "function": identity["function"], "target_sha256": TARGET_SHA,
            "source_sha256": identity["source_sha256"], "object_sha256": "c" * 64,
            "candidate_record_sha256": candidate_record, "status": "improved",
        })["record_sha256"])

    def test_self_asserted_manager_permit_is_rejected(self) -> None:
        approval_path, permit_path = self.write_inputs()
        loaded = harness.load_approval(self.root, approval_path)
        forged_key = Path(self.secret_temp.name) / "forged.key"; forged_key.write_bytes(b"F" * 32)
        permit = json.loads(permit_path.read_text()); permit["key_id"] = sha(forged_key)
        permit.pop("signature"); permit["signature"] = hmac.new(forged_key.read_bytes(), harness._canonical(permit), hashlib.sha256).hexdigest()
        permit_path.write_text(json.dumps(permit), encoding="utf-8")
        loaded["permit_sha256"] = sha(permit_path)
        stop = json.loads((self.state / "STOP").read_text()); stop["authorized_permit_sha256"] = sha(permit_path)
        (self.state / "STOP").write_text(json.dumps(stop), encoding="utf-8")
        with self.assertRaisesRegex(harness.CrackHarnessError, "key_id|signature"):
            harness._load_permit(self.root, loaded, permit_path, self.state, manager_key_path=self.manager_key, expected_key_id=sha(self.manager_key))

    def test_future_timestamps_and_wrong_toolchain_are_rejected(self) -> None:
        approval, _ = self.write_inputs(); value = json.loads(approval.read_text())
        future = datetime.now(timezone.utc) + timedelta(minutes=5)
        value["issued_at"] = future.isoformat(); value["expires_at"] = (future + timedelta(minutes=10)).isoformat()
        approval.write_text(json.dumps(value), encoding="utf-8")
        with self.assertRaisesRegex(harness.CrackHarnessError, "future"):
            harness.load_approval(self.root, approval)
        approval, _ = self.write_inputs(); value = json.loads(approval.read_text()); value["toolchain_key"] = "0" * 64
        approval.write_text(json.dumps(value), encoding="utf-8")
        with self.assertRaisesRegex(harness.CrackHarnessError, "Ninja-inclusive"):
            harness.load_approval(self.root, approval)

    def test_live_source_and_sealed_base_must_use_distinct_paths(self) -> None:
        approval, _ = self.write_inputs()
        value = json.loads(approval.read_text(encoding="utf-8"))
        value["base"] = dict(value["source"])
        approval.write_text(json.dumps(value), encoding="utf-8")
        with self.assertRaisesRegex(
            harness.CrackHarnessError,
            "live source and sealed base must use separate paths",
        ):
            harness.load_approval(self.root, approval)

    def test_future_manager_permit_is_rejected_even_when_signed(self) -> None:
        approval_path, permit_path = self.write_inputs(); loaded = harness.load_approval(self.root, approval_path)
        permit = json.loads(permit_path.read_text()); permit.pop("signature")
        future = datetime.now(timezone.utc) + timedelta(minutes=5)
        permit["issued_at"] = future.isoformat(); permit["deadline"] = (future + timedelta(minutes=10)).isoformat()
        permit["signature"] = hmac.new(self.manager_key.read_bytes(), harness._canonical(permit), hashlib.sha256).hexdigest()
        permit_path.write_text(json.dumps(permit), encoding="utf-8"); loaded["permit_sha256"] = sha(permit_path)
        stop = json.loads((self.state / "STOP").read_text()); stop["authorized_permit_sha256"] = sha(permit_path)
        (self.state / "STOP").write_text(json.dumps(stop), encoding="utf-8")
        with self.assertRaisesRegex(harness.CrackHarnessError, "future"):
            harness._load_permit(self.root, loaded, permit_path, self.state, manager_key_path=self.manager_key, expected_key_id=sha(self.manager_key))

    def test_controller_scripts_cannot_resolve_from_detached_worktree(self) -> None:
        approval, _ = self.write_inputs(); value = json.loads(approval.read_text())
        value["commands"]["compile"]["argv"][1] = "{RUN_ROOT}/tools/crack_evidence_bundle.py"
        approval.write_text(json.dumps(value), encoding="utf-8")
        with self.assertRaisesRegex(harness.CrackHarnessError, "pinned script|front door"):
            harness.load_approval(self.root, approval)

    def test_fake_admission_shape_is_rejected(self) -> None:
        approval, _ = self.write_inputs()
        value = json.loads(approval.read_text()); value["commands"]["precompile"]["argv"].append("--fake")
        approval.write_text(json.dumps(value), encoding="utf-8")
        with self.assertRaisesRegex(harness.CrackHarnessError, "exact supported"):
            harness.load_approval(self.root, approval)

    def test_arbitrary_repo_local_proof_hook_is_rejected(self) -> None:
        approval, _ = self.write_inputs(); fake = self.root / "fake-proof.py"
        fake.write_text("print('{}')\n", encoding="utf-8")
        value = json.loads(approval.read_text()); descriptor = value["commands"]["strict"]
        descriptor["script"] = {"path":str(fake), "sha256":sha(fake)}
        descriptor["argv"][1] = "{RUN_ROOT}/fake-proof.py"
        approval.write_text(json.dumps(value), encoding="utf-8")
        with self.assertRaisesRegex(harness.CrackHarnessError, "pinned script|canonical registered script"):
            harness.load_approval(self.root, approval)

    def test_embedded_production_path_and_mutated_script_are_rejected(self) -> None:
        approval, _ = self.write_inputs(); value = json.loads(approval.read_text())
        value["commands"]["compile"]["argv"].append(str(self.root / "build/out"))
        approval.write_text(json.dumps(value), encoding="utf-8")
        with self.assertRaisesRegex(harness.CrackHarnessError, "evidence-bundle front door|production writable path"):
            harness.load_approval(self.root, approval)
        self.write_inputs(); self.hook.write_text(self.hook.read_text() + "# mutation\n", encoding="utf-8")
        with self.assertRaisesRegex(harness.CrackHarnessError, "script hash mismatch"):
            harness.load_approval(self.root, self.approval)

    def test_cmd_executable_cannot_replace_controller_python(self) -> None:
        approval, _ = self.write_inputs(); value = json.loads(approval.read_text())
        command = value["commands"]["compile"]; cmd = Path(os.environ["COMSPEC"]).resolve()
        command["executable"] = {"path": str(cmd), "sha256": sha(cmd)}; command["argv"][0] = str(cmd)
        approval.write_text(json.dumps(value), encoding="utf-8")
        with self.assertRaisesRegex(harness.CrackHarnessError, "exact controller interpreter"):
            harness.load_approval(self.root, approval)

    def test_fake_python_and_casing_alias_are_rejected(self) -> None:
        approval, _ = self.write_inputs(); fake = self.root / "fake-python.exe"
        fake.write_bytes(b"not a Python interpreter")
        value = json.loads(approval.read_text()); command = value["commands"]["strict"]
        command["executable"] = {"path": str(fake), "sha256": sha(fake)}; command["argv"][0] = str(fake)
        approval.write_text(json.dumps(value), encoding="utf-8")
        with self.assertRaisesRegex(harness.CrackHarnessError, "exact controller interpreter"):
            harness.load_approval(self.root, approval)
        approval, _ = self.write_inputs(); value = json.loads(approval.read_text())
        command = value["commands"]["record"]; alias = str(Path(sys.executable).resolve()).swapcase()
        command["executable"]["path"] = alias; command["argv"][0] = alias
        approval.write_text(json.dumps(value), encoding="utf-8")
        with self.assertRaisesRegex(harness.CrackHarnessError, "exact controller interpreter"):
            harness.load_approval(self.root, approval)

    def test_fake_proof_identity_is_rejected(self) -> None:
        approval, _ = self.write_inputs(); loaded = harness.load_approval(self.root, approval)
        payload = {"schema":"crack_proof_focus/v1","owner":"wrong","function":"Owner","candidate_source_sha256":sha(self.candidate),"target_object_sha256":"b"*64,"candidate_object_sha256":"c"*64,"report_sha256":"d"*64,"differing_rows":0}
        with self.assertRaisesRegex(harness.CrackHarnessError, "does not bind owner"):
            harness._validate_proof("focus", payload, loaded, None)
        payload["owner"] = "main:board/test"; payload["target_object_sha256"] = "0" * 64
        with self.assertRaisesRegex(harness.CrackHarnessError, "approved target"):
            harness._validate_proof("focus", payload, loaded, None)

    def test_assessment_must_bind_proven_object_pair(self) -> None:
        approval, _ = self.write_inputs(); loaded = harness.load_approval(self.root, approval)
        payload = {"schema":"crack_assessment/v1","owner":"main:board/test","function":"Owner","candidate_source_sha256":sha(self.candidate),"target_object_sha256":"b"*64,"candidate_object_sha256":"0"*64,"owner_gain":1,"data_gain":0,"data_diff_delta":0,"baseline_data_target_bytes":8,"baseline_data_candidate_bytes":8,"data_target_bytes":8,"data_candidate_bytes":8,"size_diff_delta":0,"physical_diff_delta":0}
        with self.assertRaisesRegex(harness.CrackHarnessError, "does not bind"):
            harness._validate_assessment(payload, loaded, ("b" * 64, "c" * 64))

    def test_tampered_evidence_receipt_is_rejected_and_rolled_back(self) -> None:
        approval, permit = self.write_inputs()
        os.environ["HARNESS_TEST_GAIN"] = "1"; os.environ["HARNESS_TEST_FOCUS"] = "0"
        os.environ["HARNESS_TEST_BAD_RECEIPT"] = "1"
        try:
            result = harness._run_approved_for_test(
                self.root, approval, permit_path=permit, state_root=self.state,
                manager_key_path=self.manager_key,
            )
        finally:
            os.environ.pop("HARNESS_TEST_BAD_RECEIPT", None)
            os.environ.pop("HARNESS_TEST_GAIN", None); os.environ.pop("HARNESS_TEST_FOCUS", None)
        self.assertEqual(result["status"], "failed", result)
        self.assertIn("receipt digest", result["reason"])
        self.assertEqual(result["cleanup_status"], "complete", result)
        self.assertIn("return 1", self.source.read_text())
        self.assertIn("discard", result["receipts"])
        self.assertNotIn("record", result["receipts"])
        diagnostic = next(self.state.glob("owners/*/*/latest-failure.json"))
        self.assertEqual(
            json.loads(diagnostic.read_text(encoding="utf-8"))["cleanup_errors"],
            [],
        )

    def test_stop_is_rechecked_before_every_discard_command(self) -> None:
        original = harness._validate_assessment

        def revoke_stop(*args, **kwargs):
            value = original(*args, **kwargs)
            stop_path = self.state / "STOP"
            stop = json.loads(stop_path.read_text(encoding="utf-8"))
            stop["authorized_permit_sha256"] = "0" * 64
            stop_path.write_text(json.dumps(stop), encoding="utf-8")
            return value

        with (
            patch.object(harness, "_validate_assessment", side_effect=revoke_stop),
            patch.object(harness, "_run_canonical_discard") as discard,
        ):
            result = self.execute(gain=0)
        self.assertEqual(result["status"], "failed")
        discard.assert_not_called()

    def test_real_proof_adapter_derives_closed_payloads_from_reports(self) -> None:
        import copy
        from tools.tests.test_focus_symbol_report import FUNCTION, _physical_receipt, _report

        adapter = self.root / "adapter"; adapter.mkdir()
        baseline_report = _report(focus_exact=False, sibling_exact=True)
        candidate_report = _report(focus_exact=True, sibling_exact=True)
        candidate_data_report = _report(focus_exact=True, sibling_exact=False)
        paths = {}
        for name, value in (("baseline-strict", baseline_report), ("baseline-data", copy.deepcopy(baseline_report)), ("candidate-strict", candidate_report), ("candidate-data", candidate_data_report)):
            paths[name] = adapter / f"{name}.json"; paths[name].write_text(json.dumps(value), encoding="utf-8")
        physical = _physical_receipt(); physical["report"]["sha256"] = sha(paths["candidate-strict"])
        physical_path = adapter / "physical.json"; physical_path.write_text(json.dumps(physical), encoding="utf-8")
        baseline_physical = _physical_receipt()
        baseline_physical["report"]["sha256"] = sha(paths["baseline-strict"])
        baseline_physical_path = adapter / "baseline-physical.json"
        baseline_physical_path.write_text(
            json.dumps(baseline_physical), encoding="utf-8"
        )
        target = adapter / "target.o"; target.write_bytes(b"target-object")
        candidate = adapter / "candidate.o"; candidate.write_bytes(b"candidate-object")
        arguments = dict(owner="main:board/test", function=FUNCTION, candidate_source=self.candidate, candidate_source_sha256=sha(self.candidate), approved_target_object_sha256=sha(target), target_object=target, candidate_object=candidate, baseline_strict_report=paths["baseline-strict"], baseline_data_report=paths["baseline-data"], candidate_strict_report=paths["candidate-strict"], candidate_data_report=paths["candidate-data"], baseline_physical_receipt=baseline_physical_path, physical_receipt=physical_path)
        strict = harness._proof_adapter_payload(kind="strict", **arguments)
        assess = harness._proof_adapter_payload(kind="assess", **arguments)
        physical_proof = harness._proof_adapter_payload(kind="physical", **arguments)
        sibling_proof = harness._proof_adapter_payload(kind="siblings", **arguments)
        self.assertEqual(strict["schema"], "crack_proof_strict/v1")
        self.assertEqual(strict["strict_percent"], 100.0)
        self.assertEqual(assess["owner_gain"], 25.0)
        self.assertEqual(assess["data_gain"], 25.0)
        self.assertLessEqual(assess["data_diff_delta"], 0)
        self.assertEqual(assess["physical_diff_delta"], 0)
        self.assertEqual(physical_proof["differences"], 0)
        self.assertEqual(sibling_proof["protected_losses"], 1)

    def test_non_elevatable_limits_and_natural_c_markers(self) -> None:
        approval, _ = self.write_inputs(); value = json.loads(approval.read_text()); value["limits"]["active_seconds"] = 1801
        approval.write_text(json.dumps(value), encoding="utf-8")
        with self.assertRaisesRegex(harness.CrackHarnessError, "non-elevatable"):
            harness.load_approval(self.root, approval)
        self.candidate.write_text("int Owner(void) {\n    volatile int x = 2;\n}\n", encoding="utf-8")
        with self.assertRaisesRegex(harness.CrackHarnessError, "shaping marker"):
            harness._validate_natural_cell(self.base, self.candidate, 1, 3)

    def test_natural_cell_accepts_insert_delete_and_declaration_use_hunks(self) -> None:
        inserted = self.root / "inserted.c"
        inserted.write_text("int Owner(void) {\n    int value = 2;\n    return value;\n}\n", encoding="utf-8")
        harness._validate_natural_cell(self.base, inserted, 1, 3)
        larger_base = self.root / "larger-base.c"
        deleted = self.root / "deleted.c"
        larger_base.write_text("int Owner(void) {\n    int value = 2;\n    value += 0;\n    return value;\n}\n", encoding="utf-8")
        deleted.write_text("int Owner(void) {\n    int value = 2;\n    return value;\n}\n", encoding="utf-8")
        harness._validate_natural_cell(larger_base, deleted, 1, 5)

    def test_natural_cell_rejects_directives_and_function_boundary_injection(self) -> None:
        directive = self.root / "directive.c"
        directive.write_text(
            "int Owner(void) {\n#if ENABLE_OWNER\n    return 2;\n#endif\n}\n",
            encoding="utf-8",
        )
        with self.assertRaisesRegex(harness.CrackHarnessError, "preprocessor directive"):
            harness._validate_natural_cell(self.base, directive, 1, 3)

        nested_function = self.root / "nested-function.c"
        nested_function.write_text(
            "int Owner(void) {\n    int Evil(void) { return 4; }\n    return 2;\n}\n",
            encoding="utf-8",
        )
        with self.assertRaisesRegex(harness.CrackHarnessError, "nested function boundary"):
            harness._validate_natural_cell(self.base, nested_function, 1, 3)

        trailing_function = self.root / "trailing-function.c"
        trailing_function.write_text(
            "int Owner(void) {\n    return 2;\n}\nint Evil(void) { return 4; }\n",
            encoding="utf-8",
        )
        with self.assertRaisesRegex(
            harness.CrackHarnessError, "outside the approved function span|function boundary"
        ):
            harness._validate_natural_cell(self.base, trailing_function, 1, 3)

    def test_natural_cell_allows_legitimate_nested_body_control_blocks(self) -> None:
        candidate = self.root / "nested-control.c"
        candidate.write_text(
            "int Owner(void) {\n    if (1) {\n        return 2;\n    }\n    return 1;\n}\n",
            encoding="utf-8",
        )
        harness._validate_natural_cell(self.base, candidate, 1, 3)

    def test_translation_unit_cell_allows_static_data_move_across_unchanged_function(self) -> None:
        base = self.root / "translation-unit-base.c"
        candidate = self.root / "translation-unit-candidate.c"
        base_text = (
            "static int first = 1;\n"
            "int Keep(void) { return first; }\n"
            "static int second = 2;\n"
            "int Tail(void) { return second; }\n"
        )
        candidate_text = (
            "static int second = 2;\n"
            "int Keep(void) { return first; }\n"
            "static int first = 1;\n"
            "int Tail(void) { return second; }\n"
        )
        base.write_bytes(base_text.encode("utf-8"))
        candidate.write_bytes(candidate_text.encode("utf-8"))
        scope = {
            "kind": "translation_unit",
            "start_line": 1,
            "end_line": len(base_text.splitlines()),
            "base_span_sha256": sha(base),
        }
        harness._validate_natural_cell(
            base, candidate, 2, 2,
            hashlib.sha256(base.read_bytes().splitlines(keepends=True)[1]).hexdigest(),
            cell_scope=scope,
        )

    def test_translation_unit_cell_rejects_function_boundary_mutation(self) -> None:
        base = self.root / "translation-unit-boundary-base.c"
        candidate = self.root / "translation-unit-boundary-candidate.c"
        base_text = (
            "static int first = 1;\n"
            "int Keep(void) { return first; }\n"
            "static int second = 2;\n"
            "int Tail(void) { return second; }\n"
        )
        candidate_text = base_text.replace("int Tail(void)", "int Renamed(void)")
        base.write_bytes(base_text.encode("utf-8"))
        candidate.write_bytes(candidate_text.encode("utf-8"))
        scope = {
            "kind": "translation_unit",
            "start_line": 1,
            "end_line": 4,
            "base_span_sha256": sha(base),
        }
        with self.assertRaisesRegex(
            harness.CrackHarnessError, "top-level function boundary"
        ):
            harness._validate_natural_cell(
                base, candidate, 2, 2, cell_scope=scope,
            )

    def test_translation_unit_cell_rejects_bad_scope_hash_and_outside_edit(self) -> None:
        base = self.root / "translation-unit-scope-base.c"
        candidate = self.root / "translation-unit-scope-candidate.c"
        base_text = (
            "static int first = 1;\n"
            "int Keep(void) { return first; }\n"
            "static int second = 2;\n"
            "int Tail(void) { return second; }\n"
        )
        base.write_bytes(base_text.encode("utf-8"))
        candidate.write_bytes(base_text.replace("second = 2", "second = 3").encode("utf-8"))
        wrong_hash_scope = {
            "kind": "translation_unit", "start_line": 1, "end_line": 4,
            "base_span_sha256": "0" * 64,
        }
        with self.assertRaisesRegex(
            harness.CrackHarnessError, "translation-unit span hash"
        ):
            harness._validate_natural_cell(
                base, candidate, 2, 2, cell_scope=wrong_hash_scope,
            )

        outside_candidate = self.root / "translation-unit-outside.c"
        outside_candidate.write_bytes(base_text.replace("first = 1", "first = 9").encode("utf-8"))
        scope_text = b"".join(base.read_bytes().splitlines(keepends=True)[1:])
        scope = {
            "kind": "translation_unit", "start_line": 2, "end_line": 4,
            "base_span_sha256": hashlib.sha256(scope_text).hexdigest(),
        }
        with self.assertRaisesRegex(
            harness.CrackHarnessError, "outside the approved translation-unit span"
        ):
            harness._validate_natural_cell(
                base, outside_candidate, 2, 2, cell_scope=scope,
            )

    def test_default_function_scope_rejects_static_move_across_function(self) -> None:
        base = self.root / "function-scope-base.c"
        candidate = self.root / "function-scope-candidate.c"
        base_text = (
            "static int first = 1;\n"
            "int Keep(void) { return first; }\n"
            "static int second = 2;\n"
            "int Tail(void) { return second; }\n"
        )
        candidate_text = (
            "static int second = 2;\n"
            "int Keep(void) { return first; }\n"
            "static int first = 1;\n"
            "int Tail(void) { return second; }\n"
        )
        base.write_bytes(base_text.encode("utf-8"))
        candidate.write_bytes(candidate_text.encode("utf-8"))
        with self.assertRaisesRegex(
            harness.CrackHarnessError, "approved function span|function boundary"
        ):
            harness._validate_natural_cell(
                base, candidate, 2, 2,
                hashlib.sha256(base.read_bytes().splitlines(keepends=True)[1]).hexdigest(),
            )

    def test_predicted_rows_must_be_unique_at_runtime(self) -> None:
        approval, _ = self.write_inputs()
        value = json.loads(approval.read_text(encoding="utf-8"))
        value["predicted_rows"] = [STRICT_ROW_0, STRICT_ROW_0]
        approval.write_text(json.dumps(value), encoding="utf-8")
        with self.assertRaisesRegex(harness.CrackHarnessError, "unique rows"):
            harness.load_approval(self.root, approval)

    def test_result_schema_closes_receipt_shapes_and_predicted_rows(self) -> None:
        schema = json.loads(
            (Path(__file__).parents[1] / "CRACK_HARNESS_RESULT_V1.schema.json").read_text(
                encoding="utf-8"
            )
        )
        self.assertTrue(schema["properties"]["predicted_rows"]["uniqueItems"])
        receipts = schema["properties"]["receipts"]["properties"]
        self.assertEqual(receipts["strict"]["$ref"], "#/$defs/compact_receipt")
        self.assertEqual(receipts["discard_failure"]["$ref"], "#/$defs/failure_receipt")
        self.assertEqual(receipts["secondary_failures"]["$ref"], "#/$defs/secondary_failures")
        self.assertTrue(schema["$defs"]["compact_receipt"]["additionalProperties"] is False)
        self.assertIn("command", schema["$defs"]["failure_receipt"]["required"])
        self.assertIn(
            "object_observed",
            schema["$defs"]["failure_command_receipt"]["required"],
        )
        exact_contract = schema["allOf"][0]["then"]
        self.assertEqual(
            set(exact_contract["required"]), {"owner_gain", "report_sha256"}
        )
        self.assertEqual(
            exact_contract["properties"]["owner_gain"]["exclusiveMinimum"], 0
        )

    def test_forged_minimal_terminal_result_rolls_back_and_is_deleted(self) -> None:
        self.source.write_bytes(self.candidate.read_bytes())
        body = {"status": "improved", "receipts": {"record": {"payload_sha256": "e" * 64}}}
        self._write_recovery_journal(result=body | {"result_sha256": harness._digest_json(body)})
        harness._recover_interrupted(self.root, self.state)
        self.assertEqual(self.source.read_bytes(), self.base.read_bytes())
        self.assertFalse((self.state / "transaction.json").exists())
        self.assertFalse(any(self.state.glob("owners/*/*/latest/result.json")))

    def test_exact_crash_without_bound_report_rolls_back(self) -> None:
        self.source.write_bytes(self.candidate.read_bytes())
        body = {"status":"exact","candidate_sha256":sha(self.candidate),"receipts":{"record":{"payload_sha256":"e"*64}}}
        self._write_recovery_journal(result=body | {"result_sha256":harness._digest_json(body)}, exact_commit=True)
        harness._recover_interrupted(self.root, self.state)
        self.assertEqual(self.source.read_bytes(), self.base.read_bytes())
        self.assertFalse((self.state / "RECOVERY_REQUIRED.json").exists())
        self.assertFalse((self.state / "transaction.json").exists())
        diagnostic_path = self.state / "owners" / "test" / "Owner" / "latest-failure.json"
        diagnostic = json.loads(diagnostic_path.read_text(encoding="utf-8"))
        diagnostic_body = dict(diagnostic)
        diagnostic_sha256 = diagnostic_body.pop("diagnostic_sha256")
        self.assertEqual(diagnostic_sha256, harness._digest_json(diagnostic_body))
        self.assertIn("lacked a complete hash-bound CRACK_REPORT", diagnostic["primary_reason"])

    def test_interrupted_cleanup_failure_retains_journal_and_recovery_marker(self) -> None:
        self.write_inputs()
        binding = {
            "input_key": "a" * 64,
            "owner": "main:board/test",
            "function": "Owner",
            "source_sha256": sha(self.candidate),
            "target_object_sha256": TARGET_SHA,
            "object_sha256": "b" * 64,
            "candidate_record_sha256": "c" * 64,
            "status": "exact",
        }
        journal, approval = self._write_bound_recovery_journal(
            central_record_binding=binding,
        )
        run = harness._run_dir(self.state, approval)
        result_path = run / "result.json"
        report_path = run / "CRACK_REPORT_v1.json"
        metadata = {
            "approval_sha256": approval["_approval_sha256"],
            "owner": approval["owner"],
            "function": approval["function"],
            "base_commit": approval["base_commit"],
            "candidate_sha256": approval["candidate"]["sha256"],
        }
        result_path.write_text(
            json.dumps({"status": "exact", **metadata}), encoding="utf-8",
        )
        report_path.write_text(
            json.dumps({"status": "exact", **metadata}), encoding="utf-8",
        )
        self.source.write_bytes(self.candidate.read_bytes())
        (run / "temp/worktree").mkdir(parents=True)
        (self.state / "attempt.json").write_text("{}", encoding="utf-8")

        with patch.object(
            harness, "_central_record_matches", return_value=False,
        ), patch.object(
            harness, "_terminal_binding_from_result", return_value=binding,
        ), patch.object(
            harness, "_valid_terminal_result", return_value=True,
        ), patch.object(
            harness, "_remove_disposable_worktree",
            side_effect=OSError("worktree cleanup denied"),
        ):
            with self.assertRaisesRegex(OSError, "worktree cleanup denied"):
                harness._recover_interrupted(self.root, self.state)

        self.assertTrue(journal.is_file())
        marker = self.state / "RECOVERY_REQUIRED.json"
        self.assertTrue(marker.is_file())
        self.assertEqual(
            json.loads(marker.read_text(encoding="utf-8"))["schema"],
            harness.RECOVERY_REQUIRED_SCHEMA,
        )

    def test_permit_must_bind_stop_and_approval(self) -> None:
        approval, permit = self.write_inputs(); value = json.loads(permit.read_text()); value["owner"] = "wrong"
        permit.write_text(json.dumps(value), encoding="utf-8")
        loaded = harness.load_approval(self.root, approval)
        with self.assertRaisesRegex(harness.CrackHarnessError, "signature|exact assignment permit"):
            harness._load_permit(self.root, loaded, permit, self.state, manager_key_path=self.manager_key, expected_key_id=sha(self.manager_key))

    def test_permit_deadline_cannot_outlive_bound_approval(self) -> None:
        approval_path, permit_path = self.write_inputs()
        approval_value = json.loads(approval_path.read_text(encoding="utf-8"))
        approval_value["expires_at"] = (
            datetime.fromisoformat(approval_value["issued_at"]) + timedelta(minutes=10)
        ).isoformat()
        permit_value = json.loads(permit_path.read_text(encoding="utf-8"))
        permit_value["approval_identity_sha256"] = harness._approval_permit_identity(approval_value)
        permit_value.pop("signature")
        permit_value["signature"] = hmac.new(
            self.manager_key.read_bytes(), harness._canonical(permit_value), hashlib.sha256
        ).hexdigest()
        permit_path.write_text(json.dumps(permit_value), encoding="utf-8")
        approval_value["permit_sha256"] = sha(permit_path)
        approval_path.write_text(json.dumps(approval_value), encoding="utf-8")
        loaded = harness.load_approval(self.root, approval_path)
        with self.assertRaisesRegex(harness.CrackHarnessError, "outlives"):
            harness._load_permit(
                self.root, loaded, permit_path, self.state,
                manager_key_path=self.manager_key,
                expected_key_id=sha(self.manager_key),
            )

    def test_child_environment_cannot_redirect_central_state(self) -> None:
        run_temp = self.root / "env-test"; run_temp.mkdir()
        prior = {name: os.environ.get(name) for name in ("MP6_RECOVERY_MEMORY", "MP6_AGENT_QUEUE", "GIT_DIR")}
        os.environ.update({name: "redirected" for name in prior})
        try:
            payload, _ = harness._run_command(
                [sys.executable, "-c", "import json,os;print(json.dumps({k:os.environ.get(k) for k in ('MP6_RECOVERY_MEMORY','MP6_AGENT_QUEUE','GIT_DIR')}))"],
                root=self.root, run_temp=run_temp,
                deadline=time.monotonic() + 5, storage_limit=4096,
                expect_json=True,
            )
        finally:
            for name, value in prior.items():
                if value is None: os.environ.pop(name, None)
                else: os.environ[name] = value
        self.assertEqual(payload, {"MP6_RECOVERY_MEMORY": None, "MP6_AGENT_QUEUE": None, "GIT_DIR": None})

    def test_controller_import_cannot_materialize_bytecode_outside_run_roots(self) -> None:
        module = self.root / "controller_bytecode_probe.py"
        module.write_text("VALUE = 7\n", encoding="utf-8")
        command_root = self.state / "bytecode-command"
        run_temp = self.state / "bytecode-temp"
        command_root.mkdir()
        run_temp.mkdir()
        forced_cache = self.root / "forced-pycache"
        try:
            payload, _ = harness._run_command(
                [
                    sys.executable,
                    "-c",
                    "import controller_bytecode_probe,json;"
                    "print(json.dumps({'value':controller_bytecode_probe.VALUE}))",
                ],
                root=command_root,
                run_temp=run_temp,
                deadline=time.monotonic() + 5,
                storage_limit=4096,
                expect_json=True,
                production_root=self.root,
                state_root=self.state,
                extra_env={
                    "PYTHONPATH": str(self.root),
                    "PYTHONDONTWRITEBYTECODE": "",
                    "PYTHONPYCACHEPREFIX": str(forced_cache),
                },
            )
            self.assertEqual(payload, {"value": 7})
            self.assertFalse(forced_cache.exists())
            self.assertFalse(any(
                (self.root / "__pycache__").glob("controller_bytecode_probe.*.pyc")
            ))
        finally:
            module.unlink(missing_ok=True)
            shutil.rmtree(forced_cache, ignore_errors=True)

    def test_external_write_is_rejected_with_changed_path(self) -> None:
        command_root = self.state / "external-write-command"
        run_temp = self.state / "external-write-temp"
        command_root.mkdir()
        run_temp.mkdir()
        marker = self.root / "forbidden-write.txt"
        command = (
            "from pathlib import Path;"
            f"Path({str(marker)!r}).write_text('forbidden')"
        )
        try:
            with self.assertRaisesRegex(
                harness.CrackHarnessError, "forbidden-write.txt"
            ):
                harness._run_command(
                    [sys.executable, "-c", command],
                    root=command_root,
                    run_temp=run_temp,
                    deadline=time.monotonic() + 5,
                    storage_limit=4096,
                    expect_json=False,
                    production_root=self.root,
                    state_root=self.state,
                )
        finally:
            marker.unlink(missing_ok=True)

    def test_started_callback_does_not_mask_concurrent_state_write(self) -> None:
        run_temp = self.state / "manifest-race-temp"
        run_temp.mkdir()
        allowed = self.state / "allowed-start-marker.json"
        rogue = self.state / "rogue-start-write.json"

        def started() -> None:
            allowed.write_text("authorized callback marker", encoding="utf-8")
            time.sleep(0.2)

        command = (
            "from pathlib import Path;import time;"
            f"Path({str(rogue)!r}).write_text('rogue', encoding='utf-8');"
            "time.sleep(1)"
        )
        try:
            with self.assertRaisesRegex(
                harness.CrackHarnessError, "reviewed command wrote outside its monitored run root"
            ):
                harness._run_command(
                    [sys.executable, "-c", command],
                    root=self.root, run_temp=run_temp,
                    deadline=time.monotonic() + 5,
                    storage_limit=4096, expect_json=False,
                    production_root=self.root, state_root=self.state,
                    on_started=started,
                    on_started_state_paths=(allowed,),
                )
        finally:
            allowed.unlink(missing_ok=True)
            rogue.unlink(missing_ok=True)

    def test_compile_failure_primary_cause_survives_cleanup_failure(self) -> None:
        with patch.object(
            harness, "_validate_evidence_receipt",
            side_effect=harness.CrackHarnessError("primary compile evidence failure"),
        ), patch.object(
            harness, "_cleanup_raw", side_effect=OSError("secondary cleanup failure")
        ):
            result = self.execute()
        self.assertEqual(result["status"], "failed")
        self.assertIn("primary compile evidence failure", result["reason"])
        diagnostic = next(self.state.glob("owners/*/*/latest-failure.json"))
        value = json.loads(diagnostic.read_text(encoding="utf-8"))
        self.assertIn("primary compile evidence failure", value["primary_reason"])
        self.assertTrue(any("secondary cleanup failure" in item for item in value["cleanup_errors"]))

    def test_source_rollback_failure_preserves_transaction_and_recovery_marker(self) -> None:
        original_copy = harness._atomic_copy

        def fail_only_source_rollback(source: Path, destination: Path) -> None:
            if (
                Path(destination) == self.source
                and Path(source).is_file()
                and Path(source).read_bytes() == self.base.read_bytes()
            ):
                raise OSError("source rollback denied")
            original_copy(Path(source), Path(destination))

        with patch.object(
            harness, "_run_canonical_record",
            side_effect=harness.CrackHarnessError("record response unavailable"),
        ), patch.object(
            harness, "_central_record_matches", return_value=False,
        ), patch.object(
            harness, "_atomic_copy", side_effect=fail_only_source_rollback,
        ):
            with self.assertRaisesRegex(
                harness.CrackHarnessError, "record response unavailable"
            ):
                self.execute()
        self.assertEqual(self.source.read_bytes(), self.candidate.read_bytes())
        self.assertTrue((self.state / "transaction.json").is_file())
        self.assertTrue((self.state / "RECOVERY_REQUIRED.json").is_file())
        self.assertTrue((self.state / "attempt.json").is_file())
        self.assertTrue(self.approval.is_file())

        with patch.object(harness, "_central_record_matches", return_value=False):
            harness._recover_interrupted(self.root, self.state)
        self.assertEqual(self.source.read_bytes(), self.base.read_bytes())
        self.assertFalse((self.state / "transaction.json").exists())
        self.assertFalse((self.state / "RECOVERY_REQUIRED.json").exists())

    def test_command_nonzero_exit_is_not_masked_by_quiesce_failure(self) -> None:
        run_temp = self.root / "command-primary-temp"
        run_temp.mkdir()
        with patch.object(
            harness, "_quiesce_windows_job",
            side_effect=OSError("secondary quiesce failure"),
        ):
            with self.assertRaisesRegex(
                harness.CrackHarnessError, r"reviewed command failed \(7\)"
            ) as raised:
                harness._run_command(
                    [sys.executable, "-c", "raise SystemExit(7)"],
                    root=self.root, run_temp=run_temp,
                    deadline=time.monotonic() + 5,
                    storage_limit=4096, expect_json=False,
                )
        receipt = getattr(raised.exception, "_crack_command_receipt")
        self.assertEqual(receipt["returncode"], 7)
        self.assertEqual(receipt["stdout_sha256"], hashlib.sha256(b"").hexdigest())
        self.assertEqual(receipt["stderr_sha256"], hashlib.sha256(b"").hexdigest())
        self.assertFalse(receipt["object_observed"])
        self.assertTrue(any("secondary quiesce failure" in item for item in receipt["cleanup_errors"]))

    def test_command_failure_seals_streams_and_observed_object(self) -> None:
        run_temp = self.root / "command-evidence-temp"
        run_temp.mkdir()
        script = (
            "import pathlib,sys;"
            f"pathlib.Path({str(run_temp / 'observed.o')!r}).write_bytes(b'partial');"
            "print('known stdout');"
            "print('known stderr',file=sys.stderr);"
            "raise SystemExit(7)"
        )
        with self.assertRaisesRegex(
            harness.CrackHarnessError, r"reviewed command failed \(7\)"
        ) as raised:
            harness._run_command(
                [sys.executable, "-c", script], root=self.root,
                run_temp=run_temp, deadline=time.monotonic() + 5,
                storage_limit=4096, expect_json=False,
            )
        receipt = getattr(raised.exception, "_crack_command_receipt")
        self.assertEqual(receipt["returncode"], 7)
        self.assertEqual(
            receipt["stdout_sha256"],
            hashlib.sha256(("known stdout" + os.linesep).encode()).hexdigest(),
        )
        self.assertEqual(
            receipt["stderr_sha256"],
            hashlib.sha256(("known stderr" + os.linesep).encode()).hexdigest(),
        )
        self.assertTrue(receipt["object_observed"])

    def test_failed_terminal_diagnostic_preserves_command_receipt(self) -> None:
        os.environ["HARNESS_TEST_COMMAND_FAIL"] = "1"
        try:
            result = self.execute()
        finally:
            os.environ.pop("HARNESS_TEST_COMMAND_FAIL", None)
        self.assertEqual(result["status"], "failed")
        command = result["receipts"]["failure"]["command"]
        self.assertEqual(command["returncode"], 7)
        self.assertTrue(command["object_observed"])
        diagnostic_path = next(self.state.glob("owners/*/*/latest-failure.json"))
        diagnostic = json.loads(diagnostic_path.read_text(encoding="utf-8"))
        body = dict(diagnostic)
        digest = body.pop("diagnostic_sha256")
        self.assertEqual(digest, harness._digest_json(body))
        self.assertEqual(diagnostic["schema"], "crack_harness_failure_diagnostic/v2")
        self.assertEqual(diagnostic["failure_receipt"]["command"], command)

    def test_command_success_seals_cleanup_failures_in_receipt(self) -> None:
        run_temp = self.root / "command-cleanup-temp"
        run_temp.mkdir()
        with patch.object(
            harness, "_quiesce_windows_job",
            side_effect=OSError("secondary quiesce failure"),
        ), patch.object(
            harness, "_close_windows_job",
            side_effect=OSError("secondary close failure"),
        ):
            with self.assertRaisesRegex(
                harness.CrackHarnessError, "did not quiesce"
            ) as raised:
                harness._run_command(
                    [sys.executable, "-c", "import json;print(json.dumps({'ok': True}))"],
                    root=self.root, run_temp=run_temp,
                    deadline=time.monotonic() + 5,
                    storage_limit=4096, expect_json=True,
                )
        receipt = raised.exception._crack_command_receipt
        self.assertEqual(receipt["returncode"], 0)
        self.assertEqual(
            receipt["cleanup_errors"],
            ["quiesce: secondary quiesce failure", "close job: secondary close failure"],
        )
        self.assertFalse(receipt["object_observed"])
        self.assertRegex(receipt["stdout_sha256"], r"^[0-9a-f]{64}$")
        self.assertRegex(receipt["stderr_sha256"], r"^[0-9a-f]{64}$")

    def test_exact_success_survives_post_terminal_cleanup_failure(self) -> None:
        with patch.object(
            harness, "_cleanup_raw", side_effect=OSError("cleanup exact failed")
        ):
            result = self.execute()
        self.assertEqual(result["status"], "exact")
        self.assertEqual(result["cleanup_status"], "cleanup_incomplete")
        self.assertIn("return 2", self.source.read_text())
        self.assertTrue(any("cleanup exact failed" in item for item in result["cleanup_errors"]))
        self.assertFalse(any(self.state.glob("owners/*/*/latest-failure.json")))

    def test_improved_cleanup_failure_preserves_primary_frontier(self) -> None:
        with patch.object(
            harness, "_cleanup_raw", side_effect=OSError("cleanup improved failed")
        ):
            result = self.execute(gain=2, focus_rows=1)
        self.assertEqual(result["status"], "improved")
        self.assertEqual(result["cleanup_status"], "cleanup_incomplete")
        self.assertTrue(any("cleanup improved failed" in item for item in result["cleanup_errors"]))
        self.assertEqual(
            self.source.read_text(), "int Owner(void) {\n    return 2;\n}\n"
        )
        frontier_path = next(self.state.glob("owners/*/*/latest-frontier.json"))
        self.assertEqual(
            harness._validate_frontier(
                self.root, frontier_path, manager_key_path=self.manager_key,
                expected_key_id=sha(self.manager_key),
            )["frontier_sha256"],
            result["frontier_sha256"],
        )
        self.assertFalse(any(self.state.glob("owners/*/*/latest/CRACK_REPORT_v1.json")))
        self.assertFalse(any(self.state.glob("owners/*/*/latest-failure.json")))

    def test_pending_frontier_publication_recovers_without_rollback(self) -> None:
        real_replace = harness.os.replace

        def fail_frontier_publication(source: str, destination: str, *args: object, **kwargs: object) -> None:
            if (
                Path(source).name == "frontier.pending.json"
                and Path(destination).name == "latest-frontier.json"
            ):
                raise OSError("frontier publication failed")
            real_replace(source, destination, *args, **kwargs)

        with patch.object(harness.os, "replace", side_effect=fail_frontier_publication):
            result = self.execute(gain=2, focus_rows=1)
        self.assertEqual(result["status"], "improved", result)
        self.assertIn("secondary_failures", result["receipts"])
        function_dir = next(self.state.glob("owners/*/*"))
        pending = function_dir / "frontier.pending.json"
        frontier = function_dir / "latest-frontier.json"
        self.assertTrue(pending.is_file())
        self.assertFalse(frontier.exists())
        self.assertEqual(
            self.source.read_text(encoding="utf-8"),
            "int Owner(void) {\n    return 2;\n}\n",
        )

        status = harness._status_for_test(
            self.root, state_root=self.state, manager_key_path=self.manager_key
        )
        self.assertEqual(status["results"], [])
        self.assertFalse(pending.exists())
        self.assertTrue(frontier.is_file())
        self.assertEqual(
            harness._validate_frontier(
                self.root, frontier, manager_key_path=self.manager_key,
                expected_key_id=sha(self.manager_key),
            )["frontier_sha256"],
            result["frontier_sha256"],
        )

    def test_exact_cell_retires_prior_partial_frontier(self) -> None:
        first = self.execute(gain=2, focus_rows=1, campaign_id="partial-before-exact")
        self.assertEqual(first["status"], "improved", first)
        frontier_path = next(self.state.glob("owners/*/*/latest-frontier.json"))
        self.assertTrue(frontier_path.is_file())

        self.base.write_bytes(self.source.read_bytes())
        self.candidate.write_text(
            "int Owner(void) {\n    return 3;\n}\n", encoding="utf-8"
        )
        self.evidence = self.root / "evidence/selection-exact-after-partial.json"
        self.luna_audit = self.root / "evidence/luna5-exact-after-partial.json"
        self._write_luna5_audit()
        exact = self.execute(
            gain=1, focus_rows=0, campaign_id="exact-after-partial"
        )
        self.assertEqual(exact["status"], "exact", exact)
        self.assertEqual(
            self.source.read_text(encoding="utf-8"),
            "int Owner(void) {\n    return 3;\n}\n",
        )
        self.assertFalse(frontier_path.exists())
        self.assertFalse(any(self.state.glob("owners/*/*/latest-frontier.json")))

    def test_exact_frontier_cleanup_failure_is_retryable_after_root_cleanup(self) -> None:
        first = self.execute(
            gain=2, focus_rows=1, campaign_id="partial-before-cleanup-retry"
        )
        self.assertEqual(first["status"], "improved", first)
        frontier_path = next(self.state.glob("owners/*/*/latest-frontier.json"))

        self.base.write_bytes(self.source.read_bytes())
        self.candidate.write_text(
            "int Owner(void) {\n    return 3;\n}\n", encoding="utf-8"
        )
        self.evidence = self.root / "evidence/selection-exact-cleanup-retry.json"
        self.luna_audit = self.root / "evidence/luna5-exact-cleanup-retry.json"
        self._write_luna5_audit()

        real_unlink = harness._safe_unlink
        frontier_failure = False

        def fail_frontier_once(path: Path) -> None:
            nonlocal frontier_failure
            if Path(path) == frontier_path and not frontier_failure:
                frontier_failure = True
                raise OSError("retired frontier cleanup failed once")
            real_unlink(path)

        with patch.object(harness, "_safe_unlink", side_effect=fail_frontier_once):
            exact = self.execute(
                gain=1, focus_rows=0, campaign_id="exact-cleanup-retry"
            )
        self.assertEqual(exact["status"], "exact", exact)
        self.assertEqual(exact["cleanup_status"], "cleanup_incomplete", exact)
        self.assertTrue(frontier_path.is_file())
        run_dir = next(self.state.glob("owners/*/*/latest"))
        self.assertTrue((run_dir / "root-cleanup.receipt.json").is_file())
        self.assertFalse((self.state / "transaction.json").exists())
        self.assertFalse((self.state / "attempt.json").exists())

        with patch.object(harness, "_central_record_matches", return_value=True):
            harness._status_for_test(
                self.root, state_root=self.state,
                manager_key_path=self.manager_key,
            )
        self.assertFalse(frontier_path.exists())
        final = json.loads((run_dir / "result.json").read_text(encoding="utf-8"))
        self.assertEqual(final["status"], "exact", final)
        self.assertEqual(final["cleanup_status"], "complete", final)

    def test_partial_frontier_continues_monotonically_and_rejects_stale_base(self) -> None:
        original_base = self.base.read_bytes()

        first = self.execute(gain=2, focus_rows=1, campaign_id="frontier-a")
        self.assertEqual(first["status"], "improved", first)
        frontier_path = next(self.state.glob("owners/*/*/latest-frontier.json"))
        frontier_a = harness._validate_frontier(
            self.root, frontier_path, manager_key_path=self.manager_key,
            expected_key_id=sha(self.manager_key),
        )
        self.assertEqual(frontier_a["candidate_sha256"], sha(self.source))

        # The retained candidate is deliberately dirty in the tracked source;
        # the next cell seals it as its baseline instead of rebuilding from
        # the original commit.
        self.base.write_bytes(self.source.read_bytes())
        self.candidate.write_text(
            "int Owner(void) {\n    return 3;\n}\n", encoding="utf-8"
        )
        self.evidence = self.root / "evidence/selection-frontier-b.json"
        self.luna_audit = self.root / "evidence/luna5-frontier-b.json"
        self._write_luna5_audit()
        second = self.execute(gain=3, focus_rows=1, campaign_id="frontier-b")
        self.assertEqual(second["status"], "improved", second)
        self.assertEqual(
            self.source.read_text(encoding="utf-8"),
            "int Owner(void) {\n    return 3;\n}\n",
        )
        frontier_b = harness._validate_frontier(
            self.root, frontier_path, manager_key_path=self.manager_key,
            expected_key_id=sha(self.manager_key),
        )
        self.assertEqual(frontier_b["parent_frontier_sha256"], frontier_a["frontier_sha256"])
        self.assertEqual(frontier_b["candidate_sha256"], sha(self.source))
        self.assertNotEqual(frontier_b["frontier_sha256"], frontier_a["frontier_sha256"])

        # A cell based on the original committed source is stale after the
        # first improvement and must stop before candidate execution.
        # Build the stale packet while source/base are still identical (the
        # approval contract requires that), then restore the retained live
        # source before invoking it.  Its candidate is the already-retained
        # value, so the frontier check—not a second compile—must reject it.
        self.source.write_bytes(original_base)
        self.base.write_bytes(original_base)
        self.candidate.write_text(
            "int Owner(void) {\n    return 3;\n}\n", encoding="utf-8"
        )
        self.evidence = self.root / "evidence/selection-stale.json"
        self.luna_audit = self.root / "evidence/luna5-stale.json"
        self._write_luna5_audit()
        stale_approval, stale_permit = self.write_inputs(campaign_id="stale-base")
        self.source.write_text(
            "int Owner(void) {\n    return 3;\n}\n", encoding="utf-8"
        )
        with self.assertRaisesRegex(
            harness.CrackHarnessError, "stale relative to the retained partial frontier"
        ):
            harness._run_approved_for_test(
                self.root, stale_approval, permit_path=stale_permit,
                state_root=self.state, manager_key_path=self.manager_key,
            )
        self.assertEqual(
            self.source.read_text(encoding="utf-8"),
            "int Owner(void) {\n    return 3;\n}\n",
        )
        self.assertEqual(
            harness._validate_frontier(
                self.root, frontier_path, manager_key_path=self.manager_key,
                expected_key_id=sha(self.manager_key),
            )["frontier_sha256"],
            frontier_b["frontier_sha256"],
        )

        # A neutral/regressing cell also rolls back to the current retained
        # frontier, never to the original committed source.
        self.base.write_bytes(self.source.read_bytes())
        self.candidate.write_text(
            "int Owner(void) {\n    return 5;\n}\n", encoding="utf-8"
        )
        self.evidence = self.root / "evidence/selection-neutral.json"
        self.luna_audit = self.root / "evidence/luna5-neutral.json"
        self._write_luna5_audit()
        neutral = self.execute(
            gain=0, focus_rows=1, campaign_id="neutral-after-frontier"
        )
        self.assertEqual(neutral["status"], "no_gain", neutral)
        self.assertEqual(
            self.source.read_text(encoding="utf-8"),
            "int Owner(void) {\n    return 3;\n}\n",
        )
        self.assertEqual(
            harness._validate_frontier(
                self.root, frontier_path, manager_key_path=self.manager_key,
                expected_key_id=sha(self.manager_key),
            )["frontier_sha256"],
            frontier_b["frontier_sha256"],
        )

        # Only the compact frontier and one-shot guard files remain; no raw
        # candidate/run directories or pending frontier history accumulate.
        function_dir = frontier_path.parent
        self.assertTrue(frontier_path.is_file())
        self.assertFalse((function_dir / "frontier.pending.json").exists())
        self.assertFalse(any(child.is_dir() for child in function_dir.iterdir()))
        self.assertLessEqual(
            len(list(function_dir.glob("latest-frontier.json"))), 1
        )

    def test_partial_frontier_continues_across_source_neutral_release(self) -> None:
        first = self.execute(gain=2, focus_rows=1, campaign_id="release-a")
        self.assertEqual(first["status"], "improved", first)
        frontier_path = next(self.state.glob("owners/*/*/latest-frontier.json"))
        frontier_a = harness._validate_frontier(
            self.root, frontier_path, manager_key_path=self.manager_key,
            expected_key_id=sha(self.manager_key),
        )

        self.hook.write_text(
            self.hook.read_text(encoding="utf-8") + "\n# harness-only release\n",
            encoding="utf-8",
        )
        self._git("add", "tools/crack_harness.py")
        self._git("commit", "-qm", "harness-only release")
        self.commit = self._git("rev-parse", "HEAD")

        self.base.write_bytes(self.source.read_bytes())
        self.candidate.write_text(
            "int Owner(void) {\n    return 3;\n}\n", encoding="utf-8"
        )
        self.evidence = self.root / "evidence/selection-release-b.json"
        self.luna_audit = self.root / "evidence/luna5-release-b.json"
        self._write_luna5_audit()
        second = self.execute(gain=3, focus_rows=1, campaign_id="release-b")
        self.assertEqual(second["status"], "improved", second)
        frontier_b = harness._validate_frontier(
            self.root, frontier_path, manager_key_path=self.manager_key,
            expected_key_id=sha(self.manager_key),
        )
        self.assertEqual(
            frontier_b["parent_frontier_sha256"], frontier_a["frontier_sha256"]
        )
        self.assertEqual(frontier_b["base_commit"], self.commit)

    def test_partial_frontier_rejects_release_that_changes_source_blob(self) -> None:
        first = self.execute(gain=2, focus_rows=1, campaign_id="source-a")
        self.assertEqual(first["status"], "improved", first)
        retained_source = self.source.read_bytes()

        self.source.write_text(
            "int Owner(void) {\n    return 9;\n}\n", encoding="utf-8"
        )
        self._git("add", "src/owner.c")
        self._git("commit", "-qm", "change tracked owner source")
        self.commit = self._git("rev-parse", "HEAD")
        self.source.write_bytes(retained_source)

        self.base.write_bytes(retained_source)
        self.candidate.write_text(
            "int Owner(void) {\n    return 3;\n}\n", encoding="utf-8"
        )
        self.evidence = self.root / "evidence/selection-source-b.json"
        self.luna_audit = self.root / "evidence/luna5-source-b.json"
        self._write_luna5_audit()
        approval, permit = self.write_inputs(campaign_id="source-b")
        with self.assertRaisesRegex(
            harness.CrackHarnessError,
            "stale relative to the retained partial frontier",
        ):
            harness._run_approved_for_test(
                self.root, approval, permit_path=permit,
                state_root=self.state, manager_key_path=self.manager_key,
            )

    def test_no_gain_survives_terminal_cleanup_failure(self) -> None:
        with patch.object(
            harness, "_cleanup_raw", side_effect=OSError("cleanup no-gain failed")
        ):
            result = self.execute(gain=0)
        self.assertEqual(result["status"], "no_gain")
        self.assertEqual(result["cleanup_status"], "cleanup_incomplete")
        self.assertTrue(any("cleanup no-gain failed" in item for item in result["cleanup_errors"]))
        sealed = dict(result)
        digest = sealed.pop("result_sha256")
        self.assertEqual(digest, harness._digest_json(sealed))
        self.assertIn("return 1", self.source.read_text())
        self.assertFalse(any(self.state.glob("owners/*/*/latest/result.json")))

    def _exact_report_fixture(self) -> tuple[dict, dict, dict]:
        with patch.object(
            harness, "_cleanup_raw", side_effect=OSError("retain exact report")
        ):
            self.assertEqual(self.execute()["status"], "exact")
        result_path = next(self.state.glob("owners/*/*/latest/result.json"))
        result = json.loads(result_path.read_text(encoding="utf-8"))
        report = json.loads(
            (result_path.parent / "CRACK_REPORT_v1.json").read_text(encoding="utf-8")
        )
        record = result["receipts"]["record"]["summary"]
        binding = {
            "owner": result["owner"],
            "function": result["function"],
            "source_sha256": result["candidate_sha256"],
            "target_object_sha256": report["target_object_sha256"],
            "object_sha256": record["candidate_object_sha256"],
            "candidate_record_sha256": result["receipts"]["assess"]["payload_sha256"],
            "status": "exact",
            "input_key": record["admission_input_key"],
        }
        return report, result, binding

    def test_complete_exact_report_is_accepted_by_shared_validator(self) -> None:
        report, result, binding = self._exact_report_fixture()
        result_path = next(self.state.glob("owners/*/*/latest/result.json"))
        report_path = result_path.parent / "CRACK_REPORT_v1.json"
        store = RecoveryMemory(self.root / "terminal-memory.sqlite3")
        identity = RecoveryMemory.identity(
            owner=result["owner"], function=result["function"],
            base_commit=result["base_commit"],
            toolchain_key=harness.TOOLCHAIN_MANIFEST_KEY,
            target_sha256=binding["target_object_sha256"],
            source_sha256=binding["source_sha256"],
        )
        identity["input_key"] = binding["input_key"]
        admission = store.admit(identity, requester="terminal-test")
        central = store.record(
            identity,
            requester="terminal-test",
            object_sha256=binding["object_sha256"],
            status="exact",
            reason="zero rows",
            admission_token=admission["admission_token"],
            candidate_record_sha256=binding["candidate_record_sha256"],
            strict_report_sha256=result["receipts"]["strict"]["summary"]["report_sha256"],
            data_report_sha256=result["receipts"]["data"]["summary"]["report_sha256"],
        )
        central_record_sha256 = central["experiment"]["record_sha256"]
        record_summary = result["receipts"]["record"]["summary"]
        record_summary["record_sha256"] = central_record_sha256
        result["receipts"]["record"]["payload_sha256"] = harness._digest_json(record_summary)
        report["proof_receipts"]["record"] = {
            "sha256": result["receipts"]["record"]["payload_sha256"],
            "summary": dict(record_summary),
        }
        report_body = dict(report)
        report_body.pop("report_sha256", None)
        report["report_sha256"] = harness._digest_json(report_body)
        result["report_sha256"] = report["report_sha256"]
        result_body = dict(result)
        result_body.pop("result_sha256", None)
        result["result_sha256"] = harness._digest_json(result_body)
        report_path.write_text(json.dumps(report), encoding="utf-8")
        result_path.write_text(json.dumps(result), encoding="utf-8")
        commit_body = {
            "schema": "crack_harness_record_commit/v1",
            "outcome": "exact",
            "candidate_sha256": result["candidate_sha256"],
            "record_payload_sha256": result["receipts"]["record"]["payload_sha256"],
            "record_sha256": central_record_sha256,
        }
        commit_path = result_path.parent / "record.commit.json"
        commit_path.write_text(
            json.dumps({
                **commit_body,
                "commit_sha256": harness._digest_json(commit_body),
            }),
            encoding="utf-8",
        )
        with patch.object(RecoveryMemory, "for_root", return_value=store):
            self.assertTrue(
                harness._valid_terminal_result(self.root, result_path, commit_path, binding)
            )

    def test_exact_terminal_is_bound_to_full_approval_identity(self) -> None:
        import copy

        report, result, binding = self._exact_report_fixture()
        result_path = next(self.state.glob("owners/*/*/latest/result.json"))
        report_path = result_path.parent / "CRACK_REPORT_v1.json"
        record = result["receipts"]["record"]
        commit_body = {
            "schema": "crack_harness_record_commit/v1",
            "outcome": "exact",
            "candidate_sha256": result["candidate_sha256"],
            "record_payload_sha256": record["payload_sha256"],
            "record_sha256": record["summary"]["record_sha256"],
        }
        commit_path = result_path.parent / "record.commit.json"
        commit_path.write_text(
            json.dumps({
                **commit_body,
                "commit_sha256": harness._digest_json(commit_body),
            }),
            encoding="utf-8",
        )
        approval = {
            "_approval_sha256": result["approval_sha256"],
            "approval_id": result["approval_id"],
            "owner": result["owner"],
            "task_id": result["task_id"],
            "function": result["function"],
            "base_commit": result["base_commit"],
            "campaign": {"id": result["campaign_id"]},
            "base": {"sha256": result["base_sha256"]},
            "candidate": {"sha256": result["candidate_sha256"]},
            "predicted_rows": list(result["predicted_rows"]),
            "selection": {"expected_terminal": result["expected_terminal"]},
            "target_sha256": report["target_object_sha256"],
        }
        self.assertTrue(
            harness._valid_terminal_result(
                self.root, result_path, commit_path, binding, approval,
                central_required=False,
            )
        )

        cases = (
            ("approval_id", lambda value: value.__setitem__("approval_id", "other")),
            ("task_id", lambda value: value.__setitem__("task_id", "other")),
            ("campaign_id", lambda value: value.__setitem__("campaign_id", "other")),
            (
                "predicted_rows",
                lambda value: value.__setitem__("predicted_rows", [STRICT_ROW_1]),
            ),
        )
        for field, mutation in cases:
            with self.subTest(field=field):
                forged_result = copy.deepcopy(result)
                forged_report = copy.deepcopy(report)
                mutation(forged_result)
                if field == "task_id":
                    forged_report["task_id"] = forged_result["task_id"]
                    report_body = dict(forged_report)
                    report_body.pop("report_sha256", None)
                    forged_report["report_sha256"] = harness._digest_json(report_body)
                    forged_result["report_sha256"] = forged_report["report_sha256"]
                result_body = dict(forged_result)
                result_body.pop("result_sha256", None)
                forged_result["result_sha256"] = harness._digest_json(result_body)
                report_path.write_text(json.dumps(forged_report), encoding="utf-8")
                result_path.write_text(json.dumps(forged_result), encoding="utf-8")
                self.assertFalse(
                    harness._valid_terminal_result(
                        self.root, result_path, commit_path, binding, approval,
                        central_required=False,
                    )
                )
        report_path.write_text(json.dumps(report), encoding="utf-8")
        result_path.write_text(json.dumps(result), encoding="utf-8")

    def test_exact_report_rejects_unbound_receipt_and_target_bypasses(self) -> None:
        import copy

        report, result, binding = self._exact_report_fixture()
        original_record_sha = result["receipts"]["record"]["summary"]["record_sha256"]

        def rehash_result(value: dict) -> dict:
            body = dict(value)
            body.pop("result_sha256", None)
            return {**body, "result_sha256": harness._digest_json(body)}

        cases = (
            (
                "proof ok false",
                lambda forged_report, forged_result: forged_result["receipts"]["strict"].__setitem__(
                    "ok", False
                ),
                None,
            ),
            (
                "proof command empty",
                lambda forged_report, forged_result: forged_result["receipts"]["strict"].__setitem__(
                    "command", {}
                ),
                None,
            ),
            (
                "compile command empty",
                lambda forged_report, forged_result: forged_result["receipts"]["compile"].__setitem__(
                    "baseline_command", {}
                ),
                None,
            ),
            (
                "terminal expectation false",
                lambda forged_report, forged_result: forged_result.__setitem__(
                    "terminal_expectation_met", False
                ),
                None,
            ),
            (
                "terminal expectation invalid",
                lambda forged_report, forged_result: forged_result.__setitem__(
                    "expected_terminal", "partial"
                ),
                None,
            ),
            ("target rehashed", None, "target"),
            ("record digest forged", None, "record"),
        )
        for label, mutation, special in cases:
            with self.subTest(label=label):
                forged_report = copy.deepcopy(report)
                forged_result = copy.deepcopy(result)
                record_commit = None
                if mutation is not None:
                    mutation(forged_report, forged_result)
                elif special == "target":
                    rogue_target = "f" * 64
                    forged_report["target_object_sha256"] = rogue_target
                    for name in ("strict", "data", "focus", "siblings", "physical", "assess", "record"):
                        summary = forged_result["receipts"][name]["summary"]
                        summary["target_object_sha256"] = rogue_target
                        payload_sha256 = harness._digest_json(summary)
                        forged_result["receipts"][name]["payload_sha256"] = payload_sha256
                        forged_report["proof_receipts"][name]["summary"] = copy.deepcopy(summary)
                        forged_report["proof_receipts"][name]["sha256"] = payload_sha256
                elif special == "record":
                    forged_record = forged_result["receipts"]["record"]["summary"]
                    forged_record["record_sha256"] = "f" * 64
                    payload_sha256 = harness._digest_json(forged_record)
                    forged_result["receipts"]["record"]["payload_sha256"] = payload_sha256
                    forged_report["proof_receipts"]["record"]["summary"] = copy.deepcopy(forged_record)
                    forged_report["proof_receipts"]["record"]["sha256"] = payload_sha256
                    commit_body = {
                        "schema": "crack_harness_record_commit/v1",
                        "outcome": "exact",
                        "candidate_sha256": forged_result["candidate_sha256"],
                        "record_payload_sha256": payload_sha256,
                        "record_sha256": original_record_sha,
                    }
                    record_commit = {
                        **commit_body,
                        "commit_sha256": harness._digest_json(commit_body),
                    }
                forged_result = rehash_result(forged_result)
                report_body = dict(forged_report)
                report_body.pop("report_sha256", None)
                forged_report["report_sha256"] = harness._digest_json(report_body)
                with self.assertRaises(harness.CrackHarnessError):
                    harness._validate_exact_report(
                        forged_report,
                        forged_result,
                        binding,
                        record_commit=record_commit,
                    )

    def test_exact_report_rejects_missing_schema_completion_or_proofs(self) -> None:
        import copy

        report, result, binding = self._exact_report_fixture()
        for label, mutation in (
            ("schema", lambda value: value.__setitem__("schema", "CRACK_REPORT/other")),
            ("completed", lambda value: value.pop("completed")),
            (
                "authority",
                lambda value: value.__setitem__("authority_advanced", True),
            ),
            ("completed_at", lambda value: value.pop("completed_at")),
            (
                "proof",
                lambda value: value["proof_receipts"].pop("physical"),
            ),
        ):
            with self.subTest(label=label):
                forged = copy.deepcopy(report)
                mutation(forged)
                body = dict(forged)
                body.pop("report_sha256", None)
                forged["report_sha256"] = harness._digest_json(body)
                with self.assertRaises(harness.CrackHarnessError):
                    harness._validate_exact_report(forged, result, binding)

    def test_exact_report_rejects_wrong_identity_and_tampered_hashes(self) -> None:
        import copy

        report, result, binding = self._exact_report_fixture()
        for label, mutation in (
            ("owner", lambda value: value.__setitem__("owner", "main:board/other")),
            ("function", lambda value: value.__setitem__("function", "Other")),
            ("source", lambda value: value.__setitem__("source_sha256", "0" * 64)),
            (
                "proof source",
                lambda value: value["proof_receipts"]["strict"]["summary"].__setitem__(
                    "candidate_source_sha256", "0" * 64
                ),
            ),
        ):
            with self.subTest(label=label):
                forged = copy.deepcopy(report)
                mutation(forged)
                body = dict(forged)
                body.pop("report_sha256", None)
                forged["report_sha256"] = harness._digest_json(body)
                with self.assertRaises(harness.CrackHarnessError):
                    harness._validate_exact_report(forged, result, binding)

    def test_self_hashed_structurally_invalid_exact_report_is_not_a_terminal_result(self) -> None:
        run = self.state / "owners/test/Owner/latest"
        run.mkdir(parents=True)
        candidate_sha = sha(self.candidate)
        summary = {
            "schema": "crack_central_record_receipt/v1",
            "recorded": True,
            "owner": "main:board/test",
            "function": "Owner",
            "candidate_source_sha256": candidate_sha,
            "candidate_object_sha256": "c" * 64,
            "outcome": "exact",
            "admission_input_key": "a" * 64,
        }
        record = {"summary": summary, "payload_sha256": harness._digest_json(summary)}
        result_body = {
            "status": "exact",
            "owner": "main:board/test",
            "function": "Owner",
            "candidate_sha256": candidate_sha,
            "receipts": {"record": record},
        }
        result = {
            **result_body,
            "result_sha256": harness._digest_json(result_body),
        }
        result_path = run / "result.json"
        result_path.write_text(json.dumps(result), encoding="utf-8")
        commit_body = {
            "schema": "crack_harness_record_commit/v1",
            "outcome": "exact",
            "candidate_sha256": candidate_sha,
            "record_payload_sha256": record["payload_sha256"],
            "record_sha256": "0" * 64,
        }
        commit_path = run / "record.commit.json"
        commit_path.write_text(
            json.dumps({
                **commit_body,
                "commit_sha256": harness._digest_json(commit_body),
            }),
            encoding="utf-8",
        )
        report_body = {"source_sha256": candidate_sha}
        report = {
            **report_body,
            "report_sha256": harness._digest_json(report_body),
        }
        (run / "CRACK_REPORT_v1.json").write_text(
            json.dumps(report), encoding="utf-8"
        )
        binding = {
            "owner": "main:board/test",
            "function": "Owner",
            "source_sha256": candidate_sha,
            "object_sha256": "c" * 64,
            "status": "exact",
            "input_key": "a" * 64,
        }
        self.assertFalse(
            harness._valid_terminal_result(self.root, result_path, commit_path, binding)
        )

    def test_startup_retries_successful_terminal_cleanup_without_rollback(self) -> None:
        with patch.object(
            harness, "_remove_disposable_worktree",
            side_effect=OSError("cleanup retry pending"),
        ):
            result = self.execute()
        self.assertEqual(result["cleanup_status"], "cleanup_incomplete")
        # The command adapter in this unit fixture emits a compact central
        # receipt but does not populate the real RecoveryMemory database.
        # Startup finalization still requires that database binding in
        # production, so isolate the cleanup behavior here.
        with patch.object(harness, "_central_record_matches", return_value=True):
            status = harness._status_for_test(
                self.root, state_root=self.state,
                manager_key_path=self.manager_key,
            )
        retained = status["results"][0]
        self.assertEqual(retained["status"], "exact")
        self.assertEqual(retained["cleanup_status"], "complete")
        self.assertTrue(any("cleanup retry pending" in item for item in retained["cleanup_errors"]))
        self.assertIn("return 2", self.source.read_text())
        self.assertFalse(any(self.state.glob("owners/*/*/latest/temp")))

    def test_startup_retries_partially_deleted_root_disposables_before_completion(self) -> None:
        original_unlink = harness._safe_unlink
        failed = False

        def fail_candidate_once(path: Path) -> None:
            nonlocal failed
            if Path(path) == self.candidate and not failed:
                failed = True
                raise OSError("candidate root deletion failed")
            original_unlink(Path(path))

        with patch.object(harness, "_safe_unlink", side_effect=fail_candidate_once):
            result = self.execute()
        self.assertEqual(result["status"], "exact")
        self.assertEqual(result["cleanup_status"], "cleanup_incomplete")
        self.assertTrue(any("candidate root deletion failed" in item for item in result["cleanup_errors"]))
        self.assertFalse(self.base.exists())
        self.assertTrue(self.candidate.exists())
        self.assertTrue(self.permit.exists())
        self.assertTrue(self.approval.exists())
        self.assertTrue((self.state / "attempt.json").exists())
        self.assertTrue((self.state / "transaction.json").exists())

        with patch.object(harness, "_central_record_matches", return_value=True):
            retained = harness._status_for_test(
                self.root, state_root=self.state,
                manager_key_path=self.manager_key,
            )["results"][0]
        self.assertEqual(retained["status"], "exact")
        self.assertEqual(retained["cleanup_status"], "complete")
        self.assertFalse(self.candidate.exists())
        self.assertFalse(self.permit.exists())
        self.assertFalse(self.approval.exists())
        self.assertFalse((self.state / "attempt.json").exists())
        self.assertFalse((self.state / "transaction.json").exists())
        self.assertFalse((self.state / "RECOVERY_REQUIRED.json").exists())

    def test_tampered_attempt_cannot_redirect_permit_cleanup(self) -> None:
        original_unlink = harness._safe_unlink
        failed = False

        def fail_candidate_once(path: Path) -> None:
            nonlocal failed
            if Path(path) == self.candidate and not failed:
                failed = True
                raise OSError("candidate root deletion failed")
            original_unlink(Path(path))

        with patch.object(harness, "_safe_unlink", side_effect=fail_candidate_once):
            result = self.execute()
        self.assertEqual(result["cleanup_status"], "cleanup_incomplete")
        attempt_path = self.state / "attempt.json"
        attempt = json.loads(attempt_path.read_text(encoding="utf-8"))
        real_permit = Path(attempt["disposable_paths"][2])
        decoy = self.root / "decoy-permit.json"
        attempt["disposable_paths"][2] = str(decoy)
        unsigned = dict(attempt)
        unsigned.pop("attempt_sha256")
        attempt["attempt_sha256"] = harness._digest_json(unsigned)
        attempt_path.write_text(json.dumps(attempt), encoding="utf-8")
        with patch.object(harness, "_central_record_matches", return_value=True):
            with self.assertRaisesRegex(
                harness.CrackHarnessError, "attempt receipt (integrity|signature)"
            ):
                harness._status_for_test(
                    self.root, state_root=self.state,
                    manager_key_path=self.manager_key,
                )
        self.assertTrue(real_permit.is_file())

    def test_missing_attempt_cannot_finalize_while_root_disposables_remain(self) -> None:
        original_unlink = harness._safe_unlink
        failed = False

        def fail_candidate_once(path: Path) -> None:
            nonlocal failed
            if Path(path) == self.candidate and not failed:
                failed = True
                raise OSError("candidate root deletion failed")
            original_unlink(Path(path))

        with patch.object(harness, "_safe_unlink", side_effect=fail_candidate_once):
            result = self.execute()
        self.assertEqual(result["cleanup_status"], "cleanup_incomplete")
        (self.state / "attempt.json").unlink()

        with patch.object(harness, "_central_record_matches", return_value=True):
            retained = harness._status_for_test(
                self.root, state_root=self.state,
                manager_key_path=self.manager_key,
            )["results"][0]
        self.assertEqual(retained["status"], "exact")
        self.assertEqual(retained["cleanup_status"], "cleanup_incomplete")
        self.assertTrue(self.candidate.exists())
        self.assertTrue(self.permit.exists())
        self.assertTrue(self.approval.exists())
        run_dir = next(self.state.glob("owners/*/*/latest"))
        # Exact cleanup seals the manager-authenticated root manifest before
        # deleting any disposable.  Losing attempt.json cannot erase that
        # proof, but it also cannot authorize finalization while roots remain.
        self.assertTrue((run_dir / "root-cleanup.receipt.json").is_file())

    def test_orphan_recovery_marker_is_a_hard_cleanup_lock(self) -> None:
        original_unlink = harness._safe_unlink
        failed = False

        def fail_candidate_once(path: Path) -> None:
            nonlocal failed
            if Path(path) == self.candidate and not failed:
                failed = True
                raise OSError("candidate root deletion failed")
            original_unlink(Path(path))

        with patch.object(harness, "_safe_unlink", side_effect=fail_candidate_once):
            result = self.execute()
        self.assertEqual(result["cleanup_status"], "cleanup_incomplete")
        (self.state / "attempt.json").unlink()
        with patch.object(harness, "_central_record_matches", return_value=True):
            harness._status_for_test(
                self.root, state_root=self.state,
                manager_key_path=self.manager_key,
            )
        marker = self.state / "RECOVERY_REQUIRED.json"
        journal = self.state / "transaction.json"
        self.assertTrue(marker.is_file())
        self.assertTrue(journal.is_file())
        journal.unlink()

        status = harness._status_for_test(
            self.root, state_root=self.state,
            manager_key_path=self.manager_key,
        )
        self.assertFalse(status["interrupted_transaction"])
        self.assertTrue(marker.is_file())
        self.assertTrue(self.candidate.is_file())
        self.assertTrue(self.permit.is_file())
        retained = status["results"][0]
        self.assertEqual(retained["cleanup_status"], "cleanup_incomplete")

    def test_recreated_root_disposable_blocks_cleanup_finalization(self) -> None:
        self.execute()
        result_path = next(self.state.glob("owners/*/*/latest/result.json"))
        value = json.loads(result_path.read_text(encoding="utf-8"))
        body = dict(value)
        body.pop("result_sha256")
        body["cleanup_status"] = "cleanup_incomplete"
        result_path.write_text(
            json.dumps({**body, "result_sha256": harness._digest_json(body)}),
            encoding="utf-8",
        )
        self.candidate.write_text("recreated", encoding="utf-8")

        with patch.object(harness, "_central_record_matches", return_value=True):
            retained = harness._status_for_test(
                self.root, state_root=self.state,
                manager_key_path=self.manager_key,
            )["results"][0]
        self.assertEqual(retained["cleanup_status"], "cleanup_incomplete")
        self.assertTrue(self.candidate.exists())

    def test_root_cleanup_receipt_binds_task_campaign_and_predicted_rows(self) -> None:
        import copy

        result = self.execute()
        run_dir = next(self.state.glob("owners/*/*/latest"))
        self.assertTrue(
            harness._valid_root_cleanup_receipt(
                self.root, self.state, run_dir, result,
                manager_key_path=self.manager_key,
                expected_key_id=sha(self.manager_key),
            )
        )
        for field, replacement in (
            ("task_id", "other-task"),
            ("campaign_id", "other-campaign"),
            ("predicted_rows", [STRICT_ROW_1]),
        ):
            with self.subTest(field=field):
                forged = copy.deepcopy(result)
                forged[field] = replacement
                body = dict(forged)
                body.pop("result_sha256", None)
                forged["result_sha256"] = harness._digest_json(body)
                self.assertFalse(
                    harness._valid_root_cleanup_receipt(
                        self.root, self.state, run_dir, forged,
                        manager_key_path=self.manager_key,
                        expected_key_id=sha(self.manager_key),
                    )
                )

    def test_forged_root_cleanup_manifest_cannot_finalize(self) -> None:
        self.execute()
        result_path = next(self.state.glob("owners/*/*/latest/result.json"))
        value = json.loads(result_path.read_text(encoding="utf-8"))
        body = dict(value)
        body.pop("result_sha256")
        body["cleanup_status"] = "cleanup_incomplete"
        result_path.write_text(
            json.dumps({**body, "result_sha256": harness._digest_json(body)}),
            encoding="utf-8",
        )
        receipt_path = result_path.parent / "root-cleanup.receipt.json"
        receipt = json.loads(receipt_path.read_text(encoding="utf-8"))
        receipt["disposables"][1]["path"] = str(self.root / "forged-candidate.c")
        forged_body = dict(receipt)
        forged_body.pop("cleanup_sha256")
        receipt["cleanup_sha256"] = harness._digest_json(forged_body)
        receipt_path.write_text(json.dumps(receipt), encoding="utf-8")

        with patch.object(harness, "_central_record_matches", return_value=True):
            retained = harness._status_for_test(
                self.root, state_root=self.state,
                manager_key_path=self.manager_key,
            )["results"][0]
        self.assertEqual(retained["cleanup_status"], "cleanup_incomplete")

    def test_startup_fails_closed_if_external_tamper_removes_approval_first(self) -> None:
        original_unlink = harness._safe_unlink
        failed = False

        def fail_candidate_once(path: Path) -> None:
            nonlocal failed
            if Path(path) == self.candidate and not failed:
                failed = True
                raise OSError("candidate root deletion failed")
            original_unlink(Path(path))

        with patch.object(harness, "_safe_unlink", side_effect=fail_candidate_once):
            result = self.execute()
        self.assertEqual(result["status"], "exact")
        self.assertEqual(result["cleanup_status"], "cleanup_incomplete")
        self.assertTrue(self.approval.exists())
        self.approval.unlink()

        status = harness._status_for_test(
            self.root, state_root=self.state,
            manager_key_path=self.manager_key,
        )
        self.assertTrue(status["interrupted_transaction"])
        self.assertTrue((self.state / "RECOVERY_REQUIRED.json").exists())
        self.assertTrue(self.candidate.exists())
        self.assertTrue(self.permit.exists())
        self.assertTrue((self.state / "attempt.json").exists())

    def test_startup_cleanup_accepts_expired_hash_bound_approval(self) -> None:
        original_unlink = harness._safe_unlink
        failed = False

        def fail_candidate_once(path: Path) -> None:
            nonlocal failed
            if Path(path) == self.candidate and not failed:
                failed = True
                raise OSError("candidate root deletion failed")
            original_unlink(Path(path))

        with patch.object(harness, "_safe_unlink", side_effect=fail_candidate_once):
            result = self.execute()
        self.assertEqual(result["cleanup_status"], "cleanup_incomplete")

        real_datetime = datetime

        class FutureDatetime(datetime):
            @classmethod
            def now(cls, tz=None):
                current = real_datetime.now(tz or timezone.utc)
                return current + timedelta(hours=1)

        with (
            patch.object(harness, "datetime", FutureDatetime),
            patch.object(harness, "_central_record_matches", return_value=True),
        ):
            retained = harness._status_for_test(
                self.root, state_root=self.state,
                manager_key_path=self.manager_key,
            )["results"][0]
        self.assertEqual(retained["status"], "exact")
        self.assertEqual(retained["cleanup_status"], "complete")
        self.assertFalse(self.candidate.exists())
        self.assertFalse(self.permit.exists())
        self.assertFalse(self.approval.exists())
        self.assertFalse((self.state / "attempt.json").exists())

    def test_startup_retries_final_attempt_receipt_unlink_after_disposables_are_gone(self) -> None:
        original_unlink = harness._safe_unlink
        attempt_path = self.state / "attempt.json"
        failed = False

        def fail_attempt_once(path: Path) -> None:
            nonlocal failed
            if Path(path) == attempt_path and not failed:
                failed = True
                raise OSError("attempt receipt deletion failed")
            original_unlink(Path(path))

        with patch.object(harness, "_safe_unlink", side_effect=fail_attempt_once):
            result = self.execute()
        self.assertEqual(result["status"], "exact")
        self.assertEqual(result["cleanup_status"], "cleanup_incomplete")
        self.assertTrue(any("attempt receipt deletion failed" in item for item in result["cleanup_errors"]))
        self.assertFalse(self.base.exists())
        self.assertFalse(self.candidate.exists())
        self.assertFalse(self.permit.exists())
        self.assertFalse(self.approval.exists())
        self.assertTrue(attempt_path.exists())

        with patch.object(harness, "_central_record_matches", return_value=True):
            retained = harness._status_for_test(
                self.root, state_root=self.state,
                manager_key_path=self.manager_key,
            )["results"][0]
        self.assertEqual(retained["status"], "exact")
        self.assertEqual(retained["cleanup_status"], "complete")
        self.assertFalse(attempt_path.exists())
        self.assertFalse((self.state / "transaction.json").exists())
        self.assertFalse((self.state / "RECOVERY_REQUIRED.json").exists())

    def test_malformed_existing_record_commit_invalidates_exact_terminal(self) -> None:
        self.execute()
        result_path = next(self.state.glob("owners/*/*/latest/result.json"))
        value = json.loads(result_path.read_text(encoding="utf-8"))
        binding = harness._terminal_binding_from_result(result_path.parent, value)
        self.assertIsNotNone(binding)
        commit_path = result_path.parent / "record.commit.json"
        with patch.object(harness, "_central_record_matches", return_value=True):
            self.assertTrue(
                harness._valid_terminal_result(
                    self.root, result_path, commit_path, binding
                )
            )
        commit_path.write_text("{}", encoding="utf-8")
        with patch.object(harness, "_central_record_matches", return_value=True):
            self.assertFalse(
                harness._valid_terminal_result(
                    self.root, result_path, commit_path, binding
                )
            )

    def test_record_commit_directory_invalidates_exact_terminal(self) -> None:
        report, result, binding = self._exact_report_fixture()
        result_path = next(self.state.glob("owners/*/*/latest/result.json"))
        commit_path = result_path.parent / "record.commit.json"
        commit_path.mkdir()
        with patch.object(harness, "_central_record_matches", return_value=True):
            self.assertFalse(
                harness._valid_terminal_result(
                    self.root, result_path, commit_path, binding
                )
            )

    def test_exact_success_survives_owner_gc_exception(self) -> None:
        with patch.object(
            harness, "_gc_owner", side_effect=OSError("owner gc failed")
        ):
            result = self.execute()
        self.assertEqual(result["status"], "exact")
        self.assertEqual(result["cleanup_status"], "cleanup_incomplete")
        self.assertTrue(any("owner gc failed" in item for item in result["cleanup_errors"]))
        self.assertIn("return 2", self.source.read_text())
        self.assertFalse(any(self.state.glob("owners/*/*/latest-failure.json")))
        with patch.object(harness, "_central_record_matches", return_value=True):
            retried = harness._status_for_test(
                self.root, state_root=self.state,
                manager_key_path=self.manager_key,
            )["results"][0]
        self.assertEqual(retried["status"], "exact")
        self.assertEqual(retried["cleanup_status"], "complete")

    def test_positive_nonexact_creates_no_retention_maintenance_result(self) -> None:
        result = self.execute(gain=2, focus_rows=1)
        self.assertEqual(result["status"], "improved")
        self.assertFalse(any(self.state.glob("owners/*/*/latest/result.json")))
        self.assertFalse(any(self.state.glob("owners/*/*/latest/record.commit.json")))
        self.assertTrue(any(self.state.glob("owners/*/*/latest-frontier.json")))

    def test_generic_post_terminal_baseexception_cannot_escape(self) -> None:
        with patch.object(
            harness, "_cleanup_raw", side_effect=KeyboardInterrupt("maintenance interrupt")
        ):
            result = self.execute()
        self.assertEqual(result["status"], "exact")
        self.assertEqual(result["cleanup_status"], "cleanup_incomplete")
        self.assertTrue(any("maintenance interrupt" in item for item in result["cleanup_errors"]))
        self.assertIn("return 2", self.source.read_text())

    def test_record_primary_failure_survives_cleanup_baseexception(self) -> None:
        class CleanupBaseException(BaseException):
            pass

        with patch.object(
            harness, "_run_canonical_record",
            side_effect=harness.CrackHarnessError("primary central record failure"),
        ), patch.object(
            harness, "_cleanup_raw",
            side_effect=CleanupBaseException("secondary cleanup baseexception"),
        ):
            result = self.execute()
        self.assertEqual(result["status"], "failed")
        self.assertIn("primary central record failure", result["reason"])
        self.assertIn("return 1", self.source.read_text())
        diagnostic_path = next(self.state.glob("owners/*/*/latest-failure.json"))
        diagnostic = json.loads(diagnostic_path.read_text(encoding="utf-8"))
        self.assertIn("primary central record failure", diagnostic["primary_reason"])
        self.assertTrue(any(
            "secondary cleanup baseexception" in item
            for item in diagnostic["cleanup_errors"]
        ))

    def test_stop_authenticates_exact_manager_permit(self) -> None:
        approval, permit = self.write_inputs(); loaded = harness.load_approval(self.root, approval)
        stop = json.loads((self.state / "STOP").read_text()); stop["authorized_permit_sha256"] = "0" * 64
        (self.state / "STOP").write_text(json.dumps(stop), encoding="utf-8")
        with self.assertRaisesRegex(harness.CrackHarnessError, "authenticate"):
            harness._load_permit(self.root, loaded, permit, self.state, manager_key_path=self.manager_key, expected_key_id=sha(self.manager_key))

    def test_stop_revocation_is_rechecked_at_checkpoint(self) -> None:
        approval, permit_path = self.write_inputs(); loaded = harness.load_approval(self.root, approval)
        permit, permit_file = harness._load_permit(self.root, loaded, permit_path, self.state, manager_key_path=self.manager_key, expected_key_id=sha(self.manager_key))
        (self.state / "STOP").write_text(json.dumps({"schema":"crack_harness_stop/v1","stopped":True,"authorized_permit_sha256":"0"*64,"stop_nonce":"f"*64}), encoding="utf-8")
        with self.assertRaisesRegex(harness.CrackHarnessError, "authenticate"):
            harness._checkpoint(self.root, approval, loaded, permit_file, permit, self.state, allow_source=False)

    def test_post_apply_checkpoint_accepts_only_the_approved_candidate(self) -> None:
        approval, permit_path = self.write_inputs()
        loaded = harness.load_approval(self.root, approval)
        permit, permit_file = harness._load_permit(
            self.root, loaded, permit_path, self.state,
            manager_key_path=self.manager_key,
            expected_key_id=sha(self.manager_key),
        )
        self.source.write_bytes(self.candidate.read_bytes())
        harness._checkpoint(
            self.root, approval, loaded, permit_file, permit, self.state,
            allow_source=True,
        )
        self.source.write_text(
            "int Owner(void) {\n    return 3;\n}\n", encoding="utf-8"
        )
        with self.assertRaisesRegex(
            harness.CrackHarnessError,
            "live source changed outside the approved cell",
        ):
            harness._checkpoint(
                self.root, approval, loaded, permit_file, permit, self.state,
                allow_source=True,
            )

    def test_missing_approval_is_rejected(self) -> None:
        with self.assertRaises((harness.CrackHarnessError, FileNotFoundError)):
            harness.load_approval(self.root, self.root / "missing-approval.json")

    def test_invalid_objdiff_unit_is_rejected_before_any_command(self) -> None:
        invalid_units = (
            "main:board/captrap", "main/../board/captrap",
            r"main\board\captrap", "main/board/cap trap", "main/board/captráp",
            "/main/board/captrap", "main//board/captrap",
            "main/board/captrap/", "main/./board/captrap", "captrap",
        )
        for unit in invalid_units:
            with self.subTest(unit=unit):
                approval, _ = self.write_inputs()
                value = json.loads(approval.read_text(encoding="utf-8"))
                value["unit"] = unit
                approval.write_text(json.dumps(value), encoding="utf-8")
                with patch.object(harness, "_run_command") as command:
                    with self.assertRaisesRegex(
                        harness.CrackHarnessError, "closed objdiff unit name"
                    ):
                        harness._dry_run_for_test(
                            self.root, approval, state_root=self.state
                        )
                    command.assert_not_called()

    def test_valid_slash_form_objdiff_unit_passes_dry_run(self) -> None:
        approval, _ = self.write_inputs()
        value = json.loads(approval.read_text(encoding="utf-8"))
        value["unit"] = "main/board/test"
        approval.write_text(json.dumps(value), encoding="utf-8")
        result = harness._dry_run_for_test(
            self.root, approval, state_root=self.state
        )
        self.assertEqual(result["status"], "ready")

    def test_lane_reconciliation_field_is_rejected(self) -> None:
        approval, _ = self.write_inputs()
        value = json.loads(approval.read_text(encoding="utf-8"))
        value["retry"] = {
            "schema": "crack_harness_legacy_reconciliation/v1",
            "one_shot": True,
        }
        approval.write_text(json.dumps(value), encoding="utf-8")
        with self.assertRaisesRegex(
            harness.CrackHarnessError,
            "retry must be a strict crack_harness_legacy_reconciliation/v1 object",
        ):
            harness.load_approval(self.root, approval)

    def test_explicit_null_retry_is_rejected(self) -> None:
        approval, _ = self.write_inputs()
        value = json.loads(approval.read_text(encoding="utf-8"))
        value["retry"] = None
        approval.write_text(json.dumps(value), encoding="utf-8")
        with self.assertRaisesRegex(harness.CrackHarnessError, "retry must not be null"):
            harness.load_approval(self.root, approval)

    def test_published_approval_schema_has_luna_audit_and_bounded_retry(self) -> None:
        schema_path = Path(harness.__file__).with_name(
            "CRACK_HARNESS_APPROVAL_V1.schema.json"
        )
        schema = json.loads(schema_path.read_text(encoding="utf-8"))
        properties = schema["properties"]
        self.assertIn("retry", properties)
        selection = properties["selection"]
        self.assertIn("luna5_audit", selection["required"])
        self.assertIn("luna5_audit", selection["properties"])

    def test_legacy_v1_exact_retry_is_ready_once_then_durably_consumed(self) -> None:
        approval_path, _ = self.write_inputs(campaign_id="retry-campaign")
        run_dir, _ = self.add_legacy_retry(approval_path)
        ready = harness._dry_run_for_test(
            self.root, approval_path, state_root=self.state
        )
        self.assertEqual(ready["status"], "ready", ready)
        loaded = harness.load_approval(self.root, approval_path)
        harness._consume_legacy_retry(run_dir, loaded)
        marker = run_dir.parent / "retry-used.json"
        self.assertTrue(marker.is_file())
        blocked = harness._dry_run_for_test(
            self.root, approval_path, state_root=self.state
        )
        self.assertEqual(blocked["status"], "blocked", blocked)
        self.assertTrue(any("tombstone" in item for item in blocked["blockers"]))

    def test_legacy_retry_revalidates_historical_evidence_hash_at_execution_boundary(self) -> None:
        approval_path, _ = self.write_inputs(campaign_id="retry-evidence-hash")
        run_dir, _ = self.add_legacy_retry(approval_path)
        loaded = harness.load_approval(self.root, approval_path)
        evidence_path = loaded["_retry"]["historical_exact_evidence_path"]
        evidence = json.loads(evidence_path.read_text(encoding="utf-8"))
        evidence["target_bytes"] = 31
        evidence_path.write_text(json.dumps(evidence), encoding="utf-8")

        with self.assertRaisesRegex(
            harness.CrackHarnessError,
            "retry.historical_exact_evidence hash mismatch",
        ):
            harness._consume_legacy_retry(run_dir, loaded)
        self.assertFalse((run_dir.parent / "retry-used.json").exists())

    def test_legacy_retry_revalidates_historical_evidence_binding_and_content(self) -> None:
        mutations = (
            ("path", lambda value: value.__setitem__("path", "evidence/other.json"), "binding changed"),
            ("owner", lambda value: value.__setitem__("owner", "main:board/other"), "not bound"),
            ("function", lambda value: value.__setitem__("function", "Other"), "not bound"),
            ("candidate", lambda value: value.__setitem__("candidate_sha256", "e" * 64), "not bound"),
            ("target bytes", lambda value: value.__setitem__("target_bytes", 31), "byte counts differ"),
            ("candidate bytes", lambda value: value.__setitem__("candidate_bytes", 31), "byte counts differ"),
            ("strict percent", lambda value: value.__setitem__("strict_percent", 99), "strict proof is not 100"),
            ("data percent", lambda value: value.__setitem__("data_percent", 99), "data proof is not 100"),
            ("strict rows", lambda value: value.__setitem__("strict_diff_rows", 1), "residual rows"),
            ("data rows", lambda value: value.__setitem__("data_diff_rows", 1), "residual rows"),
        )
        for label, mutate, expected in mutations:
            with self.subTest(label=label):
                approval_path, _ = self.write_inputs(campaign_id=f"retry-evidence-{label.replace(' ', '-')}")
                run_dir, _ = self.add_legacy_retry(approval_path)
                loaded = harness.load_approval(self.root, approval_path)
                evidence_path = loaded["_retry"]["historical_exact_evidence_path"]
                if label == "path":
                    other = self.root / "evidence/other.json"
                    other.write_bytes(evidence_path.read_bytes())
                    loaded["_retry"]["historical_exact_evidence_path"] = other
                    with self.assertRaisesRegex(harness.CrackHarnessError, expected):
                        harness._legacy_reconciliation_eligible(run_dir, loaded)
                    continue
                evidence = json.loads(evidence_path.read_text(encoding="utf-8"))
                mutate(evidence)
                evidence_path.write_text(json.dumps(evidence), encoding="utf-8")
                digest = sha(evidence_path)
                loaded["retry"]["historical_exact_evidence"]["sha256"] = digest
                loaded["_retry"]["historical_exact_evidence_sha256"] = digest
                with self.assertRaisesRegex(harness.CrackHarnessError, expected):
                    harness._legacy_reconciliation_eligible(run_dir, loaded)

    def test_legacy_retry_rejects_partial_historical_evidence(self) -> None:
        approval_path, _ = self.write_inputs(campaign_id="retry-partial")
        self.add_legacy_retry(approval_path, strict_percent=99)
        with self.assertRaisesRegex(
            harness.CrackHarnessError, "strict proof is not 100 percent"
        ):
            harness.load_approval(self.root, approval_path)

    def test_legacy_retry_rejects_self_asserted_historical_report(self) -> None:
        with self.assertRaisesRegex(
            harness.CrackHarnessError,
            "must use crack_harness_historical_exact_evidence/v1",
        ):
            harness._historical_exact_values(
                {
                    "schema": harness.REPORT_SCHEMA,
                    "owner": "main:board/test", "function": "Owner",
                    "status": "exact", "completed": True,
                    "authority_advanced": False,
                    "source_sha256": sha(self.candidate),
                    "result": {
                        "target_bytes": 1, "candidate_bytes": 1,
                        "strict_percent": 100, "data_percent": 100,
                    },
                },
                "main:board/test", "Owner", sha(self.candidate),
            )

    def test_legacy_retry_never_releases_v2_tombstone(self) -> None:
        approval_path, _ = self.write_inputs(campaign_id="retry-v2")
        self.add_legacy_retry(
            approval_path,
            tombstone_schema="crack_harness_function_tombstone/v2",
        )
        with self.assertRaisesRegex(
            harness.CrackHarnessError,
            "must be an exact consumed legacy v1 tombstone",
        ):
            harness.load_approval(self.root, approval_path)

    def test_legacy_retry_partial_central_publication_still_blocks_second_use(self) -> None:
        approval_path, _ = self.write_inputs(campaign_id="retry-partial-publish")
        run_dir, _ = self.add_legacy_retry(approval_path)
        loaded = harness.load_approval(self.root, approval_path)
        publish = harness._append_consumed_cell

        def publish_then_fail(*args: object, **kwargs: object) -> None:
            publish(*args, **kwargs)
            raise OSError("post-publication failure")

        with patch.object(
            harness, "_append_consumed_cell", side_effect=publish_then_fail
        ):
            with self.assertRaisesRegex(OSError, "post-publication failure"):
                harness._consume_legacy_retry(run_dir, loaded)
        self.assertFalse((run_dir.parent / "retry-used.json").exists())
        with self.assertRaisesRegex(
            harness.CrackHarnessError, "legacy retry is not eligible"
        ):
            harness._consume_legacy_retry(run_dir, loaded)

    def test_marker_rollback_preserves_primary_and_fails_closed(self) -> None:
        approval_path, _ = self.write_inputs(campaign_id="retry-rollback")
        run_dir, _ = self.add_legacy_retry(approval_path)
        run_dir.mkdir(parents=True, exist_ok=True)
        loaded = harness.load_approval(self.root, approval_path)
        marker_path = run_dir.parent / "retry-used.json"

        with patch.object(
            harness, "_append_consumed_cell", side_effect=OSError("central publish failed")
        ), patch.object(
            harness, "_safe_unlink", side_effect=OSError("marker rollback denied")
        ):
            with self.assertRaisesRegex(OSError, "central publish failed") as raised:
                harness._consume_legacy_retry(run_dir, loaded)
        self.assertTrue(any("marker rollback denied" in note for note in raised.exception.__notes__))
        self.assertTrue(marker_path.is_file())
        self.assertTrue(harness._function_consumed(run_dir, loaded))

    def test_started_marker_is_retained_when_central_publish_fails(self) -> None:
        approval_path, _ = self.write_inputs(campaign_id="retry-started")
        run_dir, _ = self.add_legacy_retry(approval_path)
        run_dir.mkdir(parents=True, exist_ok=True)
        loaded = harness.load_approval(self.root, approval_path)
        marker_path = run_dir.parent / "retry-used.json"
        with patch.object(
            harness, "_append_consumed_cell", side_effect=OSError("central publish failed")
        ):
            with self.assertRaisesRegex(OSError, "central publish failed"):
                harness._consume_legacy_retry(
                    run_dir, loaded, execution_started=True
                )
        self.assertTrue(marker_path.is_file())
        self.assertTrue(harness._function_consumed(run_dir, loaded))

    def test_signed_permit_is_one_shot_without_consuming_function(self) -> None:
        approval, _ = self.write_inputs()
        loaded = harness.load_approval(self.root, approval)
        run_dir = harness._run_dir(self.state, loaded)
        harness._consume_permit(run_dir, loaded)
        self.assertTrue(harness._permit_attempted(run_dir, loaded))
        self.assertFalse(harness._function_consumed(run_dir, loaded))
        with self.assertRaisesRegex(harness.CrackHarnessError, "already been attempted"):
            harness._consume_permit(run_dir, loaded)

    def test_permit_ledger_does_not_end_function_after_32_distinct_permits(self) -> None:
        approval, _ = self.write_inputs()
        loaded = harness.load_approval(self.root, approval)
        run_dir = harness._run_dir(self.state, loaded)
        attempted = []
        for index in range(33):
            current = dict(loaded)
            current["permit_sha256"] = hashlib.sha256(
                f"distinct-permit-{index}".encode()
            ).hexdigest()
            harness._consume_permit(run_dir, current)
            attempted.append(current["permit_sha256"])
        self.assertEqual(harness._permit_attempts(run_dir, loaded), attempted)
        replay = dict(loaded)
        replay["permit_sha256"] = attempted[-1]
        with self.assertRaisesRegex(
            harness.CrackHarnessError, "already been attempted"
        ):
            harness._consume_permit(run_dir, replay)

    def test_v2_tombstone_blocks_same_cell_but_allows_distinct_candidate(self) -> None:
        approval_a, _ = self.write_inputs(campaign_id="candidate-a")
        loaded_a = harness.load_approval(self.root, approval_a)
        run_dir = harness._run_dir(self.state, loaded_a)
        harness._consume_function(run_dir, loaded_a)
        self.assertTrue(harness._function_consumed(run_dir, loaded_a))

        self.candidate.write_text(
            "int Owner(void) {\n    return 3;\n}\n", encoding="utf-8"
        )
        self.evidence = self.root / "evidence/selection-candidate-b.json"
        self.luna_audit = self.root / "evidence/luna5-candidate-b.json"
        self._write_luna5_audit()
        approval_b, _ = self.write_inputs(campaign_id="candidate-b")
        loaded_b = harness.load_approval(self.root, approval_b)
        self.assertFalse(harness._function_consumed(run_dir, loaded_b))
        ready = harness._dry_run_for_test(
            self.root, approval_b, state_root=self.state
        )
        self.assertEqual(ready["status"], "ready", ready)

    def test_legacy_v1_tombstone_does_not_end_distinct_future_candidate(self) -> None:
        approval_a, _ = self.write_inputs(campaign_id="legacy-a")
        loaded_a = harness.load_approval(self.root, approval_a)
        run_dir = harness._run_dir(self.state, loaded_a)
        run_dir.parent.mkdir(parents=True, exist_ok=True)
        (run_dir.parent / "latest-function.json").write_text(
            json.dumps({
                "schema": "crack_harness_function_tombstone/v1",
                "function_key": harness._function_key(loaded_a),
                "owner": loaded_a["owner"],
                "function": loaded_a["function"],
                "first_campaign_id": "legacy-a",
                "consumed": True,
            }),
            encoding="utf-8",
        )
        self.assertTrue(harness._function_consumed(run_dir, loaded_a))

        self.candidate.write_text(
            "int Owner(void) {\n    return 3;\n}\n", encoding="utf-8"
        )
        self.evidence = self.root / "evidence/selection-legacy-b.json"
        self.luna_audit = self.root / "evidence/luna5-legacy-b.json"
        self._write_luna5_audit()
        approval_b, _ = self.write_inputs(campaign_id="legacy-b")
        loaded_b = harness.load_approval(self.root, approval_b)
        self.assertFalse(harness._function_consumed(run_dir, loaded_b))

    def test_baseline_infrastructure_failure_does_not_consume_function(self) -> None:
        approval, permit = self.write_inputs()
        loaded = harness.load_approval(self.root, approval)
        run_dir = harness._run_dir(self.state, loaded)
        original = harness._run_command

        def fail_baseline(*args, **kwargs):
            phase = (kwargs.get("extra_env") or {}).get("CRACK_HARNESS_PHASE")
            if phase == "baseline":
                raise harness.CrackHarnessError("baseline infrastructure failure")
            return original(*args, **kwargs)

        with patch.object(harness, "_run_command", side_effect=fail_baseline):
            result = harness._run_approved_for_test(
                self.root, approval, permit_path=permit,
                state_root=self.state, manager_key_path=self.manager_key,
            )
        self.assertEqual(result["status"], "failed")
        self.assertTrue(harness._permit_attempted(run_dir, loaded))
        self.assertFalse(harness._function_consumed(run_dir, loaded))

    def test_candidate_execution_boundary_consumes_function(self) -> None:
        approval, permit = self.write_inputs()
        loaded = harness.load_approval(self.root, approval)
        run_dir = harness._run_dir(self.state, loaded)
        original = harness._run_command

        def fail_candidate(*args, **kwargs):
            phase = (kwargs.get("extra_env") or {}).get("CRACK_HARNESS_PHASE")
            if phase == "candidate":
                started = kwargs.get("on_started")
                self.assertIsNotNone(started)
                started()
                raise harness.CrackHarnessError("candidate compiler failure")
            return original(*args, **kwargs)

        with patch.object(harness, "_run_command", side_effect=fail_candidate):
            result = harness._run_approved_for_test(
                self.root, approval, permit_path=permit,
                state_root=self.state, manager_key_path=self.manager_key,
            )
        self.assertEqual(result["status"], "failed")
        self.assertTrue(harness._function_consumed(run_dir, loaded))

    def test_launch_callback_runs_after_containment_before_process_resume(self) -> None:
        run_temp = self.root / "launch-callback"
        run_temp.mkdir()
        events: list[str] = []
        marker = run_temp / "reserved-before-resume"
        real_popen = harness.subprocess.Popen
        real_assign = harness._assign_windows_job
        real_resume = harness._resume_windows_process

        def popen(*args, **kwargs):
            events.append("popen")
            process = real_popen(*args, **kwargs)
            events.append("popen-return")
            return process

        def assign(process):
            events.append("assign")
            return real_assign(process)

        def resume(process, job_handle):
            self.assertTrue(marker.is_file())
            events.append("resume")
            return real_resume(process, job_handle)

        def reserve() -> None:
            marker.write_text("reserved", encoding="utf-8")
            events.append("started")

        with patch.object(harness.subprocess, "Popen", side_effect=popen), patch.object(
            harness, "_assign_windows_job", side_effect=assign
        ), patch.object(harness, "_resume_windows_process", side_effect=resume):
            harness._run_command(
                [sys.executable, "-c", "pass"],
                root=self.root,
                run_temp=run_temp,
                deadline=time.monotonic() + 5,
                storage_limit=4096,
                expect_json=False,
                on_started=reserve,
            )

        self.assertEqual(events, ["popen", "popen-return", "assign", "started", "resume"])

    def test_assignment_failure_does_not_invoke_consumption_callback(self) -> None:
        run_temp = self.root / "launch-failure"
        run_temp.mkdir()
        events: list[str] = []
        with patch.object(
            harness, "_assign_windows_job", side_effect=OSError("assignment failed")
        ):
            with self.assertRaisesRegex(OSError, "assignment failed"):
                harness._run_command(
                    [sys.executable, "-c", "pass"],
                    root=self.root,
                    run_temp=run_temp,
                    deadline=time.monotonic() + 5,
                    storage_limit=4096,
                    expect_json=False,
                    on_started=lambda: events.append("started"),
                )
        self.assertNotIn("started", events)

    def test_callback_failure_prevents_process_resume(self) -> None:
        run_temp = self.root / "callback-failure"
        run_temp.mkdir()
        events: list[str] = []
        real_resume = harness._resume_windows_process

        def fail_callback() -> None:
            events.append("callback")
            raise harness.CrackHarnessError("reservation failed")

        def resume(process, job_handle):
            events.append("resume")
            return real_resume(process, job_handle)

        with patch.object(harness, "_resume_windows_process", side_effect=resume):
            with self.assertRaisesRegex(harness.CrackHarnessError, "reservation failed"):
                harness._run_command(
                    [sys.executable, "-c", "pass"],
                    root=self.root,
                    run_temp=run_temp,
                    deadline=time.monotonic() + 5,
                    storage_limit=4096,
                    expect_json=False,
                    on_started=fail_callback,
                )
        self.assertEqual(events, ["callback"])

    def test_resume_failure_rolls_back_launch_reservation(self) -> None:
        run_temp = self.root / "resume-failure"
        run_temp.mkdir()
        marker = run_temp / "reservation"
        rolled_back: list[str] = []

        def reserve() -> object:
            marker.write_text("reserved", encoding="utf-8")

            def rollback() -> None:
                marker.unlink()
                rolled_back.append("rollback")

            return rollback

        with patch.object(
            harness, "_resume_windows_process", side_effect=OSError("resume failed")
        ):
            with self.assertRaisesRegex(OSError, "resume failed"):
                harness._run_command(
                    [sys.executable, "-c", "pass"],
                    root=self.root,
                    run_temp=run_temp,
                    deadline=time.monotonic() + 5,
                    storage_limit=4096,
                    expect_json=False,
                    on_started=reserve,
                )
        self.assertEqual(rolled_back, ["rollback"])
        self.assertFalse(marker.exists())

    def test_successful_resume_leaves_v2_and_legacy_markers_consumed(self) -> None:
        for legacy in (False, True):
            with self.subTest(legacy=legacy):
                if (self.state / "owners").exists():
                    shutil.rmtree(self.state / "owners")
                (self.state / "consumed-cells.json").unlink(missing_ok=True)
                approval, _ = self.write_inputs(
                    campaign_id="legacy-success" if legacy else "v2-success"
                )
                if legacy:
                    run_dir, _ = self.add_legacy_retry(approval)
                else:
                    loaded = harness.load_approval(self.root, approval)
                    run_dir = harness._run_dir(self.state, loaded)
                loaded = harness.load_approval(self.root, approval)
                run_temp = self.state / (
                    "legacy-marker-success" if legacy else "v2-marker-success"
                )
                run_temp.mkdir()
                marker_name = "retry-used.json" if legacy else "latest-function.json"
                marker = run_dir.parent / marker_name
                ledger = self.state / "consumed-cells.json"
                events: list[str] = []
                real_resume = harness._resume_windows_process

                def reserve() -> object:
                    rollback = harness._consume_function(run_dir, loaded)
                    self.assertTrue(marker.is_file())
                    self.assertTrue(ledger.is_file())
                    events.append("reserved")
                    return rollback

                def resume(process: object, job_handle: int | None) -> None:
                    self.assertTrue(marker.is_file())
                    events.append("resume")
                    real_resume(process, job_handle)

                with patch.object(
                    harness, "_resume_windows_process", side_effect=resume
                ):
                    harness._run_command(
                        [sys.executable, "-c", "pass"],
                        root=self.root, run_temp=run_temp,
                        deadline=time.monotonic() + 5,
                        storage_limit=4096, expect_json=False,
                        state_root=self.state,
                        on_started=reserve,
                        on_started_state_paths=(marker, ledger),
                    )
                self.assertEqual(events, ["reserved", "resume"])
                self.assertTrue(marker.is_file())
                self.assertTrue(ledger.is_file())
                self.assertTrue(harness._function_consumed(run_dir, loaded))

    def test_resume_failure_rolls_back_v2_and_legacy_markers(self) -> None:
        for legacy in (False, True):
            with self.subTest(legacy=legacy):
                if (self.state / "owners").exists():
                    shutil.rmtree(self.state / "owners")
                (self.state / "consumed-cells.json").unlink(missing_ok=True)
                approval, _ = self.write_inputs(
                    campaign_id="legacy-resume-failure" if legacy else "v2-resume-failure"
                )
                if legacy:
                    run_dir, _ = self.add_legacy_retry(approval)
                else:
                    loaded = harness.load_approval(self.root, approval)
                    run_dir = harness._run_dir(self.state, loaded)
                loaded = harness.load_approval(self.root, approval)
                run_temp = self.state / (
                    "legacy-marker-failure" if legacy else "v2-marker-failure"
                )
                run_temp.mkdir()
                marker_name = "retry-used.json" if legacy else "latest-function.json"
                marker = run_dir.parent / marker_name
                ledger = self.state / "consumed-cells.json"

                def reserve() -> object:
                    self.assertFalse(marker.exists())
                    self.assertFalse(ledger.exists())
                    return harness._consume_function(run_dir, loaded)

                def fail_resume(process: object, job_handle: int | None) -> None:
                    self.assertTrue(marker.is_file())
                    raise OSError("resume failed")

                with patch.object(
                    harness, "_resume_windows_process", side_effect=fail_resume
                ):
                    with self.assertRaisesRegex(OSError, "resume failed"):
                        harness._run_command(
                            [sys.executable, "-c", "pass"],
                            root=self.root, run_temp=run_temp,
                            deadline=time.monotonic() + 5,
                            storage_limit=4096, expect_json=False,
                            state_root=self.state,
                            on_started=reserve,
                            on_started_state_paths=(marker, ledger),
                        )
                self.assertFalse(marker.exists())
                self.assertFalse(ledger.exists())
                self.assertFalse(harness._function_consumed(run_dir, loaded))

    def test_setup_failure_cleans_process_job_and_pipes(self) -> None:
        class FakeStream:
            def __init__(self) -> None:
                self.closed = False

            def read(self, _size: int) -> bytes:
                return b""

            def close(self) -> None:
                self.closed = True

        class FakeProcess:
            pid = 1234

            def __init__(self) -> None:
                self.stdout = FakeStream()
                self.stderr = FakeStream()

            def poll(self) -> None:
                return None

        run_temp = self.root / "setup-cleanup"
        run_temp.mkdir()
        for failure in ("assignment", "resume"):
            with self.subTest(failure=failure):
                process = FakeProcess()
                events: list[tuple[str, object]] = []

                def assign(_process: object) -> int | None:
                    events.append(("assign", _process))
                    if failure == "assignment":
                        raise OSError("assignment failed")
                    return 17

                def resume(_process: object, _job: int | None) -> None:
                    events.append(("resume", _job))
                    raise OSError("resume failed")

                with patch.object(
                    harness.subprocess, "Popen", return_value=process
                ), patch.object(
                    harness, "_assign_windows_job", side_effect=assign
                ), patch.object(
                    harness, "_resume_windows_process", side_effect=resume
                ), patch.object(
                    harness, "_terminate_process",
                    side_effect=lambda value: events.append(("terminate", value)),
                ), patch.object(
                    harness, "_quiesce_windows_job",
                    side_effect=lambda value, **kwargs: events.append(("quiesce", value)),
                ), patch.object(
                    harness, "_close_windows_job",
                    side_effect=lambda value: events.append(("close", value)),
                ):
                    with self.assertRaisesRegex(OSError, f"{failure} failed"):
                        harness._run_command(
                            [sys.executable, "-c", "pass"],
                            root=self.root, run_temp=run_temp,
                            deadline=time.monotonic() + 5,
                            storage_limit=4096, expect_json=False,
                        )

                self.assertTrue(any(name == "terminate" for name, _ in events))
                self.assertTrue(process.stdout.closed)
                self.assertTrue(process.stderr.closed)
                if failure == "resume":
                    self.assertIn(("quiesce", 17), events)
                    self.assertIn(("close", 17), events)

    def test_post_candidate_proof_failure_consumes_function(self) -> None:
        approval, permit = self.write_inputs()
        loaded = harness.load_approval(self.root, approval)
        run_dir = harness._run_dir(self.state, loaded)

        with patch.object(
            harness, "_validate_proof",
            side_effect=harness.CrackHarnessError("proof infrastructure failure"),
        ):
            result = harness._run_approved_for_test(
                self.root, approval, permit_path=permit,
                state_root=self.state, manager_key_path=self.manager_key,
            )
        self.assertEqual(result["status"], "failed")
        self.assertTrue(harness._function_consumed(run_dir, loaded))

    def test_failure_text_cannot_reopen_legacy_v1_tombstone(self) -> None:
        approval, _ = self.write_inputs()
        loaded = harness.load_approval(self.root, approval)
        run_dir = harness._run_dir(self.state, loaded)
        run_dir.parent.mkdir(parents=True, exist_ok=True)
        tombstone = {
            "schema": "crack_harness_function_tombstone/v1",
            "function_key": harness._function_key(loaded),
            "owner": loaded["owner"], "function": loaded["function"],
            "first_campaign_id": loaded["campaign"]["id"],
            "consumed": True,
        }
        (run_dir.parent / "latest-function.json").write_text(
            json.dumps(tombstone), encoding="utf-8",
        )
        (run_dir.parent / "latest-failure.json").write_text(
            json.dumps({
                "schema": "crack_harness_failure_diagnostic/v1",
                "owner": loaded["owner"], "function": loaded["function"],
                "approval_sha256": loaded["_approval_sha256"],
                "primary_reason": (
                    "proof strict compiler wrote outside the disposable worktree"
                ),
            }),
            encoding="utf-8",
        )
        self.assertTrue(harness._function_consumed(run_dir, loaded))

    def test_same_candidate_is_one_shot_across_campaign_ids(self) -> None:
        approval, _ = self.write_inputs()
        loaded = harness.load_approval(self.root, approval)
        run_dir = harness._run_dir(self.state, loaded)
        harness._consume_function(run_dir, loaded)
        other_approval, _ = self.write_inputs(campaign_id="different-campaign")
        other = harness.load_approval(self.root, other_approval)
        self.assertTrue(harness._function_consumed(run_dir, other))
        dry_run = harness._dry_run_for_test(
            self.root, other_approval, state_root=self.state
        )
        self.assertEqual(dry_run["status"], "blocked")
        self.assertTrue(any("lifetime cell" in item for item in dry_run["blockers"]))
        with self.assertRaisesRegex(harness.CrackHarnessError, "already consumed"):
            harness._consume_function(run_dir, other)

    def test_different_candidate_is_allowed_after_a_prior_cell(self) -> None:
        approval, _ = self.write_inputs(campaign_id="first-cell")
        first = harness.load_approval(self.root, approval)
        run_dir = harness._run_dir(self.state, first)
        harness._consume_function(run_dir, first)

        self.candidate.write_text(
            "int Owner(void) {\n    return 3;\n}\n", encoding="utf-8"
        )
        self._write_luna5_audit()
        second_path, _ = self.write_inputs(campaign_id="second-cell")
        second = harness.load_approval(self.root, second_path)
        self.assertNotEqual(first["candidate"]["sha256"], second["candidate"]["sha256"])
        self.assertFalse(harness._function_consumed(run_dir, second))
        harness._consume_function(run_dir, second)
        self.assertTrue(harness._function_consumed(run_dir, second))
        self.assertTrue(harness._function_consumed(run_dir, first))

    def test_same_candidate_on_a_different_base_is_not_the_same_cell(self) -> None:
        approval, _ = self.write_inputs(campaign_id="first-base")
        first = harness.load_approval(self.root, approval)
        run_dir = harness._run_dir(self.state, first)
        harness._consume_function(run_dir, first)

        self.source.write_text(
            "int Owner(void) {\n    return 0;\n}\n", encoding="utf-8"
        )
        self.base.write_bytes(self.source.read_bytes())
        self.evidence = self.root / "evidence/selection-second-base.json"
        self.current_residual = self.root / "evidence/residual-second-base.json"
        self.luna_audit = self.root / "evidence/luna5-second-base.json"
        self._write_luna5_audit()
        second_path, _ = self.write_inputs(campaign_id="second-base")
        second = harness.load_approval(self.root, second_path)
        self.assertEqual(
            first["candidate"]["sha256"], second["candidate"]["sha256"]
        )
        self.assertNotEqual(first["base"]["sha256"], second["base"]["sha256"])
        self.assertFalse(harness._function_consumed(run_dir, second))

    def test_no_gain_consumes_only_the_same_base_and_candidate(self) -> None:
        first = self.execute(gain=0, campaign_id="no-gain-first")
        self.assertEqual(first["status"], "no_gain", first)

        self.base.write_bytes(self.source.read_bytes())
        self.candidate.write_text(
            "int Owner(void) {\n    return 3;\n}\n", encoding="utf-8"
        )
        self.evidence = self.root / "evidence/selection-after-no-gain.json"
        self.current_residual = self.root / "evidence/residual-after-no-gain.json"
        self.luna_audit = self.root / "evidence/luna5-after-no-gain.json"
        self._write_luna5_audit()
        second_path, _ = self.write_inputs(campaign_id="no-gain-second")
        second = harness.load_approval(self.root, second_path)
        run_dir = harness._run_dir(self.state, second)
        self.assertFalse(harness._function_consumed(run_dir, second))
        self.assertEqual(
            harness._dry_run_for_test(
                self.root, second_path, state_root=self.state,
            )["status"],
            "ready",
        )

    def test_completed_exact_report_closes_function_for_new_candidate(self) -> None:
        first = self.execute(campaign_id="exact-first")
        self.assertEqual(first["status"], "exact", first)

        self.base.write_bytes(self.source.read_bytes())
        self.candidate.write_text(
            "int Owner(void) {\n    return 3;\n}\n", encoding="utf-8"
        )
        self.evidence = self.root / "evidence/selection-after-exact.json"
        self.current_residual = self.root / "evidence/residual-after-exact.json"
        self.luna_audit = self.root / "evidence/luna5-after-exact.json"
        self._write_luna5_audit()
        second_path, _ = self.write_inputs(campaign_id="exact-second")
        readiness = harness._dry_run_for_test(
            self.root, second_path, state_root=self.state,
        )
        self.assertEqual(readiness["status"], "blocked")
        self.assertIn("function already has a terminal result", readiness["blockers"])

    def test_new_campaign_is_allowed_before_any_function_cell_is_consumed(self) -> None:
        approval, _ = self.write_inputs(campaign_id="different-campaign")
        dry_run = harness._dry_run_for_test(
            self.root, approval, state_root=self.state
        )
        self.assertEqual(dry_run["status"], "ready")

    def test_function_tombstone_tamper_fails_closed(self) -> None:
        approval, _ = self.write_inputs()
        loaded = harness.load_approval(self.root, approval)
        run_dir = harness._run_dir(self.state, loaded)
        harness._consume_function(run_dir, loaded)
        tombstone_path = run_dir.parent / "latest-function.json"
        tombstone = json.loads(tombstone_path.read_text(encoding="utf-8"))
        tombstone["owner"] = "main:board/other"
        tombstone_path.write_text(json.dumps(tombstone), encoding="utf-8")
        with self.assertRaisesRegex(harness.CrackHarnessError, "binding is invalid"):
            harness._function_consumed(run_dir, loaded)

    def test_retention_gc_preserves_function_and_permit_guards(self) -> None:
        approval, _ = self.write_inputs()
        loaded = harness.load_approval(self.root, approval)
        run_dir = harness._run_dir(self.state, loaded)
        harness._consume_permit(run_dir, loaded)
        harness._consume_function(run_dir, loaded)
        payload = run_dir / "raw" / "bulk.bin"
        payload.parent.mkdir(parents=True)
        payload.write_bytes(b"x" * 8192)
        owner_limit = harness._tree_size(run_dir.parents[1]) - 4096
        harness._gc_owner(run_dir, owner_limit)
        self.assertFalse(payload.exists())
        self.assertTrue(harness._permit_attempted(run_dir, loaded))
        self.assertTrue(harness._function_consumed(run_dir, loaded))

        payload.parent.mkdir(parents=True)
        payload.write_bytes(b"x" * 8192)
        global_limit = harness._tree_size(self.state) - 4096
        harness._gc_global(self.state, global_limit)
        self.assertFalse(payload.exists())
        self.assertTrue(harness._permit_attempted(run_dir, loaded))
        self.assertTrue(harness._function_consumed(run_dir, loaded))

    @unittest.skipUnless(os.name == "nt", "Windows path semantics")
    def test_windows_paths(self) -> None:
        approval, _ = self.write_inputs(); self.assertEqual(harness.load_approval(self.root, approval)["_paths"]["source"], self.source)

    def test_timeout_output_cap_and_storage_cap_kill_children(self) -> None:
        run_temp = self.root / "limits"; run_temp.mkdir()
        with self.assertRaisesRegex(harness.CrackHarnessError, "active-time"):
            harness._run_command([sys.executable, "-c", "import time;time.sleep(2)"], root=self.root, run_temp=run_temp, deadline=time.monotonic() + 0.2, storage_limit=4096, expect_json=False)
        with self.assertRaisesRegex(harness.CrackHarnessError, "1 MiB"):
            harness._run_command([sys.executable, "-c", "print('x'*1100000)"], root=self.root, run_temp=run_temp, deadline=time.monotonic() + 5, storage_limit=4096, expect_json=False)
        command = "import os,pathlib,time;pathlib.Path(os.environ['CRACK_HARNESS_TEMP'],'large').write_bytes(b'x'*8192);time.sleep(1)"
        with self.assertRaisesRegex(harness.CrackHarnessError, "storage"):
            harness._run_command([sys.executable, "-c", command], root=self.root, run_temp=run_temp, deadline=time.monotonic() + 5, storage_limit=4096, expect_json=False)

    def test_output_paths_cannot_escape_disposable_roots(self) -> None:
        with self.assertRaisesRegex(harness.CrackHarnessError, "escapes"):
            harness._expand_argv([sys.executable, "hook.py", "../outside"], self.root / "worktree", self.root / "out", self.root)

    def test_tree_manifest_rejects_symlink_or_reparse_components(self) -> None:
        tree = self.root / "indirection-tree"; tree.mkdir()
        target = self.root / "indirection-target"; target.mkdir()
        link = tree / "link"
        try:
            link.symlink_to(target, target_is_directory=True)
        except OSError as exc:
            self.skipTest(f"symlink creation unavailable: {exc}")
        with self.assertRaisesRegex(harness.CrackHarnessError, "indirection"):
            harness._tree_manifest(tree)

    def test_current_owner_state_is_hard_capped(self) -> None:
        approval, _ = self.write_inputs(); loaded = harness.load_approval(self.root, approval)
        run = harness._run_dir(self.state, loaded); run.mkdir(parents=True)
        (run / "oversized").write_bytes(b"x" * (harness.MAX_RETAINED_OWNER_BYTES + 1))
        harness._gc_owner(run, harness.MAX_RETAINED_OWNER_BYTES)
        self.assertLessEqual(harness._tree_size(run.parents[1]), harness.MAX_RETAINED_OWNER_BYTES)

    def test_owner_gc_accepts_deleted_latest_after_failed_finalization(self) -> None:
        approval, _ = self.write_inputs()
        loaded = harness.load_approval(self.root, approval)
        run = harness._run_dir(self.state, loaded)
        function_dir = run.parent
        function_dir.mkdir(parents=True)
        (function_dir / "latest-failure.json").write_text(
            "{}", encoding="utf-8",
        )
        self.assertFalse(run.exists())

        harness._gc_owner(run, harness.MAX_RETAINED_OWNER_BYTES)

        self.assertTrue((function_dir / "latest-failure.json").is_file())
        self.assertFalse(run.exists())

    def test_global_state_cap_removes_old_compact_frontiers(self) -> None:
        for index in range(2):
            latest = self.state / "owners" / f"owner{index}" / "Owner" / "latest"
            latest.mkdir(parents=True); (latest / "result.json").write_bytes(b"x" * 800)
        harness._gc_global(self.state, 1024)
        self.assertLessEqual(harness._tree_size(self.state), 1024)

    def test_startup_scavenges_abandoned_inputs_and_temp(self) -> None:
        approval, permit = self.write_inputs()
        temp = self.state / "owners" / "owner" / "Owner" / "latest" / "temp"
        temp.mkdir(parents=True); (temp / "raw.bin").write_bytes(b"raw")
        body = {"schema":harness.ATTEMPT_SCHEMA,"run_dir":str(temp.parent),"source_path":str(self.source),"approval_path":str(approval),"approval_sha256":sha(approval),"disposable_paths":[str(self.base),str(self.candidate),str(permit),str(approval)]}
        signed_attempt = harness._sign_attempt_receipt(
            self.root, body, manager_key_path=self.manager_key,
            expected_key_id=sha(self.manager_key),
        )
        (self.state / "attempt.json").write_text(json.dumps(signed_attempt), encoding="utf-8")
        harness._scavenge_disposable_worktrees(
            self.root, self.state, manager_key_path=self.manager_key,
            expected_key_id=sha(self.manager_key),
        )
        self.assertFalse(approval.exists()); self.assertFalse(temp.exists())

    def test_recovery_journal_cannot_escape_state_temp(self) -> None:
        outside = self.root / "outside.snapshot"; outside.write_bytes(self.base.read_bytes())
        run = self.state / "owners/test/Owner/latest"; run.mkdir(parents=True)
        body = {"schema":"crack_harness_transaction/v1","source_relpath":"src/owner.c","baseline_snapshot":str(outside),"baseline_sha256":sha(self.base),"approval_sha256":"a"*64,"target_object_sha256":TARGET_SHA,"candidate_sha256":sha(self.candidate),"result_path":str(run/"result.json"),"record_commit_path":str(run/"record.commit.json"),"worktree":str(run/"temp/worktree"),"central_record_binding":None}
        (self.state / "transaction.json").write_text(json.dumps(body | {"transaction_sha256":harness._digest_json(body)}), encoding="utf-8")
        with self.assertRaisesRegex(harness.CrackHarnessError, "exact run temp"):
            harness._recover_interrupted(self.root, self.state)
        self.assertTrue(outside.exists())

    @unittest.skipUnless(os.name == "nt", "Windows Job Object semantics")
    def test_successful_parent_cannot_leave_writing_descendant(self) -> None:
        run_temp = self.root / "descendant"; run_temp.mkdir()
        marker = run_temp / "late.txt"
        child = "import pathlib,time;time.sleep(0.5);pathlib.Path(r'%s').write_text('late')" % marker
        parent = "import subprocess,sys;subprocess.Popen([sys.executable,'-c',%r])" % child
        harness._run_command([sys.executable, "-c", parent], root=self.root, run_temp=run_temp, deadline=time.monotonic() + 5, storage_limit=4096, expect_json=False)
        time.sleep(0.8)
        self.assertFalse(marker.exists())


if __name__ == "__main__":
    unittest.main()
