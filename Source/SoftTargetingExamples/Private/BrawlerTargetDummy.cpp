

#include "BrawlerTargetDummy.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"

ABrawlerTargetDummy::ABrawlerTargetDummy()
{
	PrimaryActorTick.bCanEverTick = false;

	GetCapsuleComponent()->InitCapsuleSize(35.0f, 90.0f);


	GetMesh()->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, -90.0f), FRotator(0.0f, -90.0f, 0.0f));


	Tags.Add(FName("Enemy"));
}

void ABrawlerTargetDummy::BeginPlay()
{
	Super::BeginPlay();

	CurrentHP = MaxHP;
}


bool ABrawlerTargetDummy::CanBeSoftTargeted_Implementation() const
{


	return IsTargetAlive_Implementation() && !bIsKnockedOut && !bIsInvulnerable;
}

bool ABrawlerTargetDummy::IsTargetAlive_Implementation() const
{
	return CurrentHP > 0.0f;
}

FVector ABrawlerTargetDummy::GetSoftTargetLocation_Implementation() const
{
	if (!SoftTargetSocketName.IsNone())
	{

		if (const USkeletalMeshComponent* const CharacterMesh = GetMesh())
		{
			if (CharacterMesh->DoesSocketExist(SoftTargetSocketName))
			{
				return CharacterMesh->GetSocketLocation(SoftTargetSocketName);
			}
		}
	}


	return GetActorLocation() + FVector(0.0f, 0.0f, SoftTargetFallbackHeight);
}

float ABrawlerTargetDummy::GetSoftTargetPriority_Implementation() const
{
	return SoftTargetPriority;
}


void ABrawlerTargetDummy::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	TagContainer.Reset();
	TagContainer.AppendTags(BaseGameplayTags);

	if (IsTargetAlive_Implementation())
	{
		if (AliveTag.IsValid())
		{
			TagContainer.AddTag(AliveTag);
		}
	}
	else if (DeadTag.IsValid())
	{
		TagContainer.AddTag(DeadTag);
	}

	if (bIsKnockedOut && KnockedOutTag.IsValid())
	{
		TagContainer.AddTag(KnockedOutTag);
	}
}


void ABrawlerTargetDummy::ApplyTestDamage(float Damage)
{
	if (CurrentHP <= 0.0f)
	{
		return;
	}

	CurrentHP = FMath::Max(0.0f, CurrentHP - Damage);

	if (CurrentHP <= 0.0f)
	{
		bIsKnockedOut = true;
	}
}

void ABrawlerTargetDummy::SetKnockedOut(bool bKnockedOut)
{
	bIsKnockedOut = bKnockedOut;
}

void ABrawlerTargetDummy::ResetDummy()
{
	CurrentHP = MaxHP;
	bIsKnockedOut = false;
	bIsInvulnerable = false;
}
