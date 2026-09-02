#include "KineticPhysicsPaddle.h"

#include "CollisionProfile.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"

namespace KineticPaddleDefaults
{
	constexpr double DefaultPaddleLengthCm = 300.0;
	constexpr double DefaultPaddleMassKg = 8.0;
	constexpr double DefaultLinearDamping = 0.25;
	constexpr double DefaultAngularDamping = 0.45;
	constexpr double DefaultPaddleWidthCm = 25.0;
	constexpr double DefaultPaddleDepthCm = 15.0;
	constexpr double DefaultAnchorRadiusCm = 12.0;
	constexpr double DefaultAnchorVisualScale = 0.15;
}

AKineticPhysicsPaddle::AKineticPhysicsPaddle()
	: PaddleLengthCm(KineticPaddleDefaults::DefaultPaddleLengthCm)
	, PaddleMassKg(KineticPaddleDefaults::DefaultPaddleMassKg)
	, LinearDamping(KineticPaddleDefaults::DefaultLinearDamping)
	, AngularDamping(KineticPaddleDefaults::DefaultAngularDamping)
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	AnchorBody = CreateDefaultSubobject<USphereComponent>(TEXT("AnchorBody"));
	AnchorBody->SetupAttachment(Root);
	AnchorBody->SetMobility(EComponentMobility::Movable);
	AnchorBody->SetSphereRadius(KineticPaddleDefaults::DefaultAnchorRadiusCm);
	AnchorBody->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	AnchorBody->SetCollisionResponseToAllChannels(ECR_Ignore);
	AnchorBody->SetSimulatePhysics(false);

	AnchorVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AnchorVisual"));
	AnchorVisual->SetupAttachment(AnchorBody);
	AnchorVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PaddleBody = CreateDefaultSubobject<UBoxComponent>(TEXT("PaddleBody"));
	PaddleBody->SetupAttachment(Root);
	PaddleBody->SetMobility(EComponentMobility::Movable);

	PaddleVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PaddleVisual"));
	PaddleVisual->SetupAttachment(PaddleBody);
	PaddleVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PhysicsConstraint = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("PhysicsConstraint"));
	PhysicsConstraint->SetupAttachment(Root);
}

void AKineticPhysicsPaddle::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	const double HalfLengthCm = PaddleLengthCm * 0.5;

	AnchorBody->SetRelativeLocation(FVector::ZeroVector);
	AnchorBody->SetSphereRadius(KineticPaddleDefaults::DefaultAnchorRadiusCm);

	AnchorVisual->SetRelativeLocation(FVector::ZeroVector);
	AnchorVisual->SetRelativeScale3D(FVector(KineticPaddleDefaults::DefaultAnchorVisualScale));

	PaddleBody->SetRelativeLocation(FVector(0.0, 0.0, -HalfLengthCm));
	PaddleBody->SetRelativeRotation(FRotator::ZeroRotator);
	PaddleBody->SetBoxExtent(FVector(
		KineticPaddleDefaults::DefaultPaddleWidthCm * 0.5,
		KineticPaddleDefaults::DefaultPaddleDepthCm * 0.5,
		HalfLengthCm));

	PaddleVisual->SetRelativeLocation(FVector::ZeroVector);
	PaddleVisual->SetRelativeRotation(FRotator::ZeroRotator);
	PaddleVisual->SetRelativeScale3D(FVector(
		KineticPaddleDefaults::DefaultPaddleWidthCm / 100.0,
		KineticPaddleDefaults::DefaultPaddleDepthCm / 100.0,
		PaddleLengthCm / 100.0));

	PhysicsConstraint->SetRelativeLocation(FVector::ZeroVector);

	ConfigurePaddleBody();
	ConfigureConstraint();
}

void AKineticPhysicsPaddle::BeginPlay()
{
	Super::BeginPlay();

	ConfigurePaddleBody();
	ConfigureConstraint();

	PhysicsConstraint->SetConstrainedComponents(AnchorBody, NAME_None, PaddleBody, NAME_None);
	PhysicsConstraint->SetConstraintReferencePosition(EConstraintFrame::Frame1, FVector::ZeroVector);
	PhysicsConstraint->SetConstraintReferencePosition(EConstraintFrame::Frame2, FVector(0.0, 0.0, PaddleLengthCm * 0.5));
	PaddleBody->WakeRigidBody();
}

void AKineticPhysicsPaddle::SetAnchorWorldPosition(const FVector& NewPosition)
{
	SetActorLocation(NewPosition, false, nullptr, ETeleportType::TeleportPhysics);
	PaddleBody->WakeRigidBody();
}

FVector AKineticPhysicsPaddle::GetAnchorWorldPosition() const
{
	return GetActorLocation();
}

void AKineticPhysicsPaddle::ConfigurePaddleBody() const
{
	PaddleBody->SetCollisionProfileName(UCollisionProfile::PhysicsActor_ProfileName);
	PaddleBody->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	PaddleBody->SetSimulatePhysics(true);
	PaddleBody->SetEnableGravity(true);
	PaddleBody->SetUseCCD(true);
	PaddleBody->SetMassOverrideInKg(NAME_None, PaddleMassKg, true);
	PaddleBody->SetLinearDamping(LinearDamping);
	PaddleBody->SetAngularDamping(AngularDamping);
}

void AKineticPhysicsPaddle::ConfigureConstraint() const
{
	PhysicsConstraint->SetDisableCollision(true);
	PhysicsConstraint->SetLinearXLimit(LCM_Locked, 0.0f);
	PhysicsConstraint->SetLinearYLimit(LCM_Locked, 0.0f);
	PhysicsConstraint->SetLinearZLimit(LCM_Locked, 0.0f);

	// This project currently uses Swing2 as the single free axis for X-Z plane rotation.
	PhysicsConstraint->SetAngularSwing1Limit(ACM_Locked, 0.0f);
	PhysicsConstraint->SetAngularSwing2Limit(ACM_Free, 0.0f);
	PhysicsConstraint->SetAngularTwistLimit(ACM_Locked, 0.0f);

	PhysicsConstraint->SetProjectionEnabled(true);
	PhysicsConstraint->SetProjectionParams(1.0f, 0.0f, 5.0f, 180.0f);
}
