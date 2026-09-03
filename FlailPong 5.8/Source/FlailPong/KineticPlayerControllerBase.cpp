#include "KineticPlayerControllerBase.h"

#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

AKineticPlayerControllerBase::AKineticPlayerControllerBase()
{
	static ConstructorHelpers::FClassFinder<AActor> GameCameraBlueprint(TEXT("/Game/Controller/BP_GameCamera"));
	if (GameCameraBlueprint.Succeeded())
	{
		GameCameraClass = GameCameraBlueprint.Class;
	}
}

void AKineticPlayerControllerBase::BeginPlay()
{
	Super::BeginPlay();

	if (!GameCameraClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("KineticPlayerControllerBase could not find BP_GameCamera class."));
		return;
	}

	AActor* GameCamera = UGameplayStatics::GetActorOfClass(this, GameCameraClass);
	if (!GameCamera)
	{
		UE_LOG(LogTemp, Warning, TEXT("KineticPlayerControllerBase found no BP_GameCamera actor in the level."));
		return;
	}

	SetViewTargetWithBlend(GameCamera, CameraBlendTime, CameraBlendFunction, CameraBlendExp);
}
