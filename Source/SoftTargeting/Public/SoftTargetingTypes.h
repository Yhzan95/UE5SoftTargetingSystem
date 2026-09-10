

#pragma once

#include "CoreMinimal.h"
#include "SoftTargetingTypes.generated.h"

class AActor;


USTRUCT(BlueprintType)
struct FSoftTargetingRequest
{
	GENERATED_BODY()


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Targeting", meta = (ClampMin = 0, Units = "cm"))
	float MaxRange = 250.0f;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Targeting", meta = (ClampMin = 0, ClampMax = 180, Units = "deg"))
	float MaxAngle = 90.0f;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Targeting", meta = (ClampMin = 0, Units = "cm"))
	float RangeTolerance = 80.0f;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Targeting")
	bool bUseInputDirection = true;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Targeting")
	bool bUseCameraDirection = true;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Targeting")
	bool bRequireLineOfSight = true;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Targeting")
	bool bApplyPersistence = true;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Targeting")
	bool bRespectTargetLock = true;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Targeting")
	bool bUpdateCurrentTarget = true;
};


USTRUCT(BlueprintType)
struct FSoftTargetData
{
	GENERATED_BODY()


	UPROPERTY(BlueprintReadOnly, Category = "Soft Targeting")
	TObjectPtr<AActor> Actor = nullptr;


	UPROPERTY(BlueprintReadOnly, Category = "Soft Targeting")
	float FinalScore = 0.0f;


	UPROPERTY(BlueprintReadOnly, Category = "Soft Targeting")
	float Distance = 0.0f;


	UPROPERTY(BlueprintReadOnly, Category = "Soft Targeting")
	float Angle = 0.0f;


	UPROPERTY(BlueprintReadOnly, Category = "Soft Targeting")
	float DirectionScore = 0.0f;


	UPROPERTY(BlueprintReadOnly, Category = "Soft Targeting")
	float InputScore = 0.0f;


	UPROPERTY(BlueprintReadOnly, Category = "Soft Targeting")
	float CameraScore = 0.0f;


	UPROPERTY(BlueprintReadOnly, Category = "Soft Targeting")
	float DistanceScore = 0.0f;


	UPROPERTY(BlueprintReadOnly, Category = "Soft Targeting")
	float PriorityScore = 0.0f;


	UPROPERTY(BlueprintReadOnly, Category = "Soft Targeting")
	float PersistenceBonus = 0.0f;


	UPROPERTY(BlueprintReadOnly, Category = "Soft Targeting")
	bool bInAttackRange = false;


	UPROPERTY(BlueprintReadOnly, Category = "Soft Targeting")
	bool bHasLineOfSight = true;


	UPROPERTY(BlueprintReadOnly, Category = "Soft Targeting")
	FVector TargetLocation = FVector::ZeroVector;

	FSoftTargetData() = default;

	explicit FSoftTargetData(AActor* InActor)
		: Actor(InActor)
	{
	}
};


struct FSoftTargetingContext
{

	AActor* Owner = nullptr;


	FVector OwnerLocation = FVector::ZeroVector;


	FVector OwnerForward = FVector::ForwardVector;


	FVector CameraLocation = FVector::ZeroVector;


	FVector CameraForward = FVector::ForwardVector;


	FVector InputDirection = FVector::ZeroVector;


	float InputMagnitude = 0.0f;


	bool bHasInput = false;


	bool bHasCamera = false;
};
