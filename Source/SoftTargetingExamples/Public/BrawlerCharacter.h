

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BrawlerCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UInputAction;
class UInputMappingContext;
class USoftTargetingComponent;
class UBrawlerCombatComponent;
struct FInputActionValue;


UCLASS(abstract)
class SOFTTARGETINGEXAMPLES_API ABrawlerCharacter : public ACharacter
{
	GENERATED_BODY()


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FollowCamera;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USoftTargetingComponent> SoftTargeting;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBrawlerCombatComponent> CombatComponent;

protected:


	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;


	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;


	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> LookAction;


	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> PunchAction;


	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> KickAction;


	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> HeavyAttackAction;


	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> SweepAction;


	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> DashAttackAction;


	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> GrabAction;

public:


	ABrawlerCharacter();

protected:


	virtual void NotifyControllerChanged() override;


	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;


	void Move(const FInputActionValue& Value);


	void MoveCompleted();


	void Look(const FInputActionValue& Value);


	void PunchPressed();
	void KickPressed();
	void HeavyAttackPressed();
	void SweepPressed();
	void DashAttackPressed();
	void GrabPressed();

public:


	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoMove(float Right, float Forward);


	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoLook(float Yaw, float Pitch);


	UFUNCTION(BlueprintPure, Category = "Components")
	USoftTargetingComponent* GetSoftTargetingComponent() const { return SoftTargeting; }


	UFUNCTION(BlueprintPure, Category = "Components")
	UBrawlerCombatComponent* GetBrawlerCombatComponent() const { return CombatComponent; }


	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }


	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};
