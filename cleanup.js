(() => {
  "use strict";

  const REPOSITORY = "https://github.com/iirg4x/marioparty6";
  const FAMILY_DEFS = [
    { id: "pointer-access", label: "Pointer access" },
    { id: "pointer-cast", label: "Pointer casts" },
    { id: "match-hack", label: "Match workarounds" },
  ];
  const FAMILY_BY_ID = new Map(FAMILY_DEFS.map((family) => [family.id, family]));
  const FILE_PAGE_SIZE = 24;
  const FLAT_PAGE_SIZE = 100;
  const GROUP_PAGE_SIZE = 40;
  const BULK_GROUP_LIMIT = 28;
  const BULK_SITE_LIMIT = 240;
  const numberFormat = new Intl.NumberFormat("en-US");
  const stateByMain = new WeakMap();
  let activeMain = null;
  let activeState = null;

  function isObject(value) {
    return value !== null && typeof value === "object" && !Array.isArray(value);
  }

  function asText(value, fallback = "") {
    return typeof value === "string" ? value.trim() : fallback;
  }

  function finiteNumber(value) {
    if (typeof value === "number" && Number.isFinite(value)) return value;
    if (typeof value === "string" && value.trim() !== "") {
      const number = Number(value);
      return Number.isFinite(number) ? number : null;
    }
    return null;
  }

  function integer(value) {
    const number = finiteNumber(value);
    return number === null ? null : Math.trunc(number);
  }

  function formatNumber(value) {
    const number = finiteNumber(value);
    return number === null ? "—" : numberFormat.format(number);
  }

  function escapeHtml(value) {
    return String(value ?? "")
      .replace(/&/g, "&amp;")
      .replace(/</g, "&lt;")
      .replace(/>/g, "&gt;")
      .replace(/"/g, "&quot;")
      .replace(/'/g, "&#39;");
  }

  function humanize(value) {
    const text = asText(value, "Unknown category").replace(/[_-]+/g, " ");
    return text.replace(/\b\w/g, (letter) => letter.toUpperCase());
  }

  function normalizeFamily(value) {
    const raw = asText(value).toLowerCase().replace(/[\s_]+/g, "-");
    if (FAMILY_BY_ID.has(raw)) return raw;
    if (raw.includes("match") || raw.includes("hack") || raw.includes("pragma")) return "match-hack";
    if (raw.includes("cast") || raw.includes("retype") || raw.includes("deref")) return "pointer-cast";
    if (raw.includes("pointer") || raw.includes("offset") || raw.includes("access")) return "pointer-access";
    return "pointer-access";
  }

  function familyLabel(id) {
    return FAMILY_BY_ID.get(id)?.label || "Pointer access";
  }

  function directoryFor(path) {
    const parts = asText(path).split("/").filter(Boolean);
    if (parts[0]?.toLowerCase() === "src") parts.shift();
    // The last path segment is the source file. Keep the first two directory
    // levels so REL/module paths remain useful without putting every file in
    // its own facet.
    if (parts.length) parts.pop();
    return parts.length ? parts.slice(0, 2).join("/") : "root";
  }

  function categoryDescription(category) {
    return asText(category?.description);
  }

  function normalizeCleanup(snapshot) {
    const raw = snapshot?.cleanup;
    if (!isObject(raw) || Number(raw.schemaVersion) !== 1 || !Array.isArray(raw.sites)) return null;

    const categories = [];
    const categoryById = new Map();
    const addCategory = (value, fallbackFamily = "pointer-access") => {
      const id = asText(value?.id || value);
      if (!id || categoryById.has(id)) return categoryById.get(id);
      const category = {
        id,
        label: asText(value?.label, humanize(id)),
        family: normalizeFamily(value?.family || fallbackFamily),
        description: categoryDescription(value),
      };
      categoryById.set(id, category);
      categories.push(category);
      return category;
    };
    for (const category of Array.isArray(raw.categories) ? raw.categories : []) {
      if (isObject(category)) addCategory(category);
    }

    const sites = [];
    for (const item of raw.sites) {
      if (!isObject(item)) continue;
      const path = asText(item.path || item.file);
      const line = integer(item.line);
      if (!path || line === null || line < 1) continue;
      const categoryId = asText(item.category || item.type, "uncategorized");
      const category = addCategory(categoryId, item.family);
      const sourceValue = isObject(item.source)
        ? asText(item.source.snippet || item.source.text || item.source.value)
        : asText(item.source || item.snippet);
      const endLine = integer(item.endLine ?? item.end_line);
      sites.push({
        path,
        line,
        endLine: endLine !== null && endLine >= line ? endLine : line,
        category: category.id,
        categoryLabel: category.label,
        categoryDescription: category.description,
        family: normalizeFamily(item.family || category.family),
        detail: asText(item.detail || item.type),
        source: sourceValue,
        offset: normalizeOffset(item.offset),
        directory: directoryFor(path),
      });
    }

    const scannedValue = Array.isArray(raw.scannedFiles)
      ? raw.scannedFiles.length
      : finiteNumber(raw.scannedFiles);
    return {
      commit: asText(raw.commit || snapshot?.commit),
      generatedAt: asText(raw.generatedAt),
      scannedFiles: scannedValue,
      categories,
      sites,
      methodology: Array.isArray(raw.methodology)
        ? raw.methodology.map((note) => asText(note)).filter(Boolean)
        : [],
    };
  }

  function createState(main) {
    return {
      main,
      bound: false,
      data: null,
      search: "",
      families: new Set(),
      categories: new Set(),
      directories: new Set(),
      sort: "count",
      view: "file",
      fileLimit: FILE_PAGE_SIZE,
      flatLimit: FLAT_PAGE_SIZE,
      groupLimits: new Map(),
      openGroups: new Set(),
      downloadUrl: null,
    };
  }

  function getState(main) {
    let state = stateByMain.get(main);
    if (!state) {
      state = createState(main);
      stateByMain.set(main, state);
    }
    return state;
  }

  function rootFor(state) {
    return state?.main?.querySelector?.("#cleanup-index") || null;
  }

  function element(main, id) {
    return main?.querySelector?.(`#${id}`) || null;
  }

  function setText(main, id, value) {
    const node = element(main, id);
    if (node) node.textContent = value;
  }

  function searchWords(state) {
    return state.search.trim().toLowerCase().split(/\s+/).filter(Boolean);
  }

  function siteSearchText(site) {
    return [
      site.path,
      site.directory,
      site.category,
      site.categoryLabel,
      familyLabel(site.family),
      site.detail,
      site.source,
      site.offset,
    ].join(" ").toLowerCase();
  }

  function siteMatches(site, state, ignore = "") {
    if (ignore !== "search" && !searchWords(state).every((word) => siteSearchText(site).includes(word))) return false;
    if (ignore !== "family" && state.families.size && !state.families.has(site.family)) return false;
    if (ignore !== "category" && state.categories.size && !state.categories.has(site.category)) return false;
    if (ignore !== "directory" && state.directories.size && !state.directories.has(site.directory)) return false;
    return true;
  }

  function filteredSites(state, ignore = "") {
    return state.data?.sites.filter((site) => siteMatches(site, state, ignore)) || [];
  }

  function groupSites(sites, state) {
    const groupsByPath = new Map();
    for (const site of sites) {
      let group = groupsByPath.get(site.path);
      if (!group) {
        group = { path: site.path, directory: site.directory, sites: [] };
        groupsByPath.set(site.path, group);
      }
      group.sites.push(site);
    }
    const groups = Array.from(groupsByPath.values());
    groups.sort((a, b) => state.sort === "path"
      ? a.path.localeCompare(b.path, undefined, { numeric: true })
      : (b.sites.length - a.sites.length) || a.path.localeCompare(b.path, undefined, { numeric: true }));
    return groups;
  }

  function activeFilterCount(state) {
    return state.families.size + state.categories.size + state.directories.size + (state.search.trim() ? 1 : 0);
  }

  function sourceHref(site, data) {
    if (!data?.commit || !site?.path || !site.line) return "";
    const path = site.path.replace(/^\/+/, "").split("/").map((part) => encodeURIComponent(part)).join("/");
    return `${REPOSITORY}/blob/${encodeURIComponent(data.commit)}/${path}#L${site.line}`;
  }

  function lineLabel(site) {
    return site.endLine > site.line ? `L${site.line}–${site.endLine}` : `L${site.line}`;
  }

  function normalizeOffset(value) {
    if (typeof value === "number" && Number.isFinite(value)) {
      return `+0x${Math.trunc(value).toString(16).toUpperCase()}`;
    }
    const text = asText(value);
    if (/^\d+$/.test(text)) return `+0x${Number(text).toString(16).toUpperCase()}`;
    return text;
  }

  function renderSite(site, data, options = {}) {
    const href = sourceHref(site, data);
    const line = href
      ? `<a class="cleanup-site-link" href="${escapeHtml(href)}" target="_blank" rel="noreferrer">${escapeHtml(lineLabel(site))}<span class="visually-hidden"> in ${escapeHtml(site.path)}</span></a>`
      : `<span class="cleanup-site-line">${escapeHtml(lineLabel(site))}</span>`;
    const detail = site.detail ? `<span class="cleanup-site-detail">${escapeHtml(site.detail)}</span>` : "";
    const source = site.source ? `<code class="cleanup-site-source">${escapeHtml(site.source)}</code>` : "";
    const offset = site.offset ? `<span class="cleanup-site-offset">${escapeHtml(site.offset)}</span>` : "";
    const path = options.showPath ? `<span class="cleanup-site-path mono">${escapeHtml(site.path)}</span>` : "";
    return `<article class="cleanup-site-row" data-family="${escapeHtml(site.family)}">
      <div class="cleanup-site-head">
        ${path}
        <span class="cleanup-site-category" data-family="${escapeHtml(site.family)}" title="${escapeHtml(site.categoryDescription)}">${escapeHtml(site.categoryLabel)}</span>
        ${detail}
        ${offset}
      </div>
      ${line}
      ${source}
      <span class="cleanup-site-family">${escapeHtml(familyLabel(site.family))}</span>
    </article>`;
  }

  function groupBody(group, state) {
    const limit = state.groupLimits.get(group.path) || GROUP_PAGE_SIZE;
    const shown = group.sites.slice(0, limit);
    const remaining = group.sites.length - shown.length;
    return `<div class="cleanup-group-body">
      ${shown.map((site) => renderSite(site, state.data)).join("")}
      ${remaining > 0 ? `<div class="cleanup-group-more"><span>${formatNumber(remaining)} more site${remaining === 1 ? "" : "s"} in this file</span><button class="button button-quiet" type="button" data-cleanup-group-more="${escapeHtml(group.path)}">Show more</button></div>` : ""}
    </div>`;
  }

  function renderFileGroup(group, state) {
    const open = state.openGroups.has(group.path);
    const body = open
      ? groupBody(group, state)
      : `<div class="cleanup-group-closed">Open this file to inspect its ${formatNumber(group.sites.length)} indexed site${group.sites.length === 1 ? "" : "s"}.</div>`;
    return `<details class="cleanup-file-group" data-cleanup-file="${escapeHtml(group.path)}"${open ? " open" : ""}>
      <summary>
        <span class="cleanup-group-chevron" aria-hidden="true">›</span>
        <span class="cleanup-group-path">${escapeHtml(group.path)}</span>
        <span class="cleanup-group-badge">${formatNumber(group.sites.length)}</span>
      </summary>
      ${body}
    </details>`;
  }

  function renderFlatSite(site, state) {
    return renderSite(site, state.data, { showPath: true });
  }

  function currentDirectories(state) {
    const counts = new Map();
    for (const site of filteredSites(state, "directory")) counts.set(site.directory, (counts.get(site.directory) || 0) + 1);
    const entries = Array.from(counts, ([id, count]) => ({ id, count }));
    entries.sort((a, b) => (b.count - a.count) || a.id.localeCompare(b.id, undefined, { numeric: true }));
    for (const selected of state.directories) {
      if (!entries.some((entry) => entry.id === selected)) {
        const count = filteredSites(state, "directory").filter((site) => site.directory === selected).length;
        entries.push({ id: selected, count });
      }
    }
    return entries;
  }

  function facetOption(kind, id, label, count, checked, family = "") {
    const name = `cleanup-${kind}`;
    const dataFamily = family ? ` data-family="${escapeHtml(family)}"` : "";
    return `<label class="cleanup-facet-option"${dataFamily}>
      <input type="checkbox" name="${name}" value="${escapeHtml(id)}" data-cleanup-filter="${kind}"${checked ? " checked" : ""} />
      <span class="cleanup-facet-option-text"><span>${escapeHtml(label)}</span><span class="cleanup-facet-count">${formatNumber(count)}</span></span>
    </label>`;
  }

  function renderFacets(state) {
    const familyRoot = element(state.main, "cleanup-family-facets");
    const categoryRoot = element(state.main, "cleanup-category-facets");
    const directoryRoot = element(state.main, "cleanup-directory-facets");
    if (!familyRoot || !categoryRoot || !directoryRoot) return;
    if (!state.data) {
      familyRoot.innerHTML = '<span class="cleanup-facet-empty">Scan unavailable.</span>';
      categoryRoot.innerHTML = '<span class="cleanup-facet-empty">Scan unavailable.</span>';
      directoryRoot.innerHTML = '<span class="cleanup-facet-empty">Scan unavailable.</span>';
      return;
    }

    familyRoot.innerHTML = FAMILY_DEFS.map((family) => facetOption(
      "family",
      family.id,
      family.label,
      filteredSites(state, "family").filter((site) => site.family === family.id).length,
      state.families.has(family.id),
      family.id,
    )).join("");

    const categoryCounts = new Map();
    for (const site of filteredSites(state, "category")) categoryCounts.set(site.category, (categoryCounts.get(site.category) || 0) + 1);
    const categories = state.data.categories
      .filter((category) => state.data.sites.some((site) => site.category === category.id) || state.categories.has(category.id))
      .map((category) => ({ category, count: categoryCounts.get(category.id) || 0 }))
      .sort((a, b) => (b.count - a.count) || a.category.label.localeCompare(b.category.label));
    categoryRoot.innerHTML = categories.length
      ? categories.map(({ category, count }) => facetOption("category", category.id, category.label, count, state.categories.has(category.id), category.family)).join("")
      : '<span class="cleanup-facet-empty">No categories in this scan.</span>';

    const directories = currentDirectories(state);
    directoryRoot.innerHTML = directories.length
      ? directories.map((entry) => facetOption("directory", entry.id, entry.id, entry.count, state.directories.has(entry.id))).join("")
      : '<span class="cleanup-facet-empty">No directories in this scan.</span>';
  }

  function syncOpenGroups(state) {
    const root = rootFor(state);
    if (!root) return;
    for (const detail of root.querySelectorAll("details[data-cleanup-file]")) {
      const path = detail.dataset.cleanupFile;
      if (!path) continue;
      if (detail.open) state.openGroups.add(path);
      else state.openGroups.delete(path);
    }
  }

  function updateBulkControls(state, groups, sites) {
    const expand = element(state.main, "cleanup-expand-all");
    const collapse = element(state.main, "cleanup-collapse-all");
    const manageable = state.view === "file" && groups.length > 0 && groups.length <= BULK_GROUP_LIMIT && sites.length <= BULK_SITE_LIMIT;
    if (expand) {
      expand.disabled = !manageable;
      expand.title = manageable ? "Open every visible file group" : "Narrow the filters to expand groups";
    }
    if (collapse) collapse.disabled = state.view !== "file" || state.openGroups.size === 0;
  }

  function renderResults(state) {
    const list = element(state.main, "cleanup-list");
    const empty = element(state.main, "cleanup-empty");
    const unavailable = element(state.main, "cleanup-unavailable");
    const more = element(state.main, "cleanup-more");
    const moreNote = element(state.main, "cleanup-more-note");
    const showMore = element(state.main, "cleanup-show-more");
    if (!list || !empty || !unavailable || !more || !moreNote || !showMore) return;
    if (!state.data) {
      list.innerHTML = "";
      empty.hidden = true;
      unavailable.hidden = false;
      more.hidden = true;
      updateBulkControls(state, [], []);
      return;
    }

    const sites = filteredSites(state);
    const groups = groupSites(sites, state);
    unavailable.hidden = true;
    empty.hidden = sites.length > 0;
    if (state.view === "flat") {
      const groupRank = new Map(groups.map((group, index) => [group.path, index]));
      const orderedSites = sites.slice().sort((a, b) => state.sort === "path"
        ? a.path.localeCompare(b.path, undefined, { numeric: true }) || (a.line - b.line)
        : (groupRank.get(a.path) - groupRank.get(b.path)) || (a.line - b.line));
      const visible = orderedSites.slice(0, state.flatLimit);
      list.className = "cleanup-flat-list";
      list.innerHTML = visible.map((site) => renderFlatSite(site, state)).join("");
      more.hidden = orderedSites.length <= visible.length;
      moreNote.textContent = `Showing ${formatNumber(visible.length)} of ${formatNumber(orderedSites.length)} sites.`;
      showMore.textContent = "Show more sites";
      updateBulkControls(state, groups, sites);
      return;
    }

    const visibleGroups = groups.slice(0, state.fileLimit);
    list.className = "cleanup-file-list";
    list.innerHTML = visibleGroups.map((group) => renderFileGroup(group, state)).join("");
    more.hidden = groups.length <= visibleGroups.length;
    moreNote.textContent = `Showing ${formatNumber(visibleGroups.length)} of ${formatNumber(groups.length)} files.`;
    showMore.textContent = "Show more files";
    updateBulkControls(state, groups, sites);
  }

  function renderSummary(state) {
    const data = state.data;
    if (!data) {
      setText(state.main, "cleanup-site-count", "—");
      setText(state.main, "cleanup-file-count", "—");
      setText(state.main, "cleanup-scanned-count", "—");
      setText(state.main, "cleanup-site-note", "Awaiting cleanup scan");
      setText(state.main, "cleanup-file-note", "Files represented by sites");
      setText(state.main, "cleanup-scanned-note", "Pinned source census");
      const commitLink = element(state.main, "cleanup-commit-link");
      if (commitLink) {
        commitLink.textContent = "Unavailable";
        commitLink.href = "#";
        commitLink.title = "Cleanup scan commit unavailable";
        commitLink.setAttribute("aria-disabled", "true");
      }
      return;
    }
    const fileCount = new Set(data.sites.map((site) => site.path)).size;
    setText(state.main, "cleanup-site-count", formatNumber(data.sites.length));
    setText(state.main, "cleanup-file-count", formatNumber(fileCount));
    setText(state.main, "cleanup-scanned-count", formatNumber(data.scannedFiles));
    setText(state.main, "cleanup-site-note", data.generatedAt ? `Generated ${formatDate(data.generatedAt)}` : "Syntactic candidates in the scan");
    setText(state.main, "cleanup-file-note", "Files represented by indexed sites");
    setText(state.main, "cleanup-scanned-note", data.scannedFiles === null ? "Scan total unavailable" : "Files in the pinned census");
    const commitLink = element(state.main, "cleanup-commit-link");
    if (commitLink) {
      if (data.commit) {
        const shortCommit = data.commit.length > 12 ? `${data.commit.slice(0, 12)}…` : data.commit;
        commitLink.textContent = shortCommit;
        commitLink.title = data.commit;
        commitLink.href = `${REPOSITORY}/tree/${encodeURIComponent(data.commit)}`;
        commitLink.setAttribute("aria-disabled", "false");
      } else {
        commitLink.textContent = "Unavailable";
        commitLink.href = "#";
        commitLink.title = "Cleanup scan commit unavailable";
        commitLink.setAttribute("aria-disabled", "true");
      }
    }
  }

  function formatDate(value) {
    const date = new Date(value);
    if (Number.isNaN(date.getTime())) return value;
    return new Intl.DateTimeFormat("en", { dateStyle: "medium" }).format(date);
  }

  function renderMethodology(state) {
    const status = element(state.main, "cleanup-methodology-status");
    const body = element(state.main, "cleanup-methodology-body");
    if (!status || !body) return;
    if (!state.data) {
      status.textContent = "Scan notes unavailable";
      body.innerHTML = "";
      return;
    }
    status.textContent = state.data.methodology.length
      ? `${state.data.methodology.length} scan note${state.data.methodology.length === 1 ? "" : "s"}`
      : "Candidate index";
    const notes = state.data.methodology.length
      ? state.data.methodology
      : [
        "Rows are syntactic cleanup candidates emitted by the pinned source scan.",
        "A candidate is not proof that an expression is incorrect, unsafe, or required to change.",
        "Counts describe indexed sites and source files; they do not change recovery coverage totals.",
      ];
    body.innerHTML = `<ul>${notes.map((note) => `<li>${escapeHtml(note)}</li>`).join("")}</ul>`;
  }

  function revokeDownload(state) {
    if (!state.downloadUrl) return;
    try { window.URL?.revokeObjectURL(state.downloadUrl); } catch { /* URL APIs may be unavailable in tests. */ }
    state.downloadUrl = null;
  }

  function tsvValue(value) {
    return String(value ?? "").replace(/\t/g, " ").replace(/\r?\n/g, " ").trim();
  }

  function updateDownload(state, sites) {
    const link = element(state.main, "cleanup-download");
    if (!link) return;
    revokeDownload(state);
    if (!state.data || !sites.length || typeof Blob === "undefined" || !window.URL?.createObjectURL) {
      link.href = "#";
      link.setAttribute("aria-disabled", "true");
      link.removeAttribute("data-cleanup-ready");
      return;
    }
    const lines = ["path\tline\tendLine\tcategory\tfamily\tdetail\tsource\toffset"];
    for (const site of sites) {
      lines.push([
        site.path,
        site.line,
        site.endLine,
        site.category,
        site.family,
        site.detail,
        site.source,
        site.offset,
      ].map(tsvValue).join("\t"));
    }
    state.downloadUrl = window.URL.createObjectURL(new Blob([`${lines.join("\n")}\n`], { type: "text/tab-separated-values;charset=utf-8" }));
    link.href = state.downloadUrl;
    link.setAttribute("aria-disabled", "false");
    link.setAttribute("data-cleanup-ready", "true");
  }

  function syncControls(state) {
    const search = element(state.main, "cleanup-search");
    const sort = element(state.main, "cleanup-sort");
    const fileButton = element(state.main, "cleanup-view-file");
    const flatButton = element(state.main, "cleanup-view-flat");
    if (search && search.value !== state.search) search.value = state.search;
    if (sort && sort.value !== state.sort) sort.value = state.sort;
    if (fileButton) fileButton.setAttribute("aria-pressed", String(state.view === "file"));
    if (flatButton) flatButton.setAttribute("aria-pressed", String(state.view === "flat"));
  }

  function renderResultStatus(state, sites, groups) {
    const count = element(state.main, "cleanup-result-count");
    const note = element(state.main, "cleanup-result-note");
    if (!count || !note) return;
    const allSites = state.data?.sites.length || 0;
    if (!state.data) {
      count.textContent = "No cleanup scan loaded.";
      note.textContent = "Candidate sites will appear when the snapshot includes a scan.";
      return;
    }
    count.textContent = state.view === "flat"
      ? `Showing ${formatNumber(sites.length)} of ${formatNumber(allSites)} sites`
      : `Showing ${formatNumber(groups.length)} file${groups.length === 1 ? "" : "s"} · ${formatNumber(sites.length)} site${sites.length === 1 ? "" : "s"}`;
    const filterCount = activeFilterCount(state);
    const hint = state.view === "file" ? "Open a file to inspect its sites." : "Source links use the pinned commit.";
    note.textContent = filterCount
      ? `${formatNumber(filterCount)} filter${filterCount === 1 ? "" : "s"} active · ${hint}`
      : hint;
  }

  function renderState(state, options = {}) {
    const root = rootFor(state);
    if (!root) return;
    if (options.syncOpen !== false) syncOpenGroups(state);
    syncControls(state);
    renderSummary(state);
    renderFacets(state);
    const sites = filteredSites(state);
    const groups = groupSites(sites, state);
    renderResultStatus(state, sites, groups);
    renderResults(state);
    renderMethodology(state);
    updateDownload(state, sites);
  }

  function resetPaging(state) {
    state.fileLimit = FILE_PAGE_SIZE;
    state.flatLimit = FLAT_PAGE_SIZE;
    state.groupLimits.clear();
  }

  function clearFilters(state) {
    state.search = "";
    state.families.clear();
    state.categories.clear();
    state.directories.clear();
    resetPaging(state);
    renderState(state);
    element(state.main, "cleanup-search")?.focus();
  }

  function materializeOpenGroup(state, detail, path) {
    if (detail.querySelector(".cleanup-group-body")) return;
    const group = groupSites(filteredSites(state), state).find((candidate) => candidate.path === path);
    if (!group) return;
    const closed = detail.querySelector(".cleanup-group-closed");
    const body = groupBody(group, state);
    if (closed) closed.outerHTML = body;
    else detail.insertAdjacentHTML("beforeend", body);
  }

  function focusFilter(state, kind, value) {
    const root = rootFor(state);
    if (!root) return;
    const input = Array.from(root.querySelectorAll("input[data-cleanup-filter]"))
      .find((candidate) => candidate.dataset.cleanupFilter === kind && candidate.value === value);
    input?.focus?.({ preventScroll: true });
  }

  function toggleFilter(state, kind, value, checked) {
    const target = kind === "family" ? state.families : kind === "category" ? state.categories : state.directories;
    if (!target) return;
    if (checked) target.add(value);
    else target.delete(value);
    resetPaging(state);
    renderState(state);
    focusFilter(state, kind, value);
  }

  function handleToggle(event, state) {
    const detail = event.target;
    if (!detail?.matches?.("details[data-cleanup-file]")) return;
    const path = detail.dataset.cleanupFile;
    if (!path) return;
    if (detail.open) state.openGroups.add(path);
    else state.openGroups.delete(path);
    if (detail.open) materializeOpenGroup(state, detail, path);
    else updateBulkControls(state, groupSites(filteredSites(state), state), filteredSites(state));
  }

  function handleClick(event, state) {
    const target = event.target.closest?.("button, a");
    if (!target) return;
    if (target.id === "cleanup-download" && target.getAttribute("aria-disabled") === "true") {
      event.preventDefault();
      return;
    }
    if (target.id === "cleanup-view-file" || target.id === "cleanup-view-flat") {
      const view = target.id.endsWith("flat") ? "flat" : "file";
      if (state.view === view) return;
      syncOpenGroups(state);
      state.view = view;
      resetPaging(state);
      renderState(state);
      return;
    }
    if (target.id === "cleanup-reset") {
      event.preventDefault();
      clearFilters(state);
      return;
    }
    if (target.id === "cleanup-expand-all") {
      const groups = groupSites(filteredSites(state), state);
      if (groups.length > BULK_GROUP_LIMIT || filteredSites(state).length > BULK_SITE_LIMIT) return;
      state.openGroups = new Set(groups.map((group) => group.path));
      renderState(state, { syncOpen: false });
      return;
    }
    if (target.id === "cleanup-collapse-all") {
      state.openGroups.clear();
      renderState(state, { syncOpen: false });
      return;
    }
    if (target.id === "cleanup-show-more") {
      if (state.view === "flat") state.flatLimit += FLAT_PAGE_SIZE;
      else state.fileLimit += FILE_PAGE_SIZE;
      renderState(state);
      return;
    }
    const groupMore = target.dataset.cleanupGroupMore;
    if (groupMore) {
      state.openGroups.add(groupMore);
      state.groupLimits.set(groupMore, (state.groupLimits.get(groupMore) || GROUP_PAGE_SIZE) + GROUP_PAGE_SIZE);
      renderState(state);
    }
  }

  function handleInput(event, state) {
    if (event.target?.id !== "cleanup-search") return;
    state.search = event.target.value || "";
    resetPaging(state);
    renderState(state);
  }

  function handleChange(event, state) {
    const target = event.target;
    if (target?.matches?.("input[data-cleanup-filter]")) {
      toggleFilter(state, target.dataset.cleanupFilter, target.value, target.checked);
      return;
    }
    if (target?.id === "cleanup-sort") {
      state.sort = target.value === "path" ? "path" : "count";
      resetPaging(state);
      renderState(state);
    }
  }

  function handleSubmit(event) {
    event.preventDefault();
  }

  function bind(main) {
    if (!main?.querySelector) return;
    const root = main.querySelector("#cleanup-index");
    if (!root) return;
    const state = getState(main);
    activeMain = main;
    activeState = state;
    if (state.bound) return;
    state.bound = true;
    const filters = element(main, "cleanup-filters-disclosure");
    if (filters && window.matchMedia?.("(max-width: 760px)")?.matches) filters.open = false;
    root.addEventListener("submit", handleSubmit);
    root.addEventListener("input", (event) => handleInput(event, state));
    root.addEventListener("change", (event) => handleChange(event, state));
    root.addEventListener("click", (event) => handleClick(event, state));
    root.addEventListener("toggle", (event) => handleToggle(event, state), true);
  }

  function render(snapshot) {
    const currentMain = document.getElementById?.("main-content") || document.querySelector?.("main#main-content");
    // Prefer the document's active main after route swaps; fall back to the
    // bound owner for small DOM harnesses that do not expose getElementById.
    const main = currentMain || activeMain;
    if (!main?.querySelector?.("#cleanup-index")) return;
    const state = stateByMain.get(main) || getState(main);
    activeMain = main;
    activeState = state;
    state.data = normalizeCleanup(snapshot);
    renderState(state);
  }

  window.MP6Cleanup = { bind, render };
})();
