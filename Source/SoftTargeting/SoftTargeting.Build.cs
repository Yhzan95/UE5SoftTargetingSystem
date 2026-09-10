using UnrealBuildTool;

public class SoftTargeting : ModuleRules
{
	public SoftTargeting(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// Kept deliberately minimal. This module must never grow a dependency on a game module,
		// on an input plugin, or on an animation plugin: it selects a target and nothing else.
		// GameplayTags is here only for the optional IGameplayTagAssetInterface based filtering.
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"GameplayTags"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
			}
		);
	}
}
