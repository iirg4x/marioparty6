(() => {
  "use strict";

  const SNAPSHOT_ENDPOINT = "./snapshot.json";
  const reducedMotion = window.matchMedia("(prefers-reduced-motion: reduce)");
  const resultAnimations = new WeakMap();
  let detailExit = null;

  function animateResults(element) {
    if (!element || reducedMotion.matches || !element.animate) return;
    resultAnimations.get(element)?.cancel();
    resultAnimations.set(element, element.animate(
      [{ opacity: .55 }, { opacity: 1 }],
      { duration: 180, easing: "ease-out" },
    ));
  }

  function cancelDetailExit() {
    const animation = detailExit;
    detailExit = null;
    animation?.cancel();
    elements.detail?.classList.remove("is-closing");
  }

  function dismissDetail() {
    if (!elements.detail?.open || detailExit) return;
    if (reducedMotion.matches || !elements.detail.animate) {
      clearSelection();
      return;
    }
    elements.detail.classList.add("is-closing");
    const animation = elements.detail.animate(
      [{ opacity: 1, transform: "none" }, { opacity: 0, transform: "translateY(8px) scale(.985)" }],
      { duration: 140, easing: "ease-in", fill: "forwards" },
    );
    detailExit = animation;
    animation.finished.catch(() => {}).then(() => {
      if (detailExit === animation) clearSelection();
    });
  }

  reducedMotion.addEventListener("change", () => {
    if (reducedMotion.matches) document.getAnimations().forEach((animation) => animation.cancel());
  });

  const STATUS_META = {
    complete: {
      label: "Complete",
      shortLabel: "Complete",
      description: "All committed source-selection gates represented here are satisfied.",
      tone: "complete",
    },
    partial: {
      label: "Partial",
      shortLabel: "Partial",
      description: "Some committed source-selection evidence remains outstanding.",
      tone: "partial",
    },
    notRecovered: {
      label: "Not recovered",
      shortLabel: "Not recovered",
      description: "No source-selection recovery has been recorded for this module.",
      tone: "not-recovered",
    },
    unavailable: {
      label: "Unavailable",
      shortLabel: "Unavailable",
      description: "The snapshot does not contain enough evidence to assess this module.",
      tone: "unavailable",
    },
  };

  const CATEGORY_LABELS = {
    minigame: "Minigame",
    mode: "Mode",
    board: "Board",
    system: "System",
  };

  const SUMMARY_DEFS = [
    { key: "overall", label: "Overall", detail: "Whole game modules" },
    { key: "dol", label: "DOL", detail: "Main executable" },
    { key: "rel", label: "REL", detail: "Relocatable modules" },
    { key: "modes", label: "Modes", detail: "Mode modules" },
    { key: "minigames", label: "Minigames", detail: "Whole minigame modules" },
    { key: "boards", label: "Boards", detail: "Board modules" },
  ];

  const DOL_GROUP_DEFS = [
    { key: "game", label: "Game engine", roots: ["game", "libhu"] },
    { key: "board", label: "Board engine", roots: ["board"] },
    { key: "sdk", label: "Dolphin SDK", roots: ["dolphin"] },
    { key: "speech", label: "Speech recognition", roots: ["gssdk_lib"] },
    { key: "audio", label: "Audio", roots: ["musyx", "msm"] },
    {
      key: "runtime",
      label: "C / C++ runtime",
      roots: ["msl_c.ppceabi.bare.h", "runtime.ppceabi.h"],
    },
    {
      key: "debug",
      label: "Debug support",
      roots: ["trk_minnow_dolphin", "odemuexi2", "amcstubs", "odenotstub"],
    },
    { key: "compression", label: "Compression", roots: ["zlib"] },
    { key: "other", label: "Other", roots: [] },
  ];

  const state = {
    snapshot: null,
    modules: [],
    selectedId: null,
    query: "",
    stateFilter: "all",
    kindFilter: "all",
    categoryFilter: "all",
    functionFilter: "all",
    functionQuery: "",
    detailGroup: "all",
    detailTrigger: null,
    loading: false,
    hasLoaded: false,
  };

  const elements = {
    main: document.getElementById("main-content"),
    banner: document.getElementById("app-banner"),
    connection: document.getElementById("connection-state"),
    refresh: document.getElementById("refresh-button"),
    receiptCommit: document.getElementById("receipt-commit"),
    receiptSubject: document.getElementById("receipt-subject"),
    receiptCommitted: document.getElementById("receipt-committed-at"),
    receiptCommittedRaw: document.getElementById("receipt-committed-raw"),
    receiptRefreshed: document.getElementById("receipt-refreshed-at"),
    receiptRefreshedRaw: document.getElementById("receipt-refreshed-raw"),
    receiptRemote: document.getElementById("receipt-remote"),
    coverageAside: document.getElementById("coverage-aside"),
    summary: document.getElementById("summary-ribbon"),
    dolLedger: document.getElementById("dol-ledger"),
    countingNote: document.getElementById("counting-note"),
    countingNoteStatus: document.getElementById("counting-note-status"),
    countingNoteBody: document.getElementById("counting-note-body"),
    form: document.getElementById("filter-form"),
    search: document.getElementById("module-search"),
    stateFilter: document.getElementById("state-filter"),
    kindFilter: document.getElementById("kind-filter"),
    categoryFilter: document.getElementById("category-filter"),
    functionFilter: document.getElementById("function-filter"),
    clearFilters: document.getElementById("clear-filters"),
    libraryTotal: document.getElementById("library-total"),
    libraryCaption: document.getElementById("library-caption"),
    moduleList: document.getElementById("module-list"),
    moduleEmpty: document.getElementById("module-empty"),
    snapshotEmpty: document.getElementById("snapshot-empty"),
    detail: document.getElementById("module-detail"),
    detailPlaceholder: document.getElementById("detail-placeholder"),
    detailContent: document.getElementById("detail-content"),
  };

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

  function escapeHtml(value) {
    return String(value ?? "")
      .replace(/&/g, "&amp;")
      .replace(/</g, "&lt;")
      .replace(/>/g, "&gt;")
      .replace(/"/g, "&quot;")
      .replace(/'/g, "&#39;");
  }

  function normalizeState(value) {
    const raw = asText(value).replace(/[\s_-]+/g, "").toLowerCase();
    if (raw === "complete" || raw === "matching") return "complete";
    if (raw === "partial") return "partial";
    if (raw === "notrecovered" || raw === "missing" || raw === "none") {
      return "notRecovered";
    }
    return "unavailable";
  }

  function normalizeSelection(value) {
    const raw = asText(value).replace(/[\s_-]+/g, "").toLowerCase();
    if (raw === "matching" || raw === "selected" || raw === "sourceselected") {
      return "matching";
    }
    if (raw === "remaining" || raw === "unmatched") return "remaining";
    if (raw === "unowned" || raw === "ownerless") return "unowned";
    return "unknown";
  }

  function normalizeFunctionState(value) {
    const raw = asText(value).replace(/[\s_-]+/g, "").toLowerCase();
    if (raw === "matching" || raw === "selected") return "matching";
    if (raw === "remaining" || raw === "unmatched") return "remaining";
    return "unknown";
  }

  function normalizeMetric(value) {
    if (!isObject(value)) return null;
    const matched = finiteNumber(value.matched);
    const total = finiteNumber(value.total);
    let percent = finiteNumber(value.percent);
    if (total === 0) percent = null;
    if (percent === null && matched !== null && total !== null && total > 0) {
      percent = (matched / total) * 100;
    }
    if (matched === null && total === null && percent === null) return null;
    return { matched, total, percent };
  }

  function normalizeSources(value) {
    if (!Array.isArray(value)) return [];
    return value
      .map((source) => {
        if (typeof source === "string") return { label: source, href: "" };
        if (!isObject(source)) return null;
        const label = asText(source.label || source.path || source.name || source.source);
        return {
          label: label || "Unnamed source",
          href: asText(source.href || source.url),
        };
      })
      .filter(Boolean);
  }

  function normalizeFunction(value) {
    if (!isObject(value)) return null;
    return {
      name: asText(value.name, "Unnamed function"),
      address: asText(value.address, "—"),
      size: finiteNumber(value.size),
      state: normalizeFunctionState(value.state),
    };
  }

  function normalizeOwner(value) {
    if (!isObject(value)) return null;
    return {
      path: asText(value.path || value.owner || value.name, "Owner path unavailable"),
      selection: normalizeSelection(value.selection),
      code: normalizeMetric(value.code),
      data: normalizeMetric(value.data),
      bss: normalizeMetric(value.bss),
      functions: Array.isArray(value.functions)
        ? value.functions.map(normalizeFunction).filter(Boolean)
        : [],
    };
  }

  function normalizeModule(value, index) {
    const source = isObject(value) ? value : {};
    const id = asText(source.id, `module-${String(index + 1).padStart(3, "0")}`);
    const title = asText(source.title);
    const category = Object.prototype.hasOwnProperty.call(CATEGORY_LABELS, source.category)
      ? source.category
      : "system";
    const kind = source.kind === "DOL" ? "DOL" : source.kind === "REL" ? "REL" : "REL";
    const provenance = isObject(source.provenance) ? source.provenance : {};
    return {
      id,
      title,
      titleVerified: source.titleVerified === true,
      aliases: Array.isArray(source.aliases)
        ? source.aliases.map((alias) => asText(alias)).filter(Boolean)
        : [],
      category,
      kind,
      provenance: {
        sources: normalizeSources(provenance.sources),
        evidence: asText(provenance.evidence),
      },
      code: normalizeMetric(source.code),
      data: normalizeMetric(source.data),
      bss: normalizeMetric(source.bss),
      state: normalizeState(source.state),
      owners: Array.isArray(source.owners)
        ? source.owners.map(normalizeOwner).filter(Boolean)
        : [],
      notes: Array.isArray(source.notes)
        ? source.notes.map((note) => asText(note)).filter(Boolean)
        : [],
    };
  }

  function normalizeSummary(value) {
    if (!isObject(value)) return null;
    const counts = isObject(value.counts) ? value.counts : null;
    return {
      published: value.published === true,
      code: normalizeMetric(value.code),
      data: normalizeMetric(value.data),
      counts: counts
        ? {
            total: finiteNumber(counts.total),
            complete: finiteNumber(counts.complete),
            partial: finiteNumber(counts.partial),
            notRecovered: finiteNumber(counts.notRecovered),
            unavailable: finiteNumber(counts.unavailable),
          }
        : null,
    };
  }

  function normalizeSnapshot(value) {
    if (!isObject(value)) throw new Error("The site returned an invalid snapshot.");
    if (value.error && !value.schemaVersion) {
      throw new Error(asText(value.error, "The site returned an error."));
    }
    if (value.schemaVersion !== undefined && Number(value.schemaVersion) !== 1) {
      throw new Error(`Unsupported snapshot schema: ${asText(value.schemaVersion)}`);
    }
    const modules = Array.isArray(value.modules)
      ? value.modules.map(normalizeModule)
      : [];
    return {
      schemaVersion: Number(value.schemaVersion) || 1,
      version: asText(value.version, "GP6E01"),
      commit: asText(value.commit),
      committedAt: asText(value.committedAt),
      refreshedAt: asText(value.refreshedAt),
      commitSubject: asText(value.commitSubject),
      remote: asText(value.remote),
      summary: SUMMARY_DEFS.reduce((result, definition) => {
        result[definition.key] = normalizeSummary(value.summary?.[definition.key]);
        return result;
      }, {}),
      modules,
      notes: Array.isArray(value.notes)
        ? value.notes.map((note) => asText(note)).filter(Boolean)
        : [],
      validation: isObject(value.validation) ? value.validation : null,
    };
  }

  function formatNumber(value) {
    if (value === null || value === undefined || !Number.isFinite(value)) return "—";
    return Number.isInteger(value)
      ? value.toLocaleString("en-US")
      : value.toLocaleString("en-US", { maximumFractionDigits: 1 });
  }

  function formatPercent(value) {
    if (value === null || value === undefined || !Number.isFinite(value)) return "Unavailable";
    return `${value.toLocaleString("en-US", { maximumFractionDigits: 1 })}%`;
  }

  function metricRatio(metric) {
    if (!metric) return "Unavailable";
    if (metric.matched === null && metric.total === null) return "Unavailable";
    if (metric.total === 0) return "Unavailable";
    return `${formatNumber(metric.matched)} / ${formatNumber(metric.total)}`;
  }

  function metricHtml(metric, options = {}) {
    const label = options.label || "metric";
    if (!metric) {
      return `<span class="metric metric-unavailable" aria-label="${escapeHtml(label)} unavailable">Unavailable</span>`;
    }
    const percent = formatPercent(metric.percent);
    const ratio = metricRatio(metric);
    const meter = metric.percent === null
      ? '<span class="metric-meter is-unavailable" aria-hidden="true"><i></i></span>'
      : `<span class="metric-meter" aria-hidden="true"><i style="width:${Math.max(0, Math.min(100, metric.percent))}%"></i></span>`;
    return `<span class="metric" aria-label="${escapeHtml(label)} ${escapeHtml(percent)}, ${escapeHtml(ratio)}">
      <strong>${escapeHtml(percent)}</strong>${meter}<small>${escapeHtml(ratio)}</small>
    </span>`;
  }

  function metricCompactHtml(metric, options = {}) {
    const label = options.label || "metric";
    if (!metric) {
      return `<span class="compact-metric compact-unavailable" aria-label="${escapeHtml(label)} unavailable">Unavailable</span>`;
    }
    return `<span class="compact-metric" aria-label="${escapeHtml(label)} ${escapeHtml(formatPercent(metric.percent))}, ${escapeHtml(metricRatio(metric))}">
      <strong>${escapeHtml(formatPercent(metric.percent))}</strong>
      <small>${escapeHtml(metricRatio(metric))}</small>
    </span>`;
  }

  function statusMeta(value) {
    return STATUS_META[value] || STATUS_META.unavailable;
  }

  function statePill(value, options = {}) {
    const meta = statusMeta(value);
    return `<span class="state-pill state-${meta.tone}" data-state="${escapeHtml(value)}">
      <i class="state-dot" aria-hidden="true"></i>${escapeHtml(options.short ? meta.shortLabel : meta.label)}
    </span>`;
  }

  function selectionMeta(value) {
    const map = {
      matching: { label: "Source selected", tone: "complete" },
      remaining: { label: "Remaining", tone: "partial" },
      unowned: { label: "Unowned", tone: "not-recovered" },
      unknown: { label: "Unknown", tone: "unavailable" },
    };
    return map[value] || map.unknown;
  }

  function selectionPill(value) {
    const meta = selectionMeta(value);
    return `<span class="selection-pill selection-${meta.tone}"><i class="state-dot" aria-hidden="true"></i>${escapeHtml(meta.label)}</span>`;
  }

  function functionStatePill(value) {
    const labels = {
      matching: "Source selected",
      remaining: "Unmatched",
      unknown: "Unavailable",
    };
    const tone = value === "matching" ? "complete" : value === "remaining" ? "partial" : "unavailable";
    return `<span class="function-state function-${tone}">${escapeHtml(labels[value] || labels.unknown)}</span>`;
  }

  function primaryTitle(module) {
    return module.titleVerified && module.title ? module.title : module.id;
  }

  function displayId(module) {
    return module.id || "ID unavailable";
  }

  function categoryLabel(category) {
    return CATEGORY_LABELS[category] || "System";
  }

  function formatTimestamp(value) {
    if (!value) return "Unavailable";
    const date = new Date(value);
    if (Number.isNaN(date.getTime())) return value;
    return new Intl.DateTimeFormat(undefined, {
      dateStyle: "medium",
      timeStyle: "short",
    }).format(date);
  }

  function rawTimestamp(value) {
    return value || "—";
  }

  function setText(element, value) {
    if (element) element.textContent = value;
  }

  function setConnection(text, tone = "muted") {
    setText(elements.connection, text);
    if (elements.connection) elements.connection.dataset.tone = tone;
  }

  function setBanner(message, tone = "info") {
    if (!elements.banner) return;
    elements.banner.textContent = message;
    elements.banner.hidden = !message;
    elements.banner.dataset.tone = tone;
  }

  function setLoading(isLoading) {
    state.loading = isLoading;
    if (elements.refresh) {
      elements.refresh.disabled = isLoading;
      elements.refresh.setAttribute("aria-busy", String(isLoading));
      elements.refresh.innerHTML = isLoading
        ? '<span class="button-spinner" aria-hidden="true"></span>Reading snapshot'
        : '<span aria-hidden="true">↻</span>Reload snapshot';
    }
    if (elements.main) elements.main.setAttribute("aria-busy", String(isLoading));
  }

  function renderReceipt(snapshot) {
    const commit = snapshot?.commit || "Unavailable";
    setText(elements.receiptCommit, commit);
    if (elements.receiptCommit) elements.receiptCommit.title = commit;
    setText(elements.receiptSubject, snapshot?.commitSubject || "No commit subject supplied");

    const committed = snapshot?.committedAt || "";
    const refreshed = snapshot?.refreshedAt || "";
    setText(elements.receiptCommitted, formatTimestamp(committed));
    setText(elements.receiptCommittedRaw, rawTimestamp(committed));
    setText(elements.receiptRefreshed, formatTimestamp(refreshed));
    setText(elements.receiptRefreshedRaw, rawTimestamp(refreshed));
    if (elements.receiptCommitted) elements.receiptCommitted.dateTime = committed;
    if (elements.receiptRefreshed) elements.receiptRefreshed.dateTime = refreshed;
    setText(elements.receiptRemote, snapshot?.remote || "Unavailable");
  }

  function summaryCountsHtml(counts) {
    if (!counts) {
      return '<span class="summary-counts unavailable-counts">Whole-module counts unavailable</span>';
    }
    const parts = [];
    if (counts.total !== null) parts.push(`<strong>${escapeHtml(formatNumber(counts.total))}</strong> whole`);
    if (counts.complete !== null) parts.push(`<span class="count-complete">${escapeHtml(formatNumber(counts.complete))} complete</span>`);
    if (counts.partial !== null) parts.push(`<span class="count-partial">${escapeHtml(formatNumber(counts.partial))} partial</span>`);
    if (counts.notRecovered !== null) parts.push(`<span>${escapeHtml(formatNumber(counts.notRecovered))} not recovered</span>`);
    if (counts.unavailable !== null) parts.push(`<span>${escapeHtml(formatNumber(counts.unavailable))} unavailable</span>`);
    return `<span class="summary-counts">${parts.length ? parts.join('<i aria-hidden="true">·</i>') : "Whole-module counts unavailable"}</span>`;
  }

  function renderSummary(snapshot) {
    if (!elements.summary) return;
    elements.summary.innerHTML = SUMMARY_DEFS.filter((definition) => definition.key !== "dol").map((definition) => {
      const summary = snapshot?.summary?.[definition.key];
      if (!summary) {
        return `<article class="summary-entry summary-missing">
          <div class="summary-entry-head"><h3>${escapeHtml(definition.label)}</h3><span>${escapeHtml(definition.detail)}</span></div>
          <p class="summary-unavailable">Unavailable</p>
          <small>Published breakdown not supplied</small>
        </article>`;
      }
      return `<article class="summary-entry">
        <div class="summary-entry-head">
          <h3>${escapeHtml(definition.label)}</h3>
          <span>${escapeHtml(definition.detail)}</span>
          ${summary.published ? '<em class="published-mark">Published</em>' : ""}
        </div>
        <div class="summary-metrics">
          <div><span>Code</span>${metricHtml(summary.code, { label: `${definition.label} code` })}</div>
          <div><span>Data</span>${metricHtml(summary.data, { label: `${definition.label} data` })}</div>
        </div>
        ${summaryCountsHtml(summary.counts)}
        ${summaryFunctionsHtml(snapshot.modules, definition.key)}
      </article>`;
    }).join("");

    const overall = snapshot?.summary?.overall;
    let aside = overall?.published ? "Published official totals · current snapshot" : "Selection ledger · current snapshot";
    const validation = snapshot?.validation;
    if (isObject(validation)) {
      const status = asText(validation.status || validation.result);
      if (status) aside += ` · Validation ${status}`;
    }
    setText(elements.coverageAside, aside);
  }

  function summaryFunctionsHtml(modules, key) {
    const family = { minigames: "minigame", modes: "mode", boards: "board" }[key];
    const counts = { matching: 0, remaining: 0, unknown: 0 };
    for (const module of modules) {
      if (key === "dol" && module.kind !== "DOL") continue;
      if (key === "rel" && module.kind !== "REL") continue;
      if (family && module.category !== family) continue;
      for (const owner of module.owners) {
        for (const fn of owner.functions) counts[fn.state] += 1;
      }
    }
    const total = counts.matching + counts.remaining + counts.unknown;
    return `<div class="summary-functions">
      <div class="summary-functions-heading">Functions</div>
      <div class="summary-function-counts">
        <span><strong>${formatNumber(total)}</strong><span>Recorded</span></span>
        <span><strong>${formatNumber(counts.matching)}</strong><span>Source selected</span></span>
        <span><strong>${formatNumber(counts.remaining)}</strong><span>Unmatched</span></span>
        <span><strong>${formatNumber(counts.unknown)}</strong><span>Unavailable</span></span>
      </div>
    </div>`;
  }

  function firstPathComponent(path) {
    return asText(path)
      .replace(/\\/g, "/")
      .split("/")
      .map((part) => part.trim())
      .find(Boolean)
      ?.toLowerCase() || "";
  }

  function dolGroupKey(owner) {
    const component = firstPathComponent(owner?.path);
    return DOL_GROUP_DEFS.find((definition) => definition.roots.includes(component))?.key || "other";
  }

  function aggregateMetric(owners, key) {
    if (!owners.length) return null;
    let matched = 0;
    let total = 0;
    for (const owner of owners) {
      const metric = owner[key];
      if (!metric || !Number.isFinite(metric.matched) || !Number.isFinite(metric.total)) return null;
      matched += metric.matched;
      total += metric.total;
    }
    return {
      matched,
      total,
      percent: total > 0 ? (matched / total) * 100 : null,
    };
  }

  function countDolFunctions(owners) {
    const counts = { matching: 0, remaining: 0, unknown: 0 };
    for (const owner of owners) {
      for (const fn of owner.functions) {
        if (Object.prototype.hasOwnProperty.call(counts, fn.state)) counts[fn.state] += 1;
        else counts.unknown += 1;
      }
    }
    return {
      ...counts,
      total: counts.matching + counts.remaining + counts.unknown,
    };
  }

  function countDolFiles(owners) {
    const counts = { matching: 0, remaining: 0, unknown: 0 };
    for (const owner of owners) {
      if (owner.selection === "matching") counts.matching += 1;
      else if (owner.selection === "remaining") counts.remaining += 1;
      else counts.unknown += 1;
    }
    return {
      ...counts,
      total: counts.matching + counts.remaining + counts.unknown,
    };
  }

  function dolGroupEntries(module) {
    if (!module || module.kind !== "DOL") return [];
    const groups = new Map(DOL_GROUP_DEFS.map((definition) => [definition.key, {
      key: definition.key,
      label: definition.label,
      owners: [],
    }]));
    for (const owner of module.owners) groups.get(dolGroupKey(owner)).owners.push(owner);
    return Array.from(groups.values())
      .filter((group) => group.owners.length)
      .map((group) => ({
        ...group,
        code: aggregateMetric(group.owners, "code"),
        data: aggregateMetric(group.owners, "data"),
        files: countDolFiles(group.owners),
        functions: countDolFunctions(group.owners),
      }));
  }

  function dolGroupEntry(module, key) {
    if (!module || module.kind !== "DOL" || key === "all") return null;
    return dolGroupEntries(module).find((group) => group.key === key) || null;
  }

  function dolModule(snapshot = state.snapshot) {
    return snapshot?.modules?.find((module) => module.kind === "DOL" && module.id === "main.dol")
      || snapshot?.modules?.find((module) => module.kind === "DOL")
      || null;
  }

  function renderCountingNote(snapshot) {
    if (!elements.countingNote || !elements.countingNoteBody || !elements.countingNoteStatus) return;
    if (!snapshot) {
      elements.countingNote.hidden = true;
      return;
    }

    const reconciliation = isObject(snapshot.validation?.reconciliation)
      ? snapshot.validation.reconciliation
      : null;
    const reconciliationKeys = ["overall", "dol", "rel"];
    const statuses = reconciliationKeys.map((key) => ({
      key,
      value: reconciliation && typeof reconciliation[key] === "boolean" ? reconciliation[key] : null,
    }));
    const hasFailure = statuses.some((status) => status.value === false);
    const fullyKnown = statuses.every((status) => status.value !== null);
    const allPassed = fullyKnown && statuses.every((status) => status.value === true);
    const statusText = allPassed
      ? "Byte totals reconciled with published snapshot"
      : hasFailure
        ? "Published reconciliation incomplete"
        : "Reconciliation status unavailable";
    const statusTone = allPassed ? "success" : hasFailure ? "warning" : "muted";
    setText(elements.countingNoteStatus, statusText);
    elements.countingNoteStatus.dataset.tone = statusTone;

    const reconciliationHtml = reconciliation
      ? `<ul class="reconciliation-list">${statuses.map((status) => {
          const label = status.key === "dol" ? "DOL" : status.key === "rel" ? "REL" : "Overall";
          const value = status.value === true ? "Reconciled" : status.value === false ? "Needs review" : "Unavailable";
          const tone = status.value === true ? "success" : status.value === false ? "warning" : "muted";
          return `<li><span>${label}</span><strong data-tone="${tone}">${value}</strong></li>`;
        }).join("")}</ul>`
      : '<p class="inline-unavailable">Published reconciliation details unavailable.</p>';
    const notesHtml = snapshot.notes.length
      ? `<ul class="counting-notes-list">${snapshot.notes.map((note) => `<li>${escapeHtml(note)}</li>`).join("")}</ul>`
      : '<p class="inline-unavailable">No additional snapshot notes supplied.</p>';
    elements.countingNoteBody.innerHTML = `<p class="counting-note-explanation">Whole-module counts come from the module records. Owner splits and compiler translation units are evidence beneath a module and are never added to whole-module totals.</p>${reconciliationHtml}<div class="counting-notes-heading">Snapshot notes</div>${notesHtml}`;
    elements.countingNote.hidden = false;
  }

  function renderDolLedger(snapshot) {
    const ledger = elements.dolLedger;
    if (!ledger) return;
    const dol = dolModule(snapshot);
    if (!dol) {
      ledger.hidden = true;
      ledger.innerHTML = "";
      return;
    }

    const files = countDolFiles(dol.owners);
    const functions = countDolFunctions(dol.owners);
    const groups = dolGroupEntries(dol).sort((a, b) =>
      (b.functions.remaining - a.functions.remaining) || a.label.localeCompare(b.label));
    const functionUnknown = functions.unknown
      ? ` · ${formatNumber(functions.unknown)} unavailable`
      : "";
    const fileUnknown = files.unknown
      ? ` · ${formatNumber(files.unknown)} unavailable`
      : "";
    const rows = groups.map((group) => `<tr class="dol-row">
      <td class="dol-name">
        <button class="dol-group-select" type="button" data-dol-group="${escapeHtml(group.key)}" aria-label="Inspect DOL ${escapeHtml(group.label)}">
          <strong>${escapeHtml(group.label)}</strong><span aria-hidden="true">›</span>
        </button>
        <span class="dol-file-count">${formatNumber(group.files.matching)} / ${formatNumber(group.files.total)} source files selected</span>
      </td>
      <td class="dol-metric" data-label="Code">${metricCompactHtml(group.code, { label: `${group.label} code` })}</td>
      <td class="dol-metric" data-label="Data">${metricCompactHtml(group.data, { label: `${group.label} data` })}</td>
      <td class="dol-selected" data-label="Functions selected"><strong>${formatNumber(group.functions.matching)}</strong> / ${formatNumber(group.functions.total)}</td>
      <td class="dol-remaining" data-label="Unmatched">${formatNumber(group.functions.remaining)}${group.functions.unknown ? `<small>${formatNumber(group.functions.unknown)} unavailable</small>` : ""}</td>
    </tr>`).join("");

    ledger.hidden = false;
    ledger.innerHTML = `<div class="dol-heading section-heading-row">
      <div class="section-intro">
        <h2 id="dol-heading">DOL · Main executable</h2>
        <p>See what remains in the engine, board system, SDK, and supporting libraries.</p>
      </div>
      <div class="dol-actions">
        <button class="button button-primary" type="button" data-dol-group="all" data-dol-functions="remaining">Unmatched functions</button>
        <button class="button button-quiet" type="button" data-dol-group="all" data-dol-functions="all">Browse source files</button>
      </div>
    </div>
    <div class="dol-overview">
      <div class="dol-progress">
        <div><span>Code</span>${metricHtml(dol.code, { label: "DOL code coverage" })}</div>
        <div><span>Data</span>${metricHtml(dol.data, { label: "DOL data coverage" })}</div>
      </div>
      <div class="dol-totals">
        <div class="dol-stat">
          <span>Functions selected</span>
          <strong>${formatNumber(functions.matching)} <small>/ ${formatNumber(functions.total)}</small></strong>
          <span class="dol-remaining-count">${formatNumber(functions.remaining)} unmatched functions${functionUnknown}</span>
        </div>
        <div class="dol-stat">
          <span>Source files selected</span>
          <strong>${formatNumber(files.matching)} <small>/ ${formatNumber(files.total)}</small></strong>
          <span class="dol-remaining-count">${formatNumber(files.remaining)} files remaining${fileUnknown}</span>
        </div>
      </div>
    </div>
    <div class="dol-groups table-wrap">
      <table class="dol-table">
        <caption>DOL recovery by subsystem</caption>
        <thead><tr><th scope="col">Subsystem</th><th scope="col">Code</th><th scope="col">Data</th><th scope="col">Functions selected</th><th scope="col">Unmatched</th></tr></thead>
        <tbody>${rows || '<tr><td colspan="5"><p class="inline-unavailable">No DOL source-owner evidence is recorded.</p></td></tr>'}</tbody>
      </table>
    </div>
    <p class="dol-note">Subsystem totals are grouped by source file. Function status follows committed source selection.</p>`;
  }

  function searchableText(module) {
    const owners = module.owners.flatMap((owner) => [
      owner.path,
      ...owner.functions.map((fn) => fn.name),
    ]);
    return [
      module.id,
      module.title,
      ...module.aliases,
      module.category,
      module.kind,
      ...owners,
    ].join(" ").toLowerCase();
  }

  function filteredModules() {
    const query = state.query.trim().toLowerCase();
    return state.modules.filter((module) => {
      if (state.stateFilter !== "all" && module.state !== state.stateFilter) return false;
      if (state.kindFilter !== "all" && module.kind !== state.kindFilter) return false;
      if (state.categoryFilter !== "all" && module.category !== state.categoryFilter) return false;
      if (state.functionFilter !== "all" && !module.owners.some((owner) => owner.functions.some(functionStateMatches))) return false;
      return !query || searchableText(module).includes(query);
    });
  }

  function functionStateMatches(fn) {
    return state.functionFilter === "all" || fn.state === state.functionFilter;
  }

  function ownerPathMatches(owner) {
    const query = state.functionQuery.trim().toLowerCase();
    return Boolean(query) && owner.path.toLowerCase().includes(query);
  }

  function filteredOwnerFunctions(owner, options = {}) {
    const query = state.functionQuery.trim().toLowerCase();
    const selectedModule = state.modules.find((module) => module.id === state.selectedId);
    const allowOwnerPath = options.allowOwnerPath ?? selectedModule?.kind === "DOL";
    const pathMatch = allowOwnerPath && ownerPathMatches(owner);
    return owner.functions.filter((fn) => functionStateMatches(fn)
      && (!query || pathMatch || `${fn.name} ${fn.address}`.toLowerCase().includes(query)));
  }

  function functionFilterLabel() {
    return { all: "All functions", remaining: "Unmatched", matching: "Source selected", unknown: "Unavailable" }[state.functionFilter];
  }

  function moduleSort(a, b) {
    const kindOrder = { DOL: 0, REL: 1 };
    const categoryOrder = { system: 0, mode: 1, board: 2, minigame: 3 };
    return (kindOrder[a.kind] - kindOrder[b.kind])
      || (categoryOrder[a.category] - categoryOrder[b.category])
      || a.id.localeCompare(b.id, undefined, { numeric: true });
  }

  function renderModuleRow(module) {
    const selected = state.selectedId === module.id;
    const title = primaryTitle(module);
    const titleNote = module.titleVerified ? "Verified title" : module.title ? "Unverified label" : "ID only";
    const meta = statusMeta(module.state);
    return `<tr class="module-row${selected ? " is-selected" : ""}" data-row-module="${escapeHtml(module.id)}">
      <td class="module-cell">
        <button class="module-select" type="button" data-select-module="${escapeHtml(module.id)}" aria-label="Open ${escapeHtml(title)} (${escapeHtml(module.id)})">
          <span class="module-title-line"><strong>${escapeHtml(title)}</strong>${module.titleVerified ? '<span class="verified-mark" title="Verified game mapping" aria-label="Verified game mapping">✓</span>' : ""}</span>
          <span class="module-id mono">${escapeHtml(displayId(module))}</span>
        </button>
        <span class="module-title-note">${escapeHtml(titleNote)}</span>
        ${state.functionFilter !== "all" ? `<span class="module-title-note function-match-count">${formatNumber(module.owners.reduce((count, owner) => count + owner.functions.filter(functionStateMatches).length, 0))} ${escapeHtml(functionFilterLabel().toLowerCase())} functions</span>` : ""}
      </td>
      <td class="module-family"><span class="kind-label">${escapeHtml(module.kind)}</span><span>${escapeHtml(categoryLabel(module.category))}</span></td>
      <td class="module-metric numeric-cell" data-label="Code">${metricCompactHtml(module.code, { label: `${title} code` })}</td>
      <td class="module-metric numeric-cell" data-label="Data">${metricCompactHtml(module.data, { label: `${title} data` })}</td>
      <td class="module-state">${statePill(module.state, { short: true })}</td>
    </tr>`;
  }

  function renderModuleList(snapshot) {
    const allModules = snapshot?.modules || [];
    const filtered = filteredModules().sort(moduleSort);
    const hasSnapshot = state.hasLoaded && Boolean(snapshot);
    const noSnapshot = state.hasLoaded && !snapshot;
    if (elements.moduleList) {
      elements.moduleList.innerHTML = filtered.map(renderModuleRow).join("");
      if (!elements.detail?.open) animateResults(elements.moduleList);
    }
    if (elements.libraryTotal) setText(elements.libraryTotal, `${allModules.length.toLocaleString("en-US")} whole module${allModules.length === 1 ? "" : "s"}`);
    if (elements.libraryCaption) setText(elements.libraryCaption, `Showing ${filtered.length.toLocaleString("en-US")} of ${allModules.length.toLocaleString("en-US")} whole modules`);
    if (elements.moduleEmpty) elements.moduleEmpty.hidden = !hasSnapshot || allModules.length === 0 || filtered.length > 0;
    if (elements.snapshotEmpty) elements.snapshotEmpty.hidden = !noSnapshot;
  }

  function ownerFunctionHtml(fn) {
    return `<li class="function-line" data-function-state="${escapeHtml(fn.state)}">
      <span class="function-name mono" title="${escapeHtml(fn.name)}">${escapeHtml(fn.name)}</span>
      <span class="function-address mono">${escapeHtml(fn.address)}</span>
      <span class="function-size mono">${fn.size === null ? "size unavailable" : `${escapeHtml(formatNumber(fn.size))} B`}</span>
      ${functionStatePill(fn.state)}
    </li>`;
  }

  function ownerHtml(owner, index, options = {}) {
    const selection = selectionMeta(owner.selection);
    const title = owner.path || `Owner ${index + 1}`;
    const functions = filteredOwnerFunctions(owner, options);
    return `<details class="owner-entry">
      <summary>
        <span class="owner-summary-path mono">${escapeHtml(title)}</span>
        <span class="owner-summary-meta">${selectionPill(owner.selection)}<span class="owner-chevron" aria-hidden="true">›</span></span>
      </summary>
      <div class="owner-body">
        <div class="owner-evidence-head">
          <span>${escapeHtml(selection.label)} source owner</span>
          <span class="mono">${escapeHtml(title)}</span>
        </div>
        <div class="owner-metrics">
          <div><span>Code</span>${metricCompactHtml(owner.code, { label: `${title} code` })}</div>
          <div><span>Data</span>${metricCompactHtml(owner.data, { label: `${title} data` })}</div>
          <div><span>BSS</span>${metricCompactHtml(owner.bss, { label: `${title} BSS` })}</div>
        </div>
        <div class="owner-functions">
          <div class="subheading-row"><h5>Functions</h5><span>${owner.functions.length ? `${functions.length} of ${owner.functions.length} recorded` : "Unavailable"}</span></div>
          ${functions.length
            ? `<ul class="function-list">${functions.map(ownerFunctionHtml).join("")}</ul>`
            : '<p class="inline-unavailable">Function-level evidence unavailable.</p>'}
        </div>
      </div>
    </details>`;
  }

  function renderFunctionResults(module) {
    if (!module) return;
    const isDol = module.kind === "DOL";
    const group = isDol ? dolGroupEntry(module, state.detailGroup) : null;
    const scopedOwners = group ? group.owners : module.owners;
    const filtering = state.functionFilter !== "all" || state.functionQuery.trim() !== "";
    const owners = scopedOwners.filter((owner) => {
      const functions = filteredOwnerFunctions(owner, { allowOwnerPath: isDol });
      const pathMatch = isDol && ownerPathMatches(owner);
      return !filtering || functions.length > 0 || (pathMatch && state.functionFilter === "all");
    });
    const total = scopedOwners.reduce((count, owner) => count + owner.functions.length, 0);
    const shown = owners.reduce((count, owner) => count + filteredOwnerFunctions(owner, { allowOwnerPath: isDol }).length, 0);
    const scopeLabel = isDol
      ? `${group ? group.label : "All DOL subsystems"} · ${formatNumber(owners.length)} of ${formatNumber(scopedOwners.length)} source files`
      : "Whole module";
    setText(document.getElementById("function-result-count"), `Showing ${formatNumber(shown)} of ${formatNumber(total)} recorded functions · ${functionFilterLabel()} · ${scopeLabel}`);
    const results = document.getElementById("owner-results");
    if (results) results.innerHTML = owners.length ? owners.map((owner, index) => ownerHtml(owner, index, { allowOwnerPath: isDol })).join("")
      : `<div class="inline-empty">${total ? "No functions match these filters in this module." : "No function-level evidence is recorded for this module."}</div>`;
    if (elements.detail?.open) animateResults(results);
  }

  function updateFunctionFilter(value) {
    state.functionFilter = value;
    if (elements.functionFilter) elements.functionFilter.value = value;
    const detailFilter = document.getElementById("detail-function-filter");
    if (detailFilter) detailFilter.value = value;
    renderModuleList(state.snapshot);
    renderFunctionResults(state.modules.find((module) => module.id === state.selectedId));
  }

  function sourceHtml(source) {
    const label = source.label || "Unnamed source";
    if (/^https?:\/\//i.test(source.href)) {
      return `<li><a href="${escapeHtml(source.href)}" target="_blank" rel="noreferrer">${escapeHtml(label)}</a></li>`;
    }
    return `<li>${escapeHtml(label)}</li>`;
  }

  function renderDetail(module) {
    if (!elements.detailContent || !elements.detailPlaceholder) return;
    cancelDetailExit();
    if (!module) {
      elements.detailPlaceholder.hidden = false;
      elements.detailContent.hidden = true;
      if (elements.detail.open) elements.detail.close();
      document.body.classList.remove("dialog-open");
      elements.detail.classList.remove("detail-open");
      elements.detail.removeAttribute("aria-labelledby");
      elements.detail.setAttribute("aria-label", "Module detail");
      return;
    }
    const title = primaryTitle(module);
    const titleNote = module.titleVerified
      ? "Verified mapping"
      : module.title
        ? `Unverified label supplied: ${module.title}`
        : "Human-readable title unavailable";
    const isDol = module.kind === "DOL";
    const dolGroups = isDol ? dolGroupEntries(module) : [];
    if (!isDol || (state.detailGroup !== "all" && !dolGroups.some((group) => group.key === state.detailGroup))) {
      state.detailGroup = "all";
    }
    const matchingOwners = module.owners.filter((owner) => owner.selection === "matching").length;
    const remainingOwners = module.owners.filter((owner) => owner.selection === "remaining").length;
    const provenance = module.provenance;
    elements.detailPlaceholder.hidden = true;
    elements.detailContent.hidden = false;
    elements.detail.classList.add("detail-open");
    elements.detail.removeAttribute("aria-label");
    elements.detail.setAttribute("aria-labelledby", "detail-heading");
    elements.detailContent.innerHTML = `<div class="detail-header">
      <div class="detail-header-copy">
        <p class="section-index">${escapeHtml(module.kind)} / ${escapeHtml(categoryLabel(module.category))}</p>
        <h3 id="detail-heading">${escapeHtml(title)}</h3>
        <p class="detail-subtitle"><code>${escapeHtml(displayId(module))}</code><span>${escapeHtml(titleNote)}</span></p>
      </div>
      <button class="icon-button detail-close" id="detail-close" type="button" aria-label="Close module detail">×</button>
    </div>
    <div class="detail-state-row">
      ${statePill(module.state)}
      <span class="owner-count"><strong>${matchingOwners}</strong> source-selected owner${matchingOwners === 1 ? "" : "s"}</span>
      ${remainingOwners ? `<span class="owner-count remaining-count"><strong>${remainingOwners}</strong> remaining owner${remainingOwners === 1 ? "" : "s"}</span>` : ""}
    </div>
    <div class="detail-metric-grid">
      <div class="detail-metric"><span>Code coverage</span>${metricHtml(module.code, { label: `${title} code coverage` })}</div>
      <div class="detail-metric"><span>Data coverage</span>${metricHtml(module.data, { label: `${title} data coverage` })}</div>
      <div class="detail-metric"><span>BSS subset</span>${metricHtml(module.bss, { label: `${title} BSS` })}</div>
    </div>
    <p class="detail-metric-note">Data includes initialized data and BSS. BSS is shown separately because its gate can keep a module partial.</p>
    ${module.aliases.length ? `<div class="detail-line"><span>Aliases</span><span class="alias-list">${module.aliases.map((alias) => `<code>${escapeHtml(alias)}</code>`).join("")}</span></div>` : ""}
    <section class="detail-section owners-section" aria-labelledby="owners-heading">
      <div class="detail-section-heading"><div><p class="section-index">SOURCE EVIDENCE</p><h4 id="owners-heading">Owners and functions</h4></div><span class="detail-section-count">${module.owners.length ? `${module.owners.length} owner${module.owners.length === 1 ? "" : "s"}` : "Unavailable"}</span></div>
      <p class="detail-section-copy">Unmatched means the committed source owner is not selected. Unavailable functions have insufficient evidence. These labels follow owner selection; independent per-function objdiff proof is unavailable. Coverage totals above always describe the whole module.</p>
      <div class="function-filter-bar${isDol ? " with-dol-group" : ""}">
        <label><span>${isDol ? "Find a file or function" : "Find a function"}</span><input id="function-search" type="search" placeholder="${isDol ? "File path, function, or address" : "Function name or address"}" autocomplete="off" value="${escapeHtml(state.functionQuery)}" aria-controls="owner-results" /></label>
        <label><span>Show functions</span><select id="detail-function-filter" aria-controls="module-list owner-results">
          ${[["all", "All functions"], ["remaining", "Unmatched"], ["matching", "Source selected"], ["unknown", "Unavailable"]].map(([value, label]) => `<option value="${value}"${state.functionFilter === value ? " selected" : ""}>${label}</option>`).join("")}
        </select></label>
        ${isDol ? `<label><span>Subsystem</span><select id="detail-dol-group" aria-controls="owner-results">
          <option value="all"${state.detailGroup === "all" ? " selected" : ""}>All DOL subsystems</option>
          ${dolGroups.map((group) => `<option value="${escapeHtml(group.key)}"${state.detailGroup === group.key ? " selected" : ""}>${escapeHtml(group.label)}</option>`).join("")}
        </select></label>` : ""}
      </div>
      <p class="function-result-count" id="function-result-count" role="status"></p>
      <div class="owner-list" id="owner-results"></div>
    </section>
    <section class="detail-section provenance-section" aria-labelledby="provenance-heading">
      <div class="detail-section-heading"><div><p class="section-index">TRACE</p><h4 id="provenance-heading">Provenance</h4></div></div>
      ${provenance.sources.length ? `<ul class="source-list">${provenance.sources.map(sourceHtml).join("")}</ul>` : '<p class="inline-unavailable">Provenance sources unavailable.</p>'}
      ${provenance.evidence ? `<p class="evidence-copy">${escapeHtml(provenance.evidence)}</p>` : ""}
    </section>
    ${module.notes.length ? `<section class="detail-section notes-section" aria-labelledby="notes-heading"><div class="detail-section-heading"><div><p class="section-index">NOTES</p><h4 id="notes-heading">Caveats</h4></div></div><ul class="notes-list">${module.notes.map((note) => `<li>${escapeHtml(note)}</li>`).join("")}</ul></section>` : ""}`;
    renderFunctionResults(module);
    if (!elements.detail.open) {
      elements.detail.showModal();
      document.body.classList.add("dialog-open");
      elements.detailContent.scrollTop = 0;
      document.getElementById("detail-close")?.focus();
    }
  }

  function renderSnapshot() {
    const snapshot = state.snapshot;
    renderReceipt(snapshot);
    renderSummary(snapshot);
    renderCountingNote(snapshot);
    renderDolLedger(snapshot);
    renderModuleList(snapshot);
    renderDetail(state.modules.find((module) => module.id === state.selectedId) || null);
  }

  function selectModule(id, options = {}) {
    const module = state.modules.find((candidate) => candidate.id === id);
    if (!module) return;
    if (state.selectedId !== module.id) state.functionQuery = "";
    state.selectedId = module.id;
    state.detailGroup = "all";
    state.detailTrigger = options.trigger?.isConnected ? options.trigger : null;
    renderModuleList(state.snapshot);
    renderDetail(module);
  }

  function clearSelection() {
    cancelDetailExit();
    const previousId = state.selectedId;
    const previousTrigger = state.detailTrigger;
    state.selectedId = null;
    state.detailGroup = "all";
    state.detailTrigger = null;
    renderModuleList(state.snapshot);
    renderDetail(null);
    const fallback = Array.from(document.querySelectorAll('[data-select-module]'))
      .find((button) => button.dataset.selectModule === previousId);
    const focusTarget = previousTrigger?.isConnected ? previousTrigger : fallback;
    focusTarget?.focus({ preventScroll: true });
  }

  function openDolDetail(groupKey = "all", functionState = "all", trigger = null) {
    const module = dolModule();
    if (!module) return;
    const validGroup = groupKey === "all" || dolGroupEntry(module, groupKey) ? groupKey : "all";
    const validFunctionState = ["all", "remaining", "matching", "unknown"].includes(functionState)
      ? functionState
      : "all";
    state.selectedId = module.id;
    state.detailGroup = validGroup;
    state.detailTrigger = trigger?.isConnected ? trigger : null;
    state.functionQuery = "";
    state.functionFilter = validFunctionState;
    if (elements.functionFilter) elements.functionFilter.value = validFunctionState;
    renderModuleList(state.snapshot);
    renderDetail(module);
  }

  function handleDolLedgerClick(event) {
    const trigger = event.target.closest("[data-dol-group]");
    if (!trigger || !elements.dolLedger?.contains(trigger)) return;
    openDolDetail(trigger.dataset.dolGroup, trigger.dataset.dolFunctions || "all", trigger);
  }

  function handleSelection(event) {
    const trigger = event.target.closest("[data-select-module]");
    if (!trigger) return;
    selectModule(trigger.dataset.selectModule, { scroll: true, trigger });
  }

  function updateFilters() {
    state.query = elements.search?.value || "";
    state.stateFilter = elements.stateFilter?.value || "all";
    state.kindFilter = elements.kindFilter?.value || "all";
    state.categoryFilter = elements.categoryFilter?.value || "all";
    renderModuleList(state.snapshot);
  }

  function clearFilters() {
    state.query = "";
    state.stateFilter = "all";
    state.kindFilter = "all";
    state.categoryFilter = "all";
    state.functionFilter = "all";
    state.functionQuery = "";
    state.detailGroup = "all";
    if (elements.search) elements.search.value = "";
    if (elements.stateFilter) elements.stateFilter.value = "all";
    if (elements.kindFilter) elements.kindFilter.value = "all";
    if (elements.categoryFilter) elements.categoryFilter.value = "all";
    if (elements.functionFilter) elements.functionFilter.value = "all";
    renderModuleList(state.snapshot);
    renderDetail(state.modules.find((module) => module.id === state.selectedId) || null);
    elements.search?.focus();
  }

  async function readJson(response) {
    let body = null;
    try {
      body = await response.json();
    } catch {
      body = null;
    }
    if (!response.ok) {
      const message = isObject(body) && body.error ? body.error : `Site returned HTTP ${response.status}.`;
      throw new Error(asText(message, "The site returned an error."));
    }
    if (isObject(body) && body.error && !body.schemaVersion) {
      throw new Error(asText(body.error, "The site returned an error."));
    }
    return body;
  }

  async function fetchSnapshot(endpoint, options) {
    const response = await fetch(endpoint, {
      cache: "no-store",
      headers: { Accept: "application/json" },
      ...options,
    });
    return normalizeSnapshot(await readJson(response));
  }

  async function loadSnapshot(isRefresh = false) {
    if (state.loading) return;
    setLoading(true);
    setBanner(isRefresh ? "Reloading the published snapshot…" : "Reading the published snapshot…", "info");
    setConnection(isRefresh ? "Reloading published snapshot" : "Reading published snapshot", "muted");
    try {
      const snapshot = await fetchSnapshot(SNAPSHOT_ENDPOINT);
      state.snapshot = snapshot;
      state.modules = snapshot.modules;
      state.hasLoaded = true;
      if (!state.selectedId || !state.modules.some((module) => module.id === state.selectedId)) {
        state.selectedId = null;
        state.detailGroup = "all";
        state.detailTrigger = null;
      }
      renderSnapshot();
      setBanner(isRefresh ? "Published snapshot reloaded." : "Published snapshot loaded.", "success");
      setConnection("Published snapshot loaded", "success");
    } catch (error) {
      const message = error instanceof Error ? error.message : "Could not read the published snapshot.";
      if (state.snapshot) {
        setBanner(`Refresh failed: ${message} Previous snapshot remains in view.`, "error");
        setConnection("Refresh failed · previous snapshot", "error");
      } else {
        state.hasLoaded = true;
        renderSnapshot();
        setBanner(message, "error");
        setConnection("Snapshot unavailable", "error");
      }
    } finally {
      setLoading(false);
    }
  }

  function handleGlobalKeydown(event) {
    if (event.key === "Escape" && state.selectedId) {
      event.preventDefault();
      dismissDetail();
    }
  }

  function bindEvents() {
    elements.refresh?.addEventListener("click", () => loadSnapshot(true));
    elements.form?.addEventListener("submit", (event) => event.preventDefault());
    elements.search?.addEventListener("input", updateFilters);
    elements.stateFilter?.addEventListener("change", updateFilters);
    elements.kindFilter?.addEventListener("change", updateFilters);
    elements.categoryFilter?.addEventListener("change", updateFilters);
    elements.functionFilter?.addEventListener("change", (event) => updateFunctionFilter(event.target.value));
    elements.clearFilters?.addEventListener("click", clearFilters);
    elements.dolLedger?.addEventListener("click", handleDolLedgerClick);
    elements.moduleList?.addEventListener("click", handleSelection);
    elements.detail?.addEventListener("cancel", (event) => {
      event.preventDefault();
      dismissDetail();
    });
    elements.detail?.addEventListener("click", (event) => {
      if (event.target !== elements.detail) return;
      const rect = elements.detail.getBoundingClientRect();
      if (event.clientX < rect.left || event.clientX > rect.right || event.clientY < rect.top || event.clientY > rect.bottom) dismissDetail();
    });
    elements.detailContent?.addEventListener("click", (event) => {
      if (event.target.closest("#detail-close")) dismissDetail();
    });
    elements.detailContent?.addEventListener("input", (event) => {
      if (event.target.id !== "function-search") return;
      state.functionQuery = event.target.value;
      renderFunctionResults(state.modules.find((module) => module.id === state.selectedId));
    });
    elements.detailContent?.addEventListener("change", (event) => {
      if (event.target.id === "detail-function-filter") updateFunctionFilter(event.target.value);
      if (event.target.id === "detail-dol-group") {
        const module = state.modules.find((candidate) => candidate.id === state.selectedId);
        if (module?.kind !== "DOL") return;
        state.detailGroup = event.target.value === "all" || dolGroupEntry(module, event.target.value)
          ? event.target.value
          : "all";
        renderFunctionResults(module);
      }
    });
    document.addEventListener("keydown", handleGlobalKeydown);
  }

  function registerPageTools() {
    if (!document.modelContext?.registerTool) return;
    const lifecycle = new AbortController();
    window.addEventListener("pagehide", () => lifecycle.abort(), { once: true });
    const tool = {
      name: "filter_recovery_functions",
      title: "Filter recovery functions",
      description: "Filter the recovery ledger by function state, clearing other module filters. Optionally open a module's scrollable detail and search its functions by name or address. Changes only the visible page.",
      inputSchema: {
        type: "object",
        properties: {
          functionState: { type: "string", enum: ["all", "remaining", "matching", "unknown"] },
          moduleId: { type: "string" },
          functionQuery: { type: "string", maxLength: 200 },
        },
        required: ["functionState"],
        additionalProperties: false,
      },
      annotations: { readOnlyHint: false, untrustedContentHint: false },
      execute(input) {
        if (!isObject(input) || Object.keys(input).some((key) => !["functionState", "moduleId", "functionQuery"].includes(key)) || !["all", "remaining", "matching", "unknown"].includes(input.functionState)) throw new Error("Choose a valid function state.");
        if (!state.snapshot) throw new Error("The snapshot is still loading.");
        if (input.functionQuery !== undefined && (typeof input.functionQuery !== "string" || input.functionQuery.length > 200 || !input.moduleId)) throw new Error("A function query requires a module ID and at most 200 characters.");
        const module = input.moduleId === undefined ? null : state.modules.find((item) => item.id === input.moduleId);
        if (input.moduleId !== undefined && !module) throw new Error("Unknown module ID.");
        clearSelection();
        clearFilters();
        state.detailGroup = "all";
        updateFunctionFilter(input.functionState);
        if (module) {
          selectModule(module.id);
          state.functionQuery = input.functionQuery || "";
          document.getElementById("function-search").value = state.functionQuery;
          renderFunctionResults(module);
        }
        return { modules: elements.libraryCaption.textContent, functions: module ? document.getElementById("function-result-count").textContent : null };
      },
    };
    try {
      Promise.resolve(document.modelContext.registerTool(tool, { signal: lifecycle.signal })).catch(() => {});
    } catch { /* Unsupported registrations leave the normal controls available. */ }
  }

  bindEvents();
  registerPageTools();
  renderSnapshot();
  loadSnapshot(false);
})();
