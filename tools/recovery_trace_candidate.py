#!/usr/bin/env python3
"""Compile, capture and measure one hash-bound recovery candidate once.

The native wrapper owns the compiler/debugger lifecycle.  This coordinator only
builds a disposable request, authenticates the returned capture, and passes its
actual object to the existing evaluator; it never edits source or authority.
"""
from __future__ import annotations

import argparse
import copy
import hashlib
import json
import os
from collections.abc import Mapping, Sequence
from pathlib import Path
import re
import subprocess
import sys
from typing import Any, Callable

if __package__ in {None, ""}:
    sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

from tools import bounded_process
from tools import compile_recovery_candidate as compiler
from tools import recovery_evaluate
from tools import recovery_frontier as frontier
from tools import recovery_temp_epochs


SCHEMA = "recovery_trace_candidate/v1"
MAX_SOURCE_BYTES = 4 * 1024 * 1024
MAX_OBJECT_BYTES = 16 * 1024 * 1024
MAX_CAPTURE_OUTPUT = 4 * 1024 * 1024
MAX_RESULT_BYTES = 1024 * 1024
MAX_VREGS = 8
_SHA256 = re.compile(r"[0-9a-f]{64}\Z")
_C_IDENTIFIER = re.compile(r"[A-Za-z_][A-Za-z0-9_]*\Z")


class TraceError(ValueError):
    """The request or native evidence is not safely bound."""


def _sha(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def _path(value: Any, base: Path, label: str) -> Path:
    if not isinstance(value, str) or not value.strip() or "\x00" in value:
        raise TraceError(f"{label} path is invalid")
    raw = Path(value.strip())
    return (raw if raw.is_absolute() else base / raw).resolve()


def _file(path: Path, label: str, *, maximum: int | None = None) -> Path:
    path = path.resolve()
    if not path.is_file():
        raise TraceError(f"{label} is not a file: {path}")
    if maximum is not None and path.stat().st_size > maximum:
        raise TraceError(f"{label} exceeds {maximum} bytes")
    return path


def _descriptor(path: Path) -> dict[str, Any]:
    path = path.resolve()
    return {"path": str(path), "sha256": compiler.digest(path), "size_bytes": path.stat().st_size}


def _declared_descriptor(raw: Any, base: Path, label: str) -> tuple[Path, str, int | None]:
    if not isinstance(raw, Mapping):
        raise TraceError(f"{label} must be an object")
    path = _path(raw.get("path"), base, f"{label}.path")
    digest = raw.get("sha256")
    if not isinstance(digest, str) or _SHA256.fullmatch(digest) is None:
        raise TraceError(f"{label}.sha256 must be a lowercase SHA-256")
    size = raw.get("size", raw.get("size_bytes"))
    if size is not None and (isinstance(size, bool) or not isinstance(size, int) or size < 0):
        raise TraceError(f"{label}.size must be a non-negative integer")
    return path, digest, size


def _check_descriptor(raw: Any, expected: Path, base: Path, label: str,
                      *, maximum: int | None = None) -> dict[str, Any]:
    path, digest, size = _declared_descriptor(raw, base, label)
    expected = expected.resolve()
    if path != expected:
        raise TraceError(f"{label}.path is not bound to {expected}")
    _file(expected, label, maximum=maximum)
    actual = _descriptor(expected)
    if digest != actual["sha256"] or (size is not None and size != actual["size_bytes"]):
        raise TraceError(f"{label} bytes drifted")
    return actual


def _read_json(root: Path, path: Path, limit: int) -> tuple[dict[str, Any], dict[str, Any]]:
    raw, binding = frontier.read_bound(root, path, limit)
    value = frontier.load_json(raw)
    if not isinstance(value, dict):
        raise TraceError(f"JSON document is not an object: {path}")
    return value, binding


def _index(root: Path, index: Path) -> tuple[dict[str, Any], dict[str, Any], Path, Path, dict[str, Any]]:
    index = frontier.local(root, index)
    raw, binding = frontier.read_bound(root, index, frontier.INDEX_LIMIT)
    value = frontier.load_json(raw)
    if not isinstance(value, dict):
        raise TraceError("current index is not an object")
    frontier.verify(root, value)
    inputs = value.get("inputs")
    if not isinstance(inputs, dict):
        raise TraceError("current index lacks bound inputs")
    source_desc = inputs.get("source")
    baseline_desc = inputs.get("candidate_object")
    if not isinstance(source_desc, dict) or not isinstance(baseline_desc, dict):
        raise TraceError("current index lacks source or baseline object")
    source = frontier.local(root, Path(source_desc["path"]))
    baseline = frontier.local(root, Path(baseline_desc["path"]))
    return value, binding, source, baseline, dict(source_desc)


def _span(template: Mapping[str, Any], source_size: int) -> tuple[int, int]:
    raw = None
    for key in ("function_source_span", "function_span", "span"):
        if isinstance(template.get(key), Mapping):
            raw = template[key]
            break
    if raw is None and isinstance(template.get("function"), Mapping):
        raw = template["function"].get("span")
    if not isinstance(raw, Mapping):
        raise TraceError("request template lacks a byte function span")
    start, end = raw.get("start_byte", raw.get("start")), raw.get("end_byte", raw.get("end"))
    if any(isinstance(v, bool) or not isinstance(v, int) for v in (start, end)):
        raise TraceError("function span offsets must be integers")
    if not 0 <= start < end <= source_size:
        raise TraceError("function span is outside the baseline source")
    return start, end


def _without_trailing_crlf(value: bytes) -> bytes:
    """Normalize only EOF CR/LF for support-prelude comparison."""

    return value.rstrip(b"\r\n")


def _argv(template: Mapping[str, Any], root: Path, source: Path, output: Path) -> tuple[list[str], Path]:
    value = template.get("argv")
    if (not isinstance(value, list) or not value or
            any(not isinstance(item, str) or not item or "\x00" in item for item in value)):
        raise TraceError("request argv must be a nonempty string list")
    argv = list(value)
    compiler_path, _, _ = _declared_descriptor(template.get("compiler"), root, "request.compiler")
    if len(argv) < 2 or _path(argv[1], root, "request argv compiler") != compiler_path:
        raise TraceError("request argv compiler is not bound to request.compiler")
    c_positions = [i for i, item in enumerate(argv) if item == "-c"]
    o_positions = [i for i, item in enumerate(argv) if item == "-o"]
    if len(c_positions) != 1 or len(o_positions) != 1:
        raise TraceError("request argv must contain one -c and one -o")
    ci, oi = c_positions[0], o_positions[0]
    if ci + 1 >= len(argv) or _path(argv[ci + 1], root, "request argv source") != source:
        raise TraceError("request argv source is not bound to request.source")
    if oi + 1 >= len(argv):
        raise TraceError("request argv -o is missing a path")
    argv[ci + 1] = str(source)
    argv[oi + 1] = str(output)
    return argv, compiler_path


def _prepare_request(root: Path, template_path: Path, candidate: Path, output: Path,
                     function: str, definition_prefix: str, index_source: Path,
                     index_source_desc: Mapping[str, Any], support_prelude: Path | None = None
                     ) -> tuple[dict[str, Any], dict[str, Any], Path, str, dict[str, Any] | None]:
    template, _ = _read_json(root, frontier.local(root, template_path), MAX_CAPTURE_OUTPUT)
    if template.get("schema") != "mwcc_capsule_same_session_capture_request/v1":
        raise TraceError("request template schema is not the native capture request")
    if not _C_IDENTIFIER.fullmatch(function) or template.get("function") != function:
        raise TraceError("request function is not the requested C identifier")
    if not isinstance(definition_prefix, str) or not definition_prefix or "\x00" in definition_prefix:
        raise TraceError("definition prefix is invalid")
    try:
        prefix = (definition_prefix + "\n{").encode("ascii")
    except UnicodeEncodeError as exc:
        raise TraceError("definition prefix must be ASCII") from exc
    source_declared, source_hash, source_size = _declared_descriptor(template.get("source"), root, "request.source")
    source_actual = _check_descriptor(template.get("source"), source_declared, root, "request.source",
                                      maximum=MAX_SOURCE_BYTES)
    if (source_hash != index_source_desc.get("sha256") or source_actual["sha256"] != index_source_desc.get("sha256") or
            source_actual["size_bytes"] != index_source_desc.get("size_bytes") or
            (source_size is not None and source_size != source_actual["size_bytes"])):
        raise TraceError("request template is not bound to the current index source")
    compiler_path, _, _ = _declared_descriptor(template.get("compiler"), root, "request.compiler")
    compiler_path = _file(compiler_path, "request compiler")
    compiler_actual = _check_descriptor(template.get("compiler"), compiler_path, root, "request.compiler")
    wrapper_raw = template.get("wrapper")
    wrapper_path = _path(template["argv"][0], root, "request argv wrapper") if isinstance(template.get("argv"), list) and template["argv"] else None
    if wrapper_raw is not None:
        if wrapper_path is None:
            raise TraceError("request wrapper is missing argv")
        _check_descriptor(wrapper_raw, wrapper_path, root, "request.wrapper")
    cwd = _path(template.get("cwd"), root, "request.cwd")
    if cwd != root.resolve():
        raise TraceError("request cwd is not the supplied repository root")
    base_bytes = index_source.read_bytes()
    start, end = _span(template, len(base_bytes))
    base_span = base_bytes[start:end]
    if not base_span.startswith(prefix) or not base_span.endswith(b"}"):
        raise TraceError("template function span has the wrong definition boundary")
    template_hash = template.get("function_sha256")
    if template_hash is not None and template_hash != _sha(base_span):
        raise TraceError("template function hash does not bind its span")
    candidate_bytes = candidate.read_bytes()
    prelude_binding = None
    candidate_start = start
    inserted_prelude = b""
    if support_prelude is not None:
        prelude = frontier.local(root, support_prelude)
        prelude_bytes, prelude_binding = frontier.read_bound(root, prelude, MAX_SOURCE_BYTES)
        if not prelude_bytes:
            raise TraceError("support prelude is empty")
        baseline_prefix = base_bytes[:start]
        if candidate_bytes[:start] != baseline_prefix:
            raise TraceError("candidate support prelude or prefix drifted")
        candidate_start = candidate_bytes.find(prefix, start)
        if candidate_start < start:
            raise TraceError("candidate function definition prefix is missing")
        inserted_prelude = candidate_bytes[start:candidate_start]
        if (not inserted_prelude
                or _without_trailing_crlf(inserted_prelude) != _without_trailing_crlf(prelude_bytes)):
            raise TraceError("candidate support prelude or prefix drifted")
        prelude_binding = dict(prelude_binding)
        prelude_binding.update({
            "actual_insertion_sha256": _sha(inserted_prelude),
            "actual_insertion_size_bytes": len(inserted_prelude),
            "trailing_crlf_normalized": inserted_prelude != prelude_bytes,
        })
    candidate_end = end + len(candidate_bytes) - len(base_bytes)
    if not start < candidate_end <= len(candidate_bytes):
        raise TraceError("candidate function span is outside the candidate source")
    if (candidate_bytes[:candidate_start] != base_bytes[:start] + inserted_prelude
            or candidate_bytes[candidate_end:] != base_bytes[end:]):
        raise TraceError("candidate changes bytes outside the selected function span")
    candidate_span = candidate_bytes[candidate_start:candidate_end]
    if not candidate_span.startswith(prefix) or not candidate_span.endswith(b"}"):
        raise TraceError("candidate function definition boundary differs")
    request = copy.deepcopy(template)
    request["function"] = function
    request["function_sha256"] = _sha(candidate_span)
    request["function_source_span"] = {"start_byte": candidate_start, "end_byte": candidate_end}
    source = dict(request["source"])
    source.update(path=str(candidate.resolve()), sha256=_sha(candidate_bytes), size=len(candidate_bytes))
    if "size_bytes" in source:
        source["size_bytes"] = len(candidate_bytes)
    request["source"] = source
    argv, compiler_path = _argv(request, root, source_declared, output)
    argv[argv.index("-c") + 1] = str(candidate.resolve())
    request["argv"] = argv
    request["cwd"] = str(root.resolve())
    compiler_actual["path"] = str(compiler_path)
    return request, compiler_actual, compiler_path, _sha(candidate_bytes), prelude_binding


def _json_stdout(raw: bytes) -> dict[str, Any]:
    if len(raw) > MAX_CAPTURE_OUTPUT:
        raise TraceError("capture wrapper output exceeds bound")
    try:
        value = json.loads(raw.decode("utf-8"))
    except (UnicodeDecodeError, json.JSONDecodeError):
        value = None
        for line in reversed(raw.decode("utf-8", "ignore").splitlines()):
            try:
                value = json.loads(line)
                break
            except json.JSONDecodeError:
                continue
    if not isinstance(value, dict):
        raise TraceError("capture wrapper did not return a JSON summary")
    return value


def run_capture(capture_script: Path, *, root: Path, request: Path, source_sha256: str,
                baseline: Path, baseline_sha256: str, output_prefix: Path,
                function: str, function_sha256: str, definition_prefix: str,
                timeout: float = 35.0) -> subprocess.CompletedProcess[bytes]:
    capture_script = _file(capture_script, "capture script")
    command = [str(capture_script)]
    if capture_script.suffix.lower() in {".py", ".pyw"}:
        command = [sys.executable, str(capture_script)]
    command += [
        "--request", str(request), "--source-sha256", source_sha256,
        "--baseline", str(baseline), "--baseline-sha256", baseline_sha256,
        "--output-prefix", str(output_prefix), "--function", function,
        "--function-sha256", function_sha256, "--definition-prefix", definition_prefix,
        "--timeout", "30",
    ]
    return bounded_process.run(command, cwd=root, timeout=timeout, max_output=MAX_CAPTURE_OUTPUT)


def _validate_capture(root: Path, capture: Path, output_object: Path, request: Path,
                      summary: Mapping[str, Any], candidate: Path, compiler_path: Path,
                      baseline: Path, source_sha256: str, compiler_desc: Mapping[str, Any],
                      baseline_sha256: str) -> tuple[dict[str, Any], dict[str, Any]]:
    if (summary.get("status") != "CAPTURED" or type(summary.get("exit_code")) is not int
            or summary.get("exit_code") != 0):
        raise TraceError("native capture did not complete with exit_code 0/CAPTURED")
    if summary.get("cleanup_errors") != []:
        raise TraceError("native capture reported cleanup errors")
    if _path(summary.get("output"), root, "capture summary output") != capture.resolve():
        raise TraceError("capture summary output path drifted")
    document, binding = _read_json(root, capture, recovery_temp_epochs.MAX_CAPTURE_BYTES)
    if document.get("schema") != recovery_temp_epochs.CAPTURE_SCHEMA or document.get("status") != "CAPTURED":
        raise TraceError("capture document is not CAPTURED")
    if document.get("diagnostic_only") is not True or document.get("authority_advanced") is not False:
        raise TraceError("capture is not diagnostic-only")
    events = document.get("events")
    if (not isinstance(events, list) or not events or document.get("event_count") != len(events) or
            not isinstance(document.get("capture_end_reason"), str) or not document["capture_end_reason"].strip()):
        raise TraceError("capture event stream is missing or incomplete")
    _check_descriptor(document.get("source"), candidate, root, "capture.source", maximum=MAX_SOURCE_BYTES)
    capture_compiler = _check_descriptor(document.get("compiler"), compiler_path, root, "capture.compiler")
    if capture_compiler != dict(compiler_desc):
        raise TraceError("capture compiler differs from the request compiler")
    capture_baseline = _check_descriptor(document.get("baseline_object"), baseline, root, "capture.baseline_object",
                                         maximum=MAX_OBJECT_BYTES)
    if capture_baseline["sha256"] != baseline_sha256:
        raise TraceError("capture baseline differs from the current index")
    output_desc = _check_descriptor(document.get("output_object"), output_object, root, "capture.output_object",
                                    maximum=MAX_OBJECT_BYTES)
    request_raw = request.read_bytes()
    request_descriptor = document.get("request")
    if not isinstance(request_descriptor, Mapping):
        raise TraceError("capture request binding is missing")
    _check_descriptor(request_descriptor, request, root, "capture.request")
    if request_descriptor.get("sha256") != _sha(request_raw):
        raise TraceError("capture request hash differs from generated request")
    launch = document.get("launch")
    if launch is not None and not isinstance(launch, Mapping):
        raise TraceError("capture launch binding is malformed")
    if isinstance(launch, Mapping):
        if _path(launch.get("cwd"), root, "capture launch cwd") != root.resolve():
            raise TraceError("capture launch cwd drifted")
        launch_object = launch.get("output_object")
        if launch_object is not None and _path(launch_object, root, "capture launch output") != output_object.resolve():
            raise TraceError("capture launch output drifted")
        launch_baseline = launch.get("baseline_object")
        if launch_baseline is not None and _path(launch_baseline, root, "capture launch baseline") != baseline.resolve():
            raise TraceError("capture launch baseline drifted")
        launch_compiler = launch.get("compiler")
        if launch_compiler is not None and _check_descriptor(launch_compiler, compiler_path, root,
                                                              "capture launch compiler") != dict(compiler_desc):
            raise TraceError("capture launch compiler drifted")
        launch_request = launch.get("request_object")
        if launch_request is not None and _path(launch_request, root, "capture launch request object") != output_object.resolve():
            raise TraceError("capture launch request object drifted")
    request_argv = document.get("request_argv")
    if request_argv is None and isinstance(launch, Mapping):
        request_argv = launch.get("request_argv")
    if (not isinstance(request_argv, list) or any(not isinstance(v, str) for v in request_argv)):
        raise TraceError("capture request_argv is malformed")
    if str(candidate.resolve()) not in [str(Path(v).resolve()) for v in request_argv if v]:
        raise TraceError("capture request_argv does not bind candidate source")
    if not any(Path(v).resolve() == compiler_path for v in request_argv if v):
        raise TraceError("capture request_argv does not bind compiler")
    comparison = document.get("object_comparison")
    if not isinstance(comparison, Mapping) or comparison.get("byte_compare_only") is not True or comparison.get("objdiff_or_link_exactness_proven") is not False:
        raise TraceError("capture object comparison is not byte-only")
    baseline_equal = compiler.digest(output_object) == compiler.digest(baseline)
    claimed_equal = comparison.get("status") == "byte_identical"
    if comparison.get("status") not in {"byte_identical", "different"} or claimed_equal != baseline_equal:
        raise TraceError("capture object comparison disagrees with actual bytes")
    if not isinstance(summary.get("baseline_equal"), bool) or summary["baseline_equal"] != baseline_equal:
        raise TraceError("capture summary baseline equality disagrees with actual bytes")
    if document.get("source", {}).get("sha256") != source_sha256:
        raise TraceError("capture source hash differs from request")
    # Run the authoritative schema/event checks once more through the shared analyzer.
    recovery_temp_epochs._validate_capture(document)
    return document, {"capture": binding, "output_object": output_desc, "baseline_equal": baseline_equal}


def _write(path: Path, value: Mapping[str, Any]) -> None:
    raw = frontier.canonical(value) + b"\n"
    if len(raw) > MAX_RESULT_BYTES:
        raise TraceError("trace result exceeds output bound")
    compiler.atomic(path, raw)


def _out_dir(root: Path, value: Path) -> Path:
    path = frontier.local(root, value)
    try:
        path.relative_to(root / "build")
    except ValueError as exc:
        raise TraceError("output directory must be under repository build/") from exc
    if path == root / "build":
        raise TraceError("output directory must be a private build subdirectory")
    if path.exists() and (not path.is_dir() or any(path.iterdir())):
        raise TraceError("output directory is not empty")
    path.mkdir(parents=True, exist_ok=True)
    return path


def trace_candidate(*, root: Path, index: Path, request_template: Path, capture_script: Path,
                    candidate: Path, function: str, definition_prefix: str, vregs: Sequence[int],
                    out_dir: Path, objdiff: Path, readelf: Path,
                    support_prelude: Path | None = None,
                    keep_reports: bool = False,
                    capture_runner: Callable[..., subprocess.CompletedProcess[bytes]] | None = None,
                    evaluator: Callable[..., dict[str, Any]] | None = None) -> dict[str, Any]:
    root = Path(os.path.abspath(root)).resolve()
    out = _out_dir(root, out_dir)
    if isinstance(vregs, (str, bytes, bytearray)) or not vregs or len(vregs) > MAX_VREGS:
        raise TraceError(f"vregs must contain 1..{MAX_VREGS} values")
    if any(isinstance(v, bool) or not isinstance(v, int) for v in vregs):
        raise TraceError("vregs must be integers")
    normalized_vregs = list(vregs)
    if len(set(normalized_vregs)) != len(normalized_vregs) or any(v < 0 or v > 0xFFFFFFFF for v in normalized_vregs):
        raise TraceError("vregs must be distinct uint32 values")
    result: dict[str, Any] = {
        "schema": SCHEMA, "diagnostic_only": True, "authority_advanced": False,
        "status": "failed", "stage": "preflight", "capture_runs": 0,
        "evaluation_runs": 0, "unknowns": [],
    }
    capture_runner = capture_runner or run_capture
    evaluator = evaluator or recovery_evaluate.evaluate
    try:
        base, index_binding, source, baseline, source_desc = _index(root, index)
        candidate = frontier.local(root, candidate)
        candidate = _file(candidate, "candidate source", maximum=MAX_SOURCE_BYTES)
        candidate_bytes, candidate_binding = frontier.read_bound(root, candidate, MAX_SOURCE_BYTES)
        candidate_sha = _sha(candidate_bytes)
        result.update({"index": index_binding, "function": function,
                       "candidate_source": candidate_binding,
                       "baseline_source": source_desc,
                       "baseline_object": _descriptor(baseline)})
        if not any(isinstance(row, dict) and row.get("function") == function
                   for row in base.get("functions", [])):
            raise TraceError("requested function is absent from the current index")
        if candidate_sha == source_desc.get("sha256"):
            result.update(status="duplicate_source", stage="complete",
                          reason="candidate source hash equals the current retained source")
            _write(out / "result.json", result)
            return result
        output_prefix = out / "capture"
        output_object = output_prefix.with_suffix(".o")
        capture = output_prefix.with_suffix(".json")
        request_path = out / "request.json"
        request, compiler_desc, compiler_path, request_source_sha, prelude_binding = _prepare_request(
            root, request_template, candidate, output_object, function, definition_prefix,
            source, source_desc, support_prelude)
        _write(request_path, request)
        result["stage"] = "capture"
        result["capture_runs"] = 1
        baseline_sha = compiler.digest(baseline)
        completed = capture_runner(
            capture_script, root=root, request=request_path, source_sha256=request_source_sha,
            baseline=baseline, baseline_sha256=baseline_sha, output_prefix=output_prefix,
            function=function, function_sha256=request["function_sha256"],
            definition_prefix=definition_prefix)
        if completed.returncode not in (0, 1):
            raise TraceError(f"capture wrapper exited unexpectedly: {completed.returncode}")
        summary = _json_stdout(completed.stdout)
        document, capture_binding = _validate_capture(
            root, capture, output_object, request_path, summary, candidate, compiler_path,
            baseline, request_source_sha, compiler_desc, baseline_sha)
        epochs = [recovery_temp_epochs.analyze_capture(document, vreg) for vreg in normalized_vregs]
        result["stage"] = "evaluation"
        result["evaluation_runs"] = 1
        evaluation = evaluator(
            root=root, index=frontier.local(root, index), candidate=candidate,
            functions=[function], out=out / "evaluation.json",
            objdiff=(Path(objdiff) if Path(objdiff).is_absolute() else root / Path(objdiff)),
            readelf=(Path(readelf) if Path(readelf).is_absolute() else root / Path(readelf)),
            candidate_object=output_object, keep_reports=keep_reports)
        if not isinstance(evaluation, dict):
            raise TraceError("evaluator returned a non-object result")
        result.update({
            "status": evaluation.get("status", "evaluated"), "stage": "complete",
            "evaluation_status": evaluation.get("status"), "capture_summary": summary,
            "capture": capture_binding["capture"], "binding": {
                "request": _descriptor(request_path), "source": _descriptor(candidate),
                "compiler": compiler_desc, "baseline_object": _descriptor(baseline),
                "output_object": capture_binding["output_object"],
                "object_comparison": document["object_comparison"],
                "evaluator_compile_binding": evaluation.get("compile_binding"),
                "caller_supplied_object_is_not_source_proof": True,
            },
            "epochs": epochs, "evaluation": evaluation,
        })
        if prelude_binding is not None:
            result["support_prelude"] = prelude_binding
    except (OSError, ValueError, RuntimeError, KeyError, TypeError, AttributeError, TraceError) as exc:
        result.update(status="failed", reason=str(exc)[:4000], stage=result.get("stage", "preflight"))
    _write(out / "result.json", result)
    return result


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, required=True)
    parser.add_argument("--index", type=Path, required=True)
    parser.add_argument("--request-template", type=Path, required=True)
    parser.add_argument("--capture-script", type=Path, required=True)
    parser.add_argument("--candidate", type=Path, required=True)
    parser.add_argument("--function", required=True)
    parser.add_argument("--definition-prefix", required=True)
    parser.add_argument("--support-prelude", type=Path)
    parser.add_argument("--keep-reports", action="store_true",
                        help="retain evaluator strict/data reports for follow-up analysis")
    parser.add_argument("--vreg", type=int, action="append", required=True)
    parser.add_argument("--out-dir", type=Path, required=True)
    parser.add_argument("--objdiff", type=Path, required=True)
    parser.add_argument("--readelf", type=Path, required=True)
    return parser


def main(argv: Sequence[str] | None = None) -> int:
    args = build_parser().parse_args(argv)
    try:
        result = trace_candidate(
            root=args.root, index=args.index, request_template=args.request_template,
            capture_script=args.capture_script, candidate=args.candidate,
            function=args.function, definition_prefix=args.definition_prefix,
            vregs=args.vreg, out_dir=args.out_dir, objdiff=args.objdiff, readelf=args.readelf,
            support_prelude=args.support_prelude, keep_reports=args.keep_reports)
        out_dir = Path(args.out_dir)
        if not out_dir.is_absolute():
            out_dir = Path(args.root).resolve() / out_dir
        print(json.dumps({"status": result["status"], "out": str(out_dir.resolve() / "result.json")}, sort_keys=True))
        return 0 if result["status"] != "failed" else 1
    except (OSError, ValueError, RuntimeError, KeyError, TypeError, AttributeError, TraceError) as exc:
        print(f"recovery trace candidate: {exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
