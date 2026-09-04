#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"

#if PLATFORM_WINDOWS
#include "Windows/WindowsApplication.h"
#endif

#include "KineticPlayerControllerBase.generated.h"

UCLASS()
class FLAILPONG_API AKineticPlayerControllerBase
	: public APlayerController
#if PLATFORM_WINDOWS
	, public IWindowsMessageHandler
#endif
{
	GENERATED_BODY()

public:
	AKineticPlayerControllerBase();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

#if PLATFORM_WINDOWS
	virtual bool ProcessMessage(HWND HWnd, uint32 Message, WPARAM WParam, LPARAM LParam, int32& OutResult) override;
#endif

	bool IsSecondMouseActive() const { return bSecondMouseActive; }
	bool HasRawMouseInput() const { return RawInputWindowHandle != nullptr; }
	void EnsureRawMouseInput();
	FVector2D GetPrimaryMousePosition() const { return PrimaryMousePosition; }
	FVector2D GetSecondMousePosition() const { return SecondMousePosition; }

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Camera")
	TSubclassOf<AActor> GameCameraClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Camera", meta=(ClampMin="0.0", UIMin="0.0"))
	float CameraBlendTime = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Camera")
	TEnumAsByte<EViewTargetBlendFunction> CameraBlendFunction = VTBlend_Linear;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Camera")
	float CameraBlendExp = 0.0f;

private:
	bool bSecondMouseActive = false;
	FVector2D PrimaryMousePosition = FVector2D(700.0f, 0.0f);
	FVector2D SecondMousePosition = FVector2D(-700.0f, 0.0f);

#if PLATFORM_WINDOWS
	void SetupRawMouseInput();
	void* PrimaryMouseDevice = nullptr;
	void* SecondMouseDevice = nullptr;
	double LastMouseDebugTime = 0.0;
	void* RawInputWindowHandle = nullptr;
#endif
};
