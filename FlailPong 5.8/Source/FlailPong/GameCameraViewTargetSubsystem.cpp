#include "GameCameraViewTargetSubsystem.h"

#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

bool UGameCameraViewTargetSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	const UWorld* World = Cast<UWorld>(Outer);
	return World && (World->IsGameWorld() || World->WorldType == EWorldType::PIE);
}

void UGameCameraViewTargetSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	SetGameCameraViewTarget(&InWorld);
	InWorld.GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateUObject(this, &UGameCameraViewTargetSubsystem::SetGameCameraViewTarget, &InWorld));
}

void UGameCameraViewTargetSubsystem::SetGameCameraViewTarget(UWorld* World)
{
	if (!World)
	{
		return;
	}

	UClass* GameCameraClass = StaticLoadClass(AActor::StaticClass(), nullptr, TEXT("/Game/Controller/BP_GameCamera.BP_GameCamera_C"));
	if (!GameCameraClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("GameCameraViewTargetSubsystem could not load BP_GameCamera."));
		return;
	}

	AActor* GameCamera = UGameplayStatics::GetActorOfClass(World, GameCameraClass);
	if (!GameCamera)
	{
		UE_LOG(LogTemp, Warning, TEXT("GameCameraViewTargetSubsystem found no BP_GameCamera actor in the level."));
		return;
	}

	if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(World, 0))
	{
		PlayerController->SetViewTargetWithBlend(GameCamera, 1.0f, VTBlend_Linear);
	}
}
