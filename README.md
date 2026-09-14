# Mario Party 6 Recovery Ledger

[Open the recovery ledger](https://iirg4x.github.io/marioparty6/).

The site updates automatically after a push to `main` or `gh-pages`. The
`Recovery ledger` GitHub Actions workflow reads one committed revision of `main`,
generates recovery progress and the Cleanup Index together, and deploys a GitHub
Pages artifact. A game build and local game files are not required.

## Deployment

In **Settings → Pages**, the publishing source is **GitHub Actions**. Keep
`.github/workflows/recovery-ledger.yml` on both `main` and `gh-pages`: the copy on
`main` observes source updates; the copy on `gh-pages` observes website updates.
The `github-pages` environment must allow deployments from both branches.

The workflow checks out the site from `gh-pages` and source from `main`. Only the
explicit HTML, JavaScript, CSS, and generated JSON files enter the public artifact.
The **Run workflow** action can refresh the site manually. A failed build leaves
the last successful deployment available.

For a local generation:

```sh
python tools/build_snapshot.py --repo /path/to/marioparty6 --commit FULL_COMMIT_SHA --metadata source-metadata.json --output public
```

The generator reads committed Git objects without executing source configuration
or changing the source checkout. `source-metadata.json` retains binary SHA-1 and
DTK-version-bound code/data budgets and module names. Changes to the original
binary hash or DTK version require renewed size evidence; affected modules are
shown as unavailable until that evidence is updated. Data includes BSS.

## Shared snapshot

All five pages share one in-memory snapshot and browser cache. Navigation keeps
the document, rendered content, and filters. Page templates preload in the
background; direct URLs and browser back/forward remain supported.

The browser shows cached progress immediately and checks `snapshot-version.json`
at startup, periodically while visible, and on returning to the tab. When a new
revision is available, it loads and validates `snapshot.json` before replacing the
shared snapshot. Navigation itself never requests snapshot data. **Check for
updates** performs an explicit refresh; failed background checks keep existing
content visible. Updates become available after the GitHub Actions deployment
finishes, rather than immediately when a commit is pushed.

The source revision and snapshot generation time are visible on the Snapshot page.
Recovery completion follows committed source selection and range coverage; it is
not an independent per-function objdiff or byte-identical binary proof.

## Cleanup Index

Cleanup candidates can be searched and filtered by file, family, and category,
shown as file groups or a flat list, and exported as TSV. These are syntactic review
candidates, not confirmed defects or unmatched functions. The index uses the same
commit as recovery progress, and source links remain pinned to that revision.

The site contains public progress metadata and short excerpts from public source.
No game binaries, machine-specific repository paths, or credentials are included.
