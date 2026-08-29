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
        self.source.parent.mkdir(parents=True)
        self.admission.parent.mkdir()
        self.evidence = self.root / "evidence/selection.json"
        self.evidence.parent.mkdir()
        self.source.write_text("int Owner(void) {\n    return 1;\n}\n", encoding="utf-8")
        self.base = self.root / "base.c"
        self.candidate = self.root / "candidate.c"
        self.base.write_bytes(self.source.read_bytes())
        self.candidate.write_text("int Owner(void) {\n    return 2;\n}\n", encoding="utf-8")
        self._write_selection_evidence()
        self.admission.write_text(
            self._admission_source(),
            encoding="utf-8",
        )
        self.hook.write_text(self._hook_source(), encoding="utf-8")
        self.compile_hook.write_text(self._hook_source(), encoding="utf-8")
        self._git("init", "-q")
        self._git("config", "user.email", "test@example.invalid")
        self._git("config", "user.name", "Harness Test")
        self._git("add", "src/owner.c", "evidence/selection.json", "tools/candidate_compile_admission.py", "tools/crack_harness.py", "tools/crack_evidence_bundle.py")
        self._git("commit", "-qm", "fixture")
        self.commit = self._git("rev-parse", "HEAD")
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

    def _write_selection_evidence(self) -> None:
        predicted_rows = ["Owner:ARG:0"]
        value = {
            "schema": harness.WINNING_CELL_EVIDENCE_SCHEMA,
            "owner": "main:board/test", "function": "Owner",
            "strategy": "winning_cell_first", "rank": 1,
            "expected_terminal": "exact",
            "candidate_sha256": sha(self.candidate),
            "predicted_rows_sha256": harness._digest_json(predicted_rows),
            "alternatives_compiled": 0, "negative_controls": 0,
            "pivot_if_unranked": True, "source_class": "test-natural-cell",
            "inputs": [{
                "path": "base.c", "sha256": sha(self.base),
                "role": "sealed_baseline_source",
            }],
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

    def _hook_source(self) -> str:
        return r'''import hashlib,json,os,pathlib,sys,time
if pathlib.Path(sys.argv[0]).name=='crack_evidence_bundle.py':
 out=pathlib.Path(os.environ['CRACK_HARNESS_OUT_ROOT']); out.mkdir(parents=True,exist_ok=True)
 desc=lambda p:{'sha256':hashlib.sha256(p.read_bytes()).hexdigest(),'size_bytes':p.stat().st_size}
 digest=lambda v:hashlib.sha256(json.dumps(v,sort_keys=True,separators=(',',':')).encode()).hexdigest()
 phase=os.environ['CRACK_HARNESS_PHASE']; source_rel=os.environ['CRACK_HARNESS_SOURCE_PATH']
 def receipt(phase,names):
  value={'schema':'crack_evidence_phase_receipt/v1','phase':phase,'owner':os.environ['CRACK_HARNESS_OWNER'],'function':os.environ['CRACK_HARNESS_FUNCTION'],'unit':os.environ['CRACK_HARNESS_UNIT'],'source_relpath':source_rel,'base_commit':os.environ['CRACK_HARNESS_BASE_COMMIT'],'approval_sha256':os.environ['CRACK_HARNESS_APPROVAL_SHA256'],'approval_context_sha256':os.environ['CRACK_HARNESS_CONTEXT_SHA256'],'phase_nonce':os.environ['CRACK_HARNESS_PHASE_NONCE'],'issued_at':os.environ['CRACK_HARNESS_ISSUED_AT'],'artifacts':{name:desc(out/name) for name in names},'tools':{'fixture':{'sha256':'f'*64}},'authority_advanced':False}
  value['receipt_sha256']=digest(value); (out/(phase+'-receipt.json')).write_text(json.dumps(value)); return value
 if os.environ['CRACK_HARNESS_PHASE']=='baseline':
  (out/'target.o').write_bytes(b'target'); (out/'baseline-candidate.o').write_bytes(b'baseline'); (out/'baseline-strict.json').write_text('{}'); (out/'baseline-data.json').write_text('{}')
  receipt('baseline',['target.o','baseline-candidate.o','baseline-strict.json','baseline-data.json'])
  value={'schema':'crack_evidence_bundle_context/v1','owner':os.environ['CRACK_HARNESS_OWNER'],'function':os.environ['CRACK_HARNESS_FUNCTION'],'unit':os.environ['CRACK_HARNESS_UNIT'],'source_relpath':source_rel,'target_sha256':os.environ['CRACK_HARNESS_TARGET_SHA256'],'base_commit':os.environ['CRACK_HARNESS_BASE_COMMIT'],'approval_sha256':os.environ['CRACK_HARNESS_APPROVAL_SHA256'],'approval_context_sha256':os.environ['CRACK_HARNESS_CONTEXT_SHA256'],'phase_nonces':{'baseline':os.environ['CRACK_HARNESS_PHASE_NONCE'],'candidate':hashlib.sha256((os.environ['CRACK_HARNESS_CONTEXT_SHA256']+':candidate').encode()).hexdigest()},'baseline_receipt':desc(out/'baseline-receipt.json'),'authority_advanced':False}; value['evidence_context_sha256']=digest(value); (out/'evidence-context.json').write_text(json.dumps(value))
 else:
  (out/'candidate.o').write_bytes(b'candidate'); (out/'candidate-strict.json').write_text('{}'); (out/'candidate-data.json').write_text('{}'); (out/'physical.json').write_text('{}'); (out/'fixture.json').write_text(json.dumps({'gain':float(os.environ['HARNESS_TEST_GAIN']),'focus':int(os.environ['HARNESS_TEST_FOCUS']),'data_gain':float(os.environ.get('HARNESS_TEST_DATA_GAIN','0')),'sibling_losses':int(os.environ.get('HARNESS_TEST_SIBLING_LOSSES','0')),'physical_diff':int(os.environ.get('HARNESS_TEST_PHYSICAL_DIFF','0')),'size_delta':int(os.environ.get('HARNESS_TEST_SIZE_DELTA','0'))}))
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
 else: value={'schema':'crack_assessment/v1','owner':get('--owner'),'function':get('--function'),'candidate_source_sha256':candidate_source,'target_object_sha256':target,'candidate_object_sha256':candidate,'owner_gain':fixture['gain'],'data_gain':fixture['data_gain'],'data_diff_delta':1 if fixture['data_gain']<0 else 0}
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
elif kind=='assess': print(json.dumps({'schema':'crack_assessment/v1','owner':'main:board/test','function':'Owner','candidate_source_sha256':candidate_source,'target_object_sha256':target,'candidate_object_sha256':candidate,'owner_gain':float(rest[0]),'data_gain':0.0,'data_diff_delta':0}))
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
            argv = [str(Path(sys.executable).resolve()), "{CONTROLLER_ROOT}/tools/crack_harness.py", "proof-adapter", "--kind", adapter_kind, "--owner", "main:board/test", "--function", "Owner", "--candidate-source", "{RUN_ROOT}/src/owner.c", "--candidate-source-sha256", sha(self.candidate), "--approved-target-object-sha256", TARGET_SHA, "--target-object", "{OUT_ROOT}/target.o", "--candidate-object", "{OUT_ROOT}/candidate.o", "--baseline-strict-report", "{OUT_ROOT}/baseline-strict.json", "--baseline-data-report", "{OUT_ROOT}/baseline-data.json", "--candidate-strict-report", "{OUT_ROOT}/candidate-strict.json", "--candidate-data-report", "{OUT_ROOT}/candidate-data.json", "--physical-receipt", "{OUT_ROOT}/physical.json"]
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
        sibling_losses: int = 0, physical_diff: int = 0, size_delta: int = 0,
        campaign_id: str = "campaign",
    ) -> tuple[Path, Path]:
        self._write_selection_evidence()
        issued = datetime.now(timezone.utc).replace(microsecond=0)
        deadline = issued + timedelta(minutes=20)
        stop_nonce = "f" * 64
        precompile = {
            "kind": "canonical_admission",
            "argv": [str(Path(sys.executable).resolve()), str(self.admission), "--root", str(self.root), "admit", "--owner", "main:board/test", "--function", "Owner", "--base-commit", self.commit, "--toolchain-key", harness.TOOLCHAIN_MANIFEST_KEY, "--target-sha256", TARGET_SHA, "--source-sha256", sha(self.candidate), "--source-path", str(self.source), "--json"],
            "executable": {"path": str(Path(sys.executable).resolve()), "sha256": sha(Path(sys.executable))},
            "script": {"path": str(self.admission), "sha256": sha(self.admission)},
        }
        predicted_rows = ["Owner:ARG:0"]
        value = {
            "schema": harness.APPROVAL_SCHEMA, "approval_id": "cell-1", "owner": "main:board/test", "task_id": "task", "function": "Owner", "unit": "main/board/test", "base_commit": self.commit, "toolchain_key": harness.TOOLCHAIN_MANIFEST_KEY, "target_sha256": TARGET_SHA, "permit_sha256": "0" * 64, "issued_at": issued.isoformat(), "expires_at": deadline.isoformat(),
            "source": {"path": str(self.source), "sha256": sha(self.source)}, "base": {"path": str(self.base), "sha256": sha(self.base)}, "candidate": {"path": str(self.candidate), "sha256": sha(self.candidate)}, "function_span": {"start_line": 1, "end_line": 3, "base_span_sha256": hashlib.sha256(self.base.read_bytes()).hexdigest()}, "predicted_rows": predicted_rows,
            "selection": {"strategy": "winning_cell_first", "rank": 1, "expected_terminal": "exact", "evidence": {"path": "evidence/selection.json", "sha256": sha(self.evidence)}, "candidate_sha256": sha(self.candidate), "predicted_rows_sha256": harness._digest_json(predicted_rows), "alternatives_compiled": 0, "negative_controls": 0, "pivot_if_unranked": True, "source_class": "test-natural-cell"},
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

    def execute(self, **kwargs: int) -> dict:
        approval, permit = self.write_inputs(**kwargs)
        settings = {
            "HARNESS_TEST_GAIN": kwargs.get("gain", 1),
            "HARNESS_TEST_FOCUS": kwargs.get("focus_rows", 0),
            "HARNESS_TEST_DATA_GAIN": kwargs.get("data_gain", 0),
            "HARNESS_TEST_SIBLING_LOSSES": kwargs.get("sibling_losses", 0),
            "HARNESS_TEST_PHYSICAL_DIFF": kwargs.get("physical_diff", 0),
            "HARNESS_TEST_SIZE_DELTA": kwargs.get("size_delta", 0),
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
        self.assertEqual(self.source.read_text(), "int Owner(void) {\n    return 2;\n}\n", result)
        self.assertFalse(self.base.exists()); self.assertFalse(self.candidate.exists()); self.assertFalse(self.permit.exists()); self.assertFalse(self.approval.exists())
        run = next((self.state / "owners").glob("*/*/*"))
        self.assertFalse((run / "temp").exists()); self.assertFalse((run / "retained_winner.c").exists())
        self.assertTrue((run / "CRACK_REPORT_v1.json").is_file())

    def test_positive_nonexact_is_improved_without_report(self) -> None:
        result = self.execute(gain=2, focus_rows=1)
        self.assertEqual(result["status"], "improved", result)
        run = next((self.state / "owners").glob("*/*/*"))
        self.assertFalse((run / "CRACK_REPORT_v1.json").exists())
        self.assertIn("return 2", self.source.read_text())

    def test_no_gain_restores(self) -> None:
        result = self.execute(gain=0)
        self.assertEqual(result["status"], "no_gain", result)
        self.assertIn("return 1", self.source.read_text())
        self.assertFalse(any(self.state.glob("owners/*/*/latest/result.json")))
        self.assertFalse(any(self.state.glob("owners/*/*/latest/temp")))
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

    def test_signed_permit_rejects_approval_identity_drift(self) -> None:
        approval_path, permit_path = self.write_inputs()
        value = json.loads(approval_path.read_text(encoding="utf-8"))
        value["predicted_rows"] = ["Owner:ARG:1"]
        value["selection"]["predicted_rows_sha256"] = harness._digest_json(
            value["predicted_rows"]
        )
        evidence = json.loads(self.evidence.read_text(encoding="utf-8"))
        evidence["predicted_rows_sha256"] = value["selection"]["predicted_rows_sha256"]
        evidence["causal_prediction"]["predicted_rows"] = value["predicted_rows"]
        self.evidence.write_text(json.dumps(evidence), encoding="utf-8")
        value["selection"]["evidence"]["sha256"] = sha(self.evidence)
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

    def test_winning_cell_must_predict_an_exact_terminal(self) -> None:
        approval_path, _ = self.write_inputs()
        value = json.loads(approval_path.read_text(encoding="utf-8"))
        value["selection"]["expected_terminal"] = "improved"
        approval_path.write_text(json.dumps(value), encoding="utf-8")
        with self.assertRaisesRegex(harness.CrackHarnessError, "must be exact"):
            harness.load_approval(self.root, approval_path)

        approval_path, _ = self.write_inputs()
        evidence = json.loads(self.evidence.read_text(encoding="utf-8"))
        evidence["expected_terminal"] = "improved"
        self.evidence.write_text(json.dumps(evidence), encoding="utf-8")
        approval = json.loads(approval_path.read_text(encoding="utf-8"))
        approval["selection"]["evidence"]["sha256"] = sha(self.evidence)
        approval_path.write_text(json.dumps(approval), encoding="utf-8")
        with self.assertRaisesRegex(harness.CrackHarnessError, "expected_terminal"):
            harness.load_approval(self.root, approval_path)

    def test_winning_cell_evidence_semantics_are_owner_function_and_rows_bound(self) -> None:
        for mutation, expected_message in (
            (lambda value: value.__setitem__("owner", "main:board/other"), "owner"),
            (lambda value: value.__setitem__("function", "Other"), "function"),
            (
                lambda value: value["causal_prediction"].__setitem__(
                    "predicted_rows", ["Owner:ARG:1"]
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

    def test_crash_after_central_record_invalidates_before_source_rollback(self) -> None:
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
            harness._recover_interrupted(self.root, self.state)
        self.assertEqual(self.source.read_bytes(), self.base.read_bytes())
        with contextlib.closing(sqlite3.connect(store.path)) as connection:
            self.assertEqual(connection.execute("SELECT COUNT(*) FROM experiments").fetchone()[0], 0)

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
        payload = {"schema":"crack_assessment/v1","owner":"main:board/test","function":"Owner","candidate_source_sha256":sha(self.candidate),"target_object_sha256":"b"*64,"candidate_object_sha256":"0"*64,"owner_gain":1,"data_gain":0,"data_diff_delta":0}
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
        self.assertIn("return 1", self.source.read_text())
        self.assertIn("discard", result["receipts"])
        self.assertNotIn("record", result["receipts"])

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
        target = adapter / "target.o"; target.write_bytes(b"target-object")
        candidate = adapter / "candidate.o"; candidate.write_bytes(b"candidate-object")
        arguments = dict(owner="main:board/test", function=FUNCTION, candidate_source=self.candidate, candidate_source_sha256=sha(self.candidate), approved_target_object_sha256=sha(target), target_object=target, candidate_object=candidate, baseline_strict_report=paths["baseline-strict"], baseline_data_report=paths["baseline-data"], candidate_strict_report=paths["candidate-strict"], candidate_data_report=paths["candidate-data"], physical_receipt=physical_path)
        strict = harness._proof_adapter_payload(kind="strict", **arguments)
        assess = harness._proof_adapter_payload(kind="assess", **arguments)
        physical_proof = harness._proof_adapter_payload(kind="physical", **arguments)
        sibling_proof = harness._proof_adapter_payload(kind="siblings", **arguments)
        self.assertEqual(strict["schema"], "crack_proof_strict/v1")
        self.assertEqual(strict["strict_percent"], 100.0)
        self.assertEqual(assess["owner_gain"], 25.0)
        self.assertEqual(assess["data_gain"], 25.0)
        self.assertLessEqual(assess["data_diff_delta"], 0)
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
        self.assertTrue((self.state / "RECOVERY_REQUIRED.json").is_file())

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

    def test_improved_success_survives_post_terminal_cleanup_failure(self) -> None:
        with patch.object(
            harness, "_cleanup_raw", side_effect=OSError("cleanup improved failed")
        ):
            result = self.execute(gain=2, focus_rows=1)
        self.assertEqual(result["status"], "improved")
        self.assertEqual(result["cleanup_status"], "cleanup_incomplete")
        self.assertIn("return 2", self.source.read_text())
        self.assertTrue(any("cleanup improved failed" in item for item in result["cleanup_errors"]))
        self.assertFalse(any(self.state.glob("owners/*/*/latest-failure.json")))

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
        status = harness._status_for_test(self.root, state_root=self.state)
        retained = status["results"][0]
        self.assertEqual(retained["status"], "exact")
        self.assertEqual(retained["cleanup_status"], "complete")
        self.assertTrue(any("cleanup retry pending" in item for item in retained["cleanup_errors"]))
        self.assertIn("return 2", self.source.read_text())
        self.assertFalse(any(self.state.glob("owners/*/*/latest/temp")))

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
        retried = harness._status_for_test(self.root, state_root=self.state)["results"][0]
        self.assertEqual(retried["status"], "exact")
        self.assertEqual(retried["cleanup_status"], "complete")

    def test_improved_success_survives_global_gc_exception(self) -> None:
        with patch.object(
            harness, "_gc_global", side_effect=[None, OSError("global gc failed")]
        ):
            result = self.execute(gain=2, focus_rows=1)
        self.assertEqual(result["status"], "improved")
        self.assertEqual(result["cleanup_status"], "cleanup_incomplete")
        self.assertTrue(any("global gc failed" in item for item in result["cleanup_errors"]))
        self.assertIn("return 2", self.source.read_text())
        self.assertFalse(any(self.state.glob("owners/*/*/latest-failure.json")))
        retried = harness._status_for_test(self.root, state_root=self.state)["results"][0]
        self.assertEqual(retried["status"], "improved")
        self.assertEqual(retried["cleanup_status"], "complete")

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

    def test_signed_permit_is_one_shot_without_consuming_function(self) -> None:
        approval, _ = self.write_inputs()
        loaded = harness.load_approval(self.root, approval)
        run_dir = harness._run_dir(self.state, loaded)
        harness._consume_permit(run_dir, loaded)
        self.assertTrue(harness._permit_attempted(run_dir, loaded))
        self.assertFalse(harness._function_consumed(run_dir, loaded))
        with self.assertRaisesRegex(harness.CrackHarnessError, "already been attempted"):
            harness._consume_permit(run_dir, loaded)

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
                raise harness.CrackHarnessError("candidate compiler failure")
            return original(*args, **kwargs)

        with patch.object(harness, "_run_command", side_effect=fail_candidate):
            result = harness._run_approved_for_test(
                self.root, approval, permit_path=permit,
                state_root=self.state, manager_key_path=self.manager_key,
            )
        self.assertEqual(result["status"], "failed")
        self.assertTrue(harness._function_consumed(run_dir, loaded))

    def test_second_candidate_for_function_is_rejected_across_campaign_ids(self) -> None:
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
        body = {"schema":"crack_harness_attempt/v1","run_dir":str(temp.parent),"source_path":str(self.source),"approval_path":str(approval),"approval_sha256":sha(approval),"disposable_paths":[str(self.base),str(self.candidate),str(permit),str(approval)]}
        (self.state / "attempt.json").write_text(json.dumps(body | {"attempt_sha256":harness._digest_json(body)}), encoding="utf-8")
        harness._scavenge_disposable_worktrees(self.root, self.state)
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
