#pragma once

#include "CoreMinimal.h"
#include "Misc/CommandLine.h"

inline bool IsRiftbornDevUIEnabled()
{
	const FString Env = FPlatformMisc::GetEnvironmentVariable(TEXT("RIFTBORN_DEV_UI"));
	return FParse::Param(FCommandLine::Get(), TEXT("RiftbornDevUI")) || (!Env.IsEmpty() && Env != TEXT("0"));
}
