

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameplayTagContainer.h"
#include "GameplayTagAssetInterface.h"
#include "SoftTargetableInterface.h"
#include "BrawlerTargetDummy.generated.h"


UCLASS()
class SOFTTARGETINGEXAMPLES_API ABrawlerTargetDummy : public ACharacter, public ISoftTargetableInterface, public IGameplayTagAssetInterface
{
	GENERATED_BODY()

public:


	ABrawlerTargetDummy();

protected:


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brawler Dummy", meta = (ClampMin = 0))
	float MaxHP = 5.0f;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Brawler Dummy")
	float CurrentHP = 0.0f;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brawler Dummy")
	bool bIsKnockedOut = false;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brawler Dummy")
	bool bIsInvulnerable = false;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brawler Dummy", meta = (ClampMin = 0, ClampMax = 1))
	float SoftTargetPriority = 0.5f;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brawler Dummy")
	FName SoftTargetSocketName = FName("spine_03");


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brawler Dummy", meta = (ClampMin = 0, ClampMax = 300, Units = "cm"))
	float SoftTargetFallbackHeight = 60.0f;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brawler Dummy|Tags")
	FGameplayTagContainer BaseGameplayTags;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brawler Dummy|Tags")
	FGameplayTag AliveTag;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brawler Dummy|Tags")
	FGameplayTag DeadTag;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brawler Dummy|Tags")
	FGameplayTag KnockedOutTag;

public:


	virtual void BeginPlay() override;


	virtual bool CanBeSoftTargeted_Implementation() const override;


	virtual bool IsTargetAlive_Implementation() const override;


	virtual FVector GetSoftTargetLocation_Implementation() const override;


	virtual float GetSoftTargetPriority_Implementation() const override;


	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;


	UFUNCTION(BlueprintCallable, Category = "Brawler Dummy")
	void ApplyTestDamage(float Damage);


	UFUNCTION(BlueprintCallable, Category = "Brawler Dummy")
	void SetKnockedOut(bool bKnockedOut);


	UFUNCTION(BlueprintCallable, Category = "Brawler Dummy")
	void ResetDummy();


	UFUNCTION(BlueprintPure, Category = "Brawler Dummy")
	float GetCurrentHP() const { return CurrentHP; }
};
