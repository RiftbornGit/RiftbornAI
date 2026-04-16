// Copyright RiftbornAI. All Rights Reserved.
// Shared utilities for saving procedural meshes as UStaticMesh assets.

#pragma once

#include "CoreMinimal.h"

#if RIFTBORN_WITH_GEOMETRY_SCRIPTING

namespace UE::Geometry { class FDynamicMesh3; }
class UStaticMesh;

namespace RiftbornProceduralMeshUtils
{
	/**
	 * Save a FDynamicMesh3 as a UStaticMesh asset at the given content path.
	 * Handles package creation, Nanite settings, normal/tangent recomputation,
	 * asset registry notification, and package save.
	 * Returns the saved asset or nullptr on failure.
	 */
	RIFTBORNAI_API UStaticMesh* SaveDynamicMeshAsStaticMesh(
		UE::Geometry::FDynamicMesh3& Mesh,
		const FString& AssetPath,
		bool bNanite);
}

#endif
