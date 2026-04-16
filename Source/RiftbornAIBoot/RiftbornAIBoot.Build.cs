using UnrealBuildTool;

public class RiftbornAIBoot : ModuleRules
{
	public RiftbornAIBoot(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"Projects",
				"RenderCore",
				"Renderer",
				"RHI",
			}
		);
	}
}
