#include "KineticPlayerController.h"

#include "Math/UnrealMathUtility.h"

AKineticPlayerController::AKineticPlayerController()
	: MinPlaneRayDirectionY(0.0001)
{
	bShowMouseCursor = true;
}

bool AKineticPlayerController::GetMouseWorldPosition(FVector& WorldPosition) const
{
	FVector WorldLocation = FVector::ZeroVector;
	FVector WorldDirection = FVector::ZeroVector;

	if (!DeprojectMousePositionToWorld(WorldLocation, WorldDirection))
	{
		WorldPosition = FVector::ZeroVector;
		return false;
	}

	if (FMath::Abs(WorldDirection.Y) <= MinPlaneRayDirectionY)
	{
		WorldPosition = FVector::ZeroVector;
		return false;
	}

	const double Distance = -WorldLocation.Y / WorldDirection.Y;
	WorldPosition = WorldLocation + (WorldDirection * Distance);
	WorldPosition.Y = 0.0;

	return true;
}
