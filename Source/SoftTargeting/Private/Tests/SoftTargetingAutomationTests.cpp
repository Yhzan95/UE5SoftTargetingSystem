

#if WITH_DEV_AUTOMATION_TESTS

#include "SoftTargetingComponent.h"
#include "SoftTargetingTypes.h"
#include "Misc/AutomationTest.h"

namespace SoftTargeting::Tests
{
	class FComponentTestAccessor : public USoftTargetingComponent
	{
	public:
		static float EvaluateAngleScore(float AngleDegrees, float MaxAngleDegrees)
		{
			return AngleToScore(AngleDegrees, MaxAngleDegrees);
		}
	};

	IMPLEMENT_SIMPLE_AUTOMATION_TEST(
		FAngleScoreTest,
		"SoftTargeting.Scoring.AngleRamp",
		EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

	bool FAngleScoreTest::RunTest(const FString& Parameters)
	{
		TestEqual(TEXT("Straight ahead scores one"), FComponentTestAccessor::EvaluateAngleScore(0.0f, 90.0f), 1.0f);
		TestEqual(TEXT("Half angle scores one half"), FComponentTestAccessor::EvaluateAngleScore(45.0f, 90.0f), 0.5f);
		TestEqual(TEXT("Maximum angle scores zero"), FComponentTestAccessor::EvaluateAngleScore(90.0f, 90.0f), 0.0f);
		TestEqual(TEXT("Angles beyond the ramp stay clamped"), FComponentTestAccessor::EvaluateAngleScore(135.0f, 90.0f), 0.0f);
		return true;
	}

	IMPLEMENT_SIMPLE_AUTOMATION_TEST(
		FRequestDefaultsTest,
		"SoftTargeting.API.RequestDefaults",
		EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

	bool FRequestDefaultsTest::RunTest(const FString& Parameters)
	{
		const FSoftTargetingRequest Request;
		TestEqual(TEXT("Default range"), Request.MaxRange, 250.0f);
		TestEqual(TEXT("Default half aperture"), Request.MaxAngle, 90.0f);
		TestEqual(TEXT("Default tolerance"), Request.RangeTolerance, 80.0f);
		TestTrue(TEXT("Input direction enabled"), Request.bUseInputDirection);
		TestTrue(TEXT("Camera direction enabled"), Request.bUseCameraDirection);
		TestTrue(TEXT("Line of sight enabled"), Request.bRequireLineOfSight);
		TestTrue(TEXT("Persistence enabled"), Request.bApplyPersistence);
		TestTrue(TEXT("Target lock respected"), Request.bRespectTargetLock);
		TestTrue(TEXT("Successful request updates CurrentTarget"), Request.bUpdateCurrentTarget);
		return true;
	}
}

#endif
