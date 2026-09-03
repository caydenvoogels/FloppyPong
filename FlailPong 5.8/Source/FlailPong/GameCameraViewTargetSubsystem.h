#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameCameraViewTargetSubsystem.generated.h"

UCLASS()
class FLAILPONG_API UGameCameraViewTargetSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
	void SetGameCameraViewTarget(UWorld* World);
};
