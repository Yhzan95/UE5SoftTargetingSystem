

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SoftTargetableInterface.generated.h"


UINTERFACE(MinimalAPI, Blueprintable, BlueprintType)
class USoftTargetableInterface : public UInterface
{
	GENERATED_BODY()
};

class SOFTTARGETING_API ISoftTargetableInterface
{
	GENERATED_BODY()

public:


	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Soft Targeting")
	bool CanBeSoftTargeted() const;
	virtual bool CanBeSoftTargeted_Implementation() const;


	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Soft Targeting")
	bool IsTargetAlive() const;
	virtual bool IsTargetAlive_Implementation() const;


	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Soft Targeting")
	FVector GetSoftTargetLocation() const;
	virtual FVector GetSoftTargetLocation_Implementation() const;


	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Soft Targeting")
	float GetSoftTargetPriority() const;
	virtual float GetSoftTargetPriority_Implementation() const;
};
