// Copyright RiftbornAI. All Rights Reserved.
// Shared startup/reload bootstrap for non-built-in extension tool surfaces.

#pragma once

#include "CoreMinimal.h"

class RIFTBORNAI_API FRiftbornExtensionToolBootstrap
{
public:
	/** Refresh the full governed/public tool surface: built-ins plus extensions. */
	static void RefreshPublicSurface();

	/** Refresh the full governed/public tool surface and return extension counts. */
	static void RefreshPublicSurface(int32& OutBlueprintCount, int32& OutUserCount);

	/** Refresh all non-built-in extension tool surfaces. */
	static void RefreshRegistrations();

	/** Refresh all non-built-in extension tool surfaces and return counts. */
	static void RefreshRegistrations(int32& OutBlueprintCount, int32& OutUserCount);
};
