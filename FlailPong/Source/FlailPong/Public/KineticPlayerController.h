#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "KineticPlayerController.generated.h"

UCLASS()
class FLAILPONG_API AKineticPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AKineticPlayerController();

	/**
	 * Converts the current mouse cursor position to a world-space point on the Y=0 gameplay plane.
	 * Returns false when the mouse ray cannot be projected onto that plane.
	 */
	UFUNCTION(BlueprintCallable, Category = "Kinetic|Input", meta = (ToolTip = "Projects the mouse cursor onto the Y=0 gameplay plane."))
	bool GetMouseWorldPosition(FVector& WorldPosition) const;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kinetic|Input", meta = (ClampMin = "0.000001", ToolTip = "Minimum absolute mouse-ray Y direction required before intersecting the Y=0 plane."))
	double MinPlaneRayDirectionY;
};
