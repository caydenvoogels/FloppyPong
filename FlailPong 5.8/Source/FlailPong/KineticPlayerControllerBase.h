#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "KineticPlayerControllerBase.generated.h"

UCLASS()
class FLAILPONG_API AKineticPlayerControllerBase : public APlayerController
{
	GENERATED_BODY()

public:
	AKineticPlayerControllerBase();

	virtual void BeginPlay() override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Camera")
	TSubclassOf<AActor> GameCameraClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Camera", meta=(ClampMin="0.0", UIMin="0.0"))
	float CameraBlendTime = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Camera")
	TEnumAsByte<EViewTargetBlendFunction> CameraBlendFunction = VTBlend_Linear;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Camera")
	float CameraBlendExp = 0.0f;
};
