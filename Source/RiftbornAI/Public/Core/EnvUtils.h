// Copyright 2024-2026 RiftbornAI. All Rights Reserved.
// Tiny helpers for parsing environment variables consistently across the
// C++ surface. Keep this header tight — anything richer than truthy parsing
// should live next to its caller.

#pragma once

#include "CoreMinimal.h"
#include "HAL/PlatformMisc.h"
#include "Misc/CString.h"

namespace RiftbornEnv
{
    /**
     * Returns true iff `Value` matches one of the canonical truthy spellings
     * `1`, `true`, `yes`, `on`, case-insensitively. Empty/whitespace -> false.
     *
     * The MCP server (TypeScript) mirrors this in
     * `mcp-server/src/utils/truthy.ts` — keep both lists in sync. RIFTBORN_DEV_MODE,
     * RIFTBORN_PROOF_MODE, RIFTBORN_ENABLE_INTERNAL_TOOLS and similar boolean
     * env knobs MUST flow through this helper so Bridge / SessionTaint /
     * BetaReleaseSurface / status endpoints all agree on the same truthy value.
     */
    inline bool IsTruthy(const FString& Value)
    {
        const FString Trimmed = Value.TrimStartAndEnd();
        if (Trimmed.IsEmpty())
        {
            return false;
        }
        return Trimmed.Equals(TEXT("1"), ESearchCase::CaseSensitive)
            || Trimmed.Equals(TEXT("true"), ESearchCase::IgnoreCase)
            || Trimmed.Equals(TEXT("yes"), ESearchCase::IgnoreCase)
            || Trimmed.Equals(TEXT("on"), ESearchCase::IgnoreCase);
    }

    /** Convenience: read env var by name and apply IsTruthy. */
    inline bool GetEnvTruthy(const TCHAR* VarName)
    {
        return IsTruthy(FPlatformMisc::GetEnvironmentVariable(VarName));
    }
}
