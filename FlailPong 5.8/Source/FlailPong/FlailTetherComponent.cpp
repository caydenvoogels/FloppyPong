#include "FlailTetherComponent.h"

#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "UObject/UnrealType.h"

UFlailTetherComponent::UFlailTetherComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
	bTickInEditor = true;
}

void UFlailTetherComponent::OnRegister()
{
	Super::OnRegister();

	LoadDefaultAssets();
	EnsureVisualComponent();
	ResolveEndpoints();
	HideLegacyCable();
	ConfigureFreeStartBody();
	UpdateVisual();
}

void UFlailTetherComponent::BeginPlay()
{
	Super::BeginPlay();

	LoadDefaultAssets();
	EnsureVisualComponent();
	ResolveEndpoints();
	HideLegacyCable();
	ConfigureFreeStartBody();
	UpdateVisual();
}

void UFlailTetherComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	ResolveEndpoints();
	HideLegacyCable();
	WarnIfEndpointMissing();
	ConfigureFreeStartBody();
	if (UPrimitiveComponent* StartBody = Cast<UPrimitiveComponent>(CachedStartComponent.Get()))
	{
		ConstrainBodyToXZPlane(StartBody, LockedStartY);
	}
	DriveBluePaddleAI(DeltaTime);
	DriveTargetHeadFromMouse(DeltaTime);
	ApplySlackTetherPhysics(DeltaTime);
	ConstrainHeadToOwnHalf();
	ConstrainHeadToArenaBounds();
	UpdateVisual();
}

void UFlailTetherComponent::ConfigureFreeStartBody()
{
	if (bConfiguredFreeStartBody || !bReleaseStartBodyConstraints)
	{
		return;
	}

	UPrimitiveComponent* StartBody = Cast<UPrimitiveComponent>(CachedStartComponent.Get());
	AActor* Owner = GetOwner();
	if (!StartBody || !Owner)
	{
		return;
	}

	TArray<UPhysicsConstraintComponent*> Constraints;
	Owner->GetComponents<UPhysicsConstraintComponent>(Constraints);
	for (UPhysicsConstraintComponent* Constraint : Constraints)
	{
		if (Constraint)
		{
			Constraint->BreakConstraint();
		}
	}

	// The paddle is a free rigid body. Clear any locks left by the old setup.
	StartBody->BodyInstance.bLockXTranslation = false;
	StartBody->BodyInstance.bLockYTranslation = false;
	StartBody->BodyInstance.bLockZTranslation = false;
	StartBody->BodyInstance.bLockXRotation = false;
	StartBody->BodyInstance.bLockYRotation = false;
	StartBody->BodyInstance.bLockZRotation = false;
	StartBody->BodyInstance.bLockRotation = false;
	StartBody->RecreatePhysicsState();
	StartBody->SetEnableGravity(bStartBodyUsesGravity && !bDisableStartBodyGravity);
	StartBody->SetSimulatePhysics(true);
	StartBody->SetMassScale(NAME_None, PaddleMassScale);
	StartBody->RecreatePhysicsState();
	StartBody->BodyInstance.bUseCCD = true;
	StartBody->WakeAllRigidBodies();

	bConfiguredFreeStartBody = true;
}

void UFlailTetherComponent::EnsureVisualComponent()
{
	AActor* Owner = GetOwner();
	if (!Owner || TetherVisual)
	{
		return;
	}

	TetherVisual = NewObject<UStaticMeshComponent>(Owner, TEXT("ReliableTetherVisual"));
	TetherVisual->SetMobility(EComponentMobility::Movable);
	TetherVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TetherVisual->SetGenerateOverlapEvents(false);
	TetherVisual->SetCanEverAffectNavigation(false);
	TetherVisual->SetHiddenInGame(false);
	TetherVisual->bSelectable = false;

	if (USceneComponent* Root = Owner->GetRootComponent())
	{
		TetherVisual->AttachToComponent(Root, FAttachmentTransformRules::KeepWorldTransform);
	}

	if (TetherMesh)
	{
		TetherVisual->SetStaticMesh(TetherMesh);
	}

	TetherVisual->RegisterComponent();
	Owner->AddInstanceComponent(TetherVisual);
}

void UFlailTetherComponent::LoadDefaultAssets()
{
	if (!TetherMesh)
	{
		TetherMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	}

	if (!OrangeMaterial)
	{
		OrangeMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/MI_ChainOrange.MI_ChainOrange"));
	}

	if (!BlueMaterial)
	{
		BlueMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/MI_ChainBlue.MI_ChainBlue"));
	}
}

void UFlailTetherComponent::ResolveEndpoints()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	if (!CachedStartComponent.IsValid())
	{
		CachedStartComponent = FindSceneComponentByName(Owner, StartComponentName);
	}

	ResolveTargetActor();

	AActor* ResolvedTargetActor = TargetActor ? TargetActor.Get() : CachedTargetActor.Get();
	if (!CachedEndComponent.IsValid() && ResolvedTargetActor)
	{
		CachedEndComponent = FindSceneComponentByName(ResolvedTargetActor, EndComponentName);
		if (!CachedEndComponent.IsValid())
		{
			CachedEndComponent = ResolvedTargetActor->GetRootComponent();
		}
	}
}

void UFlailTetherComponent::ResolveTargetActor()
{
	if (TargetActor || !bAutoResolveTargetActor || CachedTargetActor.IsValid())
	{
		return;
	}

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	if (FObjectProperty* ObjectProperty = FindFProperty<FObjectProperty>(Owner->GetClass(), TargetActorPropertyName))
	{
		UObject* ObjectValue = ObjectProperty->GetObjectPropertyValue_InContainer(Owner);
		CachedTargetActor = Cast<AActor>(ObjectValue);
	}
}

void UFlailTetherComponent::HideLegacyCable() const
{
	if (!bHideLegacyCableComponent)
	{
		return;
	}

	AActor* Owner = GetOwner();
	USceneComponent* LegacyComponent = FindSceneComponentByName(Owner, LegacyCableComponentName);
	if (UPrimitiveComponent* Primitive = Cast<UPrimitiveComponent>(LegacyComponent))
	{
		Primitive->SetVisibility(false, true);
		Primitive->SetHiddenInGame(true);
	}
}

void UFlailTetherComponent::UpdateVisual()
{
	if (!TetherVisual)
	{
		return;
	}

	USceneComponent* Start = CachedStartComponent.Get();
	USceneComponent* End = CachedEndComponent.Get();
	if (!Start || !End)
	{
		TetherVisual->SetVisibility(false, true);
		return;
	}

	const FVector StartLocation = GetAnchorLocation(Start, StartLocalOffset);
	const FVector EndLocation = GetAnchorLocation(End, EndLocalOffset);
	const FVector Delta = EndLocation - StartLocation;
	const float Length = Delta.Size();

	if (Length < MinimumVisibleLength)
	{
		TetherVisual->SetVisibility(false, true);
		return;
	}

	const FVector Midpoint = StartLocation + (Delta * 0.5f);
	const FQuat Rotation = FRotationMatrix::MakeFromZ(Delta).ToQuat();

	TetherVisual->SetStaticMesh(TetherMesh);
	TetherVisual->SetMaterial(0, Side == EFlailTetherSide::Orange ? OrangeMaterial : BlueMaterial);
	TetherVisual->SetWorldLocationAndRotation(Midpoint, Rotation);
	TetherVisual->SetWorldScale3D(FVector(Radius / 50.0f, Radius / 50.0f, Length / 100.0f));
	TetherVisual->SetVisibility(true, true);
}

void UFlailTetherComponent::DriveBluePaddleAI(float DeltaTime)
{
	if (!bBlueAIEnabled || Side != EFlailTetherSide::Blue || DeltaTime <= 0.0f)
	{
		return;
	}

	UPrimitiveComponent* PaddleBody = Cast<UPrimitiveComponent>(CachedStartComponent.Get());
	UPrimitiveComponent* HeadBody = Cast<UPrimitiveComponent>(CachedEndComponent.Get());
	if (!PaddleBody || !PaddleBody->IsSimulatingPhysics() || !HeadBody)
	{
		return;
	}

	TArray<AActor*> Balls;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), TEXT("PongBall"), Balls);
	if (Balls.Num() == 0 || !Balls[0])
	{
		return;
	}

	const FVector PaddleLocation = PaddleBody->GetComponentLocation();
	UPrimitiveComponent* BallBody = Cast<UPrimitiveComponent>(Balls[0]->GetRootComponent());
	const FVector BallLocation = BallBody ? BallBody->GetComponentLocation() : Balls[0]->GetActorLocation();
	const FVector BallVelocity = BallBody
		? BallBody->GetPhysicsLinearVelocity()
		: FVector::ZeroVector;
	const FVector PredictedLocation = BallLocation + (BallVelocity * 0.22f);
	const FVector AnchorLocation = GetAnchorLocation(PaddleBody, StartLocalOffset);
	const bool bBallThreatening = BallLocation.X < 0.0f;
	BlueAISwingCooldown = FMath::Max(0.0f, BlueAISwingCooldown - DeltaTime);
	if (bBallThreatening && BlueAISwingCooldown <= 0.0f && BlueAISwingTime <= 0.0f)
	{
		BlueAISwingTime = BlueAISwingDuration;
		BlueAISwingSign = PredictedLocation.Z >= PaddleLocation.Z ? 1.0f : -1.0f;
	}

	FVector DesiredHeadLocation = BlueAIRestLocation;
	if (BlueAISwingTime > 0.0f)
	{
		BlueAISwingTime = FMath::Max(0.0f, BlueAISwingTime - DeltaTime);
		const float Progress = 1.0f - FMath::Clamp(BlueAISwingTime / FMath::Max(BlueAISwingDuration, 0.01f), 0.0f, 1.0f);
		const float SwingRadius = FMath::Max(BlueAISwingRadius, MaxLength + BlueAIOverpullDistance);
		const float TargetZ = FMath::Clamp(PredictedLocation.Z, -780.0f, 780.0f);
		const float SweepZ = TargetZ + ((Progress * 2.0f - 1.0f) * 260.0f * BlueAISwingSign);
		const float SweepX = AnchorLocation.X + (SwingRadius * (0.55f + (0.45f * FMath::Sin(Progress * PI))));
		DesiredHeadLocation = FVector(SweepX, AnchorLocation.Y, SweepZ);
		if (BlueAISwingTime <= 0.0f)
		{
			BlueAISwingCooldown = 0.45f;
		}
	}
	DesiredHeadLocation.Y = HeadBody->GetComponentLocation().Y;
	FVector HeadDelta = DesiredHeadLocation - AnchorLocation;
	const float AIReach = MaxLength + FMath::Max(BlueAIOverpullDistance, 0.0f);
	if (HeadDelta.SizeSquared() > FMath::Square(AIReach))
	{
		HeadDelta = HeadDelta.GetSafeNormal() * AIReach;
		DesiredHeadLocation = AnchorLocation + HeadDelta;
	}

	const FVector CurrentHeadLocation = HeadBody->GetComponentLocation();
	const float HeadInterpSpeed = BlueAISwingTime > 0.0f
		? FMath::Max(BlueAIResponsiveness, 0.0f) * 8.0f
		: FMath::Max(BlueAIResponsiveness, 0.0f) * 2.0f;
	const FVector NewHeadLocation = FMath::VInterpTo(CurrentHeadLocation, DesiredHeadLocation, DeltaTime, HeadInterpSpeed);
	HeadBody->SetEnableGravity(false);
	if (HeadBody->IsSimulatingPhysics())
	{
		HeadBody->SetPhysicsLinearVelocity(FVector::ZeroVector);
		HeadBody->SetSimulatePhysics(false);
	}
	HeadBody->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HeadBody->SetWorldLocation(NewHeadLocation, false, nullptr, ETeleportType::None);
}

void UFlailTetherComponent::DriveTargetHeadFromMouse(float DeltaTime)
{
	if (Side != EFlailTetherSide::Orange || !bDriveTargetHeadFromMouse || DeltaTime <= 0.0f)
	{
		return;
	}

	UPrimitiveComponent* HeadBody = Cast<UPrimitiveComponent>(CachedEndComponent.Get());
	if (!HeadBody)
	{
		return;
	}

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (!PlayerController)
	{
		return;
	}

	FVector RayOrigin = FVector::ZeroVector;
	FVector RayDirection = FVector::ForwardVector;
	if (!PlayerController->DeprojectMousePositionToWorld(RayOrigin, RayDirection) || FMath::IsNearlyZero(RayDirection.Y))
	{
		return;
	}

	const FVector CurrentLocation = HeadBody->GetComponentLocation();
	if (!MouseFollowPlaneY.IsSet())
	{
		MouseFollowPlaneY = CurrentLocation.Y;
	}

	if (!MouseFollowHeadRotation.IsSet())
	{
		MouseFollowHeadRotation = HeadBody->GetComponentQuat();
	}

	const float PlaneY = MouseFollowPlaneY.GetValue();
	const float RayDistance = (PlaneY - RayOrigin.Y) / RayDirection.Y;
	if (RayDistance < 0.0f)
	{
		return;
	}

	FVector TargetLocation = RayOrigin + (RayDirection * RayDistance);
	TargetLocation.Y = PlaneY;
	const float HeadBoundaryX = CenterLineX + (Side == EFlailTetherSide::Orange ? HeadCenterLineClearance : -HeadCenterLineClearance);
	TargetLocation.X = Side == EFlailTetherSide::Orange
		? FMath::Max(TargetLocation.X, HeadBoundaryX)
		: FMath::Min(TargetLocation.X, HeadBoundaryX);

	const float InterpSpeed = FMath::Max(MouseFollowInterpSpeed, 0.0f);
	const FVector NewLocation = InterpSpeed <= 0.0f
		? TargetLocation
		: FMath::VInterpTo(CurrentLocation, TargetLocation, DeltaTime, InterpSpeed);

	AActor* ResolvedTargetActor = TargetActor ? TargetActor.Get() : CachedTargetActor.Get();
	if (ResolvedTargetActor)
	{
		TArray<UPhysicsHandleComponent*> PhysicsHandles;
		ResolvedTargetActor->GetComponents<UPhysicsHandleComponent>(PhysicsHandles);
		for (UPhysicsHandleComponent* PhysicsHandle : PhysicsHandles)
		{
			if (PhysicsHandle)
			{
				PhysicsHandle->ReleaseComponent();
			}
		}
	}

	if (HeadBody->IsSimulatingPhysics())
	{
		HeadBody->SetPhysicsLinearVelocity(FVector::ZeroVector);
		HeadBody->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
		HeadBody->SetSimulatePhysics(false);
	}

	HeadBody->SetEnableGravity(false);
	if (bFreezeMouseDrivenHeadRotation)
	{
		HeadBody->SetWorldLocationAndRotation(NewLocation, MouseFollowHeadRotation.GetValue(), false, nullptr, ETeleportType::TeleportPhysics);
	}
	else
	{
		HeadBody->SetWorldLocation(NewLocation, false, nullptr, ETeleportType::TeleportPhysics);
	}
}

void UFlailTetherComponent::ConstrainHeadToOwnHalf()
{
	UPrimitiveComponent* HeadBody = Cast<UPrimitiveComponent>(CachedEndComponent.Get());
	if (!HeadBody)
	{
		return;
	}

	const float BoundaryX = CenterLineX + (Side == EFlailTetherSide::Orange ? HeadCenterLineClearance : -HeadCenterLineClearance);
	FVector Location = HeadBody->GetComponentLocation();
	const bool bOrange = Side == EFlailTetherSide::Orange;
	const bool bCrossedBoundary = bOrange ? Location.X < BoundaryX : Location.X > BoundaryX;
	if (!bCrossedBoundary)
	{
		return;
	}

	Location.X = BoundaryX;
	HeadBody->SetWorldLocation(Location, false, nullptr, ETeleportType::TeleportPhysics);

	if (HeadBody->IsSimulatingPhysics())
	{
		FVector Velocity = HeadBody->GetPhysicsLinearVelocity();
		const bool bMovingAcross = bOrange ? Velocity.X < 0.0f : Velocity.X > 0.0f;
		if (bMovingAcross)
		{
			Velocity.X = 0.0f;
			HeadBody->SetPhysicsLinearVelocity(Velocity);
		}
	}
}

void UFlailTetherComponent::ConstrainHeadToArenaBounds()
{
	UPrimitiveComponent* HeadBody = Cast<UPrimitiveComponent>(CachedEndComponent.Get());
	if (!HeadBody)
	{
		return;
	}

	FVector Location = HeadBody->GetComponentLocation();
	const FVector OriginalLocation = Location;
	Location.X = FMath::Clamp(Location.X, HeadMinX, HeadMaxX);
	Location.Z = FMath::Clamp(Location.Z, HeadMinZ, HeadMaxZ);

	if (Location.Equals(OriginalLocation, 0.1f))
	{
		return;
	}

	HeadBody->SetWorldLocation(Location, false, nullptr, ETeleportType::TeleportPhysics);
	if (HeadBody->IsSimulatingPhysics())
	{
		FVector Velocity = HeadBody->GetPhysicsLinearVelocity();
		if ((Location.X <= HeadMinX && Velocity.X < 0.0f) || (Location.X >= HeadMaxX && Velocity.X > 0.0f))
		{
			Velocity.X = 0.0f;
		}
		if ((Location.Z <= HeadMinZ && Velocity.Z < 0.0f) || (Location.Z >= HeadMaxZ && Velocity.Z > 0.0f))
		{
			Velocity.Z = 0.0f;
		}
		HeadBody->SetPhysicsLinearVelocity(Velocity);
	}
}

void UFlailTetherComponent::ApplyPaddleBallImpact(float DeltaTime)
{
	BallImpactCooldown = FMath::Max(0.0f, BallImpactCooldown - DeltaTime);
	if (BallImpactCooldown > 0.0f || !CachedStartComponent.IsValid())
	{
		return;
	}

	UPrimitiveComponent* PaddleBody = Cast<UPrimitiveComponent>(CachedStartComponent.Get());
	if (!PaddleBody || !PaddleBody->IsSimulatingPhysics())
	{
		return;
	}

	TArray<AActor*> BallActors;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), TEXT("PongBall"), BallActors);
	for (AActor* BallActor : BallActors)
	{
		if (!BallActor)
		{
			continue;
		}

		TArray<UPrimitiveComponent*> BallBodies;
		BallActor->GetComponents<UPrimitiveComponent>(BallBodies);
		for (UPrimitiveComponent* BallBody : BallBodies)
		{
			if (!BallBody || !BallBody->IsSimulatingPhysics())
			{
				continue;
			}

			const FVector PaddleLocation = PaddleBody->GetComponentLocation();
			const FVector BallLocation = BallBody->GetComponentLocation();
			FVector AwayFromPaddle = BallLocation - PaddleLocation;
			AwayFromPaddle.Y = 0.0f;
			const float Distance = AwayFromPaddle.Size();
			if (Distance > BallImpactRadius || Distance <= KINDA_SMALL_NUMBER)
			{
				continue;
			}

			AwayFromPaddle /= Distance;
			const FVector PaddleVelocity = PaddleBody->GetPhysicsLinearVelocityAtPoint(BallLocation);
			const FVector BallVelocity = BallBody->GetPhysicsLinearVelocity();
			const bool bBallApproaching = FVector::DotProduct(BallVelocity, AwayFromPaddle) < 0.0f;
			const bool bPaddleStriking = FVector::DotProduct(PaddleVelocity, AwayFromPaddle) > 50.0f;
			if (!bBallApproaching && !bPaddleStriking)
			{
				continue;
			}

			FVector NewVelocity = FMath::GetReflectionVector(BallVelocity, AwayFromPaddle);
			NewVelocity += PaddleVelocity * BallVelocityTransfer;
			NewVelocity.Y = 0.0f;
			const float Speed = FMath::Clamp(NewVelocity.Size(), 650.0f, 2600.0f);
			BallBody->SetPhysicsLinearVelocity(NewVelocity.GetSafeNormal() * Speed);
			BallBody->SetWorldLocation(PaddleLocation + (AwayFromPaddle * BallImpactRadius), false, nullptr, ETeleportType::TeleportPhysics);
			BallImpactCooldown = 0.08f;
			return;
		}
	}
}

void UFlailTetherComponent::ApplySlackTetherPhysics(float DeltaTime)
{
	// Mouse-driven heads are kinematic, so they still need the dynamic paddle
	// side of the tether even when the legacy physics toggle is off.
	if ((!bUseSlackTetherPhysicsAtRuntime && !bDriveTargetHeadFromMouse && !bBlueAIEnabled) || DeltaTime <= 0.0f)
	{
		return;
	}

	const EFlailTetherPhysicsMode EffectivePhysicsMode = PhysicsMode == EFlailTetherPhysicsMode::VisualOnly
		? EFlailTetherPhysicsMode::SoftPull
		: PhysicsMode;

	USceneComponent* Start = CachedStartComponent.Get();
	USceneComponent* End = CachedEndComponent.Get();
	UPrimitiveComponent* StartBody = Cast<UPrimitiveComponent>(Start);
	UPrimitiveComponent* EndBody = Cast<UPrimitiveComponent>(End);
	if (!Start || !StartBody || !StartBody->IsSimulatingPhysics() || !End)
	{
		return;
	}

	const FVector StartLocation = GetAnchorLocation(Start, StartLocalOffset);
	const FVector EndLocation = GetAnchorLocation(EndBody, EndLocalOffset);
	const FVector Delta = EndLocation - StartLocation;
	const float Distance = Delta.Size();
	const float Stretch = Distance - MaxLength;

	if (Stretch <= 0.0f || Distance <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	const FVector Direction = Delta / Distance;
	const FVector EndVelocity = EndBody && EndBody->IsSimulatingPhysics()
		? EndBody->GetPhysicsLinearVelocity()
		: FVector::ZeroVector;

	if (EffectivePhysicsMode == EFlailTetherPhysicsMode::HardLimit && EndBody && EndBody->IsSimulatingPhysics())
	{
		const FVector CorrectedEndLocation = StartLocation + (Direction * MaxLength);
		EndBody->SetWorldLocation(CorrectedEndLocation, false, nullptr, ETeleportType::TeleportPhysics);

		const float OutwardSpeed = FVector::DotProduct(EndVelocity, Direction);
		FVector CorrectedVelocity = EndVelocity;
		if (OutwardSpeed > 0.0f)
		{
			CorrectedVelocity -= Direction * OutwardSpeed;
		}

		EndBody->SetPhysicsLinearVelocity(CorrectedVelocity);
		return;
	}

	if (EffectivePhysicsMode != EFlailTetherPhysicsMode::SoftPull)
	{
		return;
	}

	const FVector StartVelocity = GetAnchorVelocity(Start, StartLocation, DeltaTime);
	const float RelativeSpeed = FVector::DotProduct(EndVelocity - StartVelocity, Direction);
	// Keep the line mostly slack and only add tension when it is stretched or
	// when the paddle is moving farther away. Inward motion stays unopposed,
	// which gives the paddle room to orbit instead of bouncing on a spring.
	const float EffectivePullStrength = FMath::Min(PullStrength, 800.0f);
	const float EffectiveDampingStrength = FMath::Min(DampingStrength, 300.0f);
	const float EffectiveMaxPullForce = FMath::Min(MaxPullForce, 20000.0f);
	const float OutwardSpeed = FMath::Max(RelativeSpeed, 0.0f);
	const float PullMagnitude = FMath::Clamp((Stretch * EffectivePullStrength) + (OutwardSpeed * EffectiveDampingStrength), 0.0f, EffectiveMaxPullForce);
	const FVector PullForce = Direction * PullMagnitude;

	if (bPullStartBody || !EndBody || !EndBody->IsSimulatingPhysics())
	{
		// Apply tension at the paddle's tether anchor so the free pole can
		// rotate around that point instead of only translating its center.
		// AddForceAtLocation uses Newtons, so scale by mass to retain the
		// acceleration strength used by the previous center-force path.
		StartBody->AddForceAtLocation(PullForce * StartBody->GetMass(), StartLocation, NAME_None);
	}
	if (EndBody && EndBody->IsSimulatingPhysics())
	{
		EndBody->AddForce(-PullForce, NAME_None, true);
	}
}

void UFlailTetherComponent::StabilizeControlledBodies()
{
	if (bRestoreStartBodyPhysics)
	{
		if (UPrimitiveComponent* StartBody = Cast<UPrimitiveComponent>(CachedStartComponent.Get()))
		{
			StartBody->SetSimulatePhysics(true);
			StartBody->SetEnableGravity(bStartBodyUsesGravity);
			StartBody->SetLinearDamping(StartLinearDamping);
			StartBody->SetAngularDamping(StartAngularDamping);
			if (bKeepStartBodyUpright && StabilizedStartBody.Get() != StartBody)
			{
				StartBody->BodyInstance.bLockXRotation = true;
				StartBody->BodyInstance.bLockYRotation = true;
				StartBody->BodyInstance.bLockZRotation = true;
				StartBody->BodyInstance.bLockRotation = true;
				StartBody->RecreatePhysicsState();
				StabilizedStartBody = StartBody;
			}
			StartBody->WakeAllRigidBodies();
			ConstrainBodyToXZPlane(StartBody, LockedStartY);
		}
	}

	if (!bTuneTargetHeadPhysics)
	{
		return;
	}

	AActor* ResolvedTargetActor = TargetActor ? TargetActor.Get() : CachedTargetActor.Get();
	if (!ResolvedTargetActor)
	{
		return;
	}

	if (UPrimitiveComponent* HeadBody = Cast<UPrimitiveComponent>(FindSceneComponentByName(ResolvedTargetActor, EndComponentName)))
	{
		if (!LockedHeadRotation.IsSet() || StabilizedTargetActor.Get() != ResolvedTargetActor)
		{
			LockedHeadRotation = HeadBody->GetComponentQuat();
		}

		HeadBody->SetEnableGravity(false);
		HeadBody->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
		HeadBody->SetLinearDamping(HeadLinearDamping);
		HeadBody->SetAngularDamping(HeadAngularDamping);
		HeadBody->BodyInstance.bUseCCD = true;

		if (HeadControlMode == EFlailHeadControlMode::KinematicBlueprintDriven)
		{
			HeadBody->SetSimulatePhysics(false);
			HeadBody->SetMobility(EComponentMobility::Movable);
			HeadBody->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		}
		else
		{
			HeadBody->SetSimulatePhysics(true);
			HeadBody->SetWorldRotation(LockedHeadRotation.GetValue(), false, nullptr, ETeleportType::TeleportPhysics);
			if (StabilizedTargetActor.Get() != ResolvedTargetActor)
			{
				HeadBody->SetMassScale(NAME_None, HeadMassScale);
				HeadBody->BodyInstance.bLockXRotation = true;
				HeadBody->BodyInstance.bLockYRotation = true;
				HeadBody->BodyInstance.bLockZRotation = true;
				HeadBody->BodyInstance.bLockRotation = true;
				HeadBody->RecreatePhysicsState();
			}

			TArray<UPhysicsHandleComponent*> PhysicsHandles;
			ResolvedTargetActor->GetComponents<UPhysicsHandleComponent>(PhysicsHandles);
			for (UPhysicsHandleComponent* PhysicsHandle : PhysicsHandles)
			{
				if (!PhysicsHandle)
				{
					continue;
				}

				PhysicsHandle->SetLinearStiffness(HeadHandleLinearStiffness);
				PhysicsHandle->SetLinearDamping(HeadHandleLinearDamping);
				PhysicsHandle->SetAngularStiffness(HeadHandleAngularStiffness);
				PhysicsHandle->SetAngularDamping(HeadHandleAngularDamping);
				PhysicsHandle->SetInterpolationSpeed(HeadHandleInterpolationSpeed);
			}

			StabilizedTargetActor = ResolvedTargetActor;
		}

		ConstrainBodyToXZPlane(HeadBody, LockedEndY);
	}
}

void UFlailTetherComponent::ConstrainBodyToXZPlane(UPrimitiveComponent* Body, TOptional<float>& LockedY)
{
	if (!bConstrainToXZPlane || !Body)
	{
		return;
	}

	FVector Location = Body->GetComponentLocation();
	if (!LockedY.IsSet())
	{
		LockedY = Location.Y;
	}

	if (!FMath::IsNearlyEqual(Location.Y, LockedY.GetValue(), 0.1f))
	{
		Location.Y = LockedY.GetValue();
		Body->SetWorldLocation(Location, false, nullptr, ETeleportType::TeleportPhysics);
	}

	if (Body->IsSimulatingPhysics())
	{
		FVector Velocity = Body->GetPhysicsLinearVelocity();
		Velocity.Y = 0.0f;
		Body->SetPhysicsLinearVelocity(Velocity);
	}
}

void UFlailTetherComponent::WarnIfEndpointMissing()
{
	if (!CachedStartComponent.IsValid() && !bWarnedMissingStart)
	{
		UE_LOG(LogTemp, Warning, TEXT("FlailTetherComponent on %s could not find start component '%s'."),
			*GetNameSafe(GetOwner()),
			*StartComponentName.ToString());
		bWarnedMissingStart = true;
	}

	if (!CachedEndComponent.IsValid() && !bWarnedMissingEnd)
	{
		UE_LOG(LogTemp, Warning, TEXT("FlailTetherComponent on %s could not find target/end component. TargetActor=%s TargetActorPropertyName='%s' EndComponentName='%s'."),
			*GetNameSafe(GetOwner()),
			*GetNameSafe(TargetActor.Get()),
			*TargetActorPropertyName.ToString(),
			*EndComponentName.ToString());
		bWarnedMissingEnd = true;
	}
}

USceneComponent* UFlailTetherComponent::FindSceneComponentByName(AActor* Actor, FName ComponentName) const
{
	if (!Actor || ComponentName.IsNone())
	{
		return nullptr;
	}

	TArray<USceneComponent*> Components;
	Actor->GetComponents<USceneComponent>(Components);
	for (USceneComponent* Component : Components)
	{
		if (Component && Component->GetFName() == ComponentName)
		{
			return Component;
		}
	}

	return nullptr;
}

FVector UFlailTetherComponent::GetAnchorLocation(const USceneComponent* Component, const FVector& LocalOffset) const
{
	if (!Component)
	{
		return FVector::ZeroVector;
	}

	return Component->GetComponentTransform().TransformPosition(LocalOffset);
}

FVector UFlailTetherComponent::GetAnchorVelocity(const USceneComponent* Component, const FVector& CurrentAnchorLocation, float DeltaTime)
{
	FVector AnchorVelocity = FVector::ZeroVector;

	if (const UPrimitiveComponent* Primitive = Cast<UPrimitiveComponent>(Component))
	{
		if (Primitive->IsSimulatingPhysics())
		{
			AnchorVelocity = Primitive->GetPhysicsLinearVelocity();
		}
	}

	if (AnchorVelocity.IsNearlyZero() && bHasPreviousStartLocation && DeltaTime > KINDA_SMALL_NUMBER)
	{
		AnchorVelocity = (CurrentAnchorLocation - PreviousStartLocation) / DeltaTime;
		AnchorVelocity = AnchorVelocity.GetClampedToMaxSize(MaxAnchorSpeed);
	}

	PreviousStartLocation = CurrentAnchorLocation;
	bHasPreviousStartLocation = true;

	return AnchorVelocity;
}
