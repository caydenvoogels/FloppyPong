#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PongBall.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UPointLightComponent;
class UNiagaraComponent;

UCLASS()
class APongBall : public AActor
{
	GENERATED_BODY()

public:
	APongBall();
	virtual void Tick(float DeltaSeconds) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ball")
	TObjectPtr<USphereComponent> BallBody;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ball|Visual")
	TObjectPtr<UStaticMeshComponent> BallTrail;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ball|Visual")
	TObjectPtr<UPointLightComponent> BallGlow;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ball|Goals")
	TObjectPtr<UPointLightComponent> GoalBurstLight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ball|Goals")
	TObjectPtr<UNiagaraComponent> GoalBurstSparks;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ball|Physics")
	float PaddleVelocityTransfer = 0.85f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ball|Physics")
	float MinimumSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ball|Physics")
	float MaximumSpeed = 2600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ball|Physics", meta=(ClampMin="0.1", UIMin="0.1"))
	float BallMassKg = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ball|Physics")
	float PlayPlaneY = -120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ball|Goals")
	float GoalBoundaryX = 1550.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ball|Goals")
	float GoalHalfHeight = 560.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ball|Goals")
	float ServeSpeed = 850.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ball|Goals")
	int32 OrangeScore = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ball|Goals")
	int32 BlueScore = 0;

	bool bGoalResetting = false;
	void CheckGoalEntry();
	void UpdateScoreDisplays() const;
	void TriggerGoalBurst(float GoalX, const FLinearColor& Color);
	float GoalBurstTimeRemaining = 0.0f;
};
