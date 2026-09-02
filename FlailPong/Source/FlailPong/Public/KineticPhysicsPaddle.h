#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KineticPhysicsPaddle.generated.h"

class UPhysicsConstraintComponent;
class UBoxComponent;
class USphereComponent;
class UStaticMeshComponent;

UCLASS()
class FLAILPONG_API AKineticPhysicsPaddle : public AActor
{
	GENERATED_BODY()

public:
	AKineticPhysicsPaddle();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

	/** Moves the physics pivot without directly moving the simulated paddle body. */
	UFUNCTION(BlueprintCallable, Category = "Kinetic|Paddle", meta = (ToolTip = "Moves the paddle pivot. The simulated paddle body is not moved directly."))
	void SetAnchorWorldPosition(const FVector& NewPosition);

	UFUNCTION(BlueprintPure, Category = "Kinetic|Paddle", meta = (ToolTip = "Returns the current world position of the paddle pivot."))
	FVector GetAnchorWorldPosition() const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kinetic|Components")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kinetic|Components")
	TObjectPtr<USphereComponent> AnchorBody;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kinetic|Components")
	TObjectPtr<UStaticMeshComponent> AnchorVisual;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kinetic|Components")
	TObjectPtr<UBoxComponent> PaddleBody;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kinetic|Components")
	TObjectPtr<UStaticMeshComponent> PaddleVisual;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kinetic|Components")
	TObjectPtr<UPhysicsConstraintComponent> PhysicsConstraint;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kinetic|Paddle", meta = (ClampMin = "1.0", ToolTip = "Full visual/physics paddle length in centimeters. Default cube meshes use 100 cm as their base size."))
	double PaddleLengthCm;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kinetic|Paddle", meta = (ToolTip = "Paddle mass override used by the physics body."))
	double PaddleMassKg;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kinetic|Paddle", meta = (ClampMin = "0.0", ToolTip = "Linear damping applied to the physics paddle."))
	double LinearDamping;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kinetic|Paddle", meta = (ClampMin = "0.0", ToolTip = "Angular damping applied to the physics paddle."))
	double AngularDamping;

private:
	void ConfigurePaddleBody() const;
	void ConfigureConstraint() const;
};
