#!/usr/bin/env python3
"""Permanent branch-boundary defaults for the AI recovery workspace."""

AI_WORKSPACE_BRANCH = "agent/recovery-context-workflow"
HUMAN_MAIN_BRANCH = "main"

# Worker branches and their diff proofs stay rooted in the AI workspace.
DEFAULT_WORKER_BASE = AI_WORKSPACE_BRANCH

# Only clean source promotion starts from the human-facing branch.
DEFAULT_PROMOTION_BASE = HUMAN_MAIN_BRANCH

# The permanent AI-workspace orchestrator may maintain only recovery workflow
# infrastructure while it holds the ownerless integration resource. Game source,
# shared interfaces, retail configuration, symbols, and splits are intentionally
# absent from this list.
ORCHESTRATOR_WRITE_PATHS = (
    ".github/",
    "AGENTS.md",
    "AI_WORKSPACE.md",
    "CLAUDE.md",
    "CONTRIBUTING.md",
    "GEMINI.md",
    "benchmarks/blind_recovery/",
    "config/recovery/",
    "docs/",
    "tools/",
)
