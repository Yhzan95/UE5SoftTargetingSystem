using UnrealBuildTool;

public class SoftTargetingExamples : ModuleRules
{
	public SoftTargetingExamples(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// This module exists so the example classes can depend on EnhancedInput without dragging
		// that dependency into the core targeting module. Remove the module from
		// SoftTargeting.uplugin to strip every example from a shipping build.
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"EnhancedInput",
				"GameplayTags",
				"SoftTargeting"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
			}
		);

		// Motion warping is opt in. To enable it:
		//   1. Enable the Motion Warping plugin in the .uproject
		//   2. Uncomment the two lines below
		//   3. Add a UMotionWarpingComponent to the character
		//   4. Add a Motion Warping notify state to the attack montages
		// PublicDependencyModuleNames.Add("MotionWarping");
		// PublicDefinitions.Add("SOFTTARGETING_WITH_MOTION_WARPING=1");
	}
}
