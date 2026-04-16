// Copyright 2024-2026 RiftbornAI. All Rights Reserved.
// RegretScope.h - Umbrella header for scoped regret taxonomy
//
// This is the backwards-compatible entry point. Include this header to get
// the full regret system. Individual headers can also be included directly
// for faster compilation when only a subset is needed.
//
// REGRET SCOPE TAXONOMY:
// ----------------------
// Tactical   - This step was bad (immediate outcome failure)
// Strategic  - This plan regressed the goal (multi-step failure)
// Optionality - This reduced future options (closing doors)
// Deferred   - Bad now, but necessary for later success (acceptable sacrifice)
//
// The key insight: not all regret should be punished equally.
// A move that looks bad in isolation might be essential setup.
// A tool that fails once might be the only path forward.
//
// PERSISTENCE:
// Regret contexts are persisted to disk as JSONL for cross-session learning.
// This enables the agent to remember WHY penalties were applied, not just that they were.
//
// SUB-HEADERS:
// - RegretEnums.h           - ERegretScope, ECausalSurface, EExplorationTier enums + helpers
// - EnvironmentFingerprint.h - FEnvironmentFingerprint context capture and decay
// - RegretRecord.h          - FRegretEvent, FUnblockProof, FRegretClassifier
// - ExplorationContract.h   - FExplorationContract, FExplorationRegistry, FFailurePattern
// - RegretPersistence.h     - FRegretMemory JSONL persistence + hash chain + rotation

#pragma once

#include "RegretEnums.h"
#include "EnvironmentFingerprint.h"
#include "RegretRecord.h"
#include "ExplorationContract.h"
#include "RegretPersistence.h"
