

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/EngineTypes.h"
#include "GameplayTagContainer.h"
#include "SoftTargetingTypes.h"
#include "SoftTargetingComponent.generated.h"

class APlayerCameraManager;

DECLARE_LOG_CATEGORY_EXTERN(LogSoftTargeting, Log, All);


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSoftTargetChanged, AActor*, OldTarget, AActor*, NewTarget);


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSoftTargetFound, AActor*, NewTarget);


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSoftTargetLost, AActor*, OldTarget);


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSoftTargetActorEvent, AActor*, Target);


UCLASS(ClassGroup = (Combat), Blueprintable, meta = (BlueprintSpawnableComponent), HideCategories = (Activation, ComponentReplication, Cooking, Replication, AssetUserData))
class SOFTTARGETING_API USoftTargetingComponent : public UActorComponent
{
	GENERATED_BODY()

public:


	USoftTargetingComponent();


	UPROPERTY(BlueprintAssignable, Category = "Soft Targeting|Events")
	FOnSoftTargetChanged OnTargetChanged;


	UPROPERTY(BlueprintAssignable, Category = "Soft Targeting|Events")
	FOnSoftTargetFound OnTargetFound;


	UPROPERTY(BlueprintAssignable, Category = "Soft Targeting|Events")
	FOnSoftTargetLost OnTargetLost;


	UPROPERTY(BlueprintAssignable, Category = "Soft Targeting|Events")
	FOnSoftTargetActorEvent OnCandidateEntered;


	UPROPERTY(BlueprintAssignable, Category = "Soft Targeting|Events")
	FOnSoftTargetActorEvent OnCandidateExited;


	UPROPERTY(BlueprintAssignable, Category = "Soft Targeting|Events")
	FOnSoftTargetActorEvent OnTargetBecameObstructed;


	UPROPERTY(BlueprintAssignable, Category = "Soft Targeting|Events")
	FOnSoftTargetActorEvent OnTargetBecameVisible;


	UPROPERTY(BlueprintAssignable, Category = "Soft Targeting|Events")
	FOnSoftTargetActorEvent OnTargetLeftAttackRange;


	UPROPERTY(BlueprintAssignable, Category = "Soft Targeting|Events")
	FOnSoftTargetActorEvent OnTargetBackInRange;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Targeting|Detection", meta = (ClampMin = 50, ClampMax = 3000, Units = "cm"))
	float TargetingRadius = 600.0f;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Targeting|Detection")
	TArray<TEnumAsByte<EObjectTypeQuery>> TargetObjectTypes;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Targeting|Detection", meta = (ClampMin = 1, ClampMax = 64))
	int32 MaxTrackedTargets = 24;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Targeting|Detection")
	bool bUse2DDirection = true;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Targeting|Detection")
	bool bUse2DDistance = true;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Targeting|Filtering")
	bool bRequireSoftTargetableInterface = false;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Targeting|Filtering")
	TArray<TSubclassOf<AActor>> RequiredTargetClasses;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Targeting|Filtering")
	TArray<FName> RequiredActorTags;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Targeting|Filtering")
	TArray<FName> BlockedActorTags;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Targeting|Filtering")
	FGameplayTagContainer RequiredGameplayTags;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Targeting|Filtering")
	bool bRequireAllGameplayTags = false;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Targeting|Filtering")
	FGameplayTagContainer BlockedGameplayTags;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Targeting|Filtering")
	bool bRejectTargetsBehindPlayer = false;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Targeting|Filtering", meta = (ClampMin = 90, ClampMax = 180, Units = "deg", EditCondition = "bRejectTargetsBehindPlayer"))
	float BehindPlayerAngle = 150.0f;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Targeting|Target Point")
	FName TargetSocketName = FName("spine_03");


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Targeting|Target Point", meta = (ClampMin = 0, ClampMax = 300, Units = "cm"))
	float FallbackTargetHeight = 60.0f;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Targeting|Scoring")
	bool bUsePlayerDirection = true;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Targeting|Scoring", meta = (ClampMin = 0, ClampMax = 1))
	float PlayerDirectionWeight = 0.35f;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Targeting|Scoring", meta = (ClampMin = 10, ClampMax = 180, Units = "deg"))
	float MaxTargetAngle = 120.0f;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Targeting|Scoring")
	bool bUseInputDirection = true;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Targeting|Scoring", meta = (ClampMin = 0, ClampMax = 1))
	float InputDirectionWeight = 0.40f;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Targeting|Scoring", meta = (ClampMin = 10, ClampMax = 180, Units = "deg"))
	float MaxInputAngle = 90.0f;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Targeting|Scoring", meta = (ClampMin = 0, ClampMax = 1))
	float InputDeadZone = 0.20f;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Targeting|Scoring")
	bool bUseCameraDirection = true;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Targeting|Scoring", meta = (ClampMin = 0, ClampMax = 1))
	float CameraWeight = 0.05f;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Targeting|Scoring", meta = (ClampMin = 10, ClampMax = 180, Units = "deg"))
	float MaxCameraAngle = 100.0f;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Targeting|Scoring", meta = (ClampMin = 0, ClampMax = 1))
	float DistanceWeight = 0.20f;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Targeting|Scoring", meta = (ClampMin = 0, ClampMax = 1))
	float PriorityWeight = 0.0f;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Targeting|Persistence", meta = (ClampMin = 0, ClampMax = 1))
	float CurrentTargetBonus = 0.15f;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Targeting|Persistence", meta = (ClampMin = 0, ClampMax = 1))
	float TargetSwitchThreshold = 0.10f;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Targeting|Persistence", meta = (ClampMin = 0, ClampMax = 1))
	float InputOverridesPersistence = 0.50f;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Targeting|Attack", meta = (ClampMin = 0, ClampMax = 2000, Units = "cm"))
	float DefaultAttackRange = 220.0f;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Targeting|Attack", meta = (ClampMin = 0, ClampMax = 180, Units = "deg"))
	float DefaultAttackAngle = 120.0f;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Targeting|Attack", meta = (ClampMin = 0, ClampMax = 500, Units = "cm"))
	float DefaultRangeTolerance = 80.0f;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Targeting|Line Of Sight")
	bool bRequireLineOfSight = true;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Targeting|Line Of Sight")
	TEnumAsByte<ECollisionChannel> LineOfSightChannel = ECollisionChannel::ECC_Visibility;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Targeting|Line Of Sight", meta = (ClampMin = 0, ClampMax = 300, Units = "cm"))
	float LineOfSightEyeHeight = 60.0f;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Targeting|Line Of Sight")
	bool bIgnoreOtherTargetsInLineOfSight = true;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Targeting|Line Of Sight", meta = (ClampMin = 1, ClampMax = 16))
	int32 MaxLineOfSightChecks = 5;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Targeting|Line Of Sight", meta = (ClampMin = 0, ClampMax = 1, Units = "s"))
	float LineOfSightGraceTime = 0.15f;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Targeting|Refresh")
	bool bAutoRefresh = true;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Targeting|Refresh", meta = (ClampMin = 0, ClampMax = 1, Units = "s"))
	float TargetRefreshInterval = 0.05f;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Targeting|Input")
	bool bAutoReadMovementInput = true;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Targeting|Input", meta = (ClampMin = 0, ClampMax = 1, Units = "s"))
	float ManualInputValidTime = 0.20f;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Targeting|Debug")
	bool bDebugTargeting = false;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Targeting|Debug", meta = (EditCondition = "bDebugTargeting"))
	bool bDebugDrawDetectionRadius = true;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Targeting|Debug", meta = (EditCondition = "bDebugTargeting"))
	bool bDebugDrawAttackRange = true;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Targeting|Debug", meta = (EditCondition = "bDebugTargeting"))
	bool bDebugDrawAttackCone = true;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Targeting|Debug", meta = (EditCondition = "bDebugTargeting"))
	bool bDebugDrawDirections = true;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Targeting|Debug", meta = (EditCondition = "bDebugTargeting"))
	bool bDebugDrawTargetLines = true;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Targeting|Debug", meta = (EditCondition = "bDebugTargeting"))
	bool bDebugDrawTargetSpheres = true;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Targeting|Debug", meta = (EditCondition = "bDebugTargeting"))
	bool bDebugDrawCandidateText = true;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Targeting|Debug", meta = (EditCondition = "bDebugTargeting"))
	bool bDebugDrawStateText = true;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Targeting|Debug", meta = (EditCondition = "bDebugTargeting && bDebugDrawStateText"))
	bool bDebugStateTextOnScreen = true;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Targeting|Debug", meta = (EditCondition = "bDebugTargeting"))
	bool bDebugVerboseScores = true;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Targeting|Debug", meta = (ClampMin = 0.5, ClampMax = 3, EditCondition = "bDebugTargeting && bDebugDrawStateText && bDebugStateTextOnScreen"))
	float DebugScreenTextScale = 1.0f;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Targeting|Debug", meta = (ClampMin = 4, ClampMax = 64, EditCondition = "bDebugTargeting && bDebugDrawAttackCone"))
	int32 DebugAttackConeSegments = 24;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Targeting|Debug", meta = (ClampMin = -500, ClampMax = 500, Units = "cm", EditCondition = "bDebugTargeting"))
	float DebugCandidateTextSideOffset = 55.0f;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Targeting|Debug", meta = (ClampMin = 0, ClampMax = 500, Units = "cm", EditCondition = "bDebugTargeting"))
	float DebugCandidateTextHeight = 70.0f;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Targeting|Debug", meta = (ClampMin = -500, ClampMax = 500, Units = "cm", EditCondition = "bDebugTargeting"))
	float DebugStateTextSideOffset = 120.0f;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Targeting|Debug", meta = (ClampMin = 0, ClampMax = 500, Units = "cm", EditCondition = "bDebugTargeting"))
	float DebugStateTextHeight = 140.0f;

protected:


	UPROPERTY(BlueprintReadOnly, Category = "Soft Targeting")
	TObjectPtr<AActor> CurrentTarget = nullptr;


	UPROPERTY(BlueprintReadOnly, Category = "Soft Targeting")
	TObjectPtr<AActor> ForcedTarget = nullptr;


	UPROPERTY(BlueprintReadOnly, Category = "Soft Targeting")
	bool bTargetLocked = false;


	UPROPERTY(Transient)
	TArray<FSoftTargetData> CachedTargets;


	TArray<TWeakObjectPtr<AActor>> IgnoredActors;


	FVector ManualInputDirection = FVector::ZeroVector;


	float ManualInputMagnitude = 0.0f;


	float ManualInputTimestamp = -1000.0f;


	float TargetRefreshTimeAccumulator = 0.0f;


	TArray<TWeakObjectPtr<AActor>> PreviousCandidates;


	mutable TMap<TWeakObjectPtr<AActor>, float> LastVisibleTargetTimes;


	TWeakObjectPtr<AActor> StatusEventTarget;
	bool bHasCurrentTargetStatusSample = false;
	bool bCurrentTargetWasVisible = false;
	bool bCurrentTargetWasInAttackRange = false;

public:


	virtual void BeginPlay() override;


	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;


	UFUNCTION(BlueprintPure, Category = "Soft Targeting")
	AActor* GetCurrentTarget() const;


	UFUNCTION(BlueprintPure, Category = "Soft Targeting")
	bool HasTarget() const;


	UFUNCTION(BlueprintCallable, Category = "Soft Targeting")
	void RefreshTargets();


	UFUNCTION(BlueprintCallable, Category = "Soft Targeting")
	AActor* FindBestTarget(const FSoftTargetingRequest& Request);


	UFUNCTION(BlueprintCallable, Category = "Soft Targeting", meta = (AdvancedDisplay = "2"))
	AActor* FindBestTargetForAttack(float AttackRange, float MaxAttackAngle, bool bUpdateCurrentTarget = true);


	UFUNCTION(BlueprintPure, Category = "Soft Targeting")
	TArray<FSoftTargetData> GetSortedTargets() const;


	UFUNCTION(BlueprintCallable, Category = "Soft Targeting")
	TArray<AActor*> GetTargetsInRange(float Range) const;


	UFUNCTION(BlueprintPure, Category = "Soft Targeting")
	bool IsValidTarget(AActor* Target) const;


	UFUNCTION(BlueprintPure, Category = "Soft Targeting")
	bool IsTargetInAttackRange(AActor* Target, float AttackRange) const;


	UFUNCTION(BlueprintPure, Category = "Soft Targeting")
	float CalculateTargetScore(AActor* Target) const;


	UFUNCTION(BlueprintCallable, Category = "Soft Targeting")
	bool ForceTarget(AActor* Target);


	UFUNCTION(BlueprintCallable, Category = "Soft Targeting")
	void ClearForcedTarget();


	UFUNCTION(BlueprintPure, Category = "Soft Targeting")
	bool HasForcedTarget() const;


	UFUNCTION(BlueprintCallable, Category = "Soft Targeting")
	void LockCurrentTarget();


	UFUNCTION(BlueprintCallable, Category = "Soft Targeting")
	void UnlockCurrentTarget();


	UFUNCTION(BlueprintPure, Category = "Soft Targeting")
	bool IsTargetLocked() const;


	UFUNCTION(BlueprintCallable, Category = "Soft Targeting")
	void AddIgnoredActor(AActor* Actor);


	UFUNCTION(BlueprintCallable, Category = "Soft Targeting")
	void RemoveIgnoredActor(AActor* Actor);


	UFUNCTION(BlueprintCallable, Category = "Soft Targeting")
	void ClearIgnoredActors();


	UFUNCTION(BlueprintPure, Category = "Soft Targeting")
	TArray<AActor*> GetIgnoredActors() const;


	UFUNCTION(BlueprintCallable, Category = "Soft Targeting|Input")
	void SetMovementInputDirection(FVector WorldDirection);


	UFUNCTION(BlueprintCallable, Category = "Soft Targeting|Input")
	void ClearMovementInputDirection();


	UFUNCTION(BlueprintPure, Category = "Soft Targeting|Input")
	FVector GetEffectiveInputDirection() const;


	UFUNCTION(BlueprintPure, Category = "Soft Targeting|Input")
	float GetEffectiveInputMagnitude() const;


	UFUNCTION(BlueprintPure, Category = "Soft Targeting", meta = (DisplayName = "Get Soft Target Location"))
	FVector GetTargetLocation(AActor* Target) const;


	UFUNCTION(BlueprintPure, Category = "Soft Targeting")
	FVector GetDirectionToCurrentTarget() const;


	UFUNCTION(BlueprintPure, Category = "Soft Targeting")
	FRotator GetRotationToCurrentTarget() const;


	UFUNCTION(BlueprintPure, Category = "Soft Targeting")
	float GetDistanceToCurrentTarget() const;


	UFUNCTION(BlueprintPure, Category = "Soft Targeting")
	FVector GetCurrentTargetLocation() const;


	UFUNCTION(BlueprintCallable, Category = "Soft Targeting|Motion Warping")
	bool GetMotionWarpTransform(AActor* Target, float StopDistance, FVector& OutLocation, FRotator& OutRotation) const;


	UFUNCTION(BlueprintPure, Category = "Soft Targeting")
	FSoftTargetingRequest MakeDefaultRequest() const;

protected:


	void BuildContext(FSoftTargetingContext& OutContext) const;


	AActor* EvaluateRequest(const FSoftTargetingRequest& Request, TArray<FSoftTargetData>& OutTargets) const;


	void GatherOverlappingActors(const FSoftTargetingContext& Context, float Radius, TArray<AActor*>& OutActors) const;


	bool ScoreTarget(const FSoftTargetingContext& Context, const FSoftTargetingRequest& Request, const FVector& TargetLocation, FSoftTargetData& Data) const;


	bool IsValidTargetWithLocation(AActor* Target, const FVector& TargetLocation) const;


	bool HasLineOfSightTo(const FSoftTargetingContext& Context, const FSoftTargetData& Data, const TArray<FSoftTargetData>& Candidates) const;


	int32 ApplyPersistence(const FSoftTargetingContext& Context, const FSoftTargetingRequest& Request, const TArray<FSoftTargetData>& Targets, int32 BestIndex) const;


	void SetCurrentTargetInternal(AActor* NewTarget);


	void ValidateCurrentTarget();


	void PruneIgnoredActors();


	void UpdateCandidateEvents(const TArray<FSoftTargetData>& Targets);


	void UpdateCurrentTargetStatusEvents();


	void ClearDebugScreenMessage() const;


	bool IsActorIgnored(const AActor* Actor) const;


	bool PassesTagFilters(AActor* Target) const;


	static float AngleToScore(float AngleDegrees, float MaxAngleDegrees);


	void DrawDebugTargeting(const FSoftTargetingContext& Context, const TArray<FSoftTargetData>& Targets) const;
};
