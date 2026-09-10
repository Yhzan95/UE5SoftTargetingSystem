

#include "BrawlerCombatComponent.h"
#include "SoftTargetingComponent.h"

#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"
#include "Engine/World.h"

#if SOFTTARGETING_WITH_MOTION_WARPING
#include "MotionWarpingComponent.h"
#endif

UBrawlerCombatComponent::UBrawlerCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;


	PrimaryComponentTick.bStartWithTickEnabled = false;

	OnAttackMontageEnded.BindUObject(this, &UBrawlerCombatComponent::HandleAttackMontageEnded);


	{
		FBrawlerAttackData Data;
		Data.Range = 180.0f;
		Data.TargetAngle = 70.0f;
		Data.RangeTolerance = 60.0f;
		Data.RotationSpeed = 900.0f;
		Data.RotationDuration = 0.12f;
		Data.FallbackDuration = 0.40f;
		Attacks.Add(EBrawlerAttackType::LightPunch, Data);
	}


	{
		FBrawlerAttackData Data;
		Data.Range = 250.0f;
		Data.TargetAngle = 120.0f;
		Data.RangeTolerance = 80.0f;
		Data.RotationSpeed = 720.0f;
		Data.RotationDuration = 0.18f;
		Data.FallbackDuration = 0.60f;
		Attacks.Add(EBrawlerAttackType::Kick, Data);
	}


	{
		FBrawlerAttackData Data;
		Data.Range = 220.0f;
		Data.TargetAngle = 90.0f;
		Data.RangeTolerance = 100.0f;
		Data.RotationSpeed = 540.0f;
		Data.RotationDuration = 0.25f;
		Data.FallbackDuration = 0.90f;
		Attacks.Add(EBrawlerAttackType::HeavyAttack, Data);
	}


	{
		FBrawlerAttackData Data;
		Data.Range = 220.0f;
		Data.TargetAngle = 180.0f;
		Data.RangeTolerance = 40.0f;
		Data.bUseInputDirection = false;
		Data.RotationSpeed = 0.0f;
		Data.RotationDuration = 0.0f;
		Data.FallbackDuration = 0.70f;
		Attacks.Add(EBrawlerAttackType::Sweep, Data);
	}


	{
		FBrawlerAttackData Data;
		Data.Range = 450.0f;
		Data.TargetAngle = 45.0f;
		Data.RangeTolerance = 100.0f;
		Data.RotationSpeed = 1080.0f;
		Data.RotationDuration = 0.10f;
		Data.bUseMotionWarping = true;
		Data.WarpTargetName = FName("DashTarget");
		Data.WarpStopDistance = 140.0f;
		Data.FallbackDuration = 0.80f;
		Attacks.Add(EBrawlerAttackType::DashAttack, Data);
	}


	{
		FBrawlerAttackData Data;
		Data.Range = 160.0f;
		Data.TargetAngle = 60.0f;
		Data.RangeTolerance = 0.0f;
		Data.bRequireTargetInRange = true;
		Data.bForceTargetDuringAttack = true;
		Data.bAllowTargetSwitch = true;
		Data.RotationSpeed = 1200.0f;
		Data.RotationDuration = 0.10f;
		Data.bUseMotionWarping = true;
		Data.WarpTargetName = FName("GrabTarget");
		Data.WarpStopDistance = 100.0f;
		Data.FallbackDuration = 1.20f;
		Attacks.Add(EBrawlerAttackType::Grab, Data);
	}
}

void UBrawlerCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (const AActor* const Owner = GetOwner())
	{
		SoftTargeting = Owner->FindComponentByClass<USoftTargetingComponent>();
	}

	if (SoftTargeting == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: no USoftTargetingComponent found on the owner, attacks will run untargeted."), *GetNameSafe(GetOwner()));
	}
}

void UBrawlerCombatComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (const UWorld* const World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(FallbackAttackTimer);
	}

	Super::EndPlay(EndPlayReason);
}

void UBrawlerCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (RotationTimeRemaining <= 0.0f)
	{
		SetComponentTickEnabled(false);
		return;
	}

	AActor* const Owner = GetOwner();
	if (Owner == nullptr)
	{
		RotationTimeRemaining = 0.0f;
		return;
	}

	RotationTimeRemaining -= DeltaTime;


	if (SoftTargeting != nullptr && IsValid(AttackTarget))
	{
		const FVector ToTarget = SoftTargeting->GetTargetLocation(AttackTarget) - Owner->GetActorLocation();
		const FVector Flat = FVector(ToTarget.X, ToTarget.Y, 0.0f);

		if (!Flat.IsNearlyZero())
		{
			DesiredAttackRotation = Flat.Rotation();
			DesiredAttackRotation.Pitch = 0.0f;
			DesiredAttackRotation.Roll = 0.0f;
		}
	}

	const FRotator Current = Owner->GetActorRotation();
	const FRotator NewRotation = (ActiveRotationSpeed > 0.0f)
		? FMath::RInterpConstantTo(Current, DesiredAttackRotation, DeltaTime, ActiveRotationSpeed)
		: DesiredAttackRotation;

	Owner->SetActorRotation(FRotator(Current.Pitch, NewRotation.Yaw, Current.Roll));

	if (RotationTimeRemaining <= 0.0f)
	{
		SetComponentTickEnabled(false);
	}
}


bool UBrawlerCombatComponent::TryAttack(EBrawlerAttackType AttackType)
{
	if (bIsAttacking)
	{

		return false;
	}

	const FBrawlerAttackData* const AttackPtr = Attacks.Find(AttackType);
	if (AttackPtr == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: no attack data configured for attack type %d."), *GetNameSafe(GetOwner()), static_cast<int32>(AttackType));
		return false;
	}

	const FBrawlerAttackData Attack = *AttackPtr;

	AActor* Target = nullptr;

	if (SoftTargeting != nullptr)
	{
		if (Attack.bAllowTargetSwitch)
		{


			const FSoftTargetingRequest Request = BuildRequest(Attack);
			Target = SoftTargeting->FindBestTarget(Request);
		}
		else
		{

			Target = SoftTargeting->GetCurrentTarget();
		}


		if (Attack.bRequireTargetInRange)
		{
			if (Target == nullptr || !SoftTargeting->IsTargetInAttackRange(Target, Attack.Range))
			{
				return false;
			}
		}
	}

	StartAttack(AttackType, Attack, Target);
	return true;
}

void UBrawlerCombatComponent::CancelAttack()
{
	if (!bIsAttacking)
	{
		return;
	}

	if (const ACharacter* const Character = GetOwner<ACharacter>())
	{
		if (const USkeletalMeshComponent* const Mesh = Character->GetMesh())
		{
			if (UAnimInstance* const AnimInstance = Mesh->GetAnimInstance())
			{
				AnimInstance->Montage_Stop(0.15f);
			}
		}
	}

	FinishAttack();
}

AActor* UBrawlerCombatComponent::GetAttackTarget() const
{
	return IsValid(AttackTarget) ? AttackTarget.Get() : nullptr;
}

bool UBrawlerCombatComponent::GetAttackData(EBrawlerAttackType AttackType, FBrawlerAttackData& OutData) const
{
	if (const FBrawlerAttackData* const Found = Attacks.Find(AttackType))
	{
		OutData = *Found;
		return true;
	}

	OutData = FBrawlerAttackData();
	return false;
}


FSoftTargetingRequest UBrawlerCombatComponent::BuildRequest(const FBrawlerAttackData& Attack) const
{
	FSoftTargetingRequest Request;

	if (SoftTargeting != nullptr)
	{
		Request = SoftTargeting->MakeDefaultRequest();
	}

	Request.MaxRange = Attack.Range;
	Request.MaxAngle = Attack.TargetAngle;
	Request.RangeTolerance = Attack.RangeTolerance;
	Request.bUseInputDirection = Attack.bUseInputDirection;
	Request.bRequireLineOfSight = Attack.bRequireLineOfSight;


	Request.bRespectTargetLock = false;
	Request.bUpdateCurrentTarget = true;
	Request.bApplyPersistence = true;

	return Request;
}

void UBrawlerCombatComponent::StartAttack(EBrawlerAttackType AttackType, const FBrawlerAttackData& Attack, AActor* Target)
{
	bIsAttacking = true;
	CurrentAttackType = AttackType;
	AttackTarget = Target;

	if (SoftTargeting != nullptr && IsValid(Target))
	{

		if (Attack.bForceTargetDuringAttack)
		{
			SoftTargeting->ForceTarget(Target);
		}
		else if (Attack.bLockTargetDuringAttack)
		{
			SoftTargeting->LockCurrentTarget();
		}

		BeginRotationAssist(Attack, Target);

		if (Attack.bUseMotionWarping)
		{
			ApplyMotionWarping(Attack, Target);
		}
	}

	const float Duration = PlayAttackAnimation(Attack);

	OnAttackStarted.Broadcast(AttackType, Target);


	if (Duration <= 0.0f)
	{

		FinishAttack();
	}
}

void UBrawlerCombatComponent::BeginRotationAssist(const FBrawlerAttackData& Attack, AActor* Target)
{
	AActor* const Owner = GetOwner();
	if (Owner == nullptr || SoftTargeting == nullptr || !IsValid(Target))
	{
		return;
	}

	if (Attack.RotationDuration <= 0.0f && Attack.RotationSpeed <= 0.0f)
	{
		return;
	}


	DesiredAttackRotation = SoftTargeting->GetRotationToCurrentTarget();

	const FVector ToTarget = SoftTargeting->GetTargetLocation(Target) - Owner->GetActorLocation();
	const FVector Flat = FVector(ToTarget.X, ToTarget.Y, 0.0f);

	if (!Flat.IsNearlyZero())
	{
		DesiredAttackRotation = Flat.Rotation();
		DesiredAttackRotation.Pitch = 0.0f;
		DesiredAttackRotation.Roll = 0.0f;
	}

	ActiveRotationSpeed = Attack.RotationSpeed;

	if (Attack.RotationDuration <= 0.0f)
	{

		const FRotator Current = Owner->GetActorRotation();
		Owner->SetActorRotation(FRotator(Current.Pitch, DesiredAttackRotation.Yaw, Current.Roll));
		RotationTimeRemaining = 0.0f;
		return;
	}

	RotationTimeRemaining = Attack.RotationDuration;
	SetComponentTickEnabled(true);
}

void UBrawlerCombatComponent::ApplyMotionWarping(const FBrawlerAttackData& Attack, AActor* Target)
{
	if (SoftTargeting == nullptr || !IsValid(Target))
	{
		return;
	}

	FVector WarpLocation = FVector::ZeroVector;
	FRotator WarpRotation = FRotator::ZeroRotator;


	if (!SoftTargeting->GetMotionWarpTransform(Target, Attack.WarpStopDistance, WarpLocation, WarpRotation))
	{
		return;
	}

#if SOFTTARGETING_WITH_MOTION_WARPING
	if (AActor* const Owner = GetOwner())
	{
		if (UMotionWarpingComponent* const WarpComponent = Owner->FindComponentByClass<UMotionWarpingComponent>())
		{
			WarpComponent->AddOrUpdateWarpTargetFromLocationAndRotation(Attack.WarpTargetName, WarpLocation, WarpRotation);
		}
	}
#else


	UE_LOG(LogTemp, Verbose, TEXT("Motion warp target '%s' would be set to %s / %s"),
		*Attack.WarpTargetName.ToString(), *WarpLocation.ToString(), *WarpRotation.ToString());
#endif
}

float UBrawlerCombatComponent::PlayAttackAnimation(const FBrawlerAttackData& Attack)
{
	ACharacter* const Character = GetOwner<ACharacter>();

	if (Attack.Montage != nullptr && Character != nullptr)
	{
		if (const USkeletalMeshComponent* const Mesh = Character->GetMesh())
		{
			if (UAnimInstance* const AnimInstance = Mesh->GetAnimInstance())
			{
				const float Duration = AnimInstance->Montage_Play(Attack.Montage);

				if (Duration > 0.0f)
				{
					if (!Attack.MontageSection.IsNone())
					{
						AnimInstance->Montage_JumpToSection(Attack.MontageSection, Attack.Montage);
					}

					AnimInstance->Montage_SetEndDelegate(OnAttackMontageEnded, Attack.Montage);
					return Duration;
				}
			}
		}
	}


	if (UWorld* const World = GetWorld())
	{
		const float Duration = FMath::Max(Attack.FallbackDuration, 0.05f);
		World->GetTimerManager().SetTimer(FallbackAttackTimer, this, &UBrawlerCombatComponent::HandleFallbackAttackFinished, Duration, false);
		return Duration;
	}

	return 0.0f;
}

void UBrawlerCombatComponent::FinishAttack()
{
	const EBrawlerAttackType FinishedAttack = CurrentAttackType;

	if (const UWorld* const World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(FallbackAttackTimer);
	}

	if (SoftTargeting != nullptr)
	{


		SoftTargeting->UnlockCurrentTarget();
		SoftTargeting->ClearForcedTarget();
	}

	bIsAttacking = false;
	AttackTarget = nullptr;
	RotationTimeRemaining = 0.0f;
	SetComponentTickEnabled(false);

	OnAttackEnded.Broadcast(FinishedAttack);
}

void UBrawlerCombatComponent::HandleAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	FinishAttack();
}

void UBrawlerCombatComponent::HandleFallbackAttackFinished()
{
	FinishAttack();
}
