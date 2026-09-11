"""Emit one scoped declaration repair from reviewed void contracts and native calls.

Contracts are ordered objects: declaration (exact C declaration text), evidence
{path, sha256, start_byte, end_byte}. Evidence spans contain a canonical header
declaration or definition signature, optionally ending with its opening brace.
This tool does not infer return contracts, compile, retain, or edit live source.
"""
from __future__ import annotations
import argparse
import hashlib
import json
from pathlib import Path
import re
import sys

if __package__ in {None, ""}:
    sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from tools import compile_recovery_candidate as compiler
from tools import recovery_frontier as frontier
from tools.recovery_snapshot_repair import braces, mask_c
from tools import recovery_expression_join as expression_join

LIMIT = 4 * 1024 * 1024
SCHEMA = "recovery_call_contract_repair/v1"
COMPILER_SHA256 = "316e2a98236c23f3fc902243b157eaebf8ef2ad6edb88cfd632a15b6676fa9a8"


def _sha(raw: bytes) -> str:
    return hashlib.sha256(raw).hexdigest()


def _read(root: Path, path: Path, expected: str | None = None) -> tuple[bytes, dict]:
    raw, desc = frontier.read_bound(root, path, LIMIT)
    if expected is not None and expected != desc["sha256"]:
        raise ValueError(f"hash drift: {path}")
    return raw, desc


def _signature(text: str) -> tuple[str, str]:
    value = mask_c(text).strip()
    match = re.fullmatch(r"(?:extern\s+)?void\s+([A-Za-z_]\w*)\s*\(([^{};]*)\)\s*[;{]?", value)
    if not match:
        raise ValueError("contract evidence must be a plain canonical void signature")
    parts = [p.strip() for p in match[2].split(',')]
    if match[2].strip() and (any(not p for p in parts)
            or (len(parts) > 1 and 'void' in parts)
            or ('...' in match[2] and (len(parts) < 2 or parts[-1] != '...'
                                      or any('...' in p for p in parts[:-1])))):
        raise ValueError("invalid parameter list: ellipsis requires preceding parameters and must be last")
    # Whitespace is non-semantic here; punctuation and parameter identifiers
    # remain bound. No typedef, argument, or return-type invention is permitted.
    return match[1], re.sub(r"\s+", "", match[2])


_C_TYPES = {
    "char": "char", "signed char": "signed char", "unsigned char": "unsigned char",
    "short": "short", "short int": "short", "signed short": "short",
    "signed short int": "short", "unsigned short": "unsigned short",
    "unsigned short int": "unsigned short", "int": "int", "signed": "int",
    "signed int": "int", "unsigned": "unsigned int", "unsigned int": "unsigned int",
    "long": "long", "long int": "long", "unsigned long": "unsigned long",
    "unsigned long int": "unsigned long", "float": "float", "double": "double",
}


def _parameter_type(text: str, aliases: dict[str, str]) -> str | None:
    """A small declaration diagnostic, not a typedef/ABI resolver."""
    text = " ".join(text.split())
    # Arrays, function pointers, attributes and qualifiers require a real type
    # context. Never turn their absence in this reader into compatibility.
    if re.search(r"[\[\](){}.]|\b(?:const|volatile|restrict|enum|_Bool)\b", text):
        return None
    pointer = re.fullmatch(r"((?:struct\s+)?[A-Za-z_]\w*)\s*\*\s*(?:[A-Za-z_]\w*)?", text)
    if pointer:
        return pointer[1] + " *"
    if text in _C_TYPES:
        return _C_TYPES[text]
    if text in aliases:
        return aliases[text]
    named = re.fullmatch(r"(.+?)\s+([A-Za-z_]\w*)", text)
    if named:
        return _C_TYPES.get(named[1], aliases.get(named[1]))
    return None


def _declaration_view(text: str, aliases: dict[str, str]) -> dict:
    masked = mask_c(text).strip()
    match = re.fullmatch(r"((?:(?:extern|static|inline)\s+)*)([A-Za-z_][\w \t*]*?)\b([A-Za-z_]\w*)\s*\(([^{};]*)\)\s*[;{]?", masked)
    if not match:
        raise ValueError("declaration diagnostic requires a plain function signature")
    spelling, name, params = " ".join(match[2].split()), match[3], match[4].strip()
    # Reuse the existing parameter syntax checks without widening the
    # separately reviewed void-only candidate generator.
    _signature("void " + name + "(" + params + ");")
    return_type = "void" if spelling == "void" else _C_TYPES.get(spelling, aliases.get(spelling))
    pointer = re.fullmatch(r"(.+?)\s*\*", spelling)
    if pointer:
        base = pointer[1].strip()
        scalar = "void" if base == "void" else _C_TYPES.get(base, aliases.get(base))
        if scalar is not None:
            return_type = scalar + " *"
    common = {"name": name, "return_type": return_type, "return_spelling": spelling,
              "declaration_status": "present"}
    if not params:
        return dict(common, kind="nonprototype", parameters=None)
    if params == "void":
        return dict(common, kind="prototype", parameters=[])
    parts = params.split(",")
    if len(parts) > 16:
        raise ValueError("declaration diagnostic supports at most 16 parameters")
    types = [_parameter_type(p.strip(), aliases) for p in parts]
    return dict(common, kind="prototype", parameters=types,
                unknown_parameters=[i for i, t in enumerate(types) if t is None])


def diagnose_declarations(*, root: Path, caller: dict, provider: dict,
                          reviewed_type_aliases: dict | None = None) -> dict:
    """Compare bound visible signatures; never infer visibility or emit a patch.

    The caller selects the declaration visible at its callsite. A missing
    declaration uses a hash-bound callsite span plus declaration_status="missing"
    and symbol; implicit int is conditional on that caller assertion. Optional
    visibility="ambiguous" forces UNKNOWN. This reader
    binds that claim to a source span but does not resolve includes/scopes or
    invent a same-session compiler-to-formal edge. The conversion model is the
    MP6 GC2.6/2.7 C ABI (32-bit int, 16-bit short), not host Python/C types.
    """
    root = Path(root).absolute()
    aliases = reviewed_type_aliases or {}
    if (not isinstance(aliases, dict) or len(aliases) > 32
            or any(not isinstance(k, str) or not re.fullmatch(r"[A-Za-z_]\w*", k)
                   or k in _C_TYPES or not isinstance(v, str) or v not in _C_TYPES for k, v in aliases.items())):
        raise ValueError("reviewed aliases must map identifiers to supported C scalar types")
    aliases = {k: _C_TYPES[v] for k, v in aliases.items()}
    views, watched = [], []
    for role, desc in (("caller", caller), ("provider", provider)):
        path = frontier.local(root, Path(desc["path"]))
        if not isinstance(desc.get("sha256"), str) or not re.fullmatch(r"[0-9a-f]{64}", desc["sha256"]):
            raise ValueError("signature descriptor requires SHA-256")
        raw, actual = _read(root, path, desc["sha256"])
        start, end = desc["start_byte"], desc["end_byte"]
        if (type(start) is not int or type(end) is not int
                or not 0 <= start < end <= len(raw) or end-start > 1024):
            raise ValueError("invalid bounded declaration span")
        span = raw[start:end]
        visibility = desc.get("visibility", "caller_selected")
        if visibility not in ("caller_selected", "ambiguous"):
            raise ValueError("visibility must be caller_selected or ambiguous")
        status = desc.get("declaration_status", "present")
        if status == "missing":
            name = desc.get("symbol")
            if (role != "caller" or not isinstance(name, str)
                    or not re.fullmatch(r"[A-Za-z_]\w*", name)
                    or not re.search(r"\b" + re.escape(name) + r"\s*\(", mask_c(span.decode("utf-8")))):
                raise ValueError("missing declaration requires a caller-selected bound callsite and symbol")
            view = {"name": name, "kind": "nonprototype", "parameters": None,
                    "return_type": "int", "return_spelling": None,
                    "declaration_status": "missing", "implicit_int_status": "old-C implicit int if caller visibility claim holds"}
        elif status == "present":
            view = _declaration_view(span.decode("utf-8"), aliases)
        else:
            raise ValueError("declaration_status must be present or missing")
        view["visibility"] = visibility
        views.append(dict(view, role=role, source=actual, start_byte=start,
                          end_byte=end, span_sha256=_sha(span), spelling=span.decode("utf-8")))
        watched.append((path, actual["sha256"]))
    left, right = views
    if left["name"] != right["name"]:
        raise ValueError("caller and provider name mismatch")
    conversions = []
    for i, typ in enumerate(right["parameters"] or []):
        if typ is None:
            continue
        promoted = ("int" if typ in {"char", "signed char", "unsigned char", "short", "unsigned short"}
                    else "double" if typ == "float" else typ)
        conversions.append({"parameter": i, "provider_type": typ,
                            "default_promoted_type": promoted,
                            "promotion_changes_type": typ != promoted,
                            "caller_conversion": ("default argument promotions" if left["kind"] == "nonprototype"
                                                  else "conversion to declared parameter type"),
                            "source_question": ("Does the earliest mismatch originate in call-argument narrowing, not allocation?"
                                                if typ in {"char", "signed char", "unsigned char", "short", "unsigned short"}
                                                else "Check the caller-visible parameter conversion before interpreting codegen.")})
    if (left.get("unknown_parameters") or right.get("unknown_parameters")
            or right["kind"] != "prototype"):
        compatibility = "UNKNOWN"
    elif left["kind"] == "nonprototype":
        compatibility = "INCOMPATIBLE" if any(c["promotion_changes_type"] for c in conversions) else "COMPATIBLE"
    else:
        # Opaque pointer typedefs may name the same type with different names;
        # only identical supported type lists establish agreement here.
        compatibility = "COMPATIBLE" if left["parameters"] == right["parameters"] else "UNKNOWN"
    parameter_compatibility = compatibility
    return_compatibility = ("UNKNOWN" if left["return_type"] is None or right["return_type"] is None
                            else "COMPATIBLE" if left["return_type"] == right["return_type"] else "INCOMPATIBLE")
    ambiguous = any(view["visibility"] == "ambiguous" for view in views)
    if ambiguous or return_compatibility == "UNKNOWN":
        compatibility = "UNKNOWN"
    elif return_compatibility == "INCOMPATIBLE":
        compatibility = "INCOMPATIBLE"
    result = {"schema": "recovery_call_declaration_diagnostic/v1", "function": left["name"],
              "observed_declarations": views, "reviewed_type_aliases": aliases,
              "model": "MP6 GC2.6/2.7 C: int32, short16; default argument promotions",
              "compatibility": compatibility, "parameter_conversions": conversions,
              "parameter_compatibility": parameter_compatibility,
              "return_comparison": {"caller_type": left["return_type"], "provider_type": right["return_type"],
                                    "compatibility": "UNKNOWN" if ambiguous else return_compatibility,
                                    "drift": None if ambiguous or return_compatibility == "UNKNOWN" else return_compatibility == "INCOMPATIBLE"},
              "visibility_status": ("UNKNOWN; ambiguous caller/provider visibility supplied" if ambiguous
                                    else "caller-selected source spans; include/scope resolution not proved"),
              "native_formal_join": "UNKNOWN; no same-session formal-parameter edge supplied",
              "requires_source_exception": compatibility == "INCOMPATIBLE",
              "missing_declaration_proof": "caller assertion only; includes/scopes not resolved",
              "diagnostic_only": True, "source_patch_emitted": False, "source_authority": False, "authority_advanced": False}
    if any(compiler.digest(path) != sha for path, sha in watched):
        raise ValueError("declaration inputs changed during analysis")
    result["diagnostic_sha256"] = _sha(frontier.canonical(result))
    return result


def _contracts(root: Path, items: list[dict]) -> tuple[list[dict], dict[str, str]]:
    if not isinstance(items, list) or not 1 <= len(items) <= 32:
        raise ValueError("1..32 ordered reviewed contracts required")
    result, watched, names = [], {}, set()
    for item in items:
        declaration = item["declaration"]
        if not isinstance(declaration, str) or not declaration.endswith(";") or len(declaration) > 1024:
            raise ValueError("bounded semicolon-terminated declaration required")
        name, parameters = _signature(declaration)
        if not parameters:
            raise ValueError("nonprototype is not a canonical repair; diagnose declaration conversions and review the source exception")
        if name in names:
            raise ValueError(f"duplicate contract: {name}")
        names.add(name)
        evidence = item["evidence"]
        if not isinstance(evidence.get("sha256"), str) or not re.fullmatch(r"[0-9a-f]{64}", evidence["sha256"]):
            raise ValueError("canonical evidence requires a SHA-256")
        path = frontier.local(root, Path(evidence["path"]))
        raw, desc = _read(root, path, evidence["sha256"])
        start, end = evidence["start_byte"], evidence["end_byte"]
        if any(isinstance(n, bool) or not isinstance(n, int) for n in (start, end)) or not 0 <= start < end <= len(raw):
            raise ValueError(f"invalid contract evidence span: {name}")
        if _signature(raw[start:end].decode("utf-8")) != (name, parameters):
            raise ValueError(f"canonical declaration mismatch: {name}")
        watched[str(path)] = desc["sha256"]
        result.append({"name": name, "declaration": declaration, "evidence": evidence})
    return result, watched


def _function_span(raw: bytes, name: str) -> tuple[int, int, int]:
    if not re.fullmatch(r"[A-Za-z_]\w*", name):
        raise ValueError("invalid function identifier")
    # Latin-1 is used solely for byte-indexed lexing, never output transcoding.
    masked = mask_c(raw.decode("latin-1"))
    pattern = r"^[ \t]*(?:static\s+)?[A-Za-z_][\w \t*]*\b" + re.escape(name) + r"\s*\([^;{}]*\)\s*\{"
    matches = list(re.finditer(pattern, masked, re.MULTILINE))
    if len(matches) != 1:
        raise ValueError("function definition missing or ambiguous")
    match = matches[0]
    opening = match.end() - 1
    return match.start(), braces(masked)[opening] + 1, opening


def inspect_calls(capture: dict, function: str, names: set[str]) -> tuple[list[dict], list[dict]]:
    if 'context' in capture:
        return inspect_envelope_calls(capture, function, names)
    events = capture.get("events")
    if not isinstance(events, list) or len(events) > 20000:
        raise ValueError("bounded native events required")
    calls, discrepancies = [], []
    for index, event in enumerate(events):
        origin = event.get("actual_call_origin", {})
        if origin.get("binding") != "native_return_allocation_frame":
            continue
        if event.get("function") != function:
            raise ValueError("actual call event belongs to another function")
        # Do not substitute event.normalized_expression_tree: it can describe
        # an enclosing statement rather than this actual return allocation.
        tree = origin.get("normalized_expression_tree", {}).get("tree", {})
        name = tree.get("callee", {}).get("object", {}).get("name")
        typ = tree.get("type", {})
        known = name in names
        if known and (origin.get("status") != "CAPTURED" or tree.get("native_kind") != 54
                      or typ.get("status") != "CAPTURED" or not isinstance(typ.get("native_kind"), int)
                      or not isinstance(typ.get("byte_width"), int)):
            raise ValueError(f"actual return type unavailable for reviewed API: {name}")
        is_void = typ.get("native_kind") == 0 and typ.get("byte_width") == 0
        row = {"event_index": index, "sequence": event.get("sequence"), "callee": name,
               "call_expression_token": origin.get("call_expression_token"),
               "compiler_seen_type": typ, "counter_before": event.get("counter_before"),
               "counter_after": event.get("counter_after"),
               "status": "discrepant" if known and not is_void else "agrees" if known else "unreviewed"}
        calls.append(row)
        if row["status"] == "discrepant":
            discrepancies.append(row)
    return calls, discrepancies


def inspect_envelope_calls(capture, function, names):
    """Actual allocation proves nonvoid lowering, never its exact return type."""
    if capture.get('context', {}).get('compiler', {}).get('sha256') != COMPILER_SHA256:
        raise ValueError('unpinned envelope compiler')
    if capture['context'].get('function') != function:
        raise ValueError('envelope function mismatch')
    expression_join.validate_session(capture, function)
    events = capture['events']
    if len(events) > 20000:
        raise ValueError('bounded native events required')
    calls = []
    for birth in events:
        if birth.get('event_kind') != 'return_temp_allocation':
            continue
        token = birth.get('expression_token')
        if (birth.get('status') != 'CAPTURED' or type(birth.get('allocated_vreg')) is not int
                or birth['allocated_vreg'] < 0 or birth.get('counter_after') != birth['allocated_vreg']+1
                or not isinstance(token, str) or not token):
            raise ValueError('invalid actual return allocation')
        origins = [e for e in events if e.get('event_kind') == 'source_pcode_origin'
                   and e.get('expression_token') == token and e.get('session_id') == birth['session_id']]
        callees = {e.get('direct_callee_name') for e in origins}
        if len(callees) > 1:
            raise ValueError('ambiguous actual allocation callee')
        if not origins or not callees or not next(iter(callees)):
            continue
        name = next(iter(callees))
        if any(e.get('status') != 'CAPTURED' or e.get('child_edge') != 'CAPTURED_ACTIVE_HANDLER'
               or e.get('expression_kind') not in (54, 55) for e in origins):
            raise ValueError('unconfirmed actual direct call origin')
        calls.append({'event_id': birth.get('event_id'), 'sequence': birth.get('sequence'),
                      'callee': name, 'call_expression_token': token,
                      'compiler_seen_type': {'status': 'NONVOID_ALLOCATION_OBSERVED', 'exact_type': None},
                      'allocated_vreg': birth['allocated_vreg'],
                      'status': 'discrepant' if name in names else 'unreviewed'})
    return calls, [r for r in calls if r['status'] == 'discrepant']


def bind_envelope(capture, capture_path, object_path, source_path, function):
    inspect_envelope_calls(capture, function, set())
    binding = expression_join.bind(capture_path, object_path, source_path, function)
    if binding['comparison']['status'] != 'exact_words':
        raise ValueError('envelope machine/object binding is not exact')
    return binding


def generate(*, root: Path, index: Path, capture: Path, capture_sha256: str,
             function: str, contracts: list[dict], out_dir: Path, reviewed: bool = False,
             source: Path | None = None, source_sha256: str | None = None) -> dict:
    if reviewed is not True:
        raise ValueError("explicit ROOT-reviewed contracts required")
    if not isinstance(capture_sha256, str) or not re.fullmatch(r"[0-9a-f]{64}", capture_sha256):
        raise ValueError("capture requires a SHA-256")
    root = Path(root).absolute()
    index, capture, out_dir = [frontier.local(root, Path(p)) for p in (index, capture, out_dir)]
    out_dir.relative_to(root / "build")
    index_raw, index_desc = _read(root, index)
    base = frontier.load_json(index_raw)
    frontier.verify(root, base)
    source_desc = base["inputs"]["source"]
    explicit_source = source is not None
    if explicit_source and source_sha256 is None:
        raise ValueError("explicit frozen source requires its SHA-256")
    source = frontier.local(root, Path(source) if explicit_source else Path(source_desc["path"]))
    raw, actual = _read(root, source, source_sha256 if explicit_source else source_desc["sha256"])
    if (not explicit_source and actual != source_desc) or function not in {r["function"] for r in base["functions"]}:
        raise ValueError("current source/function binding mismatch")
    runnable = actual["sha256"] == source_desc["sha256"]
    capture_raw, capture_desc = _read(root, capture, capture_sha256)
    document = frontier.load_json(capture_raw)
    envelope_binding = None
    if 'context' in document:
        object_path = frontier.local(root, Path(base['inputs']['candidate_object']['path']))
        envelope_binding = bind_envelope(document, capture, object_path, source, function)
        context = document['context']
        # Metadata compatibility only; native events remain unmodified.
        document = dict(document, compiler=context['compiler'], source=context['source'],
                        function=function, status='CAPTURED',
                        function_sha256=_sha(raw[slice(*_function_span(raw, function)[:2])]))
    if document.get("compiler", {}).get("sha256") != COMPILER_SHA256:
        raise ValueError("native return type format requires pinned GC2.6 compiler SHA-256")
    if document.get("status") != "CAPTURED" or document.get("function") != function:
        raise ValueError("capture is not successful evidence for selected function")
    captured_source = document.get("source", {})
    if captured_source.get("sha256") != actual["sha256"]:
        raise ValueError("capture source does not match current source")
    # A historical capture path may now contain newer retained source. Explicit
    # replay binds the frozen content hash, not that mutable historical pathname.
    captured_path = None
    if not explicit_source:
        captured_path = frontier.local(root, Path(captured_source["path"]))
        captured_bytes, _ = _read(root, captured_path, actual["sha256"])
        if captured_bytes != raw:
            raise ValueError("capture source bytes differ")
    start, end, opening = _function_span(raw, function)
    if document.get("function_sha256") != _sha(raw[start:end]):
        raise ValueError("capture function span/hash drift")
    selected, watched = _contracts(root, contracts)
    calls, discrepancies = inspect_calls(document, function, {c["name"] for c in selected})
    if not discrepancies:
        raise ValueError("no captured return-contract discrepancy; no candidate emitted")
    # Preserve ROOT's full ordered canonical set, including contracts already
    # correct at some or all callsites. Do not build a subset search matrix.
    newline = b"\r\n" if raw[opening + 1:opening + 3] == b"\r\n" else b"\n"
    position = opening + 1 + len(newline)
    if raw[opening + 1:position] != newline:
        raise ValueError("function opening brace must be followed by a newline")
    insertion = b"".join(b"    " + c["declaration"].encode("utf-8") + newline for c in selected)
    candidate = raw[:position] + insertion + raw[position:]
    candidate_path = out_dir / "candidate.c"
    selection = {"schema": SCHEMA, "baseline_index": index_desc, "capture": capture_desc,
                 "envelope_binding": envelope_binding,
                 "source": actual, "function": function, "function_sha256": document["function_sha256"],
                 "contracts": selected, "calls": [c for c in calls if c["status"] != "unreviewed"],
                 "unreviewed_call_count": sum(c["status"] == "unreviewed" for c in calls),
                 "historical_source_path": captured_source.get("path"), "discrepancies": discrepancies,
                 "insert_byte": position, "inserted_text": insertion.decode("utf-8"),
                 "candidate_sha256": _sha(candidate), "authority_advanced": False,
                 "live_guard": {"matches": runnable, "current_source": source_desc,
                                "replay_only": not runnable}}
    # The batch runner permits at most16 evidence descriptors; the selection
    # file binds larger reviewed sets without losing their exact spans.
    manifest = {"schema": "recovery_search_batch/v1", "root_reviewed": runnable,
                "causal_family": "restore reviewed canonical void call contracts at function entry",
                "baseline_index_sha256": index_desc["sha256"], "live_source_sha256": actual["sha256"],
                "candidates": [{"id": "canonical-void-contracts", "source": candidate_path.relative_to(root).as_posix(),
                                "sha256": _sha(candidate), "functions": [function],
                                "evidence": [{"path": (out_dir / "selection.json").relative_to(root).as_posix(),
                                              "sha256": _sha(frontier.canonical(selection))}]}]}
    products = {"candidate.c": candidate, "selection.json": frontier.canonical(selection),
                "manifest.json": frontier.canonical(manifest)}
    if any(len(data) > LIMIT for data in products.values()):
        raise ValueError("repair output exceeds 4 MiB per file")
    watched.update({str(index): index_desc["sha256"], str(source): actual["sha256"],
                    str(capture): capture_desc["sha256"]})
    if captured_path is not None:
        watched[str(captured_path)] = actual["sha256"]
    frontier.verify(root, base)
    if any(compiler.digest(Path(p)) != sha for p, sha in watched.items()):
        raise ValueError("repair inputs changed during generation")
    if out_dir.exists():
        raise ValueError("immutable output directory already exists")
    out_dir.mkdir(parents=True, exist_ok=False)
    for name, data in products.items():
        compiler.atomic(out_dir / name, data)
    return {"candidate": str(candidate_path), "candidate_sha256": _sha(candidate),
            "manifest": str(out_dir / "manifest.json"), "discrepant_calls": len(discrepancies),
            "inserted_contracts": len(selected), "runnable_manifest": runnable,
            "authority_advanced": False}


def main() -> int:
    if len(sys.argv) > 1 and sys.argv[1] == "declarations":
        parser = argparse.ArgumentParser(description="Read-only caller/provider return-contract and parameter-conversion diagnostic")
        parser.add_argument("--root", type=Path, required=True)
        parser.add_argument("--request", type=Path, required=True,
                            help="bound caller/provider spans; caller may assert missing declaration at a callsite; optional reviewed_type_aliases")
        args = parser.parse_args(sys.argv[2:])
        args.root = args.root.absolute()
        try:
            raw, _ = _read(args.root, frontier.local(args.root, args.request))
            request = frontier.load_json(raw)
            print(json.dumps(diagnose_declarations(root=args.root, **request), sort_keys=True))
            return 0
        except (OSError, ValueError, KeyError, TypeError) as exc:
            print(f"call-declaration diagnostic: {exc}", file=sys.stderr)
            return 2
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ("root", "index", "capture", "contracts", "out-dir"):
        parser.add_argument("--" + name, type=Path, required=True)
    parser.add_argument("--capture-sha256", required=True)
    parser.add_argument("--function", required=True)
    parser.add_argument("--reviewed", action="store_true")
    parser.add_argument("--source", type=Path, help="optional hash-bound frozen source for replay")
    parser.add_argument("--source-sha256")
    args = vars(parser.parse_args())
    try:
        path = args["contracts"]
        if path.stat().st_size > LIMIT:
            raise ValueError("contracts input exceeds 4 MiB")
        args["contracts"] = json.loads(path.read_text(encoding="utf-8"))
        print(json.dumps(generate(**args), sort_keys=True))
        return 0
    except (OSError, ValueError, KeyError, TypeError) as exc:
        print(f"call-contract repair: {exc}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
