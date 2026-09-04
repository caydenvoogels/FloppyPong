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

	// The orange tether uses the mouse to drive its target head. Keep the
	// cursor available in PIE/game builds so deprojection has a live position.
	bShowMouseCursor = true;
	FInputModeGameAndUI MouseInputMode;
	MouseInputMode.SetHideCursorDuringCapture(false);
	SetInputMode(MouseInputMode);

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
