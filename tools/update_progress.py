#!/usr/bin/env python3
"""Generate the committed progress snapshot and README badge endpoints.

The normal verified build already writes ``build/GP6E01/progress.json``. This
module converts that file into small committed JSON files used by the badges at
the top of README.md.
"""

import argparse
import json
import os
import tempfile
from pathlib import Path
from typing import Any, Dict, List, Mapping, Optional

SCHEMA_VERSION = 1
CATEGORY_LABELS = {
    "all": "Code",
    "dol": "DOL",
    "modules": "DLLs",
}
BADGE_FILENAMES = {
    "all": "all.json",
    "dol": "dol.json",
    "modules": "dlls.json",
}


class ProgressError(ValueError):
    pass


def _percentage(matched: int, total: int) -> float:
    if total <= 0:
        return 0.0
    return round((matched / total) * 100.0, 6)


def _metric(matched: int, total: int) -> Dict[str, Any]:
    return {
        "matched": matched,
        "total": total,
        "percent": _percentage(matched, total),
    }


def build_snapshot(progress: Mapping[str, Any], version: str) -> Dict[str, Any]:
    categories: Dict[str, Any] = {}
    for category, label in CATEGORY_LABELS.items():
        value = progress.get(category)
        if not isinstance(value, Mapping):
            raise ProgressError(f"missing progress category: {category}")
        try:
            code = int(value["code"])
            code_total = int(value["code/total"])
            data = int(value["data"])
            data_total = int(value["data/total"])
        except (KeyError, TypeError, ValueError) as exc:
            raise ProgressError(f"invalid progress category: {category}") from exc
        categories[category] = {
            "label": label,
            "code": _metric(code, code_total),
            "data": _metric(data, data_total),
        }
    return {
        "schema_version": SCHEMA_VERSION,
        "version": version,
        "categories": categories,
    }


def _badge_color(percent: float) -> str:
    if percent >= 100.0:
        return "brightgreen"
    if percent >= 75.0:
        return "green"
    if percent >= 50.0:
        return "yellowgreen"
    if percent >= 25.0:
        return "yellow"
    if percent >= 10.0:
        return "orange"
    return "red"


def badge_payload(label: str, percent: float) -> Dict[str, Any]:
    return {
        "schemaVersion": 1,
        "label": label,
        "message": f"{percent:.2f}%",
        "color": _badge_color(percent),
    }


def output_documents(snapshot: Mapping[str, Any]) -> Dict[str, str]:
    categories = snapshot.get("categories")
    if not isinstance(categories, Mapping):
        raise ProgressError("snapshot is missing categories")

    version = snapshot.get("version")
    if not isinstance(version, str) or not version:
        raise ProgressError("snapshot is missing version")

    documents: Dict[str, str] = {
        f"{version}.json": json.dumps(snapshot, indent=2) + "\n"
    }
    for category, filename in BADGE_FILENAMES.items():
        value = categories.get(category)
        if not isinstance(value, Mapping):
            raise ProgressError(f"snapshot is missing category: {category}")
        code = value.get("code")
        if not isinstance(code, Mapping):
            raise ProgressError(f"snapshot category has no code metric: {category}")
        try:
            percent = float(code["percent"])
        except (KeyError, TypeError, ValueError) as exc:
            raise ProgressError(f"invalid code percentage: {category}") from exc
        label = str(value.get("label") or CATEGORY_LABELS[category])
        documents[filename] = json.dumps(
            badge_payload(label, percent), indent=2
        ) + "\n"
    return documents


def _write_if_changed(path: Path, content: str) -> bool:
    if path.is_file() and path.read_text(encoding="utf-8") == content:
        return False
    path.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.NamedTemporaryFile(
        "w", encoding="utf-8", dir=path.parent, delete=False
    ) as handle:
        handle.write(content)
        temporary = Path(handle.name)
    os.replace(temporary, path)
    return True


def write_snapshot(snapshot: Mapping[str, Any], output_dir: Path) -> List[Path]:
    changed: List[Path] = []
    for filename, content in output_documents(snapshot).items():
        path = output_dir / filename
        if _write_if_changed(path, content):
            changed.append(path)
    return changed


def update_from_build(
    progress_path: Path,
    output_dir: Path = Path("progress"),
    version: str = "GP6E01",
) -> List[Path]:
    try:
        progress = json.loads(progress_path.read_text(encoding="utf-8"))
    except FileNotFoundError as exc:
        raise ProgressError(f"progress file not found: {progress_path}") from exc
    except json.JSONDecodeError as exc:
        raise ProgressError(f"invalid JSON: {progress_path}") from exc
    if not isinstance(progress, Mapping):
        raise ProgressError("progress input must be a JSON object")
    return write_snapshot(build_snapshot(progress, version), output_dir)


def check_snapshot(snapshot_path: Path, output_dir: Path) -> List[str]:
    try:
        snapshot = json.loads(snapshot_path.read_text(encoding="utf-8"))
    except FileNotFoundError as exc:
        raise ProgressError(f"snapshot not found: {snapshot_path}") from exc
    except json.JSONDecodeError as exc:
        raise ProgressError(f"invalid JSON: {snapshot_path}") from exc
    if not isinstance(snapshot, Mapping):
        raise ProgressError("snapshot must be a JSON object")

    errors: List[str] = []
    for filename, expected in output_documents(snapshot).items():
        path = output_dir / filename
        if not path.is_file():
            errors.append(f"missing generated progress file: {path}")
        elif path.read_text(encoding="utf-8") != expected:
            errors.append(f"stale generated progress file: {path}")
    return errors


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--input",
        type=Path,
        default=Path("build/GP6E01/progress.json"),
        help="DTK-template progress.json generated by the build",
    )
    parser.add_argument(
        "--snapshot",
        type=Path,
        default=Path("progress/GP6E01.json"),
        help="committed progress snapshot used for --check",
    )
    parser.add_argument(
        "--output-dir",
        type=Path,
        default=Path("progress"),
        help="directory for the snapshot and badge endpoint files",
    )
    parser.add_argument("--version", default="GP6E01")
    parser.add_argument(
        "--check",
        action="store_true",
        help="verify badge endpoint files match the committed snapshot",
    )
    args = parser.parse_args()

    try:
        if args.check:
            errors = check_snapshot(args.snapshot, args.output_dir)
            for error in errors:
                print(f"error: {error}")
            return 1 if errors else 0

        changed = update_from_build(args.input, args.output_dir, args.version)
        if changed:
            for path in changed:
                print(f"Updated {path}")
        else:
            print("Progress files are already up to date")
        return 0
    except ProgressError as exc:
        print(f"error: {exc}")
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
