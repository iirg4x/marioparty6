"""Thin current-source adapter for the proven canonical void contract generator."""
from pathlib import Path
import sys

if __package__ in {None, ""}:
    sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

from tools import recovery_call_contract_repair as repair
from tools import recovery_frontier as frontier

FAMILY = "canonical_void_contracts"
COMPOSED_FAMILY = "canonical_void_contracts_then_sequence_result_consumer"


def generate_composed(root, index, function, source, composed, out_dir):
    """Exactly two ordered stages, one output, no Cartesian search."""
    from tools import recovery_source_shapes as shapes
    stages = composed.get('stages', [])
    if (not isinstance(stages, list) or len(stages) != 2
            or [s.get('family') for s in stages] != [FAMILY, 'sequence_result_consumer']):
        raise ValueError('only canonical void then sequence-result composition supported')
    original_sha = repair._sha(source)
    for stage in stages:
        if stage.get('constraints', {}).get('source_sha256') != original_sha:
            raise ValueError('composition stage must bind original source')
    second = dict(stages[1]['constraints'])
    # Authenticate original positions before any insertion; caller cannot bypass.
    second['rebind_exact_statements'] = False
    shapes.sequence_result_consumer(source, function, second)
    impls = {str(Path(m.__file__).resolve()): repair._sha(Path(m.__file__).read_bytes())
             for m in (sys.modules[__name__], repair, shapes)}
    first = generate_shape(root, index, function, stages[0]['constraints'], Path(out_dir)/'contracts')
    if len(first) != 1:
        raise ValueError('composition requires exactly one canonical-contract candidate')
    intermediate = first[0]['candidate_source']
    if first[0]['source_sha256'] != original_sha or repair._sha(intermediate) != first[0]['candidate_sha256']:
        raise ValueError('intermediate binding mismatch')
    second.update(source_sha256=repair._sha(intermediate), rebind_exact_statements=True)
    final = shapes.enumerate_shapes(intermediate, function, ['sequence_result_consumer'], second)
    if len(final) != 1 or final[0]['source_sha256'] != repair._sha(intermediate):
        raise ValueError('composition requires exactly one source-family candidate')
    candidate = final[0]['candidate_source']
    if repair._sha(candidate) != final[0]['candidate_sha256']:
        raise ValueError('final binding mismatch')
    if any(repair._sha(Path(p).read_bytes()) != digest for p, digest in impls.items()):
        raise ValueError('composition implementation drift')
    evidence_path = Path(out_dir)/'composition.json'
    proof = {'schema': 'recovery_two_stage_composition/v1', 'original_sha256': original_sha,
             'intermediate_sha256': repair._sha(intermediate), 'final_sha256': repair._sha(candidate),
             'implementations': impls, 'stages': stages, 'original_evidence': first[0]['evidence'],
             'remap': 'exact unique AST statement bytes after validated original spans', 'authority_advanced': False}
    repair.compiler.atomic(evidence_path, frontier.canonical(proof))
    return [{'family': COMPOSED_FAMILY, 'candidate_source': candidate, 'source_sha256': original_sha,
             'candidate_sha256': repair._sha(candidate), 'rationale': 'One reviewed ordered canonical-contract and consumer composition',
             'evidence': [repair._read(Path(root), evidence_path)[1], *first[0]['evidence']],
             'authority_advanced': False}]


def generate_shape(root, index, function, constraints, out_dir):
    """Return one current-bound shape, or [] for an already-correct contract set.

    constraints: source_sha256, capture:{path,sha256}, contracts:[canonical
    declaration/evidence descriptors], reviewed:true. Frozen replay is excluded.
    """
    root = Path(root).absolute()
    index = frontier.local(root, Path(index))
    if constraints.get("reviewed") is not True:
        raise ValueError("explicit reviewed canonical contracts required")
    raw_index, _ = repair._read(root, index)
    baseline = frontier.load_json(raw_index)
    frontier.verify(root, baseline)
    source = baseline["inputs"]["source"]
    if constraints.get("source_sha256") != source["sha256"]:
        raise ValueError("contract search source binding is stale")
    capture = constraints["capture"]
    try:
        result = repair.generate(root=root, index=index, function=function,
            capture=Path(capture["path"]), capture_sha256=capture["sha256"],
            contracts=constraints["contracts"], out_dir=Path(out_dir), reviewed=True)
    except ValueError as exc:
        if str(exc) == "no captured return-contract discrepancy; no candidate emitted":
            return []
        raise
    if result.get("runnable_manifest") is not True:
        raise ValueError("historical contract replay cannot be dispatched")
    candidate_path = Path(result["candidate"])
    candidate, candidate_desc = repair._read(root, candidate_path, result["candidate_sha256"])
    selection_path = candidate_path.parent / "selection.json"
    _, evidence = repair._read(root, selection_path)
    # Detect a frontier advance while generation was in progress.
    current_index, _ = repair._read(root, index)
    if current_index != raw_index:
        raise ValueError("frontier drift during contract generation")
    repair._read(root, Path(source["path"]), source["sha256"])
    return [{"family": FAMILY, "candidate_source": candidate,
             "source_sha256": source["sha256"], "candidate_sha256": candidate_desc["sha256"],
             "constraint_id": constraints.get("id", FAMILY),
             "rationale": "Restore the ordered reviewed canonical void declarations for captured actual-call discrepancies",
             "evidence": [evidence], "authority_advanced": False}]


def main(argv=None):
    import argparse
    import json
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ("root", "index", "constraints", "out-dir"):
        parser.add_argument("--" + name, type=Path, required=True)
    parser.add_argument("--function", required=True)
    args = vars(parser.parse_args(argv))
    args["constraints"] = json.loads(args["constraints"].read_text())
    results = generate_shape(**args)
    print(json.dumps([{k: v for k, v in row.items() if k != "candidate_source"} for row in results]))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
