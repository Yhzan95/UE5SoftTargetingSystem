

#include "BrawlerCharacter.h"
#include "BrawlerCombatComponent.h"
#include "SoftTargetingComponent.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/LocalPlayer.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"

ABrawlerCharacter::ABrawlerCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	GetCapsuleComponent()->InitCapsuleSize(35.0f, 90.0f);


	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 640.0f, 0.0f);
	GetCharacterMovement()->MaxWalkSpeed = 450.0f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->bEnableCameraLag = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;


	SoftTargeting = CreateDefaultSubobject<USoftTargetingComponent>(TEXT("SoftTargeting"));


	SoftTargeting->RequiredActorTags.Add(FName("Enemy"));

	CombatComponent = CreateDefaultSubobject<UBrawlerCombatComponent>(TEXT("BrawlerCombat"));

	Tags.Add(FName("Player"));
}

void ABrawlerCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	if (const APlayerController* const PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* const Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (DefaultMappingContext != nullptr)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}
}

void ABrawlerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* const EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction != nullptr)
		{
			EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABrawlerCharacter::Move);


			EnhancedInput->BindAction(MoveAction, ETriggerEvent::Completed, this, &ABrawlerCharacter::MoveCompleted);
			EnhancedInput->BindAction(MoveAction, ETriggerEvent::Canceled, this, &ABrawlerCharacter::MoveCompleted);
		}

		if (LookAction != nullptr)
		{
			EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABrawlerCharacter::Look);
		}


		if (PunchAction != nullptr)
		{
			EnhancedInput->BindAction(PunchAction, ETriggerEvent::Started, this, &ABrawlerCharacter::PunchPressed);
		}

		if (KickAction != nullptr)
		{
			EnhancedInput->BindAction(KickAction, ETriggerEvent::Started, this, &ABrawlerCharacter::KickPressed);
		}

		if (HeavyAttackAction != nullptr)
		{
			EnhancedInput->BindAction(HeavyAttackAction, ETriggerEvent::Started, this, &ABrawlerCharacter::HeavyAttackPressed);
		}

		if (SweepAction != nullptr)
		{
			EnhancedInput->BindAction(SweepAction, ETriggerEvent::Started, this, &ABrawlerCharacter::SweepPressed);
		}

		if (DashAttackAction != nullptr)
		{
			EnhancedInput->BindAction(DashAttackAction, ETriggerEvent::Started, this, &ABrawlerCharacter::DashAttackPressed);
		}

		if (GrabAction != nullptr)
		{
			EnhancedInput->BindAction(GrabAction, ETriggerEvent::Started, this, &ABrawlerCharacter::GrabPressed);
		}
	}
}


void ABrawlerCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();
	DoMove(MovementVector.X, MovementVector.Y);
}

void ABrawlerCharacter::MoveCompleted()
{
	if (SoftTargeting != nullptr)
	{
		SoftTargeting->ClearMovementInputDirection();
	}
}

void ABrawlerCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookVector = Value.Get<FVector2D>();
	DoLook(LookVector.X, LookVector.Y);
}

void ABrawlerCharacter::PunchPressed()
{
	if (CombatComponent != nullptr)
	{
		CombatComponent->TryAttack(EBrawlerAttackType::LightPunch);
	}
}

void ABrawlerCharacter::KickPressed()
{
	if (CombatComponent != nullptr)
	{
		CombatComponent->TryAttack(EBrawlerAttackType::Kick);
	}
}

void ABrawlerCharacter::HeavyAttackPressed()
{
	if (CombatComponent != nullptr)
	{
		CombatComponent->TryAttack(EBrawlerAttackType::HeavyAttack);
	}
}

void ABrawlerCharacter::SweepPressed()
{
	if (CombatComponent != nullptr)
	{
		CombatComponent->TryAttack(EBrawlerAttackType::Sweep);
	}
}

void ABrawlerCharacter::DashAttackPressed()
{
	if (CombatComponent != nullptr)
	{
		CombatComponent->TryAttack(EBrawlerAttackType::DashAttack);
	}
}

void ABrawlerCharacter::GrabPressed()
{
	if (CombatComponent != nullptr)
	{
		CombatComponent->TryAttack(EBrawlerAttackType::Grab);
	}
}


void ABrawlerCharacter::DoMove(float Right, float Forward)
{
	const AController* const OwnerController = GetController();
	if (OwnerController == nullptr)
	{
		return;
	}


	const FRotator YawRotation(0.0f, OwnerController->GetControlRotation().Yaw, 0.0f);
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, Forward);
	AddMovementInput(RightDirection, Right);


	if (SoftTargeting != nullptr)
	{
		const FVector WorldInputDirection = (ForwardDirection * Forward) + (RightDirection * Right);
		SoftTargeting->SetMovementInputDirection(WorldInputDirection);
	}
}

void ABrawlerCharacter::DoLook(float Yaw, float Pitch)
{
	AddControllerYawInput(Yaw);
	AddControllerPitchInput(Pitch);
}
