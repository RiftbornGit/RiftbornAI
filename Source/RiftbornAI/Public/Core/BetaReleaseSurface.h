// Copyright RiftbornAI. All Rights Reserved.
//
// BetaReleaseSurface — the LOCKED beta surface that ships with the
// public Beta. Same authoritative list as
// Bridge/toolbook/public_surface.json (`beta_release_tools`); the static
// test in Tests/static/test_beta_release_surface_lock.py asserts the C++
// and JSON copies stay in lockstep.
//
// End users in a Beta build see ONLY these beta-release tools (99 in the
// current public Beta). Developers (the team
// building games on top of the plugin) flip URiftbornSettings::bDeveloperMode
// to bypass the lock and get the full ~700-tool surface back.
//
// Why duplicate the list in C++? Because the in-editor copilot's tool list
// is built from FClaudeToolRegistry, which doesn't depend on the MCP server
// or its JSON. Both paths must enforce the same lock for the lock to mean
// anything — JSON gates the MCP path, this header gates the in-editor path.

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"

namespace RiftbornBetaReleaseSurface
{
	/** The exact beta-release names. Kept inline in the .cpp so the test can
	 *  read them via the public API rather than parsing the .h file. */
	RIFTBORNAI_API TArrayView<const TCHAR* const> GetLockedToolNames();

	/** O(1) membership test. Lazily builds and caches the FName set on
	 *  first call. Pass the canonical resolved tool name (after alias
	 *  resolution); aliases must hit the canonical name to count. */
	RIFTBORNAI_API bool IsToolAllowed(const FString& ToolName);

	/** True when the running build should enforce the beta-release lock.
	 *
	 *  Resolution order:
	 *    1. Env var `RIFTBORN_DEV_MODE=true`            → false (dev override)
	 *    2. `URiftbornSettings::bDeveloperMode = true`  → false (dev override)
	 *    3. Env var `RIFTBORN_BETA_RELEASE_LOCK=false`  → false (explicit unlock)
	 *    4. Env var `RIFTBORN_BETA_RELEASE_LOCK=true`   → true  (explicit lock)
	 *    5. `URiftbornSettings::bBetaReleaseLockActive` → that value
	 *    6. Default → false (no lock — safe default for source builds)
	 *
	 *  The shipped Beta build sets the setting to true at install time so
	 *  end users get the locked surface even without an env var. Source
	 *  builds default to false so contributors aren't artificially limited. */
	RIFTBORNAI_API bool IsBetaReleaseLockActive();

	/** Filter `Tools` in place to only include the beta-release names. No-op when
	 *  the lock isn't active. Returns the new size for caller convenience. */
	RIFTBORNAI_API int32 FilterIfLockActive(TArray<FClaudeTool>& Tools);
}
