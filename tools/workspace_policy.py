#!/usr/bin/env python3
"""Permanent branch-boundary defaults for the AI recovery workspace."""

AI_WORKSPACE_BRANCH = "agent/recovery-context-workflow"
HUMAN_MAIN_BRANCH = "main"

# Worker branches and their diff proofs stay rooted in the AI workspace.
DEFAULT_WORKER_BASE = AI_WORKSPACE_BRANCH

# Only clean source promotion starts from the human-facing branch.
DEFAULT_PROMOTION_BASE = HUMAN_MAIN_BRANCH
