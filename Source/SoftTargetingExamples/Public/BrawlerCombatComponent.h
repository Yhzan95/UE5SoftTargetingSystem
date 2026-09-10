

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/TimerHandle.h"
#include "SoftTargetingTypes.h"
#include "BrawlerCombatComponent.generated.h"

class UAnimMontage;
class USoftTargetingComponent;


#ifndef SOFTTARGETING_WITH_MOTION_WARPING
#define SOFTTARGETING_WITH_MOTION_WARPING 0
#endif


UENUM(BlueprintType)
enum class EBrawlerAttackType : uint8
{
	LightPunch		UMETA(DisplayName = "Light Punch"),
	Kick			UMETA(DisplayName = "Kick"),
	HeavyAttack		UMETA(DisplayName = "Heavy Attack"),
	Sweep			UMETA(DisplayName = "Sweep"),
	DashAttack		UMETA(DisplayName = "Dash Attack"),
	Grab			UMETA(DisplayName = "Grab")
};


USTRUCT(BlueprintType)
struct FBrawlerAttackData
{
	GENERATED_BODY()


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	TObjectPtr<UAnimMontage> Montage = nullptr;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	FName MontageSection = NAME_None;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack|Targeting", meta = (ClampMin = 0, Units = "cm"))
	float Range = 180.0f;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack|Targeting", meta = (ClampMin = 0, ClampMax = 180, Units = "deg"))
	float TargetAngle = 70.0f;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack|Targeting", meta = (ClampMin = 0, Units = "cm"))
	float RangeTolerance = 80.0f;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack|Targeting")
	bool bUseInputDirection = true;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack|Targeting")
	bool bRequireLineOfSight = true;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack|Targeting")
	bool bAllowTargetSwitch = true;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack|Targeting")
	bool bLockTargetDuringAttack = true;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack|Targeting")
	bool bForceTargetDuringAttack = false;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack|Targeting")
	bool bRequireTargetInRange = false;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack|Rotation", meta = (ClampMin = 0))
	float RotationSpeed = 900.0f;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack|Rotation", meta = (ClampMin = 0, ClampMax = 2, Units = "s"))
	float RotationDuration = 0.15f;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack|Motion Warping")
	bool bUseMotionWarping = false;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack|Motion Warping")
	FName WarpTargetName = FName("AttackTarget");


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack|Motion Warping", meta = (ClampMin = 0, Units = "cm"))
	float WarpStopDistance = 120.0f;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack", meta = (ClampMin = 0.05, ClampMax = 5, Units = "s"))
	float FallbackDuration = 0.5f;
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBrawlerAttackStarted, EBrawlerAttackType, AttackType, AActor*, Target);


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBrawlerAttackEnded, EBrawlerAttackType, AttackType);


UCLASS(ClassGroup = (Combat), Blueprintable, meta = (BlueprintSpawnableComponent))
class SOFTTARGETINGEXAMPLES_API UBrawlerCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:


	UBrawlerCombatComponent();


	UPROPERTY(BlueprintAssignable, Category = "Brawler Combat|Events")
	FOnBrawlerAttackStarted OnAttackStarted;


	UPROPERTY(BlueprintAssignable, Category = "Brawler Combat|Events")
	FOnBrawlerAttackEnded OnAttackEnded;

protected:


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brawler Combat")
	TMap<EBrawlerAttackType, FBrawlerAttackData> Attacks;


	UPROPERTY(Transient, BlueprintReadOnly, Category = "Brawler Combat")
	TObjectPtr<USoftTargetingComponent> SoftTargeting = nullptr;


	UPROPERTY(Transient, BlueprintReadOnly, Category = "Brawler Combat")
	TObjectPtr<AActor> AttackTarget = nullptr;


	UPROPERTY(Transient, BlueprintReadOnly, Category = "Brawler Combat")
	bool bIsAttacking = false;


	UPROPERTY(Transient, BlueprintReadOnly, Category = "Brawler Combat")
	EBrawlerAttackType CurrentAttackType = EBrawlerAttackType::LightPunch;


	FRotator DesiredAttackRotation = FRotator::ZeroRotator;


	float RotationTimeRemaining = 0.0f;


	float ActiveRotationSpeed = 0.0f;


	FOnMontageEnded OnAttackMontageEnded;


	FTimerHandle FallbackAttackTimer;

public:


	virtual void BeginPlay() override;


	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;


	UFUNCTION(BlueprintCallable, Category = "Brawler Combat")
	bool TryAttack(EBrawlerAttackType AttackType);


	UFUNCTION(BlueprintCallable, Category = "Brawler Combat")
	void CancelAttack();


	UFUNCTION(BlueprintPure, Category = "Brawler Combat")
	bool IsAttacking() const { return bIsAttacking; }


	UFUNCTION(BlueprintPure, Category = "Brawler Combat")
	AActor* GetAttackTarget() const;


	UFUNCTION(BlueprintPure, Category = "Brawler Combat")
	bool GetAttackData(EBrawlerAttackType AttackType, FBrawlerAttackData& OutData) const;


	UFUNCTION(BlueprintPure, Category = "Brawler Combat")
	USoftTargetingComponent* GetSoftTargetingComponent() const { return SoftTargeting; }

protected:


	FSoftTargetingRequest BuildRequest(const FBrawlerAttackData& Attack) const;


	void StartAttack(EBrawlerAttackType AttackType, const FBrawlerAttackData& Attack, AActor* Target);


	void BeginRotationAssist(const FBrawlerAttackData& Attack, AActor* Target);


	void ApplyMotionWarping(const FBrawlerAttackData& Attack, AActor* Target);


	float PlayAttackAnimation(const FBrawlerAttackData& Attack);


	void FinishAttack();


	void HandleAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);


	void HandleFallbackAttackFinished();
};
