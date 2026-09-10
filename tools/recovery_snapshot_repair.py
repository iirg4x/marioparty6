"""One reviewed same-typed snapshot graph repair; no compile or source adoption."""
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

LIMIT = 1024 * 1024
IDENT = re.compile(r"[A-Za-z_]\w*\Z")


def mask_c(text: str) -> str:
    """Length-preserving lexical mask; braces in literals/comments are inert."""
    chars = list(text)
    i = 0
    while i < len(text):
        start = i
        if text.startswith("//", i):
            i = text.find("\n", i)
            i = len(text) if i < 0 else i
        elif text.startswith("/*", i):
            end = text.find("*/", i + 2)
            if end < 0:
                raise ValueError("unterminated C comment")
            i = end + 2
        elif text[i] in "\"'":
            quote = text[i]
            i += 1
            while i < len(text) and text[i] != quote:
                i += 2 if text[i] == "\\" else 1
            if i >= len(text):
                raise ValueError("unterminated C literal")
            i += 1
        else:
            i += 1
            continue
        for j in range(start, i):
            if chars[j] not in "\r\n":
                chars[j] = " "
    return "".join(chars)


def braces(mask: str) -> dict[int, int]:
    stack, pairs = [], {}
    for i, ch in enumerate(mask):
        if ch == "{":
            stack.append(i)
        elif ch == "}":
            if not stack:
                raise ValueError("unbalanced closing brace")
            pairs[stack.pop()] = i
    if stack:
        raise ValueError("unbalanced opening brace")
    return pairs


def repair(text: str, function: str, sites: list[dict]) -> tuple[str, list[dict]]:
    if not IDENT.fullmatch(function) or not isinstance(sites, list) or not 1 <= len(sites) <= 8:
        raise ValueError("named function and 1..8 reviewed sites required")
    masked = mask_c(text)
    pairs = braces(masked)
    matches = list(re.finditer(r"\b" + re.escape(function) + r"\s*\([^;{}]*\)\s*\{", masked))
    if len(matches) != 1:
        raise ValueError("function definition absent or ambiguous")
    start = matches[0].end() - 1
    end = pairs[start]
    body = masked[start:end + 1]
    if "#" in body:
        raise ValueError("preprocessor directives inside function are unsupported")
    names, edits = set(), []
    for site in sites:
        if not isinstance(site, dict) or set(site) != {"source_local", "result_local", "player_local"}:
            raise ValueError("site must name source_local, result_local, player_local")
        source, result, player = [site[key] for key in ("source_local", "result_local", "player_local")]
        if any(not isinstance(n, str) or not IDENT.fullmatch(n) for n in (source, result, player)):
            raise ValueError("invalid local identifier")
        if len({source, result, player}) != 3 or names.intersection({source, result, player}):
            raise ValueError("duplicate or colliding site owners")
        names.update((source, result, player))
        declarations = []
        for name in (source, result):
            found = list(re.finditer(r"\b(s8|u8|s16|u16)\s+" + re.escape(name) + r"\s*;", body))
            if len(found) != 1:
                raise ValueError(f"{name}: unique standalone narrow declaration required")
            if re.search(r"\b(?:const|volatile|static|register|extern)\s*$", body[:found[0].start()]):
                raise ValueError("qualified declarations require manual review")
            declarations.append(found[0])
        if declarations[0][1] != declarations[1][1]:
            raise ValueError("snapshot and result must have identical declared types")
        pattern = (r"\b" + re.escape(player) + r"\s*=[^;{}]+;\s*"
                   r"(?P<src>\b" + re.escape(source) + r"\s*=)(?P<expr>[^;{}]+);\s*"
                   r"(?P<dst>\b" + re.escape(result) + r"\s*=)\s*" + re.escape(source) + r"\s*;\s*"
                   r"if\s*\(\s*" + re.escape(result) + r"\s*<=\s*0\s*\)\s*\{")
        found = list(re.finditer(pattern, body))
        if len(found) != 1:
            raise ValueError(f"{source}: expected adjacent player/snapshot/copy/if graph")
        graph = found[0]
        if "," in graph["expr"]:
            raise ValueError("comma expressions require manual review before initialization")
        open_if = start + graph.end() - 1
        close_if = pairs[open_if]
        if re.match(r"\s*else\b", masked[close_if + 1:]):
            raise ValueError("if/else graphs require manual review")
        # Moving ownership is safe only if all uses remain in the new block.
        for name, decl in zip((source, result), declarations):
            if decl.end() > graph.start():
                raise ValueError("declarations must precede the snapshot graph")
            for use in re.finditer(r"\b" + re.escape(name) + r"\b", body):
                inside_decl = decl.start() <= use.start() < decl.end()
                inside_block = graph.start("src") <= use.start() <= close_if - start
                if not inside_decl and not inside_block:
                    raise ValueError(f"{name}: consumer outside proposed scope")
            # Keep comments and whitespace around the removed declaration.
            raw = text[start + decl.start():start + decl.end()]
            if "/*" in raw or "//" in raw:
                raise ValueError("comments embedded in declarations require manual review")
            edits.append({"start": start + decl.start(), "end": start + decl.end(), "replacement": ""})
        typ = declarations[0][1]
        # Insertions preserve complete original if bodies, including nested sites.
        edits.extend([
            {"start": start + graph.start("src"), "end": start + graph.start("src"), "replacement": "{\n" + typ + " "},
            {"start": start + graph.start("dst"), "end": start + graph.start("dst"), "replacement": typ + " "},
            {"start": close_if + 1, "end": close_if + 1, "replacement": "\n}"},
        ])
    ordered = sorted(edits, key=lambda e: (e["start"], e["end"]))
    if any(a["end"] > b["start"] for a, b in zip(ordered, ordered[1:])):
        raise ValueError("overlapping declaration edits")
    candidate = text
    for edit in reversed(ordered):
        edit["original"] = text[edit["start"]:edit["end"]]
        candidate = candidate[:edit["start"]] + edit["replacement"] + candidate[edit["end"]:]
    braces(mask_c(candidate))
    return candidate, ordered


def generate(*, root: Path, index: Path, function: str, sites: list[dict], out_dir: Path,
             reviewed: bool = False, evidence: list[dict] | None = None) -> dict:
    root = Path(root).absolute()
    index = frontier.local(root, index)
    raw, index_desc = frontier.read_bound(root, index, frontier.INDEX_LIMIT)
    base = frontier.load_json(raw)
    frontier.verify(root, base)
    descriptor = base["inputs"]["source"]
    source = frontier.local(root, Path(descriptor["path"]))
    raw_source, actual = frontier.read_bound(root, source, LIMIT)
    if actual != descriptor:
        raise ValueError("index source binding is stale")
    if function not in {r["function"] for r in base["functions"]}:
        raise ValueError("function absent from current index")
    checked_evidence = evidence or [index_desc]
    if not 1 <= len(checked_evidence) <= 16:
        raise ValueError("1..16 bounded evidence descriptors required")
    for item in checked_evidence:
        _, actual_evidence = frontier.read_bound(root, Path(item["path"]), frontier.REPORT_LIMIT)
        if actual_evidence["sha256"] != item["sha256"]:
            raise ValueError("evidence binding is stale")
    candidate, edits = repair(raw_source.decode("utf-8"), function, sites)
    data = candidate.encode("utf-8")
    out_dir = frontier.local(root, out_dir)
    out_dir.relative_to(root / "build")
    if out_dir.exists():
        raise ValueError("immutable output directory already exists")
    sha = hashlib.sha256(data).hexdigest()
    patch = {"schema": "recovery_snapshot_repair/v1", "source": descriptor, "baseline_index": index_desc,
             "function": function, "sites": sites, "candidate_sha256": sha, "edits": edits,
             "span_unit": "Unicode code points", "authority_advanced": False}
    candidate_path = out_dir / "candidate.c"
    manifest = {"schema": "recovery_search_batch/v1", "root_reviewed": reviewed,
                "causal_family": "point-of-use initialization of existing same-typed live snapshot copies",
                "baseline_index_sha256": index_desc["sha256"], "live_source_sha256": descriptor["sha256"],
                "candidates": [{"id": "snapshot-initializers", "source": candidate_path.relative_to(root).as_posix(),
                                "sha256": sha, "functions": [function], "evidence": checked_evidence}]}
    products = {"candidate.c": data, "patch.json": frontier.canonical(patch), "manifest.json": frontier.canonical(manifest)}
    if sum(map(len, products.values())) >= LIMIT:
        raise ValueError("combined repair output must be below 1 MiB")
    frontier.verify(root, base)
    if compiler.digest(source) != descriptor["sha256"] or compiler.digest(index) != index_desc["sha256"]:
        raise ValueError("inputs changed during generation")
    if any(compiler.digest(frontier.local(root, Path(e["path"]))) != e["sha256"] for e in checked_evidence):
        raise ValueError("evidence changed during generation")
    out_dir.mkdir(parents=True, exist_ok=False)
    for name, payload in products.items():
        compiler.atomic(out_dir / name, payload)
    return {"candidate": str(candidate_path), "manifest": str(out_dir / "manifest.json"),
            "candidate_sha256": sha, "root_reviewed": reviewed, "authority_advanced": False}


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ("root", "index", "sites", "out-dir"):
        parser.add_argument("--" + name, type=Path, required=True)
    parser.add_argument("--function", required=True)
    parser.add_argument("--evidence", type=Path)
    parser.add_argument("--reviewed", action="store_true")
    args = vars(parser.parse_args())
    try:
        for name in ("sites", "evidence"):
            path = args[name]
            args[name] = json.loads(path.read_text(encoding="utf-8")) if path else None
        print(json.dumps(generate(**args), sort_keys=True))
        return 0
    except (OSError, ValueError, KeyError, TypeError) as exc:
        print(f"snapshot repair: {exc}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
