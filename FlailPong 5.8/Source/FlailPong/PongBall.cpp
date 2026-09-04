#include "PongBall.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/TextRenderActor.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Components/TextRenderComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"

APongBall::APongBall()
{
	PrimaryActorTick.bCanEverTick = true;

	BallBody = CreateDefaultSubobject<USphereComponent>(TEXT("BallBody"));
	SetRootComponent(BallBody);
	BallBody->InitSphereRadius(42.0f);
	BallBody->SetCollisionProfileName(TEXT("PhysicsActor"));
	BallBody->SetSimulatePhysics(true);
	BallBody->SetEnableGravity(false);
	BallBody->SetMassOverrideInKg(NAME_None, BallMassKg, true);
	BallBody->SetNotifyRigidBodyCollision(true);
	BallBody->SetLinearDamping(0.05f);
	BallBody->SetAngularDamping(0.2f);
	BallBody->BodyInstance.bUseCCD = true;
	UPhysicalMaterial* BallPhysicsMaterial = NewObject<UPhysicalMaterial>(GetTransientPackage(), NAME_None, RF_Transient);
	BallPhysicsMaterial->Restitution = 0.95f;
	BallPhysicsMaterial->Friction = 0.02f;
	BallBody->SetPhysMaterialOverride(BallPhysicsMaterial);

	UStaticMeshComponent* Visual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BallVisual"));
	Visual->SetupAttachment(BallBody);
	Visual->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere")));
	Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Visual->SetRelativeScale3D(FVector(0.84f));
	Visual->SetMaterial(0, LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_PongArena_WhiteStripe.M_PongArena_WhiteStripe")));

	BallTrail = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BallTrail"));
	BallTrail->SetupAttachment(BallBody);
	BallTrail->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder")));
	BallTrail->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BallTrail->SetMaterial(0, LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_PongArena_Blue.M_PongArena_Blue")));

	BallGlow = CreateDefaultSubobject<UPointLightComponent>(TEXT("BallGlow"));
	BallGlow->SetupAttachment(BallBody);
	BallGlow->SetMobility(EComponentMobility::Movable);
	BallGlow->SetIntensity(2200.0f);
	BallGlow->SetAttenuationRadius(500.0f);
	BallGlow->SetLightColor(FLinearColor(0.1f, 0.35f, 1.0f));

	GoalBurstLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("GoalBurstLight"));
	GoalBurstLight->SetupAttachment(BallBody);
	GoalBurstLight->SetMobility(EComponentMobility::Movable);
	GoalBurstLight->SetIntensity(0.0f);
	GoalBurstLight->SetAttenuationRadius(850.0f);
	GoalBurstLight->SetVisibility(false);

	GoalBurstSparks = CreateDefaultSubobject<UNiagaraComponent>(TEXT("GoalBurstSparks"));
	GoalBurstSparks->SetupAttachment(BallBody);
	GoalBurstSparks->SetAsset(LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/OutsideAssets/VFX/Particles/NS_Sparks.NS_Sparks")));
	GoalBurstSparks->bAutoActivate = false;
}

void APongBall::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (GoalBurstTimeRemaining > 0.0f)
	{
		GoalBurstTimeRemaining -= DeltaSeconds;
		if (GoalBurstLight)
		{
			GoalBurstLight->SetIntensity(12000.0f * FMath::Clamp(GoalBurstTimeRemaining / 1.25f, 0.0f, 1.0f));
			if (GoalBurstTimeRemaining <= 0.0f)
			{
				GoalBurstLight->SetVisibility(false);
				if (GoalBurstSparks)
				{
					GoalBurstSparks->Deactivate();
				}
			}
		}
	}
	if (!BallBody)
	{
		return;
	}

	FVector Location = BallBody->GetComponentLocation();
	Location.Y = PlayPlaneY;
	CheckGoalEntry();
	if (bGoalResetting)
	{
		return;
	}
	const float MinX = -1450.0f;
	const float MaxX = 1450.0f;
	const float MinZ = -850.0f;
	const float MaxZ = 850.0f;
	FVector Velocity = BallBody->GetPhysicsLinearVelocity();
	const bool bInsideGoalOpening = FMath::Abs(Location.Z) <= GoalHalfHeight;
	if (!bInsideGoalOpening && (Location.X < MinX || Location.X > MaxX))
	{
		Location.X = FMath::Clamp(Location.X, MinX, MaxX);
		if ((Location.X <= MinX && Velocity.X < 0.0f) || (Location.X >= MaxX && Velocity.X > 0.0f))
		{
			Velocity.X *= -1.0f;
		}
	}
	if (Location.Z < MinZ || Location.Z > MaxZ)
	{
		Location.Z = FMath::Clamp(Location.Z, MinZ, MaxZ);
		if ((Location.Z <= MinZ && Velocity.Z < 0.0f) || (Location.Z >= MaxZ && Velocity.Z > 0.0f))
		{
			Velocity.Z *= -1.0f;
		}
	}
	if (!BallBody->GetComponentLocation().Equals(Location, 0.1f))
	{
		BallBody->SetWorldLocation(Location, false, nullptr, ETeleportType::None);
	}

	Velocity.Y = 0.0f;
	const FVector TravelDirection = Velocity.GetSafeNormal();
	const float TrailLength = FMath::Clamp(Velocity.Size() * 0.16f, 50.0f, 320.0f);
	const bool bOrangeSide = Location.X < 0.0f;
	const FLinearColor SideColor = bOrangeSide ? FLinearColor(1.0f, 0.12f, 0.01f) : FLinearColor(0.02f, 0.25f, 1.0f);
	if (IsValid(BallGlow.Get()))
	{
		BallGlow->SetLightColor(SideColor);
	}
	if (IsValid(BallTrail.Get()) && !TravelDirection.IsNearlyZero())
	{
		BallTrail->SetWorldLocation(Location - (TravelDirection * (TrailLength * 0.5f)));
		BallTrail->SetWorldScale3D(FVector(0.14f, 0.14f, TrailLength / 100.0f));
	}
	const float Speed = Velocity.Size();
	if (Speed > KINDA_SMALL_NUMBER)
	{
		BallBody->SetPhysicsLinearVelocity(Velocity.GetSafeNormal() * FMath::Clamp(Speed, MinimumSpeed, MaximumSpeed));
	}
}

void APongBall::CheckGoalEntry()
{
	if (bGoalResetting || !BallBody)
	{
		return;
	}

	const FVector Location = BallBody->GetComponentLocation();
	if (FMath::Abs(Location.Z) > GoalHalfHeight)
	{
		return;
	}

	int32* ScoringTeam = nullptr;
	if (Location.X <= -GoalBoundaryX)
	{
		ScoringTeam = &BlueScore;
	}
	else if (Location.X >= GoalBoundaryX)
	{
		ScoringTeam = &OrangeScore;
	}
	else
	{
		return;
	}

	++(*ScoringTeam);
	UE_LOG(LogTemp, Display, TEXT("Pong goal scored: Orange %d - Blue %d"), OrangeScore, BlueScore);
	UpdateScoreDisplays();
	TriggerGoalBurst(Location.X < 0.0f ? -1550.0f : 1550.0f, Location.X < 0.0f ? FLinearColor(1.0f, 0.12f, 0.01f) : FLinearColor(0.02f, 0.25f, 1.0f));

	const float ServeDirection = Location.X < 0.0f ? 1.0f : -1.0f;
	const float ServeZ = (OrangeScore + BlueScore) % 2 == 0 ? 0.35f : -0.35f;
	BallBody->SetWorldLocation(FVector(0.0f, PlayPlaneY, 0.0f), false, nullptr, ETeleportType::TeleportPhysics);
	BallBody->SetPhysicsLinearVelocity(FVector(ServeDirection * ServeSpeed, 0.0f, ServeZ * ServeSpeed));
}

void APongBall::TriggerGoalBurst(float GoalX, const FLinearColor& Color)
{
	const FVector BurstLocation(GoalX, PlayPlaneY, BallBody ? BallBody->GetComponentLocation().Z : 0.0f);
	if (GoalBurstLight)
	{
		GoalBurstLight->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		GoalBurstLight->SetWorldLocation(BurstLocation);
		GoalBurstLight->SetLightColor(Color);
		GoalBurstLight->SetIntensity(12000.0f);
		GoalBurstLight->SetVisibility(true);
	}
	if (GoalBurstSparks)
	{
		GoalBurstSparks->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		GoalBurstSparks->SetWorldLocation(BurstLocation);
		GoalBurstSparks->SetVariableLinearColor(TEXT("User.Color"), Color * 15.0f);
		GoalBurstSparks->SetVariableFloat(TEXT("User.Brightness"), 15.0f);
		GoalBurstSparks->Activate(true);
	}
	GoalBurstTimeRemaining = 1.25f;
}

void APongBall::UpdateScoreDisplays() const
{
	TArray<AActor*> Displays;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), TEXT("PongScoreDisplay"), Displays);
	for (AActor* Display : Displays)
	{
		if (ATextRenderActor* TextActor = Cast<ATextRenderActor>(Display))
		{
			if (UTextRenderComponent* Text = TextActor->GetTextRender())
			{
				const bool bOrange = Display->ActorHasTag(TEXT("ScoreOrange"));
				Text->SetText(FText::AsNumber(bOrange ? OrangeScore : BlueScore));
			}
		}
	}
}
