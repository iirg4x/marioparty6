"""Pure public evidence tests; optional loopback replay uses no model/retail."""
import copy
import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import threading
import unittest
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer

from tools import recovery_causal_groups as groups
from tools.tests.test_recovery_causal_groups import report, row


def packet():
    rows = [row("li r3, 0", 0), row("bl memcpy", 4), row("blr", 8)]
    return groups.decision_packet(report(rows, rows), "f", "void f(void) { memcpy(a,b,n); }\n",
                                  1, 1, "Is a loop around memcpy present?", 0, 0)


def answer(p):
    return {"status": "supported", "function": "f", "packet_sha256": p["packet_sha256"],
            "answer": "One call and a return, with no loop branch in the supplied function.",
            "evidence_rows": [1, 2], "missing_evidence": None}


class DecisionTests(unittest.TestCase):
    def test_prompt_preserves_ppc_return_identity_and_does_not_invent_arguments(self):
        p = packet()
        before = copy.deepcopy(p)
        prompt = groups.render_decision_prompt(p)
        self.assertIn("subf d,a,b computes b-a", prompt)
        self.assertIn("r3 after a pointer-returning call is not the old r3 input", prompt)
        self.assertIn("does not by itself prove an extra call argument", prompt)
        self.assertIn("Use supplied source/callee signatures", prompt)
        self.assertEqual(p, before)
        self.assertFalse(p["authority_advanced"])
        self.assertNotIn("max_tokens", prompt)

    def test_complete_control_census_outside_excerpt_prevents_hidden_loop_assumption(self):
        p = packet()
        self.assertEqual([r["row"] for r in p["paired_rows"]], [0])
        self.assertEqual(p["machine_census"]["target"]["calls"], [{"row": 1, "instruction": "bl memcpy"}])
        self.assertEqual(p["machine_census"]["target"]["branches_including_return"], [{"row": 2, "instruction": "blr"}])
        self.assertEqual(p["omitted_aligned_rows"], 2)
        self.assertEqual(groups.validate_decision_answer(p, answer(p))["status"], "valid_finding")
        self.assertIn("Do not solve the entire function", groups.render_decision_prompt(p))
        self.assertNotIn("max_tokens", groups.render_decision_prompt(p))

    def test_rejects_patch_response_and_uncited_or_stale_finding(self):
        p = packet()
        for mutate in (lambda a: a.update(replacements=[]), lambda a: a.update(evidence_rows=[900]),
                       lambda a: a.update(evidence_rows=[]), lambda a: a.update(evidence_rows=[1,1]),
                       lambda a: a.update(evidence_rows=[True]), lambda a: a.update(function="other"),
                       lambda a: a.update(packet_sha256="0"*64), lambda a: a.update(status="exact")):
            a = answer(p)
            mutate(a)
            with self.assertRaises(ValueError):
                groups.validate_decision_answer(p, a)
        changed = copy.deepcopy(p)
        changed["source_excerpt"] += " changed"
        with self.assertRaisesRegex(ValueError, "changed decision"):
            groups.render_decision_prompt(changed)

    def test_insufficient_is_useful_terminal_not_forced_speculation(self):
        p = packet()
        a = answer(p)
        a.update(status="insufficient", evidence_rows=[], missing_evidence="Pointee alias identities are not known.")
        self.assertEqual(groups.validate_decision_answer(p, a)["status"], "insufficient_evidence")
        a["missing_evidence"] = None
        with self.assertRaises(ValueError):
            groups.validate_decision_answer(p, a)

    def test_packet_bounds(self):
        doc = report([row("blr",0)], [row("blr",0)])
        for question, first, last in (("",0,0), ("q",-1,0), ("q",0,1)):
            with self.assertRaises(ValueError):
                groups.decision_packet(doc, "f", "source", 1, 1, question, first, last)
        with self.assertRaisesRegex(ValueError, "byte budget"):
            groups.decision_packet(doc, "f", "x"*10000, 1, 1, "q", 0, 0, max_bytes=1000)


@unittest.skipUnless(os.environ.get("MP6_QWEN_SUPPORT_ROOT") and shutil.which("powershell"),
                     "explicit installed local runner replay not selected")
class RunnerReplayTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.packet = packet()
        self.packet_path = self.root / "decision.json"
        self.packet_path.write_text(json.dumps(self.packet), encoding="utf-8")
        self.prompt = self.root / "prompt.txt"
        self.prompt.write_text(groups.render_decision_prompt(self.packet), encoding="utf-8")
        self.body = answer(self.packet)
        self.finish = "stop"
        self.calls = []
        self.slot_count = 4
        self.concurrency_barrier = None
        self.after_reasoning = None
        self.active = self.peak_active = 0
        self.active_lock = threading.Lock()
        owner = self

        class Handler(BaseHTTPRequestHandler):
            def log_message(self, *args):
                pass

            def do_GET(self):
                self.send_response(200)
                self.send_header("Content-Type", "application/json")
                self.end_headers()
                self.wfile.write(json.dumps([{"id":i,"n_ctx":65536,"is_processing":False}
                                            for i in range(owner.slot_count)]).encode())

            def do_POST(self):
                request_body = json.loads(self.rfile.read(int(self.headers["Content-Length"])))
                owner.calls.append(request_body)
                with owner.active_lock:
                    owner.active += 1
                    owner.peak_active = max(owner.peak_active, owner.active)
                if owner.concurrency_barrier:
                    owner.concurrency_barrier.wait(timeout=20)
                self.send_response(200)
                self.send_header("Content-Type", "text/event-stream")
                self.end_headers()
                events = [
                    {"choices":[{"delta":{"reasoning_content":"discard me"},"finish_reason":None}]},
                    {"choices":[{"delta":{"content":json.dumps(owner.body)},"finish_reason":None}]},
                    {"choices":[{"delta":{},"finish_reason":owner.finish}],
                     "usage":{"prompt_tokens":10,"completion_tokens":20,"total_tokens":30}}]
                try:
                    for index, event in enumerate(events):
                        self.wfile.write(("data: "+json.dumps(event)+"\n\n").encode())
                        self.wfile.flush()
                        if index == 0 and owner.after_reasoning:
                            owner.after_reasoning(request_body)
                    self.wfile.write(b"data: [DONE]\n\n")
                except (BrokenPipeError, ConnectionResetError, ConnectionAbortedError):
                    pass
                finally:
                    with owner.active_lock:
                        owner.active -= 1

        self.server = ThreadingHTTPServer(("127.0.0.1",0), Handler)
        self.thread = threading.Thread(target=self.server.serve_forever, daemon=True)
        self.thread.start()
        self.addCleanup(self.server.server_close)
        self.addCleanup(self.server.shutdown)

    def invoke(self):
        return subprocess.run(["powershell", "-NoProfile", "-File",
            str(Path(os.environ["MP6_QWEN_SUPPORT_ROOT"])/"run-job.ps1"),
            "-PromptFile", str(self.prompt), "-OutputDirectory", str(self.root/"answer"),
            "-JobId", "replay", "-Backend", "llama-cpp", "-DecisionPacket", str(self.packet_path),
            "-ValidatorScript", str(Path(groups.__file__).resolve()),
            "-Endpoint", f"http://127.0.0.1:{self.server.server_port}/v1/chat/completions"],
            capture_output=True, text=True, timeout=30)

    def test_valid_natural_completion_and_immutable_completed_answer(self):
        result = self.invoke()
        self.assertEqual(result.returncode, 0, result.stderr)
        out = self.root/"answer"
        self.assertTrue((out/"replay.answer.txt").exists())
        before = (out/"replay.metrics.json").read_bytes()
        metrics = json.loads(before)
        self.assertEqual(metrics["state"], "completed")
        self.assertEqual(metrics["validation_status"], "valid_finding")
        self.assertEqual(metrics["think"], "xhigh")
        self.assertEqual(metrics["num_predict"], -1)
        self.assertNotIn("max_tokens", self.calls[0])
        self.assertEqual(self.calls[0]["reasoning_effort"], "xhigh")
        self.assertNotEqual(self.invoke().returncode, 0)
        self.assertEqual((out/"replay.metrics.json").read_bytes(), before)
        self.assertEqual(len(self.calls), 1)
        self.assertFalse((out/"replay.lock").exists())

    def test_invalid_row_rejected_without_admitted_answer(self):
        self.body["evidence_rows"] = [999]
        self.assertNotEqual(self.invoke().returncode, 0)
        out = self.root/"answer"
        self.assertFalse((out/"replay.answer.txt").exists())
        self.assertTrue((out/"replay.rejected-answer.txt").exists())
        self.assertEqual(json.loads((out/"replay.metrics.json").read_bytes())["state"], "rejected")
        self.assertFalse((out/"replay.lock").exists())

    def test_changed_prompt_rejected_before_http(self):
        self.prompt.write_text("unbound replacement prompt", encoding="utf-8")
        self.assertNotEqual(self.invoke().returncode, 0)
        self.assertEqual(self.calls, [])

    def test_truncated_finish_not_completed(self):
        self.finish = "length"
        self.assertNotEqual(self.invoke().returncode, 0)
        out = self.root/"answer"
        self.assertFalse((out/"replay.answer.txt").exists())
        self.assertEqual(json.loads((out/"replay.metrics.json").read_bytes())["state"], "failed")

    def test_caller_cancellation_before_http_has_no_admitted_answer(self):
        out = self.root / "answer"
        out.mkdir()
        (out / "replay.cancel").write_text("Owner already closed", encoding="utf-8")
        result = self.invoke()
        self.assertNotEqual(result.returncode, 0)
        self.assertEqual(self.calls, [])
        metrics = json.loads((out / "replay.metrics.json").read_bytes())
        self.assertEqual(metrics["state"], "cancelled")
        self.assertFalse(metrics["natural_completion"])
        self.assertFalse((out / "replay.answer.txt").exists())
        self.assertFalse((out / "replay.lock").exists())

    def test_cancellation_does_not_overwrite_completed_finding(self):
        self.assertEqual(self.invoke().returncode, 0)
        out = self.root / "answer"
        before = (out / "replay.metrics.json").read_bytes()
        (out / "replay.cancel").write_text("No more work needed", encoding="utf-8")
        self.assertNotEqual(self.invoke().returncode, 0)
        self.assertEqual((out / "replay.metrics.json").read_bytes(), before)
        self.assertEqual(len(self.calls), 1)

    def test_cancel_one_streaming_question_preserves_other_batch_job(self):
        jobs = []
        for name in ("obsolete", "useful"):
            prompt = self.root / (name + ".txt")
            prompt.write_text(name, encoding="utf-8")
            jobs.append({"id":name, "prompt_file":str(prompt),
                         "output_dir":str(self.root/name)})
        def cancel_obsolete(body):
            if body["messages"][0]["content"] == "obsolete":
                (self.root / "obsolete/obsolete.cancel").write_text("Resolved locally", encoding="utf-8")
        self.after_reasoning = cancel_obsolete
        result = self.batch(jobs, endpoint=f"http://127.0.0.1:{self.server.server_port}/v1/chat/completions")
        self.assertNotEqual(result.returncode, 0)
        states = {j["id"]:j["state"] for j in json.loads((self.root/"jobs.json.status.json").read_bytes())["jobs"]}
        self.assertEqual(states, {"obsolete":"cancelled", "useful":"completed"})
        self.assertFalse((self.root/"obsolete/obsolete.answer.txt").exists())
        self.assertFalse((self.root/"obsolete/obsolete.lock").exists())
        self.assertTrue((self.root/"useful/useful.answer.txt").exists())
        self.assertEqual(len(self.calls), 2)

    def batch(self, jobs, **fields):
        path = self.root/"jobs.json"
        path.write_text(json.dumps({"backend":"llama-cpp", "parallelism":2, "jobs":jobs, **fields}), encoding="utf-8")
        return subprocess.run(["powershell", "-NoProfile", "-File",
            str(Path(os.environ["MP6_QWEN_SUPPORT_ROOT"])/"run-batch.ps1"), "-ManifestFile", str(path)],
            capture_output=True, text=True, timeout=30)

    def job(self, name="replay"):
        return {"id":name,"prompt_file":str(self.prompt),"output_dir":str(self.root/"answer"),
                "decision_packet":str(self.packet_path),"validator_script":str(Path(groups.__file__).resolve())}

    def test_batch_reuses_bound_completed_answer_without_http(self):
        self.assertEqual(self.invoke().returncode, 0)
        metrics = (self.root/"answer/replay.metrics.json").read_bytes()
        result = self.batch([self.job()])
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("no inference", result.stdout)
        self.assertEqual(len(self.calls), 1)
        self.assertEqual((self.root/"answer/replay.metrics.json").read_bytes(), metrics)
        self.prompt.write_text("changed", encoding="utf-8")
        self.assertNotEqual(self.batch([self.job()]).returncode, 0)
        self.assertEqual(len(self.calls), 1)

    def test_duplicate_prompts_and_fake_ollama_parallelism_fail_before_dispatch(self):
        result = self.batch([self.job(), self.job("duplicate")])
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("Duplicate prompt", result.stderr)
        self.assertNotEqual(self.batch([self.job()], backend="ollama").returncode, 0)
        self.assertEqual(self.calls, [])

    def test_four_real_concurrent_requests_and_no_generation_cap(self):
        self.concurrency_barrier = threading.Barrier(4)
        jobs = []
        for i in range(4):
            prompt = self.root / f"plain-{i}.txt"
            prompt.write_text(f"Independent recovery question {i}", encoding="utf-8")
            jobs.append({"id":f"worker-{i}","prompt_file":str(prompt),
                         "output_dir":str(self.root/f"out-{i}")})
        result = self.batch(jobs, parallelism=4,
                            endpoint=f"http://127.0.0.1:{self.server.server_port}/v1/chat/completions")
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(self.peak_active, 4)
        self.assertEqual(len(self.calls), 4)
        for call in self.calls:
            self.assertEqual(call["reasoning_effort"], "xhigh")
            self.assertNotIn("max_tokens", call)
        self.assertEqual({j["state"] for j in json.loads((self.root/"jobs.json.status.json").read_bytes())["jobs"]}, {"completed"})

    def test_more_workers_than_backend_slots_rejected_without_inference(self):
        self.slot_count = 2
        result = self.batch([self.job()], parallelism=4,
                            endpoint=f"http://127.0.0.1:{self.server.server_port}/v1/chat/completions")
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("requested real slots", result.stderr)
        self.assertEqual(self.calls, [])


if __name__ == "__main__":
    unittest.main()
