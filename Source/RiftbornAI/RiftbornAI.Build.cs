// Copyright RiftbornAI. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class RiftbornAI : ModuleRules
{
	// Check whether an optional UE module is present in the engine or project.
	// Works by scanning the engine Source/Runtime, Source/Editor, and Plugins trees,
	// plus the project's own Plugins tree.  No exceptions, no reflection hacks.
	private bool HasModule(ReadOnlyTargetRules Target, string ModuleName)
	{
		// EngineDirectory differs across UBT contexts: some expose Engine/,
		// others Engine/Source. Normalize before probing runtime/editor/plugins.
		string EngineDir = Path.GetFullPath(EngineDirectory);
		if (Directory.Exists(Path.Combine(EngineDir, "Runtime")) &&
			!Directory.Exists(Path.Combine(EngineDir, "Source")))
		{
			EngineDir = Path.GetFullPath(Path.Combine(EngineDir, ".."));
		}
		else if (!Directory.Exists(Path.Combine(EngineDir, "Source")) &&
			Directory.Exists(Path.Combine(EngineDir, "Engine", "Source")))
		{
			EngineDir = Path.Combine(EngineDir, "Engine");
		}

		string[] SearchRoots = new string[]
		{
			Path.Combine(EngineDir, "Source", "Runtime"),
			Path.Combine(EngineDir, "Source", "Editor"),
			Path.Combine(EngineDir, "Plugins"),
		};

		foreach (string Root in SearchRoots)
		{
			if (!Directory.Exists(Root)) continue;
			try
			{
				string[] Matches = Directory.GetFiles(Root, ModuleName + ".Build.cs", SearchOption.AllDirectories);
				if (Matches.Length > 0) return true;
			}
			catch (System.Exception)
			{
				// Permission or IO error — skip this root
			}
		}

		// Also check project-level Plugins folder
		if (Target.ProjectFile != null)
		{
			string ProjectPlugins = Path.Combine(Target.ProjectFile.Directory.FullName, "Plugins");
			if (Directory.Exists(ProjectPlugins))
			{
				try
				{
					string[] Matches = Directory.GetFiles(ProjectPlugins, ModuleName + ".Build.cs", SearchOption.AllDirectories);
					if (Matches.Length > 0) return true;
				}
				catch (System.Exception) { }
			}
		}

		// Fallback: known engine modules that may not be found via directory scan
		// (e.g., Niagara lives under Plugins/FX/Niagara, StateTree under Plugins/StateTree, etc.)
		// If we get here and the module name is a known UE5 module, assume present.
		string[] KnownEngineModules = new string[]
		{
			"Niagara", "PCG", "GameplayAbilities", "StateTreeModule",
			"MetasoundEngine", "MetasoundFrontend", "MetasoundEditor",
			"MaterialEditor", "LevelSequenceEditor", "PCGEditor",
			"MovieRenderPipelineCore", "MovieRenderPipelineSettings", "MovieRenderPipelineEditor", "MovieRenderPipelineRenderPasses",
			"GeometryScriptingCore", "GeometryFramework", "GeometryCore",
			"IKRig", "IKRigEditor", "ControlRig", "ControlRigEditor", "ControlRigDeveloper", "RigVMDeveloper",
			"OnlineSubsystem", "OnlineSubsystemUtils",
			"PythonScriptPlugin", "NiagaraEditor",
			"FractureEngine", "StateTreeEditorModule",
			"LevelSnapshots", "LevelSnapshotsEditor",
			"MassCrowd", "MassSpawner", "ZoneGraph",
			"ChaosMover", "Mover",
			"PCGGeometryScriptInterop",
			"VariantManager", "VariantManagerContent", "VariantManagerContentEditor",
			"RemoteControl",
			"SmartObjectsModule",
			"USDStage", "USDStageEditor",
			"GameplayCameras", "GameplayCamerasEditor",
			"MLDeformerFramework", "MLDeformerFrameworkEditor",
			"NeuralMorphModel", "NeuralMorphModelEditor",
			"PoseSearch", "PoseSearchEditor",
			"Chooser", "ProxyTable",
			"DataRegistry", "DataRegistryEditor",
			"ContextualAnimation", "ContextualAnimationEditor",
			"LearningAgents", "LearningAgentsTraining", "LearningAgentsTrainingEditor", "LearningTraining",
			"TakeRecorder", "TakeRecorderSources", "TakesCore",
			"LiveLink", "LiveLinkComponents", "LiveLinkInterface",
			"IrisCore",
			"WorldConditions", "WorldConditionsEditor",
			"ClothingSystemEditorInterface", "ClothingSystemEditor",
		};
		foreach (string Known in KnownEngineModules)
		{
			if (ModuleName == Known) return true;
		}

		return false;
	}

	// Conditionally add an optional module group.  Adds the modules to
	// PrivateDependencyModuleNames and sets RIFTBORN_WITH_<DefineTag>=1 (or 0).
	private bool AddOptionalModules(ReadOnlyTargetRules Target, string DefineTag, params string[] ModuleNames)
	{
		foreach (string Mod in ModuleNames)
		{
			if (!HasModule(Target, Mod))
			{
				PublicDefinitions.Add("RIFTBORN_WITH_" + DefineTag + "=0");
				return false;
			}
		}
		PrivateDependencyModuleNames.AddRange(ModuleNames);
		PublicDefinitions.Add("RIFTBORN_WITH_" + DefineTag + "=1");
		return true;
	}

	public RiftbornAI(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		// Unity builds cause ODR violations across 30+ satellite files.
		// Disabled for clean compilation. Each TU compiles independently.
		bUseUnity = false;

		// Private include paths — internal-only headers (not shipped in SDK).
		// IP-sensitive agent, governance, and provider internals live under Private/.
		PrivateIncludePaths.AddRange(new string[] {
			Path.Combine(ModuleDirectory, "Private"),
			Path.Combine(ModuleDirectory, "Private/Agent"),
		});

		// Public include paths for reorganized subdirectory structure
		PublicIncludePaths.AddRange(new string[] {
			Path.Combine(ModuleDirectory, "Public"),
			Path.Combine(ModuleDirectory, "Public/Agent"),
			Path.Combine(ModuleDirectory, "Public/Arena"),
			Path.Combine(ModuleDirectory, "Public/AssetPipeline"),
			Path.Combine(ModuleDirectory, "Public/Awareness"),
			Path.Combine(ModuleDirectory, "Public/Brain"),
			Path.Combine(ModuleDirectory, "Public/Bridge"),
			Path.Combine(ModuleDirectory, "Public/CodeGen"),
			Path.Combine(ModuleDirectory, "Public/Core"),
			Path.Combine(ModuleDirectory, "Public/EditorModes"),
			Path.Combine(ModuleDirectory, "Public/Gameplay"),
			Path.Combine(ModuleDirectory, "Public/Landscape"),
			Path.Combine(ModuleDirectory, "Public/Governance"),
			Path.Combine(ModuleDirectory, "Public/Packaging"),
			Path.Combine(ModuleDirectory, "Public/Project"),
			Path.Combine(ModuleDirectory, "Public/Providers"),
			Path.Combine(ModuleDirectory, "Public/Tools"),
			Path.Combine(ModuleDirectory, "Public/UI"),
			Path.Combine(ModuleDirectory, "Public/VFX"),
			Path.Combine(ModuleDirectory, "Public/VoxelTerrain"),
			Path.Combine(ModuleDirectory, "Public/RiftLumen"),
			Path.Combine(ModuleDirectory, "Public/SystemicWorld"),
		});

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"HTTP",
				"Json",
				"JsonUtilities",
				"SSL",
				"OpenSSL",  // Cross-platform SHA256/HMAC (Linux/Mac build farms)
			}
		);

		// Runtime dependencies (always required)
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"InputCore",
				"Projects",
				"DesktopPlatform",
				"Sockets",
				"Networking",
				"UMG",
				"AssetRegistry",
				"AIModule",
				"GameplayTags",
				"DeveloperSettings",
				"EnhancedInput",
				"NavigationSystem",
				"LevelSequence",
				"MovieScene",
				"MovieSceneTracks",
				"Landscape",
				"Foliage",
				"AudioMixerCore", // Import lib provides Audio::NAME_* FName symbols
				"ImageWrapper",
				"RenderCore",
				"RHI",
				"Renderer",
				"RiftbornAIBoot",
				"GameplayTasks",
				"PhysicsCore",
				"PhysicsUtilities",
				"Chaos",
				"AnimGraphRuntime",
				"ApplicationCore",
				"MeshDescription",
				"StaticMeshDescription",
				"GeometryCollectionEngine", // Chaos Geometry Collection runtime
				"FieldSystemEngine",     // Chaos Field System (force/kill/sleep fields)
				"CinematicCamera",       // CineCameraActor for cinematics tools
				"ClothingSystemRuntimeInterface", // Cloth sim base (UClothingAssetBase)
				"ClothingSystemRuntimeCommon",    // Cloth sim common (UClothingAssetCommon, ClothConfig)
				"MeshConversion",            // FMeshDescriptionBuilder for mesh export
				"DataflowCore",              // FDataflowSelection for Chaos fracture
				"InteractiveToolsFramework", // UInteractiveTool, input behaviors (runtime module, editor mode uses it)
				"SignificanceManager",       // USignificanceManager for Observation Bubble actor prioritization
				"ProceduralMeshComponent",   // UProceduralMeshComponent for voxel terrain chunks with working character collision
			}
		);

		// ---------------------------------------------------------------
		// Optional runtime modules — guarded by plugin availability
		// Each group sets RIFTBORN_WITH_<TAG>=1 or 0 as a PublicDefinition.
		// C++ code must use #if RIFTBORN_WITH_<TAG> around related includes.
		// ---------------------------------------------------------------

		// Niagara is a hard dependency for the editor module. Large parts of the
		// VFX/snapshot surface compile directly against Niagara runtime types, so
		// treating it as optional leaves the plugin in a compile-broken state.
		PrivateDependencyModuleNames.Add("Niagara");
		PublicDefinitions.Add("RIFTBORN_WITH_NIAGARA=1");

		AddOptionalModules(Target, "PCG", "PCG");

		AddOptionalModules(Target, "GAS", "GameplayAbilities");

		AddOptionalModules(Target, "STATE_TREE", "StateTreeModule");

		AddOptionalModules(Target, "METASOUND",
			"MetasoundEngine", "MetasoundFrontend");

		AddOptionalModules(Target, "MOVIE_RENDER_PIPELINE",
			"MovieRenderPipelineCore", "MovieRenderPipelineSettings");

		AddOptionalModules(Target, "GEOMETRY_SCRIPTING",
			"GeometryScriptingCore", "GeometryFramework", "GeometryCore");

		// DynamicMesh: FGroupTopology + FGroupEdgeInserter (runtime module).
		// Required by GeometryScriptToolsModule_Modeling for insert_edge_loop.
		AddOptionalModules(Target, "DYNAMIC_MESH", "DynamicMesh");

		AddOptionalModules(Target, "IKRIG", "IKRig");

		AddOptionalModules(Target, "ONLINE_SUBSYSTEM",
			"OnlineSubsystem", "OnlineSubsystemUtils");

		AddOptionalModules(Target, "WATER", "Water");

		AddOptionalModules(Target, "POSE_SEARCH", "PoseSearch");

		AddOptionalModules(Target, "CABLE_COMPONENT", "CableComponent");

		AddOptionalModules(Target, "LEVEL_SNAPSHOTS", "LevelSnapshots");

		AddOptionalModules(Target, "MASS_CROWD",
			"MassCrowd", "MassSpawner", "ZoneGraph");

		// Variant Manager needs both runtime/content modules plus the editor content module for
		// asset and actor authoring. The tool implementation uses exported content APIs only.
		if (HasModule(Target, "VariantManager") &&
			HasModule(Target, "VariantManagerContent") &&
			Target.bBuildEditor &&
			HasModule(Target, "VariantManagerContentEditor"))
		{
			PrivateDependencyModuleNames.AddRange(new string[]
			{
				"VariantManager",
				"VariantManagerContent",
				"VariantManagerContentEditor",
			});
			PublicDefinitions.Add("RIFTBORN_WITH_VARIANT_MANAGER=1");
		}
		else
		{
			PublicDefinitions.Add("RIFTBORN_WITH_VARIANT_MANAGER=0");
		}

		AddOptionalModules(Target, "REMOTE_CONTROL", "RemoteControl");

		AddOptionalModules(Target, "SMART_OBJECTS", "SmartObjectsModule");

		AddOptionalModules(Target, "CHOOSER", "Chooser", "ProxyTable");

		AddOptionalModules(Target, "DATA_REGISTRY", "DataRegistry");

		AddOptionalModules(Target, "CONTEXTUAL_ANIMATION", "ContextualAnimation");

		AddOptionalModules(Target, "LEARNING_AGENTS", "LearningAgents", "LearningAgentsTraining", "LearningTraining");

		AddOptionalModules(Target, "LIVE_LINK", "LiveLink", "LiveLinkComponents", "LiveLinkInterface");

		AddOptionalModules(Target, "IRIS", "IrisCore");

		AddOptionalModules(Target, "USD_STAGE", "USDStage");

		AddOptionalModules(Target, "WORLD_CONDITIONS", "WorldConditions");

		AddOptionalModules(Target, "GAMEPLAY_CAMERAS", "GameplayCameras");

		AddOptionalModules(Target, "ML_DEFORMER",
			"MLDeformerFramework", "NeuralMorphModel");

		// ChaosMover is available in UE 5.7. RiftbornAI routes its launch and
		// clear-override tool paths through public Mover/ChaosMover APIs rather
		// than relying on cross-module direct linking.
		AddOptionalModules(Target, "CHAOS_MOVER", "ChaosMover", "Mover");

		AddOptionalModules(Target, "PCG_GEOMETRY_INTEROP",
			"PCGGeometryScriptInterop");

		// Editor-only dependencies
		if (Target.bBuildEditor)
		{
			// Always-required editor modules
			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					"UnrealEd",
					"EditorSubsystem",
					"EditorFramework",
					"LevelEditor",
					"ToolMenus",
					"EditorScriptingUtilities",
					"UMGEditor",
					"Kismet",
					"KismetWidgets",
					"BlueprintGraph",
					"KismetCompiler",
					"GraphEditor",
					"AssetTools",
					"MessageLog",
					"LandscapeEditor",
					"SourceControl",
					"GameProjectGeneration",
					"HotReload",
					"LiveCoding",
					"DirectoryWatcher",
					"HTTPServer",
					"ContentBrowser",
					"AnimGraph",              // AnimBP graph nodes (UAnimStateNode, transitions)
					"AnimationBlueprintEditor", // AnimBP editor APIs
					"EditorInteractiveToolsFramework", // UEdMode-level tool context integration
					"PropertyEditor",         // IDetailsView for tool property panels
					"MeshReductionInterface", // IMeshReduction for LOD generation
				}
			);

			// Optional editor modules — mirror their runtime counterparts
			AddOptionalModules(Target, "PYTHON", "PythonScriptPlugin");

			// NiagaraEditor depends on Niagara runtime being present
			if (HasModule(Target, "NiagaraEditor"))
			{
				PrivateDependencyModuleNames.Add("NiagaraEditor");
			}

			AddOptionalModules(Target, "STATE_TREE_EDITOR", "StateTreeEditorModule");

			AddOptionalModules(Target, "METASOUND_EDITOR", "MetasoundEditor");

			AddOptionalModules(Target, "MATERIAL_EDITOR", "MaterialEditor");

			AddOptionalModules(Target, "GAMEPLAYTAGS_EDITOR", "GameplayTagsEditor");

			AddOptionalModules(Target, "LEVEL_SEQUENCE_EDITOR", "LevelSequenceEditor");

			AddOptionalModules(Target, "PCG_EDITOR", "PCGEditor");

			AddOptionalModules(Target, "CONTROL_RIG", "ControlRig");

			AddOptionalModules(Target, "CONTROL_RIG_AUTHORING",
				"ControlRigEditor", "ControlRigDeveloper", "RigVMDeveloper");

			// IKRigEditor depends on IKRig runtime being present
			AddOptionalModules(Target, "IKRIG_EDITOR", "IKRigEditor");

			AddOptionalModules(Target, "MOVIE_RENDER_PIPELINE_EDITOR",
				"MovieRenderPipelineEditor", "MovieRenderPipelineRenderPasses");

			AddOptionalModules(Target, "FRACTURE", "FractureEngine");

			AddOptionalModules(Target, "LEVEL_SNAPSHOTS_EDITOR", "LevelSnapshotsEditor");

			AddOptionalModules(Target, "USD_STAGE_EDITOR", "USDStageEditor");

			AddOptionalModules(Target, "TAKE_RECORDER",
				"TakeRecorder", "TakeRecorderSources", "TakesCore");

			AddOptionalModules(Target, "DATA_REGISTRY_EDITOR", "DataRegistryEditor");

			AddOptionalModules(Target, "CONTEXTUAL_ANIMATION_EDITOR", "ContextualAnimationEditor");

			AddOptionalModules(Target, "LEARNING_AGENTS_EDITOR", "LearningAgentsTrainingEditor");

			AddOptionalModules(Target, "WORLD_CONDITIONS_EDITOR", "WorldConditionsEditor");

			AddOptionalModules(Target, "CLOTHING_EDITOR",
				"ClothingSystemEditorInterface", "ClothingSystemEditor");

			AddOptionalModules(Target, "GAMEPLAY_CAMERAS_EDITOR", "GameplayCamerasEditor");

			AddOptionalModules(Target, "ML_DEFORMER_EDITOR",
				"MLDeformerFrameworkEditor", "NeuralMorphModelEditor");

			AddOptionalModules(Target, "POSE_SEARCH_EDITOR", "PoseSearchEditor");

			// ModelingComponentsEditorOnly: FSubdividePoly (Catmull-Clark / Loop / Bilinear).
			// Required by GeometryScriptToolsModule_Modeling for apply_mesh_subdivision.
			AddOptionalModules(Target, "MODELING_COMPONENTS_EDITOR_ONLY",
				"ModelingComponentsEditorOnly");
			
			// =================================================================
			// TEST HOOKS MACRO - Controls availability of test injection APIs
			// =================================================================
			// RIFTBORN_WITH_TEST_HOOKS=1 enables:
			//   - URiftbornLogWatcher::InjectTestMessage()
			//   - Test RPC commands (test_force_tick_stale, test_clear_taint, etc.)
			//   - Test-related INLINE_COMMANDS
			//
			// This is ONLY defined for:
			//   - Editor builds (Target.bBuildEditor)
			//   - Debug or Development configurations
			//   - NOT Shipping, NOT Test (unless explicitly enabled)
			//
			// To disable test hooks even in editor, set RIFTBORN_DISABLE_TEST_HOOKS=1
			// in your build environment or target.
			// =================================================================
			bool bEnableTestHooks = false;
			
			// Only consider enabling in editor builds
			if (Target.bBuildEditor)
			{
				// Enable in Debug and Development, NOT in Test or Shipping
				if (Target.Configuration == UnrealTargetConfiguration.Debug ||
				    Target.Configuration == UnrealTargetConfiguration.Development ||
				    Target.Configuration == UnrealTargetConfiguration.DebugGame)
				{
					bEnableTestHooks = true;
				}
			}
			
			// Allow explicit override to disable
			// (for building "clean" editor binaries without test hooks)
			// Set RIFTBORN_DISABLE_TEST_HOOKS=1 in environment to force off
			if (System.Environment.GetEnvironmentVariable("RIFTBORN_DISABLE_TEST_HOOKS") == "1")
			{
				bEnableTestHooks = false;
			}
			
			if (bEnableTestHooks)
			{
				PublicDefinitions.Add("RIFTBORN_WITH_TEST_HOOKS=1");
			}
			else
			{
				PublicDefinitions.Add("RIFTBORN_WITH_TEST_HOOKS=0");
			}
		}
		else
		{
			// Non-editor builds: test hooks are NEVER available
			PublicDefinitions.Add("RIFTBORN_WITH_TEST_HOOKS=0");
			PublicDefinitions.Add("RIFTBORN_WITH_TAKE_RECORDER=0");
			PublicDefinitions.Add("RIFTBORN_WITH_DATA_REGISTRY_EDITOR=0");
			PublicDefinitions.Add("RIFTBORN_WITH_CONTEXTUAL_ANIMATION_EDITOR=0");
			PublicDefinitions.Add("RIFTBORN_WITH_LEARNING_AGENTS_EDITOR=0");
			PublicDefinitions.Add("RIFTBORN_WITH_WORLD_CONDITIONS_EDITOR=0");
		}
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
		);
	}
}
