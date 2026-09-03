#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FlailTetherComponent.generated.h"

class UMaterialInterface;
class UPhysicsConstraintComponent;
class UPhysicsHandleComponent;
class USceneComponent;
class UStaticMesh;
class UStaticMeshComponent;

UENUM(BlueprintType)
enum class EFlailTetherSide : uint8
{
	Orange,
	Blue
};

UENUM(BlueprintType)
enum class EFlailTetherPhysicsMode : uint8
{
	VisualOnly,
	SoftPull,
	HardLimit
};

UENUM(BlueprintType)
enum class EFlailHeadControlMode : uint8
{
	KinematicBlueprintDriven,
	StabilizedPhysics
};

UCLASS(ClassGroup=(FlailPong), meta=(BlueprintSpawnableComponent))
class FLAILPONG_API UFlailTetherComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFlailTetherComponent();

	virtual void OnRegister() override;
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tether")
	TObjectPtr<AActor> TargetActor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tether")
	bool bAutoResolveTargetActor = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tether", meta=(EditCondition="bAutoResolveTargetActor"))
	FName TargetActorPropertyName = TEXT("FlailHead");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tether")
	FName StartComponentName = TEXT("PaddleBody");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tether")
	FName EndComponentName = TEXT("FlailBody");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tether")
	FVector StartLocalOffset = FVector(0.0f, 0.0f, 35.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tether")
	FVector EndLocalOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Physics", meta=(ClampMin="1.0", UIMin="1.0"))
	float MaxLength = 420.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Physics", meta=(ClampMin="0.0", UIMin="0.0"))
	float PullStrength = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Physics", meta=(ClampMin="0.0", UIMin="0.0"))
	float DampingStrength = 80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Physics", meta=(ClampMin="0.0", UIMin="0.0"))
	float MaxPullForce = 12000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Physics", meta=(ClampMin="0.0", UIMin="0.0"))
	float MaxAnchorSpeed = 6500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Physics", meta=(ClampMin="0.0", ClampMax="1.0", UIMin="0.0", UIMax="1.0"))
	float AnchorVelocityInfluence = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Physics")
	bool bPullStartBody = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Physics")
	EFlailTetherPhysicsMode PhysicsMode = EFlailTetherPhysicsMode::VisualOnly;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Physics")
	bool bUseSlackTetherPhysicsAtRuntime = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Physics")
	bool bEnforceHardLength = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Plane Constraint")
	bool bConstrainToXZPlane = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mouse Follow")
	bool bDriveTargetHeadFromMouse = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mouse Follow", meta=(EditCondition="bDriveTargetHeadFromMouse", ClampMin="0.0", UIMin="0.0"))
	float MouseFollowInterpSpeed = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mouse Follow", meta=(EditCondition="bDriveTargetHeadFromMouse"))
	bool bFreezeMouseDrivenHeadRotation = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tether")
	EFlailTetherSide Side = EFlailTetherSide::Orange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tether", meta=(ClampMin="0.1", UIMin="0.1"))
	float Radius = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tether", meta=(ClampMin="1.0", UIMin="1.0"))
	float MinimumVisibleLength = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tether")
	TObjectPtr<UStaticMesh> TetherMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tether")
	TObjectPtr<UMaterialInterface> OrangeMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tether")
	TObjectPtr<UMaterialInterface> BlueMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Legacy Cable")
	bool bHideLegacyCableComponent = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Legacy Cable", meta=(EditCondition="bHideLegacyCableComponent"))
	FName LegacyCableComponentName = TEXT("TheaterCable");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Head Stability")
	bool bTuneTargetHeadPhysics = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Head Stability", meta=(EditCondition="bTuneTargetHeadPhysics"))
	EFlailHeadControlMode HeadControlMode = EFlailHeadControlMode::StabilizedPhysics;

	UPROPERTY()
	bool bDisableStartBodyGravity = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Paddle Physics")
	bool bRestoreStartBodyPhysics = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Paddle Physics")
	bool bReleaseStartBodyConstraints = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Paddle Physics")
	bool bStartBodyUsesGravity = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Paddle Physics")
	bool bKeepStartBodyUpright = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Paddle Physics", meta=(ClampMin="0.0", UIMin="0.0"))
	float StartLinearDamping = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Paddle Physics", meta=(ClampMin="0.0", UIMin="0.0"))
	float StartAngularDamping = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Head Stability", meta=(EditCondition="bTuneTargetHeadPhysics", ClampMin="0.0", UIMin="0.0"))
	float HeadHandleLinearStiffness = 250000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Head Stability", meta=(EditCondition="bTuneTargetHeadPhysics", ClampMin="0.0", UIMin="0.0"))
	float HeadHandleLinearDamping = 50000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Head Stability", meta=(EditCondition="bTuneTargetHeadPhysics", ClampMin="0.0", UIMin="0.0"))
	float HeadHandleAngularStiffness = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Head Stability", meta=(EditCondition="bTuneTargetHeadPhysics", ClampMin="0.0", UIMin="0.0"))
	float HeadHandleAngularDamping = 50000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Head Stability", meta=(EditCondition="bTuneTargetHeadPhysics", ClampMin="0.0", UIMin="0.0"))
	float HeadHandleInterpolationSpeed = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Head Stability", meta=(EditCondition="bTuneTargetHeadPhysics", ClampMin="0.0", UIMin="0.0"))
	float HeadLinearDamping = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Head Stability", meta=(EditCondition="bTuneTargetHeadPhysics", ClampMin="0.0", UIMin="0.0"))
	float HeadAngularDamping = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Head Stability", meta=(EditCondition="bTuneTargetHeadPhysics", ClampMin="0.1", UIMin="0.1"))
	float HeadMassScale = 2.0f;

private:
	UPROPERTY(Transient)
	TObjectPtr<UStaticMeshComponent> TetherVisual = nullptr;

	TWeakObjectPtr<USceneComponent> CachedStartComponent;
	TWeakObjectPtr<USceneComponent> CachedEndComponent;
	TWeakObjectPtr<AActor> CachedTargetActor;
	TWeakObjectPtr<UPrimitiveComponent> StabilizedStartBody;
	TWeakObjectPtr<AActor> StabilizedTargetActor;
	TOptional<FQuat> LockedHeadRotation;
	TOptional<float> LockedStartY;
	TOptional<float> LockedEndY;
	TOptional<float> MouseFollowPlaneY;
	TOptional<FQuat> MouseFollowHeadRotation;
	FVector PreviousStartLocation = FVector::ZeroVector;
	bool bHasPreviousStartLocation = false;
	bool bWarnedMissingStart = false;
	bool bWarnedMissingEnd = false;
	bool bConfiguredFreeStartBody = false;

	void EnsureVisualComponent();
	void LoadDefaultAssets();
	void ResolveEndpoints();
	void ResolveTargetActor();
	void HideLegacyCable() const;
	void UpdateVisual();
	void DriveTargetHeadFromMouse(float DeltaTime);
	void ConfigureFreeStartBody();
	void ApplySlackTetherPhysics(float DeltaTime);
	void StabilizeControlledBodies();
	void ConstrainBodyToXZPlane(UPrimitiveComponent* Body, TOptional<float>& LockedY);
	void WarnIfEndpointMissing();

	USceneComponent* FindSceneComponentByName(AActor* Actor, FName ComponentName) const;
	FVector GetAnchorLocation(const USceneComponent* Component, const FVector& LocalOffset) const;
	FVector GetAnchorVelocity(const USceneComponent* Component, const FVector& CurrentAnchorLocation, float DeltaTime);
};
