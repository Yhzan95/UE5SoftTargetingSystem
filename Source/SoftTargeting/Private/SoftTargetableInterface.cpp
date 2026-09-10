

#include "SoftTargetableInterface.h"
#include "GameFramework/Actor.h"

bool ISoftTargetableInterface::CanBeSoftTargeted_Implementation() const
{

	return IsTargetAlive_Implementation();
}

bool ISoftTargetableInterface::IsTargetAlive_Implementation() const
{
	return true;
}

FVector ISoftTargetableInterface::GetSoftTargetLocation_Implementation() const
{


	if (const AActor* const OwnerActor = Cast<AActor>(this))
	{


		const float HalfHeight = OwnerActor->GetSimpleCollisionHalfHeight();
		return OwnerActor->GetActorLocation() + FVector(0.0f, 0.0f, HalfHeight * 0.5f);
	}

	return FVector::ZeroVector;
}

float ISoftTargetableInterface::GetSoftTargetPriority_Implementation() const
{

	return 0.5f;
}
