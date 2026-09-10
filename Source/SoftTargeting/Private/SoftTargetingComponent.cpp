

#include "SoftTargetingComponent.h"
#include "SoftTargetableInterface.h"

#include "Camera/PlayerCameraManager.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagAssetInterface.h"


#include "WorldCollision.h"


#include "DrawDebugHelpers.h"

DEFINE_LOG_CATEGORY(LogSoftTargeting);

USoftTargetingComponent::USoftTargetingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;


	PrimaryComponentTick.TickGroup = TG_PrePhysics;


	PrimaryComponentTick.TickInterval = TargetRefreshInterval;

	bAutoActivate = true;


	TargetObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn));
}

void USoftTargetingComponent::BeginPlay()
{
	Super::BeginPlay();


	if (TargetObjectTypes.Num() == 0)
	{
		TargetObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn));
		UE_LOG(LogSoftTargeting, Warning, TEXT("%s: TargetObjectTypes was empty, defaulting to Pawn."), *GetNameSafe(GetOwner()));
	}


	SetComponentTickInterval(bDebugTargeting ? 0.0f : TargetRefreshInterval);

	CachedTargets.Reserve(MaxTrackedTargets);
}

void USoftTargetingComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{

	CachedTargets.Reset();
	IgnoredActors.Reset();
	PreviousCandidates.Reset();
	LastVisibleTargetTimes.Reset();
	ClearDebugScreenMessage();
	CurrentTarget = nullptr;
	ForcedTarget = nullptr;
	bTargetLocked = false;

	Super::EndPlay(EndPlayReason);
}

void USoftTargetingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);


	const float DesiredTickInterval = bDebugTargeting ? 0.0f : FMath::Max(TargetRefreshInterval, 0.0f);
	if (!FMath::IsNearlyEqual(PrimaryComponentTick.TickInterval, DesiredTickInterval))
	{
		SetComponentTickInterval(DesiredTickInterval);
	}


	ValidateCurrentTarget();
	UpdateCurrentTargetStatusEvents();

	if (IsValid(ForcedTarget))
	{

		SetCurrentTargetInternal(ForcedTarget);
	}
	else if (bAutoRefresh)
	{
		TargetRefreshTimeAccumulator += DeltaTime;

		const float RefreshInterval = FMath::Max(TargetRefreshInterval, 0.0f);
		if (RefreshInterval <= KINDA_SMALL_NUMBER || TargetRefreshTimeAccumulator >= RefreshInterval)
		{
			RefreshTargets();
			TargetRefreshTimeAccumulator = (RefreshInterval > KINDA_SMALL_NUMBER)
				? FMath::Fmod(TargetRefreshTimeAccumulator, RefreshInterval)
				: 0.0f;
		}
	}
	else
	{
		TargetRefreshTimeAccumulator = 0.0f;
	}

	if (bDebugTargeting)
	{
		FSoftTargetingContext Context;
		BuildContext(Context);
		DrawDebugTargeting(Context, CachedTargets);
	}
	else
	{
		ClearDebugScreenMessage();
	}
}


AActor* USoftTargetingComponent::GetCurrentTarget() const
{
	return IsValid(CurrentTarget) ? CurrentTarget.Get() : nullptr;
}

bool USoftTargetingComponent::HasTarget() const
{
	return IsValid(CurrentTarget);
}


void USoftTargetingComponent::RefreshTargets()
{
	FSoftTargetingRequest Request = MakeDefaultRequest();


	Request.MaxRange = DefaultAttackRange;
	Request.RangeTolerance = FMath::Max(0.0f, TargetingRadius - DefaultAttackRange);
	Request.MaxAngle = 180.0f;

	AActor* const Best = EvaluateRequest(Request, CachedTargets);
	UpdateCandidateEvents(CachedTargets);


	if (!bTargetLocked && !IsValid(ForcedTarget))
	{
		SetCurrentTargetInternal(Best);
	}
}

AActor* USoftTargetingComponent::FindBestTarget(const FSoftTargetingRequest& Request)
{
	if (IsValid(ForcedTarget))
	{
		if (Request.bUpdateCurrentTarget)
		{
			SetCurrentTargetInternal(ForcedTarget);
		}
		return ForcedTarget.Get();
	}

	if (Request.bRespectTargetLock && bTargetLocked)
	{
		return GetCurrentTarget();
	}

	AActor* const Best = EvaluateRequest(Request, CachedTargets);


	if (Request.bUpdateCurrentTarget && Best != nullptr)
	{
		SetCurrentTargetInternal(Best);
	}

	if (bDebugTargeting)
	{
		FSoftTargetingContext Context;
		BuildContext(Context);
		DrawDebugTargeting(Context, CachedTargets);
	}

	return Best;
}

AActor* USoftTargetingComponent::FindBestTargetForAttack(float AttackRange, float MaxAttackAngle, bool bUpdateCurrentTarget)
{
	FSoftTargetingRequest Request = MakeDefaultRequest();
	Request.MaxRange = (AttackRange > 0.0f) ? AttackRange : DefaultAttackRange;
	Request.MaxAngle = (MaxAttackAngle > 0.0f) ? MaxAttackAngle : DefaultAttackAngle;
	Request.bUpdateCurrentTarget = bUpdateCurrentTarget;

	return FindBestTarget(Request);
}

TArray<FSoftTargetData> USoftTargetingComponent::GetSortedTargets() const
{
	return CachedTargets;
}

TArray<AActor*> USoftTargetingComponent::GetTargetsInRange(float Range) const
{
	TArray<AActor*> Result;

	FSoftTargetingContext Context;
	BuildContext(Context);

	if (Context.Owner == nullptr)
	{
		return Result;
	}

	const float EffectiveRange = (Range > 0.0f) ? FMath::Min(Range, TargetingRadius) : TargetingRadius;

	TArray<AActor*> Overlapped;
	GatherOverlappingActors(Context, EffectiveRange, Overlapped);

	Result.Reserve(Overlapped.Num());

	for (AActor* const Candidate : Overlapped)
	{
		if (!IsValidTarget(Candidate))
		{
			continue;
		}


		FVector Delta = GetTargetLocation(Candidate) - Context.OwnerLocation;
		if (bUse2DDistance)
		{
			Delta.Z = 0.0f;
		}

		if (Delta.SizeSquared() <= FMath::Square(EffectiveRange))
		{
			Result.Add(Candidate);
		}
	}

	return Result;
}


bool USoftTargetingComponent::IsValidTarget(AActor* Target) const
{
	return IsValidTargetWithLocation(Target, GetTargetLocation(Target));
}

bool USoftTargetingComponent::IsValidTargetWithLocation(AActor* Target, const FVector& TargetLocation) const
{

	if (!IsValid(Target))
	{
		return false;
	}

	AActor* const Owner = GetOwner();
	if (Owner == nullptr || Target == Owner)
	{
		return false;
	}

	if (IsActorIgnored(Target))
	{
		return false;
	}

	if (bRequireSoftTargetableInterface && !Target->Implements<USoftTargetableInterface>())
	{
		return false;
	}


	if (Target->Implements<USoftTargetableInterface>())
	{
		if (!ISoftTargetableInterface::Execute_CanBeSoftTargeted(Target))
		{
			return false;
		}
	}

	if (RequiredTargetClasses.Num() > 0)
	{
		bool bMatchesClass = false;
		for (const TSubclassOf<AActor>& RequiredClass : RequiredTargetClasses)
		{
			if (RequiredClass != nullptr && Target->IsA(RequiredClass))
			{
				bMatchesClass = true;
				break;
			}
		}

		if (!bMatchesClass)
		{
			return false;
		}
	}

	if (!PassesTagFilters(Target))
	{
		return false;
	}


	FVector Delta = TargetLocation - Owner->GetActorLocation();
	if (bUse2DDistance)
	{
		Delta.Z = 0.0f;
	}

	if (Delta.SizeSquared() > FMath::Square(TargetingRadius))
	{
		return false;
	}


	if (bRejectTargetsBehindPlayer)
	{
		FVector Direction = Delta;
		FVector Forward = Owner->GetActorForwardVector();

		if (bUse2DDirection)
		{
			Direction.Z = 0.0f;
			Forward.Z = 0.0f;
		}

		if (Direction.Normalize() && Forward.Normalize())
		{
			const float Dot = FVector::DotProduct(Forward, Direction);
			const float Angle = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(Dot, -1.0f, 1.0f)));

			if (Angle > BehindPlayerAngle)
			{
				return false;
			}
		}
	}

	return true;
}

bool USoftTargetingComponent::IsTargetInAttackRange(AActor* Target, float AttackRange) const
{
	const AActor* const Owner = GetOwner();
	if (!IsValid(Target) || Owner == nullptr)
	{
		return false;
	}

	const float EffectiveRange = (AttackRange > 0.0f) ? AttackRange : DefaultAttackRange;

	FVector Delta = GetTargetLocation(Target) - Owner->GetActorLocation();
	if (bUse2DDistance)
	{
		Delta.Z = 0.0f;
	}

	return Delta.SizeSquared() <= FMath::Square(EffectiveRange);
}

float USoftTargetingComponent::CalculateTargetScore(AActor* Target) const
{
	const FVector TargetLocation = GetTargetLocation(Target);
	if (!IsValidTargetWithLocation(Target, TargetLocation))
	{
		return 0.0f;
	}

	FSoftTargetingContext Context;
	BuildContext(Context);

	if (Context.Owner == nullptr)
	{
		return 0.0f;
	}


	FSoftTargetingRequest Request = MakeDefaultRequest();
	Request.MaxRange = TargetingRadius;
	Request.RangeTolerance = 0.0f;
	Request.MaxAngle = 180.0f;

	FSoftTargetData Data(Target);
	if (!ScoreTarget(Context, Request, TargetLocation, Data))
	{
		return 0.0f;
	}

	return Data.FinalScore;
}


bool USoftTargetingComponent::ForceTarget(AActor* Target)
{
	if (!IsValidTarget(Target))
	{
		return false;
	}

	ForcedTarget = Target;
	SetCurrentTargetInternal(Target);

	return true;
}

void USoftTargetingComponent::ClearForcedTarget()
{
	ForcedTarget = nullptr;
}

bool USoftTargetingComponent::HasForcedTarget() const
{
	return IsValid(ForcedTarget);
}


void USoftTargetingComponent::LockCurrentTarget()
{

	if (!IsValid(CurrentTarget))
	{
		return;
	}

	bTargetLocked = true;
}

void USoftTargetingComponent::UnlockCurrentTarget()
{
	bTargetLocked = false;
}

bool USoftTargetingComponent::IsTargetLocked() const
{
	return bTargetLocked;
}


void USoftTargetingComponent::AddIgnoredActor(AActor* Actor)
{
	if (!IsValid(Actor))
	{
		return;
	}

	IgnoredActors.AddUnique(Actor);


	if (CurrentTarget == Actor)
	{
		bTargetLocked = false;
		SetCurrentTargetInternal(nullptr);
	}

	if (ForcedTarget == Actor)
	{
		ClearForcedTarget();
	}
}

void USoftTargetingComponent::RemoveIgnoredActor(AActor* Actor)
{
	if (Actor == nullptr)
	{
		return;
	}

	IgnoredActors.RemoveAll([Actor](const TWeakObjectPtr<AActor>& Entry)
	{
		return !Entry.IsValid() || Entry.Get() == Actor;
	});
}

void USoftTargetingComponent::ClearIgnoredActors()
{
	IgnoredActors.Reset();
}

TArray<AActor*> USoftTargetingComponent::GetIgnoredActors() const
{
	TArray<AActor*> Result;
	Result.Reserve(IgnoredActors.Num());

	for (const TWeakObjectPtr<AActor>& Entry : IgnoredActors)
	{
		if (AActor* const Actor = Entry.Get())
		{
			Result.Add(Actor);
		}
	}

	return Result;
}


void USoftTargetingComponent::SetMovementInputDirection(FVector WorldDirection)
{
	const UWorld* const World = GetWorld();

	ManualInputMagnitude = FMath::Clamp(static_cast<float>(WorldDirection.Size()), 0.0f, 1.0f);

	if (bUse2DDirection)
	{
		WorldDirection.Z = 0.0f;
	}

	if (!WorldDirection.Normalize())
	{
		ManualInputDirection = FVector::ZeroVector;
		ManualInputMagnitude = 0.0f;
	}
	else
	{
		ManualInputDirection = WorldDirection;
	}

	ManualInputTimestamp = World ? World->GetTimeSeconds() : 0.0f;
}

void USoftTargetingComponent::ClearMovementInputDirection()
{
	ManualInputDirection = FVector::ZeroVector;
	ManualInputMagnitude = 0.0f;
	ManualInputTimestamp = -1000.0f;
}

FVector USoftTargetingComponent::GetEffectiveInputDirection() const
{
	FSoftTargetingContext Context;
	BuildContext(Context);

	return Context.InputDirection;
}

float USoftTargetingComponent::GetEffectiveInputMagnitude() const
{
	FSoftTargetingContext Context;
	BuildContext(Context);

	return Context.InputMagnitude;
}


FVector USoftTargetingComponent::GetTargetLocation(AActor* Target) const
{
	if (!IsValid(Target))
	{
		return FVector::ZeroVector;
	}


	if (Target->Implements<USoftTargetableInterface>())
	{
		return ISoftTargetableInterface::Execute_GetSoftTargetLocation(Target);
	}


	if (!TargetSocketName.IsNone())
	{
		if (const USkeletalMeshComponent* const Mesh = Target->FindComponentByClass<USkeletalMeshComponent>())
		{
			if (Mesh->DoesSocketExist(TargetSocketName))
			{
				return Mesh->GetSocketLocation(TargetSocketName);
			}
		}
	}

	return Target->GetActorLocation() + FVector(0.0f, 0.0f, FallbackTargetHeight);
}

FVector USoftTargetingComponent::GetDirectionToCurrentTarget() const
{
	const AActor* const Owner = GetOwner();
	if (Owner == nullptr || !IsValid(CurrentTarget))
	{
		return FVector::ZeroVector;
	}

	FVector Direction = GetTargetLocation(CurrentTarget) - Owner->GetActorLocation();
	if (bUse2DDirection)
	{
		Direction.Z = 0.0f;
	}

	return Direction.GetSafeNormal();
}

FRotator USoftTargetingComponent::GetRotationToCurrentTarget() const
{
	const AActor* const Owner = GetOwner();
	if (Owner == nullptr)
	{
		return FRotator::ZeroRotator;
	}

	const FVector Direction = GetDirectionToCurrentTarget();
	if (Direction.IsNearlyZero())
	{
		return Owner->GetActorRotation();
	}


	FRotator Rotation = Direction.Rotation();
	Rotation.Pitch = 0.0f;
	Rotation.Roll = 0.0f;

	return Rotation;
}

float USoftTargetingComponent::GetDistanceToCurrentTarget() const
{
	const AActor* const Owner = GetOwner();
	if (Owner == nullptr || !IsValid(CurrentTarget))
	{
		return -1.0f;
	}

	FVector Delta = GetTargetLocation(CurrentTarget) - Owner->GetActorLocation();
	if (bUse2DDistance)
	{
		Delta.Z = 0.0f;
	}

	return static_cast<float>(Delta.Size());
}

FVector USoftTargetingComponent::GetCurrentTargetLocation() const
{
	if (!IsValid(CurrentTarget))
	{
		const AActor* const Owner = GetOwner();
		return Owner ? Owner->GetActorLocation() : FVector::ZeroVector;
	}

	return GetTargetLocation(CurrentTarget);
}

bool USoftTargetingComponent::GetMotionWarpTransform(AActor* Target, float StopDistance, FVector& OutLocation, FRotator& OutRotation) const
{
	const AActor* const Owner = GetOwner();

	OutLocation = Owner ? Owner->GetActorLocation() : FVector::ZeroVector;
	OutRotation = Owner ? Owner->GetActorRotation() : FRotator::ZeroRotator;

	if (Owner == nullptr || !IsValid(Target))
	{
		return false;
	}

	const FVector OwnerLocation = Owner->GetActorLocation();
	const FVector TargetPoint = GetTargetLocation(Target);

	FVector Delta = TargetPoint - OwnerLocation;
	Delta.Z = 0.0f;

	const float Distance = static_cast<float>(Delta.Size());
	if (Distance <= KINDA_SMALL_NUMBER)
	{
		return false;
	}

	const FVector Direction = Delta / Distance;


	const float TravelDistance = FMath::Max(0.0f, Distance - FMath::Max(0.0f, StopDistance));


	OutLocation = OwnerLocation + Direction * TravelDistance;
	OutLocation.Z = Target->GetActorLocation().Z;

	OutRotation = Direction.Rotation();
	OutRotation.Pitch = 0.0f;
	OutRotation.Roll = 0.0f;

	return true;
}

FSoftTargetingRequest USoftTargetingComponent::MakeDefaultRequest() const
{
	FSoftTargetingRequest Request;
	Request.MaxRange = DefaultAttackRange;
	Request.MaxAngle = DefaultAttackAngle;
	Request.RangeTolerance = DefaultRangeTolerance;
	Request.bUseInputDirection = bUseInputDirection;
	Request.bUseCameraDirection = bUseCameraDirection;
	Request.bRequireLineOfSight = bRequireLineOfSight;
	Request.bApplyPersistence = true;
	Request.bRespectTargetLock = true;
	Request.bUpdateCurrentTarget = true;

	return Request;
}


void USoftTargetingComponent::BuildContext(FSoftTargetingContext& OutContext) const
{
	AActor* const Owner = GetOwner();

	OutContext = FSoftTargetingContext();
	OutContext.Owner = Owner;

	if (Owner == nullptr)
	{
		return;
	}

	OutContext.OwnerLocation = Owner->GetActorLocation();
	OutContext.OwnerForward = Owner->GetActorForwardVector();


	OutContext.CameraLocation = OutContext.OwnerLocation;
	OutContext.CameraForward = OutContext.OwnerForward;

	if (const APawn* const OwnerPawn = Cast<APawn>(Owner))
	{
		if (const APlayerController* const PC = Cast<APlayerController>(OwnerPawn->GetController()))
		{
			if (const APlayerCameraManager* const CameraManager = PC->PlayerCameraManager)
			{
				OutContext.CameraLocation = CameraManager->GetCameraLocation();
				OutContext.CameraForward = CameraManager->GetCameraRotation().Vector();
				OutContext.bHasCamera = true;
			}
		}
	}


	FVector InputDirection = FVector::ZeroVector;
	float InputMagnitude = 0.0f;

	const UWorld* const World = GetWorld();
	const float Now = World ? World->GetTimeSeconds() : 0.0f;

	if (!ManualInputDirection.IsNearlyZero() && (Now - ManualInputTimestamp) <= ManualInputValidTime)
	{
		InputDirection = ManualInputDirection;
		InputMagnitude = ManualInputMagnitude;
	}
	else if (bAutoReadMovementInput)
	{
		if (const APawn* const OwnerPawn = Cast<APawn>(Owner))
		{
			const FVector RawInput = OwnerPawn->GetLastMovementInputVector();
			InputMagnitude = FMath::Clamp(static_cast<float>(RawInput.Size()), 0.0f, 1.0f);
			InputDirection = RawInput.GetSafeNormal();
		}
	}

	if (bUse2DDirection)
	{
		OutContext.OwnerForward.Z = 0.0f;
		OutContext.CameraForward.Z = 0.0f;
		InputDirection.Z = 0.0f;
	}

	OutContext.OwnerForward = OutContext.OwnerForward.GetSafeNormal();
	OutContext.CameraForward = OutContext.CameraForward.GetSafeNormal();

	if (OutContext.OwnerForward.IsNearlyZero())
	{
		OutContext.OwnerForward = FVector::ForwardVector;
	}

	if (OutContext.CameraForward.IsNearlyZero())
	{
		OutContext.CameraForward = OutContext.OwnerForward;
	}

	OutContext.InputDirection = InputDirection.GetSafeNormal();
	OutContext.InputMagnitude = OutContext.InputDirection.IsNearlyZero() ? 0.0f : InputMagnitude;
	OutContext.bHasInput = OutContext.InputMagnitude >= InputDeadZone;
}

AActor* USoftTargetingComponent::EvaluateRequest(const FSoftTargetingRequest& Request, TArray<FSoftTargetData>& OutTargets) const
{
	OutTargets.Reset();

	FSoftTargetingContext Context;
	BuildContext(Context);

	if (Context.Owner == nullptr)
	{
		return nullptr;
	}


	const float EffectiveRadius = FMath::Clamp(Request.MaxRange + Request.RangeTolerance, 1.0f, TargetingRadius);

	TArray<AActor*> Candidates;
	GatherOverlappingActors(Context, EffectiveRadius, Candidates);

	if (Candidates.Num() == 0)
	{
		return nullptr;
	}


	OutTargets.Reserve(Candidates.Num());

	for (AActor* const Candidate : Candidates)
	{
		const FVector TargetLocation = GetTargetLocation(Candidate);
		if (!IsValidTargetWithLocation(Candidate, TargetLocation))
		{
			continue;
		}

		FSoftTargetData Data(Candidate);
		if (ScoreTarget(Context, Request, TargetLocation, Data))
		{
			OutTargets.Add(MoveTemp(Data));
		}
	}

	if (OutTargets.Num() == 0)
	{
		return nullptr;
	}


	OutTargets.Sort([](const FSoftTargetData& A, const FSoftTargetData& B)
	{
		return A.FinalScore > B.FinalScore;
	});


	if (OutTargets.Num() > MaxTrackedTargets)
	{
		OutTargets.SetNum(MaxTrackedTargets, EAllowShrinking::No);
	}


	int32 BestIndex = INDEX_NONE;
	const bool bTestLineOfSight = Request.bRequireLineOfSight && bRequireLineOfSight;

	if (!bTestLineOfSight)
	{
		BestIndex = 0;
	}
	else
	{
		const int32 Budget = FMath::Min(MaxLineOfSightChecks, OutTargets.Num());

		for (int32 Index = 0; Index < Budget; ++Index)
		{
			const bool bVisible = HasLineOfSightTo(Context, OutTargets[Index], OutTargets);
			OutTargets[Index].bHasLineOfSight = bVisible;

			if (bVisible)
			{
				BestIndex = Index;
				break;
			}
		}
	}


	BestIndex = ApplyPersistence(Context, Request, OutTargets, BestIndex);

	return OutTargets.IsValidIndex(BestIndex) ? OutTargets[BestIndex].Actor.Get() : nullptr;
}

void USoftTargetingComponent::GatherOverlappingActors(const FSoftTargetingContext& Context, float Radius, TArray<AActor*>& OutActors) const
{
	OutActors.Reset();

	const UWorld* const World = GetWorld();
	if (World == nullptr || Context.Owner == nullptr || TargetObjectTypes.Num() == 0)
	{
		return;
	}

	FCollisionObjectQueryParams ObjectParams;
	for (const TEnumAsByte<EObjectTypeQuery>& ObjectType : TargetObjectTypes)
	{
		ObjectParams.AddObjectTypesToQuery(UEngineTypes::ConvertToCollisionChannel(ObjectType));
	}

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(SoftTargetingOverlap),  false, Context.Owner);

	TArray<FOverlapResult> Overlaps;
	Overlaps.Reserve(32);

	World->OverlapMultiByObjectType(
		Overlaps,
		Context.OwnerLocation,
		FQuat::Identity,
		ObjectParams,
		FCollisionShape::MakeSphere(Radius),
		QueryParams);

	OutActors.Reserve(Overlaps.Num());


	for (const FOverlapResult& Overlap : Overlaps)
	{
		if (AActor* const Actor = Overlap.GetActor())
		{
			OutActors.AddUnique(Actor);
		}
	}
}

bool USoftTargetingComponent::ScoreTarget(const FSoftTargetingContext& Context, const FSoftTargetingRequest& Request, const FVector& TargetLocation, FSoftTargetData& Data) const
{
	AActor* const Target = Data.Actor.Get();
	if (Target == nullptr || Context.Owner == nullptr)
	{
		return false;
	}

	Data.TargetLocation = TargetLocation;

	FVector Delta = Data.TargetLocation - Context.OwnerLocation;

	FVector DistanceDelta = Delta;
	if (bUse2DDistance)
	{
		DistanceDelta.Z = 0.0f;
	}

	Data.Distance = static_cast<float>(DistanceDelta.Size());


	const float MaxDistance = Request.MaxRange + Request.RangeTolerance;
	if (Data.Distance > MaxDistance)
	{
		return false;
	}

	Data.bInAttackRange = Data.Distance <= Request.MaxRange;


	FVector Direction = Delta;
	if (bUse2DDirection)
	{
		Direction.Z = 0.0f;
	}

	if (!Direction.Normalize())
	{

		Direction = Context.OwnerForward;
	}

	const float FacingDot = FMath::Clamp(static_cast<float>(FVector::DotProduct(Context.OwnerForward, Direction)), -1.0f, 1.0f);
	Data.Angle = FMath::RadiansToDegrees(FMath::Acos(FacingDot));


	if (Data.Angle > Request.MaxAngle)
	{
		return false;
	}


	float WeightedSum = 0.0f;
	float TotalWeight = 0.0f;


	if (bUsePlayerDirection && PlayerDirectionWeight > 0.0f)
	{
		Data.DirectionScore = AngleToScore(Data.Angle, MaxTargetAngle);
		WeightedSum += Data.DirectionScore * PlayerDirectionWeight;
		TotalWeight += PlayerDirectionWeight;
	}


	if (Request.bUseInputDirection && bUseInputDirection && Context.bHasInput && InputDirectionWeight > 0.0f)
	{
		const float InputDot = FMath::Clamp(static_cast<float>(FVector::DotProduct(Context.InputDirection, Direction)), -1.0f, 1.0f);
		const float InputAngle = FMath::RadiansToDegrees(FMath::Acos(InputDot));

		Data.InputScore = AngleToScore(InputAngle, MaxInputAngle);
		WeightedSum += Data.InputScore * InputDirectionWeight;
		TotalWeight += InputDirectionWeight;
	}


	if (Request.bUseCameraDirection && bUseCameraDirection && Context.bHasCamera && CameraWeight > 0.0f)
	{
		FVector CameraToTarget = Data.TargetLocation - Context.CameraLocation;
		if (bUse2DDirection)
		{
			CameraToTarget.Z = 0.0f;
		}

		if (CameraToTarget.Normalize())
		{
			const float CameraDot = FMath::Clamp(static_cast<float>(FVector::DotProduct(Context.CameraForward, CameraToTarget)), -1.0f, 1.0f);
			const float CameraAngle = FMath::RadiansToDegrees(FMath::Acos(CameraDot));

			Data.CameraScore = AngleToScore(CameraAngle, MaxCameraAngle);
		}

		WeightedSum += Data.CameraScore * CameraWeight;
		TotalWeight += CameraWeight;
	}


	if (DistanceWeight > 0.0f)
	{
		Data.DistanceScore = 1.0f - FMath::Clamp(Data.Distance / FMath::Max(TargetingRadius, 1.0f), 0.0f, 1.0f);
		WeightedSum += Data.DistanceScore * DistanceWeight;
		TotalWeight += DistanceWeight;
	}


	if (PriorityWeight > 0.0f)
	{
		Data.PriorityScore = Target->Implements<USoftTargetableInterface>()
			? FMath::Clamp(ISoftTargetableInterface::Execute_GetSoftTargetPriority(Target), 0.0f, 1.0f)
			: 0.5f;

		WeightedSum += Data.PriorityScore * PriorityWeight;
		TotalWeight += PriorityWeight;
	}

	const float BaseScore = (TotalWeight > KINDA_SMALL_NUMBER) ? (WeightedSum / TotalWeight) : 0.0f;


	const bool bGrantsBonus =
		Request.bApplyPersistence &&
		IsValid(CurrentTarget) &&
		Data.Actor == CurrentTarget &&
		Context.InputMagnitude < InputOverridesPersistence;

	Data.PersistenceBonus = bGrantsBonus ? CurrentTargetBonus : 0.0f;
	Data.FinalScore = BaseScore + Data.PersistenceBonus;

	return true;
}

bool USoftTargetingComponent::HasLineOfSightTo(const FSoftTargetingContext& Context, const FSoftTargetData& Data, const TArray<FSoftTargetData>& Candidates) const
{
	const UWorld* const World = GetWorld();
	AActor* const Target = Data.Actor.Get();

	if (World == nullptr || Target == nullptr || Context.Owner == nullptr)
	{
		return false;
	}

	const FVector Start = Context.OwnerLocation + FVector(0.0f, 0.0f, LineOfSightEyeHeight);
	const FVector End = Data.TargetLocation;

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(SoftTargetingLineOfSight),  false, Context.Owner);
	QueryParams.AddIgnoredActor(Target);


	if (bIgnoreOtherTargetsInLineOfSight)
	{
		for (const FSoftTargetData& Other : Candidates)
		{
			if (AActor* const OtherActor = Other.Actor.Get())
			{
				QueryParams.AddIgnoredActor(OtherActor);
			}
		}
	}

	FHitResult Hit;
	const bool bBlocked = World->LineTraceSingleByChannel(Hit, Start, End, LineOfSightChannel, QueryParams);
	const float Now = World->GetTimeSeconds();

	if (!bBlocked)
	{
		LastVisibleTargetTimes.FindOrAdd(Target) = Now;
		return true;
	}


	if (LineOfSightGraceTime > 0.0f)
	{
		if (const float* const LastVisibleTime = LastVisibleTargetTimes.Find(Target))
		{
			return (Now - *LastVisibleTime) <= LineOfSightGraceTime;
		}
	}

	return false;
}

int32 USoftTargetingComponent::ApplyPersistence(const FSoftTargetingContext& Context, const FSoftTargetingRequest& Request, const TArray<FSoftTargetData>& Targets, int32 BestIndex) const
{
	if (!Request.bApplyPersistence || !IsValid(CurrentTarget))
	{
		return BestIndex;
	}


	if (Context.InputMagnitude >= InputOverridesPersistence)
	{
		return BestIndex;
	}


	if (Targets.IsValidIndex(BestIndex) && Targets[BestIndex].Actor == CurrentTarget)
	{
		return BestIndex;
	}

	const int32 CurrentIndex = Targets.IndexOfByPredicate([this](const FSoftTargetData& Entry)
	{
		return Entry.Actor == CurrentTarget;
	});


	if (CurrentIndex == INDEX_NONE)
	{
		return BestIndex;
	}


	if (Request.bRequireLineOfSight && bRequireLineOfSight)
	{
		if (!HasLineOfSightTo(Context, Targets[CurrentIndex], Targets))
		{
			return BestIndex;
		}
	}


	if (!Targets.IsValidIndex(BestIndex))
	{
		return CurrentIndex;
	}


	const float ChallengerScore = Targets[BestIndex].FinalScore;
	const float DefenderScore = Targets[CurrentIndex].FinalScore;

	return (ChallengerScore > DefenderScore + TargetSwitchThreshold) ? BestIndex : CurrentIndex;
}

void USoftTargetingComponent::SetCurrentTargetInternal(AActor* NewTarget)
{
	AActor* const OldTarget = IsValid(CurrentTarget) ? CurrentTarget.Get() : nullptr;

	if (OldTarget == NewTarget)
	{

		CurrentTarget = NewTarget;
		return;
	}

	CurrentTarget = NewTarget;
	StatusEventTarget = NewTarget;
	bHasCurrentTargetStatusSample = false;

	if (OldTarget != nullptr)
	{
		OnTargetLost.Broadcast(OldTarget);
	}

	if (NewTarget != nullptr)
	{
		OnTargetFound.Broadcast(NewTarget);
	}

	OnTargetChanged.Broadcast(OldTarget, NewTarget);
}

void USoftTargetingComponent::ValidateCurrentTarget()
{
	PruneIgnoredActors();

	if (ForcedTarget != nullptr && !IsValidTarget(ForcedTarget))
	{
		ClearForcedTarget();
	}

	if (CurrentTarget != nullptr && !IsValidTarget(CurrentTarget))
	{


		bTargetLocked = false;
		SetCurrentTargetInternal(nullptr);
	}
}

void USoftTargetingComponent::PruneIgnoredActors()
{
	IgnoredActors.RemoveAll([](const TWeakObjectPtr<AActor>& Entry)
	{
		return !Entry.IsValid();
	});

	const UWorld* const World = GetWorld();
	const float Now = World ? World->GetTimeSeconds() : 0.0f;
	const float VisibilityHistoryLifetime = FMath::Max(LineOfSightGraceTime + TargetRefreshInterval * 2.0f, 1.0f);

	for (auto It = LastVisibleTargetTimes.CreateIterator(); It; ++It)
	{
		if (!It.Key().IsValid() || (World != nullptr && (Now - It.Value()) > VisibilityHistoryLifetime))
		{
			It.RemoveCurrent();
		}
	}
}

void USoftTargetingComponent::UpdateCandidateEvents(const TArray<FSoftTargetData>& Targets)
{
	TArray<TWeakObjectPtr<AActor>> NewCandidates;
	NewCandidates.Reserve(Targets.Num());

	for (const FSoftTargetData& Entry : Targets)
	{
		if (AActor* const Actor = Entry.Actor.Get())
		{
			NewCandidates.AddUnique(Actor);
			if (!PreviousCandidates.Contains(Actor))
			{
				OnCandidateEntered.Broadcast(Actor);
			}
		}
	}

	for (const TWeakObjectPtr<AActor>& Previous : PreviousCandidates)
	{
		if (AActor* const Actor = Previous.Get())
		{
			if (!NewCandidates.Contains(Actor))
			{
				OnCandidateExited.Broadcast(Actor);
			}
		}
	}

	PreviousCandidates = MoveTemp(NewCandidates);
}

void USoftTargetingComponent::UpdateCurrentTargetStatusEvents()
{
	AActor* const Target = GetCurrentTarget();
	if (Target == nullptr)
	{
		StatusEventTarget.Reset();
		bHasCurrentTargetStatusSample = false;
		return;
	}

	FSoftTargetingContext Context;
	BuildContext(Context);

	FSoftTargetData Data(Target);
	Data.TargetLocation = GetTargetLocation(Target);
	const bool bVisible = !bRequireLineOfSight || HasLineOfSightTo(Context, Data, CachedTargets);
	const bool bInRange = IsTargetInAttackRange(Target, DefaultAttackRange);

	if (StatusEventTarget.Get() != Target || !bHasCurrentTargetStatusSample)
	{
		StatusEventTarget = Target;
		bCurrentTargetWasVisible = bVisible;
		bCurrentTargetWasInAttackRange = bInRange;
		bHasCurrentTargetStatusSample = true;
		return;
	}

	if (bCurrentTargetWasVisible != bVisible)
	{
		(bVisible ? OnTargetBecameVisible : OnTargetBecameObstructed).Broadcast(Target);
		bCurrentTargetWasVisible = bVisible;
	}

	if (bCurrentTargetWasInAttackRange != bInRange)
	{
		(bInRange ? OnTargetBackInRange : OnTargetLeftAttackRange).Broadcast(Target);
		bCurrentTargetWasInAttackRange = bInRange;
	}
}

void USoftTargetingComponent::ClearDebugScreenMessage() const
{
#if !UE_BUILD_SHIPPING
	if (GEngine != nullptr)
	{
		GEngine->RemoveOnScreenDebugMessage(static_cast<uint64>(GetUniqueID()));
	}
#endif
}

bool USoftTargetingComponent::IsActorIgnored(const AActor* Actor) const
{
	if (Actor == nullptr)
	{
		return true;
	}

	for (const TWeakObjectPtr<AActor>& Entry : IgnoredActors)
	{
		if (Entry.Get() == Actor)
		{
			return true;
		}
	}

	return false;
}

bool USoftTargetingComponent::PassesTagFilters(AActor* Target) const
{
	if (Target == nullptr)
	{
		return false;
	}


	for (const FName& BlockedTag : BlockedActorTags)
	{
		if (Target->ActorHasTag(BlockedTag))
		{
			return false;
		}
	}

	if (RequiredActorTags.Num() > 0)
	{
		bool bHasRequiredTag = false;
		for (const FName& RequiredTag : RequiredActorTags)
		{
			if (Target->ActorHasTag(RequiredTag))
			{
				bHasRequiredTag = true;
				break;
			}
		}

		if (!bHasRequiredTag)
		{
			return false;
		}
	}


	const bool bNeedsGameplayTags = !RequiredGameplayTags.IsEmpty() || !BlockedGameplayTags.IsEmpty();
	if (!bNeedsGameplayTags)
	{
		return true;
	}

	const IGameplayTagAssetInterface* const TagInterface = Cast<IGameplayTagAssetInterface>(Target);
	if (TagInterface == nullptr)
	{

		return RequiredGameplayTags.IsEmpty();
	}

	FGameplayTagContainer OwnedTags;
	TagInterface->GetOwnedGameplayTags(OwnedTags);

	if (!BlockedGameplayTags.IsEmpty() && OwnedTags.HasAny(BlockedGameplayTags))
	{
		return false;
	}

	if (!RequiredGameplayTags.IsEmpty())
	{
		const bool bMatches = bRequireAllGameplayTags
			? OwnedTags.HasAll(RequiredGameplayTags)
			: OwnedTags.HasAny(RequiredGameplayTags);

		if (!bMatches)
		{
			return false;
		}
	}

	return true;
}

float USoftTargetingComponent::AngleToScore(float AngleDegrees, float MaxAngleDegrees)
{
	if (MaxAngleDegrees <= KINDA_SMALL_NUMBER)
	{
		return (AngleDegrees <= KINDA_SMALL_NUMBER) ? 1.0f : 0.0f;
	}

	return FMath::Clamp(1.0f - (AngleDegrees / MaxAngleDegrees), 0.0f, 1.0f);
}


void USoftTargetingComponent::DrawDebugTargeting(const FSoftTargetingContext& Context, const TArray<FSoftTargetData>& Targets) const
{
#if ENABLE_DRAW_DEBUG
	const UWorld* const World = GetWorld();
	if (World == nullptr || Context.Owner == nullptr)
	{
		return;
	}


	constexpr float Duration = 0.0f;

	const FVector Origin = Context.OwnerLocation;
	const FVector DrawOrigin = Origin + FVector(0.0f, 0.0f, 10.0f);
	FVector DebugRight = FVector::CrossProduct(FVector::UpVector, Context.CameraForward).GetSafeNormal();
	if (DebugRight.IsNearlyZero())
	{
		DebugRight = Context.Owner->GetActorRightVector().GetSafeNormal();
	}


	if (bDebugDrawDetectionRadius)
	{
		DrawDebugCircle(World, DrawOrigin, TargetingRadius, 48, FColor(60, 60, 60), false, Duration, 0, 1.0f, FVector(1, 0, 0), FVector(0, 1, 0), false);
	}

	if (bDebugDrawAttackRange)
	{
		DrawDebugCircle(World, DrawOrigin, DefaultAttackRange, 32, FColor(120, 60, 0), false, Duration, 0, 1.5f, FVector(1, 0, 0), FVector(0, 1, 0), false);
	}

	if (bDebugDrawAttackCone && DefaultAttackRange > KINDA_SMALL_NUMBER)
	{
		const float HalfAngle = FMath::Clamp(DefaultAttackAngle, 0.0f, 180.0f);
		const int32 SegmentCount = FMath::Clamp(DebugAttackConeSegments, 4, 64);
		const FVector LeftEdge = Context.OwnerForward.RotateAngleAxis(-HalfAngle, FVector::UpVector);
		const FVector RightEdge = Context.OwnerForward.RotateAngleAxis(HalfAngle, FVector::UpVector);
		const FColor ConeColor(80, 180, 255);

		DrawDebugLine(World, DrawOrigin, DrawOrigin + LeftEdge * DefaultAttackRange, ConeColor, false, Duration, 0, 1.5f);
		DrawDebugLine(World, DrawOrigin, DrawOrigin + RightEdge * DefaultAttackRange, ConeColor, false, Duration, 0, 1.5f);

		FVector PreviousPoint = DrawOrigin + LeftEdge * DefaultAttackRange;
		for (int32 Segment = 1; Segment <= SegmentCount; ++Segment)
		{
			const float Alpha = static_cast<float>(Segment) / static_cast<float>(SegmentCount);
			const float Angle = FMath::Lerp(-HalfAngle, HalfAngle, Alpha);
			const FVector ArcPoint = DrawOrigin + Context.OwnerForward.RotateAngleAxis(Angle, FVector::UpVector) * DefaultAttackRange;
			DrawDebugLine(World, PreviousPoint, ArcPoint, ConeColor, false, Duration, 0, 1.5f);
			PreviousPoint = ArcPoint;
		}
	}

	if (bDebugDrawDirections)
	{
		DrawDebugLine(World, DrawOrigin, DrawOrigin + Context.OwnerForward * 150.0f, FColor::White, false, Duration, 0, 2.0f);

		if (Context.bHasInput)
		{
			DrawDebugDirectionalArrow(World, DrawOrigin, DrawOrigin + Context.InputDirection * 200.0f, 40.0f, FColor::Cyan, false, Duration, 0, 3.0f);
		}
	}

	for (int32 Index = 0; Index < Targets.Num(); ++Index)
	{
		const FSoftTargetData& Data = Targets[Index];
		AActor* const Actor = Data.Actor.Get();

		if (Actor == nullptr)
		{
			continue;
		}

		const bool bIsCurrent = (Actor == CurrentTarget);

		FColor Color = FColor::Yellow;
		if (!Data.bHasLineOfSight)
		{
			Color = FColor::Red;
		}
		else if (bIsCurrent)
		{
			Color = FColor::Green;
		}
		else if (Data.bInAttackRange)
		{
			Color = FColor::Orange;
		}

		if (bDebugDrawTargetLines)
		{
			DrawDebugLine(World, DrawOrigin, Data.TargetLocation, Color, false, Duration, 0, bIsCurrent ? 3.0f : 1.0f);
		}

		if (bDebugDrawTargetSpheres)
		{
			DrawDebugSphere(World, Data.TargetLocation, bIsCurrent ? 30.0f : 18.0f, 12, Color, false, Duration, 0, 1.0f);
		}

		if (bDebugDrawCandidateText)
		{
			FString Text;
			if (bDebugVerboseScores)
			{
				Text = FString::Printf(
					TEXT("#%d %s\nScore %.3f%s\nDir %.2f (%.0f deg)\nInput %.2f\nCam %.2f\nDist %.2f (%.0f cm)\n%s%s"),
					Index,
					*Actor->GetName(),
					Data.FinalScore,
					(Data.PersistenceBonus > 0.0f) ? *FString::Printf(TEXT(" (+%.2f)"), Data.PersistenceBonus) : TEXT(""),
					Data.DirectionScore,
					Data.Angle,
					Data.InputScore,
					Data.CameraScore,
					Data.DistanceScore,
					Data.Distance,
					Data.bInAttackRange ? TEXT("IN RANGE") : TEXT("out of range"),
					Data.bHasLineOfSight ? TEXT("") : TEXT("\nNO LOS"));
			}
			else
			{
				Text = FString::Printf(TEXT("#%d %.3f"), Index, Data.FinalScore);
			}

			const FVector CandidateTextLocation = Data.TargetLocation
				+ DebugRight * DebugCandidateTextSideOffset
				+ FVector::UpVector * DebugCandidateTextHeight;
			DrawDebugString(World, CandidateTextLocation, Text, nullptr, Color, Duration,  true, 1.0f);
		}
	}


	const FString StateText = FString::Printf(
		TEXT("SoftTargeting\nTarget: %s\nCandidates: %d\nInput: %.2f%s%s%s"),
		*GetNameSafe(GetCurrentTarget()),
		Targets.Num(),
		Context.InputMagnitude,
		Context.bHasInput ? TEXT("") : TEXT(" (dead zone)"),
		bTargetLocked ? TEXT("\nLOCKED") : TEXT(""),
		IsValid(ForcedTarget) ? TEXT("\nFORCED") : TEXT(""));

	if (bDebugDrawStateText)
	{
		if (bDebugStateTextOnScreen && GEngine != nullptr)
		{
			const float Scale = FMath::Clamp(DebugScreenTextScale, 0.5f, 3.0f);
			GEngine->AddOnScreenDebugMessage(
				static_cast<uint64>(GetUniqueID()),
				0.0f,
				FColor::White,
				StateText,
				false,
				FVector2D(Scale, Scale));
		}
		else
		{
			ClearDebugScreenMessage();
			const FVector StateTextLocation = Origin
				+ DebugRight * DebugStateTextSideOffset
				+ FVector::UpVector * DebugStateTextHeight;
			DrawDebugString(World, StateTextLocation, StateText, nullptr, FColor::White, Duration, true, 1.1f);
		}
	}
	else
	{
		ClearDebugScreenMessage();
	}
#endif
}
