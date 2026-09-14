# Mario Party 6 Recovery Ledger

Static recovery dashboard for the `iirg4x/marioparty6` repository.

This directory is ready to be the root of a dedicated `gh-pages` branch.
In the repository's **Settings → Pages**, choose **Deploy from a branch**,
then **gh-pages** and **/(root)**.

Expected project-site address: https://iirg4x.github.io/marioparty6/

Pages: [overview](https://iirg4x.github.io/marioparty6/),
[DOL breakdown](https://iirg4x.github.io/marioparty6/dol.html),
[module library](https://iirg4x.github.io/marioparty6/modules.html), and
[snapshot/counting details](https://iirg4x.github.io/marioparty6/snapshot.html).

The dashboard uses relative asset URLs so it also works below the repository path.
No build or dependencies are required. The `.nojekyll` file disables Jekyll processing.

Normal navigation keeps the same document and shared snapshot in memory. The other
page templates preload in the background, and visited pages retain their rendered
content and filters. Back/forward navigation and direct page URLs remain supported.

It includes the dark theme, animations, module/function filters, function counts,
and scrollable module-detail dialogs. Source-selected functions follow committed
owner selection; they are not independent per-function objdiff proofs.

`snapshot.json` is a published recovery snapshot, with its source commit and capture
time visible on the page. All four pages share one browser-cached snapshot. Normal
navigation reuses it without another snapshot request; Reload snapshot explicitly
fetches and replaces the shared copy. A failed reload keeps the previous copy.
If browser storage is unavailable, loading falls back to a network request.
To update progress,
replace it with a newly verified export using the same schema, then push to this
branch. It does not access a developer's local repository or rebuild the game.

This branch contains dashboard code and progress metadata only. No game binaries,
source bodies, machine-specific repository paths, or credentials are included.
GitHub Pages for this public repository is public; the private ChatGPT Site remains
separate and is not affected by deploying this branch.
